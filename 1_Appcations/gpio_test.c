#include "dev_gpio.h"
#include "dev_uart.h"
#include "dev_iic.h"
#include "dev_spi.h"
#include "dev_w25qx.h"
#include <string.h>
#include "printf.h"
#include "errno.h"
#include "drv_flash.h"
#include "ring_buffer.h"
#include "dev_wifi.h"
#include "dev_st7789.h"
#include "xpt2046.h"

#include "MQTTFreeRTOS.h"
#include "MQTTClient.h"

#include "mqtt_base.h"
#include "mqtt_ota_ali.h"

/* ==================== LVGL ==================== */
#include "lvgl.h"
#include "gui_guider.h"
#include "lv_port_disp_template.h"
#include "lv_port_indev_template.h"
#include "lvgl_action.h"   /* guider_ui 全局对象 + 控件事件绑定（事件回调在 action 层） */
#include "tickscreen.h"    /* 每个页面一个 tick 循环函数（ui_tick 分发） */

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#include "iap.h"

#include "ota_info.h"      /* FirmwareInfo / EEPROM 布局 / OTA_APP_ADDR 等存放位置 */
#include "dev_at24cxx.h"   /* GetAT24C02Device：读固件信息记录 */
#include "md5.h"           /* MD5_Init / MD5_Update / MD5_Final / MD5_HashToHex */

WiFiBtDevice * WIFIBTDev = NULL;

/* MQTT 任务管理（避免硬删持锁任务导致 mqtt_mutex 永久锁死）：
 * mqtt_thread      存放任务句柄，ThreadStart 每次创建时覆盖写入；
 * mqtt_alive       mqtt 任务存活标志：任务启动置1、自杀前清0，供 wifi 任务判断旧任务是否退出；
 * mqtt_quit        WiFi 掉线时 wifi 任务置1，mqtt 任务在 MQTTBase_Yield 返回（已解锁）后检查并自杀。 */
static Thread mqtt_thread;
static volatile int mqtt_alive = 0;
static volatile int mqtt_quit = 0;

/* ==================== LVGL 界面 ==================== */

/* guider_ui 全局对象定义在 6_Middlewares/lvgl/action/lvgl_action.c，经 lvgl_action.h extern 使用 */

static TaskHandle_t gDispTaskHandle = NULL;
static TaskHandle_t gWifiTaskHandle = NULL;

/* LVGL 节拍：FreeRTOS 软件定时器每 1ms 回调一次（configTICK_RATE_HZ = 1000） */
static void gui_timer_cb(TimerHandle_t handle)
{
    (void)handle;
    lv_tick_inc(1);
}

/* lv_timer_handler() 返回值的上下限：用它做动态休眠，减少空转（参考 ESP-Epoch lvgl_port_task） */
#define LVGL_TASK_MIN_DELAY_MS   5
#define LVGL_TASK_MAX_DELAY_MS   100

/* 显示设备任务：初始化 LVGL + ST7789 驱动，周期调用 ui_tick + lv_timer_handler() 驱动渲染 */
static void DispTask(void *argument)
{
    (void)argument;

    lv_init();               /* LVGL 核心初始化 */
    lv_port_disp_init();     /* 注册显示驱动（ST7789 rotation 0 竖屏 240×320，单缓冲区） */
    lv_port_indev_init();    /* 注册触摸输入设备（XPT2046 软件 SPI） */

    tickscreen_init();   /* 创建页间信号量（WiFi 连接信号量等） */

    /* 上电默认显示 SettingsPage4Info 信息页 */
    init_scr_del_flag(&guider_ui);                       /* 初始化各屏删除标志（setup_ui 里的那步，别漏） */
    if(NULL == guider_ui.SettingsPage4Info)
        setup_scr_SettingsPage4Info(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4Info);

    /* 信息页两个状态 LED 初始化为红色（WiFi 未连接 / 云平台未连接） */
    lv_led_set_color(guider_ui.SettingsPage4Info_wifi_status_led, lv_palette_main(LV_PALETTE_RED));
    lv_led_set_color(guider_ui.SettingsPage4Info_aliyun_status_led, lv_palette_main(LV_PALETTE_RED));

    /* 各屏事件在对应 setup_scr_SettingsPage4Xxx() 末尾绑定（见 generated/ 下文件），此处无需集中绑定 */

    /* 软件定时器驱动 LVGL 节拍（1ms） */
    TimerHandle_t guiTimer = xTimerCreate("LVGL Tick", pdMS_TO_TICKS(1), pdTRUE, NULL, gui_timer_cb);
    if(NULL != guiTimer)
    {
        xTimerStart(guiTimer, 0);
    }

    uint32_t task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
    while(1)
    {
        ui_tick();               /* 当前页面周期逻辑（每页一个 tick 函数） */

        task_delay_ms = lv_timer_handler();  /* LVGL 渲染与事件处理，返回下次需处理的时间 */

        if(task_delay_ms > LVGL_TASK_MAX_DELAY_MS)
            task_delay_ms = LVGL_TASK_MAX_DELAY_MS;
        else if(task_delay_ms < LVGL_TASK_MIN_DELAY_MS)
            task_delay_ms = LVGL_TASK_MIN_DELAY_MS;

        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static void mqtt_task(void *arg)
{
    (void)arg;
    /* connect to m2m.eclipse.org, subscribe to a topic, send and receive messages regularly every 1 sec */
    int count = 0;
    int rc = 0;

    mqtt_alive = 1;   /* 登记存活：wifi 任务据此知道本任务在跑，重连前要等它退出 */

    //1.MQTT 连接服务器初始化
    MQTTBase_Init();
    //2.MQTT 连接阿里云平台
    rc = MQTTBaseConnect();
    if(rc == 0)//入云成功
    {
        debugprintf("aliyun connect success\r\n");
        /* 发送"入云成功"信号量 → Info 页 tick 函数里 aliyun_status_led 变绿 */
        xSemaphoreGive(aliyun_connected_sem);
    }
    
    //初始化阿里云OTA相关topic
    MQTTOTA_TopicInit();
    MQTTOTA_InformVersion(1, "0.0.0", DeviceName);//上报当前版本号
    
    //MQTTOTA_Upgrade();//查询服务器是否有ota升级信息
    
    MQTTOTA_Subdownload_reply();// 设备端订阅服务器下发bin文件分片
    
    MQTTOTA_SubFirmwareReply();// 设备端订阅服务器返回升级信息对应的应答topic

    
	while (++count)
	{
        rc = MQTTBase_Yield(1000);
        if(rc != 0)//断开云平台连接了
        {
            /* yield 出错继续循环；若 WiFi 掉线，下面的 mqtt_quit 检查会触发自杀 */
        }

        //WiFi 掉线请求退出：此刻 MQTTBase_Yield 已返回并解锁，安全自杀
        //（不能在外面硬删本任务——若删时正持有 mqtt_mutex，互斥锁会永久锁死）
        if(mqtt_quit)
        {
            debugprintf("mqtt task self-delete\r\n");
            mqtt_quit = 0;              /* 复位退出标志，避免下次误判 */
            mqtt_alive = 0;             /* 清除存活标志，wifi 任务据此知道旧任务已退出 */
            vTaskDelete(NULL);          /* 自杀 */
        }

        vTaskDelay(1);
	}
}

/* WiFi 自动连接任务：负责 WiFi 连接/掉线自动重连（任务内容待填写） */
static void wifi_auto_connect_task(void *arg)
{
    (void)arg;
    /* TODO: WiFi 自动连接逻辑，先留空 */
    WIFIBTDev = GetWIFIBTDevice();
    if(WIFIBTDev != NULL) printf("找到wifi设备\r\n");
    else  vTaskDelete(NULL);
    
    WIFIBTDev->Init(WIFIBTDev);
    while(1)
    {
        debugprintf("wifi_auto_connect_task running\r\n");

        if(0 == WIFIBTDev->dev_status)//wifi未连接状态
        {
            if(ESUCCESS == WIFIBTDev->WIFIConnect(WIFIBTDev, "iPhone", "12345678"))//连接WiFi成功
            {
                debugprintf("wifi connect success\r\n");
                /* 发送"WiFi 已连接"信号量 → Info 页 tick 函数里 wifi_status_led 变绿 */
                xSemaphoreGive(wifi_connected_sem);

                //MQTT连接：先等旧 MQTT 任务退出（掉线时请求自杀，Yield 一轮 1s 内必退出），
                //避免新旧两个任务同时操作同一个 client/sendbuf/readbuf
                if(mqtt_alive)
                {
                    debugprintf("wait old mqtt task exit\r\n");
                    uint8_t wait_cnt = 15;   /* 最多等 ~1.5s */
                    while(wait_cnt-- && mqtt_alive)
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                if(!mqtt_alive)
                {
                    mqtt_quit = 0;   /* 新任务从干净状态开始 */
                    int rc = ThreadStart(&mqtt_thread, mqtt_task, NULL);
                    if(rc != pdPASS)    debugprintf("mqtt task creat error\r\n");
                }
                else
                {
                    debugprintf("old mqtt task still alive, skip restart\r\n");
                }
            }
        }
        else if(1 == WIFIBTDev->dev_status)//wifi已连接状态
        {
            WIFIBTDev->WIFIStaStatus(WIFIBTDev, 200);//轮询更新wifi连接状态
            
            if(0 == WIFIBTDev->dev_status)//突然断开了
            {
                mqtt_quit = 1;   /* 请求 MQTT 任务在 Yield 返回（已解锁）后自杀；不能硬删持锁任务 */
                /* 发送"WiFi 断开"信号量 → Info 页 tick 里 wifi/aliyun 两个状态 LED 变红 */
                xSemaphoreGive(wifi_disconnected_sem);
            }
            
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* ==================== 上电自检：内部 APP 校验 + 版本回退 ====================
 * 由 app_test() 在调度器启动前调用，所以是纯同步代码：只能 HAL_Delay / 忙等，不能 vTaskDelay。
 * 流程：读 EEPROM 0x00 记录 → 把内部 APP 区读出来算 md5 比对
 *         一致   → 直接跳 APP 区
 *         不一致 → 用 EEPROM 0x40 记录定位 W25Q64 里的旧固件备份，拷回内部 flash 再跳（版本回退）
 * 只要任一步拿不到可信镜像就返回，让 app_test() 继续把 bootloader 界面建起来，用户可以重新 OTA。 */

#define OTA_BOOT_CHUNK      256   /* 分块 = W25Q64 页大小，天然对齐页和扇区边界 */
#define BOOT_AUTO_JUMP      1     /* 1 = 校验通过即自动跳 APP；置 0 只打印校验结果、留在 bootloader 界面 */

/* 读一段 flash 算 md5，输出 32 位小写 hex（hex_out 至少 MD5_HASH_LEN*2+1 = 33 字节）。
 *   pq == NULL → 读内部 flash（APP 区）
 *   pq != NULL → 读外部 W25Q64（备份区）
 * 返回 ESUCCESS / -EINVAL / -EIO。 */
static int ota_md5_region(W25QDevice *pq, unsigned int addr, unsigned int size, char *hex_out)
{
    /* 4 字节对齐：FlashDrvRead 内部按 uint64_t/uint32_t 整体搬运，裸 uint8_t 缓冲区可能触发
     * M4 的对齐异常；分块长度 256、起始地址 8 字节对齐，下标自然保持对齐 */
    static uint32_t buf[OTA_BOOT_CHUNK / 4];
    MD5_CTX ctx;
    uint8_t hash[MD5_HASH_LEN];
    unsigned int off;

    if(NULL == hex_out || 0 == size)   return -EINVAL;

    MD5_Init(&ctx);
    for(off = 0; off < size; off += OTA_BOOT_CHUNK)
    {
        unsigned int chunk = (size - off >= OTA_BOOT_CHUNK) ? OTA_BOOT_CHUNK : (size - off);

        if(NULL == pq)
        {
            if((int)chunk != FlashDrvRead(addr + off, (unsigned char *)buf, chunk))
                return -EIO;
        }
        else
        {
            if((int)chunk != pq->Read(pq, addr + off, (unsigned char *)buf, chunk))
                return -EIO;
        }
        MD5_Update(&ctx, (uint8_t *)buf, chunk);
    }
    MD5_Final(&ctx, hash);
    MD5_HashToHex(hash, hex_out);

    return ESUCCESS;
}

/* 版本回退：把 W25Q64 起点的旧固件备份拷回内部 flash 的 APP 区。
 * binfo 是 EEPROM 0x40 那条记录（fileSize = 备份长度，md5 = 备份的本地摘要）。
 * 返回 ESUCCESS 表示内部 APP 区现在已经是完整的旧固件。 */
static int ota_rollback_from_w25q(const FirmwareInfo *binfo)
{
    static uint32_t buf[OTA_BOOT_CHUNK / 4];
    W25QDevice *pq = GetW25Q64Device();
    char expect[MD5_HASH_LEN * 2 + 1];
    char actual[MD5_HASH_LEN * 2 + 1];
    unsigned int size, off;

    if(NULL == pq || NULL == binfo)              return -EINVAL;
    size = binfo->fileSize;
    if(0 == size || size > OTA_APP_SIZE)         return -EINVAL;
    if(size > W25Q64_SIZE - W25Q64_BACKUP_ADDR)  return -EINVAL;

    /* 定长 md5 不带 '\0'，自己补一个再比较/打印 */
    memcpy(expect, binfo->md5, sizeof(binfo->md5));
    expect[MD5_HASH_LEN * 2] = '\0';

    /* W25Q64 平时没人用，这里补一次初始化（内部读 JEDEC ID 校验型号，不是 0xEF4017 会失败） */
    if(ESUCCESS != pq->Init(pq))
    {
        debugprintf("rollback: W25Q64 init FAIL\r\n");
        return -EIO;
    }

    /* ① 先只读校验备份本身：备份坏了就在这里返回，不碰内部 flash
     *    —— 内部那份虽然已判定损坏，但也没必要再擦一遍，擦了就真没退路了。 */
    if(ESUCCESS != ota_md5_region(pq, W25Q64_BACKUP_ADDR, size, actual))
    {
        debugprintf("rollback: read W25Q64 backup FAIL\r\n");
        return -EIO;
    }
    if(0 != strncmp(actual, expect, MD5_HASH_LEN * 2))
    {
        debugprintf("rollback: backup md5 mismatch (calc=%s rec=%s)\r\n", actual, expect);
        return -EIO;
    }

    /* ② 擦掉 APP 区再搬（擦除会连带擦掉 0x08064000 所在扇区的前 16KB 预留区，
     *    bootloader 止于 0x0805E000，不受影响） */
    if(ESUCCESS != FlashDrvErase(OTA_APP_ADDR, OTA_APP_ADDR + size))
    {
        debugprintf("rollback: erase APP area FAIL\r\n");
        return -EIO;
    }

    for(off = 0; off < size; off += OTA_BOOT_CHUNK)
    {
        unsigned int chunk = (size - off >= OTA_BOOT_CHUNK) ? OTA_BOOT_CHUNK : (size - off);

        if((int)chunk != pq->Read(pq, W25Q64_BACKUP_ADDR + off, (unsigned char *)buf, chunk))
        {
            debugprintf("rollback: read W25Q64 @0x%x FAIL\r\n", W25Q64_BACKUP_ADDR + off);
            return -EIO;
        }
        if((int)chunk != FlashDrvWrite(OTA_APP_ADDR + off, (unsigned char *)buf, chunk))
        {
            debugprintf("rollback: write internal flash @0x%x FAIL\r\n", OTA_APP_ADDR + off);
            return -EIO;
        }
    }

    /* ③ 回读内部 flash 再算一遍 md5，确认真的写进去了
     *    （和 lvgl_action.c 里 fw_backup_to_w25q 的 write-then-readback 一个套路） */
    if(ESUCCESS != ota_md5_region(NULL, OTA_APP_ADDR, size, actual))
    {
        debugprintf("rollback: read back internal flash FAIL\r\n");
        return -EIO;
    }
    if(0 != strncmp(actual, expect, MD5_HASH_LEN * 2))
    {
        debugprintf("rollback: readback md5 mismatch (calc=%s rec=%s)\r\n", actual, expect);
        return -EIO;
    }

    debugprintf("rollback OK: %d bytes restored to 0x%x, md5=%s\r\n", (int)size, OTA_APP_ADDR, actual);
    return ESUCCESS;
}

/* 读 EEPROM 里的一条固件信息记录并存进 info，顺带打印。
 * 返回 1 = 记录可用，0 = 不可用（读失败 / 空白 0xFF / 全 0 / fileSize 越界）。 */
static int ota_read_fw_record(AT24CXXDevice *ptdev, unsigned short addr, const char *tag, FirmwareInfo *info)
{
    char verbuf[sizeof(info->version) + 1];
    char md5buf[sizeof(info->md5) + 1];

    memset(info, 0, sizeof(*info));   /* 连 reserved 一起清，别把栈上的脏字节带出去 */

    if(NULL == ptdev)   return 0;
    if((int)sizeof(*info) != ptdev->Read(ptdev, addr, (unsigned char *)info, sizeof(*info)))
    {
        debugprintf("%s: read EEPROM @0x%x FAIL\r\n", tag, addr);
        return 0;
    }

    if(0 == info->fileSize || info->fileSize > OTA_APP_SIZE)
    {
        debugprintf("%s: no valid record @0x%x (fileSize=%d)\r\n", tag, addr, (int)info->fileSize);
        return 0;
    }

    /* 定长字段可能没有 '\0'，打印前自己补一个再 %s */
    memcpy(verbuf, info->version, sizeof(info->version));
    verbuf[sizeof(info->version)] = '\0';
    memcpy(md5buf, info->md5, sizeof(info->md5));
    md5buf[sizeof(info->md5)] = '\0';
    debugprintf("%s @0x%x: ver=%s size=%d md5=%s\r\n", tag, addr, verbuf, (int)info->fileSize, md5buf);

    return 1;
}

/* 开机自检：校验内部 APP 区，一致直接跳转，不一致就从 W25Q64 备份回退后再跳转。
 * 只有"拿不到可信镜像"时才返回（继续走 bootloader 界面，让用户重新 OTA）。 */
static void ota_boot_check_and_jump(void)
{
    AT24CXXDevice *ptAT24C02 = GetAT24C02Device();
    FirmwareInfo   rec;
    FirmwareInfo   binfo;
    char           calc[MD5_HASH_LEN * 2 + 1];
    char           expect[MD5_HASH_LEN * 2 + 1];

    /* FlashDrvWrite/Erase 靠 FLASH 中断回调等写完成，回退要用，先打开 */
    FlashDrvInit();

    /* 1、读 EEPROM 0x00：出厂预置、之后每次 OTA 成功刷新，描述"此刻应该在内部 flash 里的固件" */
    if(!ota_read_fw_record(ptAT24C02, EEPROM_ADDR_NEW_FW, "boot: fw record", &rec))
    {
        debugprintf("boot: no valid fw record, stay in bootloader\r\n");
        return;
    }

    /* 2、把内部 flash 的 APP 区读出来算 md5，和记录里存的比 */
    memcpy(expect, rec.md5, sizeof(rec.md5));
    expect[MD5_HASH_LEN * 2] = '\0';

    if(ESUCCESS != ota_md5_region(NULL, OTA_APP_ADDR, rec.fileSize, calc))
    {
        debugprintf("boot: read APP area FAIL, stay in bootloader\r\n");
        return;
    }
    debugprintf("boot: app md5 calc=%s rec=%s\r\n", calc, expect);

    if(0 == strncmp(calc, expect, MD5_HASH_LEN * 2))
    {
        debugprintf("boot: app md5 OK\r\n");
#if BOOT_AUTO_JUMP
        jump_to_application(OTA_APP_ADDR);   /* 跳 APP 区；正常不返回 */
        /* 只有 jump_to_application 判定目标不是合法 RAM 栈顶而提前返回时才会走到这里：
         * md5 已经对上了，说明 APP 本身没问题，绝不能再往下走回退把好镜像擦掉。 */
        debugprintf("boot: jump aborted (APP stack top invalid), stay in bootloader\r\n");
        return;
#else
        debugprintf("boot: BOOT_AUTO_JUMP=0, stay in bootloader\r\n");
        return;
#endif
    }

    /* 3、不一致 → 用 EEPROM 0x40（备份在 W25Q64 里的那份旧固件）做版本回退 */
    debugprintf("boot: app md5 mismatch, try rollback from W25Q64\r\n");
    if(!ota_read_fw_record(ptAT24C02, EEPROM_ADDR_CUR_FW, "boot: backup record", &binfo))
    {
        debugprintf("boot: no valid backup record, stay in bootloader\r\n");
        return;
    }

    if(ESUCCESS != ota_rollback_from_w25q(&binfo))
    {
        debugprintf("boot: rollback FAIL, stay in bootloader\r\n");
        return;
    }

    /* 4、回退成功：内部 flash 现在跑的是旧固件，把 0x40 那条记录写到 0x00，
     *    让记录与实际一致；否则下次上电又会判定不一致、再回退一遍 */
    if((int)sizeof(binfo) != ptAT24C02->Write(ptAT24C02, EEPROM_ADDR_NEW_FW, (unsigned char *)&binfo, sizeof(binfo)))
        debugprintf("boot: update fw record after rollback FAIL (next boot rolls back again)\r\n");
    else
        debugprintf("boot: fw record updated to rolled-back version\r\n");

    debugprintf("boot: rollback done, jump to old fw\r\n");
    jump_to_application(OTA_APP_ADDR);   /* 跳回退后的 APP 区，不返回 */
}

void app_test()
{
    GPIO_DevRegis();//全局GPIO设备注册
    UART_DevRegis();//全局串口设备注册
    IIC_DevRegis();//全局IIC设备注册
    SPI_DevRegis();//全局SPI设备注册

    /* 上电自检：内部 APP 区 md5 与 EEPROM 0x00 一致 → 直接跳 APP；
     * 不一致 → 从 W25Q64 备份回退（旧版本）后再跳；拿不到可信镜像就留在 bootloader 界面。
     * 位置要求：必须在上面四个 *_DevRegis() 之后（自检要用 I2C2 的 EEPROM 和 SPI1 的 W25Q64），
     * 且在下面 xTaskCreate 之前（一旦跳走就不该再建任务）。 */
    ota_boot_check_and_jump();


    /* 创建 LVGL 显示任务（软件定时器驱动节拍，任务内做 lv_init + lv_port_disp_init） */
    if(pdPASS != xTaskCreate(DispTask, "DisplayTask", 1024, NULL, 4, &gDispTaskHandle))
    {
        debugprintf("DisplayTask create failed\r\n");
    }

    /* 创建 WiFi 自动连接任务（骨架，任务内容待填写） */
    if(pdPASS != xTaskCreate(wifi_auto_connect_task, "WifiAutoConnect", 2048, NULL, 2, &gWifiTaskHandle))
    {
        debugprintf("WifiAutoConnect task create failed\r\n");
    }

}
