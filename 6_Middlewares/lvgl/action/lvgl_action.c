/*
 * lvgl_action.c
 * LVGL 控件事件回调模块：定义全局 UI 对象 guider_ui，集中存放各控件的事件回调函数。
 *
 * 职责：
 *   1. 定义 lv_ui guider_ui 全局变量（原定义在 1_Appcations/gpio_test.c）。
 *   2. 集中编写各屏幕控件的回调函数（GUI Guider 重新生成代码不会覆盖本文件）。
 *
 * 事件绑定策略（重要）：
 *   回调函数体写在本文件（action 层，GUI Guider 重新生成不覆盖），
 *   但"绑定"这一行写在对应屏幕的 setup_scr_SettingsPage4Xxx() 函数里
 *   （例如 setup_scr_SettingsPage4Info.c 末尾调 lv_obj_add_event_cb）。
 *   这样每个屏幕 setup 时只绑定自己页面的控件，其他屏幕事件不会因懒创建而漏绑。
 *
 * 使用方式（见 gpio_test.c 的 DispTask）：
 *   init_scr_del_flag(&guider_ui);
 *   setup_scr_SettingsPage4Info(&guider_ui);   // 各屏懒创建，默认页才 setup
 *   lv_scr_load(guider_ui.SettingsPage4Info);
 *   回调函数在 setup_scr_xxx() 末尾绑定到对应控件。
 */

#include "lvgl_action.h"
#include "lvgl.h"
#include "printf.h"   /* debugprintf */

#include "mqtt_base.h"
#include "mqtt_ota_ali.h"

#include <string.h>   /* strncmp */
#include "errno.h"    /* ESUCCESS */
#include "md5.h"      /* MD5_Init / MD5_Update / MD5_Final / MD5_HashToHex */
#include "drv_flash.h"/* FlashDrvInit / FlashDrvErase / FlashDrvWrite / FlashDrvRead */
#include "ota_info.h" /* FirmwareInfo / EEPROM 布局 */
#include "dev_at24cxx.h"/* GetAT24C02Device：固件信息记录落盘 */
#include "dev_w25qx.h"/* GetW25Q64Device：升级前把旧 APP 备份到外部 flash */

#include "FreeRTOS.h"
#include "task.h"      /* xTaskCreate / vTaskDelay / vTaskDelete */
#include "queue.h"     /* xQueueSend：检查更新结果发到 UI 线程队列 */
#include "tickscreen.h"/* g_update_result_q：检查更新结果队列 */
#include "iap.h"
#include "timers.h"

/* 全局 UI 对象：各屏控件句柄集中存放，外部模块通过 extern 访问 */
lv_ui guider_ui;

/* 检查更新任务：主动查询是否有新固件可升级，循环判断结果。
 * 本任务不直接调 LVGL（LVGL 非线程安全）：检测到 isGetUpgrade 后把结果
 * (1可升级/2不可升级)发进 g_update_result_q 队列，由 UI 线程
 * （SettingsPage4CheckUpdate1 页 tick）接收并按值 lv_scr_load 跳转页面。 */
static void check_update_task(void *arg)
{
    (void)arg;

    //清掉上次查询的结果，避免拿旧值误判，重新等新回包
    GetUpgradeInfo()->isGetUpgrade = 0;

    //主动查询是否有新固件可升级
    MQTTOTA_GetFirmware(1, DeviceName);

    while(1)
    {
        //循环判断服务器应答（GetUpgradeMsgHandler 解析回包后填 isGetUpgrade）
        UpgradeInfo *temp = GetUpgradeInfo();
        
        if(temp->isGetUpgrade == 2)//不可升级
        {
            temp->isGetUpgrade = 0;
            /* 不再直接调 LVGL：把结果(2=不可升级)发进队列交给 UI 线程，
             * CheckUpdate1 页 tick 接收后跳转 SettingsPage4CheckUpdate2。
             * 目的：LVGL 非线程安全，跨线程通信只走队列，lv_scr_load/setup_scr 留在 UI 线程。 */
            uint8_t result = 2;
            if(NULL != g_update_result_q)
                xQueueSend(g_update_result_q, &result, 0);
            vTaskDelete(NULL);
        }
        else if(temp->isGetUpgrade == 1)//可升级
        {
            temp->isGetUpgrade = 0;
            /* 结果(1=可升级)发进队列，UI 线程跳转 SettingsPage4CheckUpdate3 */
            uint8_t result = 1;
            if(NULL != g_update_result_q)
                xQueueSend(g_update_result_q, &result, 0);
            vTaskDelete(NULL);
        }
        
        printf("check_update_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(200));//回包还没到，等一会儿再查

        /* TODO 不能死等，添加超时机制，超时返回主页面 */
    }
}

/* settings_p4_info_check_update：信息页"检查更新"按钮点击 → 检查是否有新版本（占位打印） */
void settings_p4_info_check_update(lv_event_t *e)
{
    (void)e;
    debugprintf("123456789\r\n");

    //跳转到页面——SettingsPage4CheckUpdate1（检查中页面）
    if(NULL == guider_ui.SettingsPage4CheckUpdate1)
        setup_scr_SettingsPage4CheckUpdate1(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4CheckUpdate1);

    //创建检查更新任务：任务里查询固件版本并循环判断结果，跳转对应页面
    if(pdPASS != xTaskCreate(check_update_task, "CheckUpdate", 1024, NULL, 3, NULL))
    {
        debugprintf("check_update task create failed\r\n");
    }
}

/* settings_p4_checkupdate2_confirm_back：CheckUpdate2"确认"按钮点击 → 返回信息页 */
void settings_p4_checkupdate2_confirm_back(lv_event_t *e)
{
    (void)e;
    /* TODO: 返回 SettingsPage4Info 页面 */
    if(NULL == guider_ui.SettingsPage4Info)
        setup_scr_SettingsPage4Info(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4Info);
}

/* ==================== OTA 升级任务 ==================== */
#define OTA_DOWNLOAD_SIZE   1024       /* 每次申请固件分片的最大字节数 */
/* OTA_APP_ADDR / OTA_APP_SIZE / W25Q64_BACKUP_ADDR 三个存放位置常量已上移 ota_info.h 共享
 * （bootloader 的开机自检 gpio_test.c 也要用），本文件经 #include "ota_info.h" 拿到。 */
#define OTA_BACKUP_CHUNK    256        /* 备份分块 = W25Q64 页大小，天然对齐页和扇区边界 */

static int ota_upgrade_busy = 0;       /* 升级任务防重入标志 */

#define OTA_TIMEOUT_SEC     60      /* 升级超时时间（秒）：擦除+下载全程合计，超时强制判失败 */

/* 超时定时器句柄：文件作用域（不能是任务里的局部变量，否则任务一自杀句柄就丢）；
 * 删完必须置 NULL，否则下一次升级会拿一块已被释放的句柄去 xTimerStart */
static TimerHandle_t otaTimer = NULL;

static volatile int ota_time = 0;        /* 已等待秒数：定时器服务任务写、升级任务读，故 volatile */
static volatile int isota_timeout = 0;   /* 超时标志：同上，跨任务读写 */

/* 超时看门狗回调：FreeRTOS 软件定时器每 1s 回调一次（configTICK_RATE_HZ = 1000）。
 * 本回调跑在定时器服务任务里，不要在这里碰 flash / LVGL / 升级任务的局部状态，
 * 只置标志，由 ota_upgrade_task 在主循环顶部统一收尾。 */
static void ota_timer_cb(TimerHandle_t handle)
{
    (void)handle;

    ota_time += 1;
    if(ota_time >= OTA_TIMEOUT_SEC)  /* 1 分钟还没结束 */
    {
        isota_timeout = 1;
    }
}

/* 升级结束统一清理：所有出口都必须调它。
 * 不清理的话定时器会一直以 1Hz 跑下去，ota_time 继续累加，
 * 导致下一次升级刚创建就吃到上一次遗留的超时标志，第一轮直接判失败。 */
static void ota_timer_deinit(void)
{
    if(NULL != otaTimer)
    {
        xTimerStop(otaTimer, 0);
        xTimerDelete(otaTimer, pdMS_TO_TICKS(100));  /* 句柄内存由定时器服务任务异步释放 */
        otaTimer = NULL;
    }
    ota_time = 0;
    isota_timeout = 0;
}

/* 升级结束统一出口：清定时器 → 上报结果给 UI 线程 → 任务自杀。
 * 成功/失败/超时/固件太大四条路都走这里，避免漏掉某条出口导致定时器残留。
 * 注意 vTaskDelete(NULL) 之后不会返回。 */
static void ota_upgrade_finish(uint8_t result)
{
    ota_timer_deinit();

    /* 不直接调 LVGL（非线程安全）：结果发进队列，UI 线程 tick 收到后 lv_scr_load */
    if(NULL != g_ota_result_q)
        xQueueSend(g_ota_result_q, &result, 0);

    ota_upgrade_busy = 0;
    vTaskDelete(NULL);
}

/* EEPROM 里那两条固件信息记录（AT24C02，布局见 ota_info.h）不另外封装函数，
 * 就在下面两处 TODO 里直接调驱动的 Read/Write —— 整条 64 字节读写，没有别的逻辑。 */

/* 升级前备份：把内部 flash 里 [OTA_APP_ADDR, OTA_APP_ADDR+size) 的旧 APP 搬到外部 flash，
 * 顺带算出这段镜像的 md5 从 backup_md5 带出来（33 字节：32 hex + '\0'）。
 * 返回 ESUCCESS = 写完并且回读校验通过；失败只影响"能不能回滚"，不中断升级。 */
static int fw_backup_to_w25q(unsigned int size, char *backup_md5)
{
    /* 4 字节对齐：FlashDrvRead 内部按 uint64_t/uint32_t 整体搬运，裸 uint8_t 缓冲区可能触发
     * M4 的对齐异常；分块起点 OTA_APP_ADDR + n*256 天然 4 字节对齐 */
    static uint32_t buf[OTA_BACKUP_CHUNK / 4];
    W25QDevice *pq = GetW25Q64Device();
    MD5_CTX ctx;
    uint8_t hash[MD5_HASH_LEN];
    char verify_md5[MD5_HASH_LEN * 2 + 1];
    unsigned int off, sectors, done;

    if(NULL == pq || NULL == backup_md5)          return -EINVAL;
    if(0 == size || size > OTA_APP_SIZE)          return -EINVAL;  /* 没记录 or 越界，不备份 */
    if(size > W25Q64_SIZE - W25Q64_BACKUP_ADDR)   return -EINVAL;

    /* W25Q64 平时没人用，这里补一次初始化（内部读 JEDEC ID 校验型号，不是 0xEF4017 会失败） */
    if(ESUCCESS != pq->Init(pq))
    {
        debugprintf("W25Q64 init FAIL\r\n");
        return -EIO;
    }

    /* W25Q64Write 内部不做擦除（那段被注释掉了），必须自己先把覆盖到的扇区擦干净 */
    sectors = (size + W25Q64_SECTOR_SIZE - 1) / W25Q64_SECTOR_SIZE;
    if(ESUCCESS != pq->Erase(pq, W25Q64_BACKUP_ADDR, sectors))
    {
        debugprintf("W25Q64 erase %d sectors FAIL\r\n", sectors);
        return -EIO;
    }

    /* 内部 flash → 外部 flash，边搬边算这段镜像的 md5 */
    MD5_Init(&ctx);
    for(off = 0; off < size; off += OTA_BACKUP_CHUNK)
    {
        unsigned int chunk = (size - off >= OTA_BACKUP_CHUNK) ? OTA_BACKUP_CHUNK : (size - off);

        if((int)chunk != FlashDrvRead(OTA_APP_ADDR + off, (unsigned char *)buf, chunk))
        {
            debugprintf("read internal flash @0x%x FAIL\r\n", OTA_APP_ADDR + off);
            return -EIO;
        }
        MD5_Update(&ctx, (uint8_t *)buf, chunk);

        if((int)chunk != pq->Write(pq, W25Q64_BACKUP_ADDR + off, (unsigned char *)buf, chunk))
        {
            debugprintf("W25Q64 write @0x%x FAIL\r\n", W25Q64_BACKUP_ADDR + off);
            return -EIO;
        }

        /* 每搬完一个扇区让一次 CPU：备份最多 112KB，全程霸着 CPU 会把 UI/MQTT 任务饿死 */
        done = off + chunk;
        if(0 == (done % W25Q64_SECTOR_SIZE))    vTaskDelay(pdMS_TO_TICKS(1));
    }
    MD5_Final(&ctx, hash);
    MD5_HashToHex(hash, backup_md5);

    /* 回读校验：把刚写进 W25Q64 的再读回来重算一遍 md5，和写进去时算的比。
     * 两边都是本工程 md5.c 算的、同一个算法，所以不受它填充分支的影响。 */
    MD5_Init(&ctx);
    for(off = 0; off < size; off += OTA_BACKUP_CHUNK)
    {
        unsigned int chunk = (size - off >= OTA_BACKUP_CHUNK) ? OTA_BACKUP_CHUNK : (size - off);

        if((int)chunk != pq->Read(pq, W25Q64_BACKUP_ADDR + off, (unsigned char *)buf, chunk))
        {
            debugprintf("W25Q64 read back @0x%x FAIL\r\n", W25Q64_BACKUP_ADDR + off);
            return -EIO;
        }
        MD5_Update(&ctx, (uint8_t *)buf, chunk);

        done = off + chunk;
        if(0 == (done % W25Q64_SECTOR_SIZE))    vTaskDelay(pdMS_TO_TICKS(1));
    }
    MD5_Final(&ctx, hash);
    MD5_HashToHex(hash, verify_md5);

    if(0 != strncmp(verify_md5, backup_md5, MD5_HASH_LEN * 2))
    {
        debugprintf("backup verify FAIL: readback=%s written=%s\r\n", verify_md5, backup_md5);
        return -EIO;
    }

    debugprintf("backup %d bytes to W25Q64@0x%x OK, md5=%s\r\n", size, W25Q64_BACKUP_ADDR, backup_md5);
    return ESUCCESS;
}

/* 升级任务：循环"申请固件分片→烧写flash→MD5校验"，校验通过跳 SettingsPage4UpdateCplt，失败跳 SettingsPage4UpdateError。
 * 本任务不直接调 LVGL（LVGL 非线程安全）：
 *   - 下载进度只写全局 g_ota_progress，SettingsPage4Updating 页 tick（UI 线程）读取后 lv_bar_set_value；
 *   - 升级完成/失败/固件太大只把结果(1完成/2失败)发进 g_ota_result_q 队列，由 UI 线程 tick 接收后 lv_scr_load。
 * 轮询 isDownload 时必须有 vTaskDelay 让出 CPU，否则低优先级的 mqtt_task 收不到回包填 isDownload。 */
static void ota_upgrade_task(void *arg)
{
    (void)arg;
    unsigned int step = 1;
    unsigned int otasize = 0;
    unsigned int offset = 0;
    MD5_CTX ctx;

    UpgradeInfo *Gradeinfo = GetUpgradeInfo();   /* 升级包信息（fileSize / md5 / version） */
    DownloadInfo *otadata  = GetDownloadInfo();  /* 固件分片信息（data / bSize / bOffset / fileLength） */

    /* Flash 驱动初始化：启用 FLASH 中断，FlashDrvWrite/Erase 靠中断回调等待写完成 */
    FlashDrvInit();

    g_ota_progress = 0;   /* 新一次升级开始：进度归零，UI 线程据此把进度条刷回起点 */

    /* ============ 升级前：旧 APP 备份到外部 flash + 当前固件信息写 EEPROM ============
     * 位置不能动：这里在 while(1) 之前，也就是在 step 1 擦除旧 APP 之前（擦完就备份不到了），
     * 同时也在超时定时器创建之前（搬 112KB 要 1 秒左右，不能算进那 60 秒预算）。
     * 备份/写 EEPROM 失败一律只告警、继续升级：这是辅助功能，不该把 OTA 卡死。 */
    {
        AT24CXXDevice *ptAT24C02 = GetAT24C02Device();
        FirmwareInfo   curinfo;

        memset(&curinfo, 0, sizeof(curinfo));   /* 连 reserved 一起清，别把栈上的脏字节写进 EEPROM */

        /* 先读 EEPROM 0x00 那条：出厂已按实际固件写好，之后每次升级成功刷新，描述的就是现在装着的固件 */
        if(NULL != ptAT24C02)
            ptAT24C02->Read(ptAT24C02, EEPROM_ADDR_NEW_FW, (unsigned char *)&curinfo, sizeof(curinfo));

        if(curinfo.fileSize > 0 && curinfo.fileSize <= OTA_APP_SIZE)
        {
            char verbuf[sizeof(curinfo.version) + 1];
            char md5buf[sizeof(curinfo.md5) + 1];
            char backup_md5[MD5_HASH_LEN * 2 + 1];

            /* 定长字段可能没有 '\0'，打印前自己补一个再 %s */
            memcpy(verbuf, curinfo.version, sizeof(curinfo.version));
            verbuf[sizeof(curinfo.version)] = '\0';
            memcpy(md5buf, curinfo.md5, sizeof(curinfo.md5));
            md5buf[sizeof(curinfo.md5)] = '\0';
            debugprintf("current fw: ver=%s size=%d md5=%s\r\n", verbuf, curinfo.fileSize, md5buf);

            /* 长度就取记录里的 fileSize；备份成功就用现场算出的真摘要覆盖记录里的云端 md5 */
            if(ESUCCESS == fw_backup_to_w25q(curinfo.fileSize, backup_md5))
                memcpy(curinfo.md5, backup_md5, sizeof(curinfo.md5));
            else
                debugprintf("backup old app FAIL, keep upgrade going\r\n");
        }

        /* 当前固件信息写到 EEPROM 0x40 */
        if(NULL == ptAT24C02
           || (unsigned int)sizeof(curinfo) != ptAT24C02->Write(ptAT24C02, EEPROM_ADDR_CUR_FW,
                                                                (unsigned char *)&curinfo, sizeof(curinfo)))
            debugprintf("write current fw info to EEPROM FAIL\r\n");
    }

    /* 升级超时看门狗：先清掉上一次升级的残留（otaTimer/ota_time/isota_timeout 都是文件作用域
     * 静态变量，上次任务自杀时不会自动归零），再重新创建并启动。
     * pdTRUE = 自动重载，每秒回调一次，累计到 OTA_TIMEOUT_SEC 秒置超时标志。 */
    ota_timer_deinit();
    otaTimer = xTimerCreate("OTA Timeout", pdMS_TO_TICKS(1000), pdTRUE, NULL, ota_timer_cb);
    if(NULL == otaTimer)
    {
        /* 堆不够：失去超时保护，但升级流程继续走（只是卡住时没人兜底） */
        debugprintf("xTimerCreate OTA Timeout FAIL, run without timeout guard\r\n");
    }
    else
    {
        xTimerStart(otaTimer, 0);
    }

    while(1)
    {
        /* 超时兜底：1 分钟还没走完升级流程，直接按失败收尾（UI 跳 UpdateError 页）。
         * 不复用 step 5：那是"下载完成→MD5 校验"，超时时数据只有半截、ctx 甚至还没 MD5_Init，
         * 算出来的 MD5 必然不匹配，日志会误报成 md5 Not Consistent，掩盖真实原因。 */
        if(isota_timeout == 1)
        {
            debugprintf("OTA timeout(%d s), force fail\r\n", OTA_TIMEOUT_SEC);
            ota_upgrade_finish(2);
        }

        switch(step)
        {
            case 1:  /* 点击"立即升级"后直接进入：准备下载参数、擦除目标扇区 */
            {
                //查看固件总大小，每片最多申请 OTA_DOWNLOAD_SIZE 字节
                if(Gradeinfo->fileSize < OTA_DOWNLOAD_SIZE)  otasize = Gradeinfo->fileSize;
                else otasize = OTA_DOWNLOAD_SIZE;
                offset = 0;

                //新固件大小校验：必须能放进预留的 App 区（112KB），超了直接判失败
                //（否则擦除/写入会越出 512KB flash 末端，白下载一趟）
                if(Gradeinfo->fileSize >= OTA_APP_SIZE)
                {
                    debugprintf("firmware size %d >= OTA_APP_SIZE(112KB), upgrade fail\r\n", Gradeinfo->fileSize);
                    /* 走统一出口：内部会先删掉超时定时器再上报结果，不能直接 vTaskDelete */
                    ota_upgrade_finish(2);
                }

                //初始化md5
                MD5_Init(&ctx);

                //擦除目标 flash 扇区（参考流程中 First_Run 把旧 APP 备份到外部 flash 的段，本工程暂不处理）
                int ret = FlashDrvErase(OTA_APP_ADDR, OTA_APP_ADDR + Gradeinfo->fileSize);
                if(ret != ESUCCESS)
                {
                    debugprintf("FlashDrvErase ERROR\r\n");
                }

                debugprintf("OTA decide to upgrade\r\n");
                step = 2;
                break;
            }
            case 2:  /* 请求固件分片 */
            {
                MQTTOTA_GetFirmwareBin(1, otasize, offset);//请求bin文件分片
                step = 3;//处理下发的固件包
                break;
            }
            case 3:  /* 等待并处理固件包分片 */
            {
                if(otadata->isDownload == 1)//回包到了
                {
                    //烧录固件信息
                    int ret = FlashDrvWrite(OTA_APP_ADDR + offset, otadata->data, otadata->bSize);
                    if(ret == otadata->bSize)
                    {
                        debugprintf("flash Burn success   %x  %d\r\n", OTA_APP_ADDR + offset, otadata->bSize);
                    }

                    offset = otadata->bOffset + otadata->bSize;//修改偏移量

                    int remain_size = otadata->fileLength - otadata->bOffset - otadata->bSize;//剩余未下载固件大小
                    if(remain_size >= OTA_DOWNLOAD_SIZE)  otasize = OTA_DOWNLOAD_SIZE;
                    else otasize = remain_size;

                    debugprintf("remain size: %d\r\n", remain_size);
                    debugprintf("offset size: %d\r\n", offset);
                    int progress = (otadata->fileLength - remain_size) * 100 / otadata->fileLength;//计算下载进度
                    // MQTTOTA_ImportProgress(1, (unsigned char)progress, DeviceName);  // 云端进度上报暂不启用

                    //进度条联动：不直接调 lv_bar_set_value（LVGL 非线程安全），
                    //只把进度写进全局 g_ota_progress，由 SettingsPage4Updating 页 tick（UI 线程）
                    //读取后 lv_bar_set_value 刷新进度条（bar 默认范围 0~100 与 progress 直接对应）
                    g_ota_progress = progress;

                    //更新md5值
                    MD5_Update(&ctx, otadata->data, otadata->bSize);

                    if(remain_size == 0)  step = 5;
                    else step = 2;//继续申请下发固件
                }
                else
                {
                    //回包还没到，等一会儿再查（本任务优先级3 > mqtt任务优先级2，必须让出CPU）
                    vTaskDelay(pdMS_TO_TICKS(50));
                }
                break;
            }
            case 5:  /* 下载完成，MD5 校验，决定跳转页面 */
            {
                debugprintf("OTA download done\r\n");

                //将16字节MD5哈希转为32位十六进制字符串
                uint8_t hash[MD5_HASH_LEN] = {0};
                char hex_str[33];                 //32个hex字符 + '\0'（MD5_HashToHex 会写 hex_str[32]）
                MD5_Final(&ctx, hash);            //完成计算，输出哈希值
                MD5_HashToHex(hash, hex_str);     //将16字节哈希转为32位十六进制字符串
                debugprintf("local  md5:%s\r\n", hex_str);
                char smd5buf[MD5_HASH_LEN * 2 + 1];   /* 云端 md5 不保证带 '\0'，直接 %s 会越界 */
                memcpy(smd5buf, Gradeinfo->md5, MD5_HASH_LEN * 2);
                smd5buf[MD5_HASH_LEN * 2] = '\0';
                debugprintf("server md5:%s\r\n", smd5buf);

                //strncmp 比较满 32 位，避免服务端 md5 未以 '\0' 结尾时 strcmp 越界
                if(0 == strncmp(hex_str, Gradeinfo->md5, MD5_HASH_LEN * 2))//MD5一致
                {
                    debugprintf("md5 Consistent, upgrade success\r\n");

                    /* 将新固件版本信息写入 EEPROM 0 起始地址：
                     * 下次升级时，这条记录就是"设备当前跑的固件"，用来定备份长度 */
                    {
                        AT24CXXDevice *ptAT24C02 = GetAT24C02Device();
                        FirmwareInfo   newinfo;
                        char           verbuf[sizeof(newinfo.version) + 1];

                        memset(&newinfo, 0, sizeof(newinfo));
                        newinfo.fileSize = Gradeinfo->fileSize;
                        /* 都按定长拷：云端下发的 version/md5 不保证带 '\0'，strcpy 会越界 */
                        memcpy(newinfo.version, Gradeinfo->version, sizeof(newinfo.version));
                        memcpy(newinfo.md5,     Gradeinfo->md5,     sizeof(newinfo.md5));

                        memcpy(verbuf, newinfo.version, sizeof(newinfo.version));
                        verbuf[sizeof(newinfo.version)] = '\0';   /* 补 '\0' 再 %s */

                        if(NULL != ptAT24C02
                           && (unsigned int)sizeof(newinfo) == ptAT24C02->Write(ptAT24C02, EEPROM_ADDR_NEW_FW,
                                                                               (unsigned char *)&newinfo, sizeof(newinfo)))
                            debugprintf("new fw info saved to EEPROM: ver=%s size=%d\r\n",
                                        verbuf, newinfo.fileSize);
                        else
                            debugprintf("write new fw info to EEPROM FAIL\r\n");
                    }

                    //上报新版本号
                    // MQTTOTA_InformVersion(1, (const char*)Gradeinfo->version, DeviceName);

                    //升级完成：走统一出口（清定时器 + 把结果(1=完成)发进队列 + 任务自杀）
                    ota_upgrade_finish(1);
                }
                else//MD5不一致
                {
                    debugprintf("md5 Not Consistent, upgrade fail\r\n");

                    //升级失败：走统一出口（清定时器 + 把结果(2=失败)发进队列 + 任务自杀）
                    ota_upgrade_finish(2);
                }
                break;
            }
            default:
                /* 正常流程走不到这里（step 只被赋 1/2/3/5）；真到了说明状态机被写坏，
                 * 按失败收尾并清定时器，别把 UI 永远留在"更新中"页 */
                ota_upgrade_finish(2);
                break;
        }
    }
}

/* settings_p4_checkupdate3_confirm_upgrade：CheckUpdate3"立即升级"按钮点击 → 跳"更新中"页并创建升级任务 */
void settings_p4_checkupdate3_confirm_upgrade(lv_event_t *e)
{
    (void)e;
    if(ota_upgrade_busy)
    {
        debugprintf("ota upgrade already running\r\n");
        return;
    }

    /* 跳转到页面——SettingsPage4Updating（更新中页面） */
    if(NULL == guider_ui.SettingsPage4Updating)
        setup_scr_SettingsPage4Updating(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4Updating);

    /* 创建升级任务，用于向云平台申请固件分片并烧写校验 */
    ota_upgrade_busy = 1;
    if(pdPASS != xTaskCreate(ota_upgrade_task, "OtaUpgrade", 1024, NULL, 3, NULL))
    {
        ota_upgrade_busy = 0;
        debugprintf("ota upgrade task create failed\r\n");
    }
}

/* settings_updatecplt_confirm_btn_event：UpdateCplt"确认重启"按钮点击 → 确认重启跳转 */
void settings_updatecplt_confirm_btn_event(lv_event_t *e)
{
    (void)e;
    debugprintf("updatecplt confirm restart\r\n");

    /* ==================== 跳转前终审诊断（定位用，找到问题后可删） ====================
     * 把 0x08064000 起的 fileSize 字节从 flash 读回来算 md5，跟服务器给的 md5 对比。
     * 之前 OTA 里 md5 一致只证明"收到的数据==服务器文件"，不证明"flash 里的==收到的数据"；
     * 若 FlashDrvWrite 中途失败（程序提前 break 返回部分字节数，而 offset 照样推进），
     * flash 中段会留 0xFFFFFFFF，跳进去必崩——这个对比能一次说清 flash 内容到底对不对。 */
    UpgradeInfo *g = GetUpgradeInfo();
    if(g != NULL)
    {
        uint32_t fsize = g->fileSize;
        MD5_CTX rctx;
        uint8_t rhash[MD5_HASH_LEN] = {0};
        uint8_t tmp[256];
        MD5_Init(&rctx);
        uint32_t off = 0;
        while(off < fsize)
        {
            uint32_t len = ((fsize - off) > sizeof(tmp)) ? sizeof(tmp) : (fsize - off);
            FlashDrvRead(OTA_APP_ADDR + off, tmp, len);
            MD5_Update(&rctx, tmp, len);
            off += len;
        }
        MD5_Final(&rctx, rhash);
        char rhex[33];
        MD5_HashToHex(rhash, rhex);
        debugprintf("flash md5:  %s (fileSize=%d)\r\n", rhex, (int)fsize);
        debugprintf("server md5: %s\r\n", g->md5);
        if(0 == strncmp(rhex, g->md5, MD5_HASH_LEN * 2))
            debugprintf("FLASH == SERVER file, OTA content CORRECT\r\n");
        else
            debugprintf("!! FLASH != SERVER file -> OTA write CORRUPTED flash !!\r\n");
    }

    jump_to_application(OTA_APP_ADDR);
}

/* settings_updateerror_confirm_btn_event：UpdateError"确认"按钮点击 → 返回信息页 */
void settings_updateerror_confirm_btn_event(lv_event_t *e)
{
    (void)e;
    if(NULL == guider_ui.SettingsPage4Info)
        setup_scr_SettingsPage4Info(&guider_ui);
    lv_scr_load(guider_ui.SettingsPage4Info);
}


