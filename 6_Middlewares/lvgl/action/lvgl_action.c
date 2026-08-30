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
#include "drv_flash.h"/* FlashDrvInit / FlashDrvErase / FlashDrvWrite */

#include "FreeRTOS.h"
#include "task.h"   /* xTaskCreate / vTaskDelay / vTaskDelete */
#include "iap.h"

/* 全局 UI 对象：各屏控件句柄集中存放，外部模块通过 extern 访问 */
lv_ui guider_ui;

/* 检查更新任务：主动查询是否有新固件可升级，循环判断结果后跳转对应页面。
 * 优先级取 3（低于 DispTask 的 4），本任务只在 DispTask 阻塞 vTaskDelay 时被调度，
 * 此时调 lv_scr_load 不会与 lv_timer_handler 的渲染冲突。 */
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
            //跳转页面-SettingsPage4CheckUpdate2
            if(NULL == guider_ui.SettingsPage4CheckUpdate2)
                setup_scr_SettingsPage4CheckUpdate2(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4CheckUpdate2);
            vTaskDelete(NULL);
        }
        else if(temp->isGetUpgrade == 1)//可升级
        {
            temp->isGetUpgrade = 0;
            //跳转页面-SettingsPage4CheckUpdate3
            if(NULL == guider_ui.SettingsPage4CheckUpdate3)
                setup_scr_SettingsPage4CheckUpdate3(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4CheckUpdate3);
            vTaskDelete(NULL);
        }
        
        printf("check_update_task running\r\n");
        vTaskDelay(pdMS_TO_TICKS(200));//回包还没到，等一会儿再查

        /* TODO 不能死等，添加超时机制，超时返回主页面 */
    }

    vTaskDelete(NULL);//任务自杀
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
/* 新固件烧写地址：F407VET6 共 512KB（0x08000000~0x08080000），预留前 400KB 给 bootloader
 * （当前 bootloader 固件实际约 375.5KB，留点余量），新固件区 = 0x08064000~0x08080000，共 112KB。
 * 注意 0x08064000 在扇区7内、不在扇区边界：擦除会连带擦掉扇区内 0x08060000~0x08063FFF 的
 * 空闲预留区（bootloader 止于 0x0805E000，不受影响），新固件必须 ≤ 112KB。 */
#define OTA_APP_ADDR        0x08064000 /* 新固件烧写地址（bootloader 预留 400KB） */
#define OTA_APP_SIZE        0x1C000    /* 新固件区大小：512KB - 400KB = 112KB，超了无法烧写 */

static int ota_upgrade_busy = 0;       /* 升级任务防重入标志 */

/* 升级任务：循环"申请固件分片→烧写flash→MD5校验"，校验通过跳 SettingsPage4UpdateCplt，失败跳 SettingsPage4UpdateError。
 * 优先级取 3（低于 DispTask 的 4），lv_scr_load 只在 DispTask 阻塞 vTaskDelay 时被调度，与渲染不冲突；
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

    while(1)
    {
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
                    if(NULL == guider_ui.SettingsPage4UpdateError)
                        setup_scr_SettingsPage4UpdateError(&guider_ui);
                    lv_scr_load(guider_ui.SettingsPage4UpdateError);
                    ota_upgrade_busy = 0;
                    vTaskDelete(NULL);
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

                    //进度条联动：更新 SettingsPage4Updating 页面的进度条（页面已在升级开始时加载，
                    //bar 默认范围 0~100 与 progress 直接对应；任务优先级 3 < DispTask 4，DispTask 阻塞在
                    //vTaskDelay 时才被调度，此时调 lv_bar_set_value 不会与渲染冲突）
                    lv_bar_set_value(guider_ui.SettingsPage4Updating_settings_p4_updating_bar, progress, LV_ANIM_OFF);

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
                debugprintf("server md5:%s\r\n", Gradeinfo->md5);

                //strncmp 比较满 32 位，避免服务端 md5 未以 '\0' 结尾时 strcmp 越界
                if(0 == strncmp(hex_str, Gradeinfo->md5, MD5_HASH_LEN * 2))//MD5一致
                {
                    debugprintf("md5 Consistent, upgrade success\r\n");

                    //TODO 将新固件版本信息写入 EEPROM（AT24C02 驱动尚未加入，后续补充）：
                    //  记录版本号、CRC、APP起始地址、固件大小，供 bootloader 重启后校验/跳转使用
                    //firmwareInfo CurrentFirmWareInfo;
                    //memcpy(CurrentFirmWareInfo.version, Gradeinfo->version, sizeof(Gradeinfo->version));
                    //CurrentFirmWareInfo.crc = GetCRC((unsigned char*)Gradeinfo->version, sizeof(Gradeinfo->version));
                    //CurrentFirmWareInfo.code_addr = OTA_APP_ADDR;
                    //CurrentFirmWareInfo.code_size = Gradeinfo->fileSize;
                    //ptAT24C02->Write(ptAT24C02, 0, &CurrentFirmWareInfo.version[0], sizeof(firmwareInfo));

                    //上报新版本号
                    // MQTTOTA_InformVersion(1, (const char*)Gradeinfo->version, DeviceName);

                    //跳转页面——SettingsPage4UpdateCplt（升级完成）
                    if(NULL == guider_ui.SettingsPage4UpdateCplt)
                        setup_scr_SettingsPage4UpdateCplt(&guider_ui);
                    lv_scr_load(guider_ui.SettingsPage4UpdateCplt);
                    ota_upgrade_busy = 0;
                    vTaskDelete(NULL);//任务自杀
                }
                else//MD5不一致
                {
                    debugprintf("md5 Not Consistent, upgrade fail\r\n");

                    //跳转页面——SettingsPage4UpdateError（升级失败）
                    if(NULL == guider_ui.SettingsPage4UpdateError)
                        setup_scr_SettingsPage4UpdateError(&guider_ui);
                    lv_scr_load(guider_ui.SettingsPage4UpdateError);
                    ota_upgrade_busy = 0;
                    vTaskDelete(NULL);//任务自杀
                }
                break;
            }
            default:
                ota_upgrade_busy = 0;
                vTaskDelete(NULL);
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


