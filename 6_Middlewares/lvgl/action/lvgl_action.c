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

#include "FreeRTOS.h"
#include "task.h"   /* xTaskCreate / vTaskDelay / vTaskDelete */


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

/* settings_p4_checkupdate3_confirm_upgrade：CheckUpdate3"立即升级"按钮点击 → 立即升级 */
void settings_p4_checkupdate3_confirm_upgrade(lv_event_t *e)
{
    (void)e;
    /* TODO: 立即升级 */
}


