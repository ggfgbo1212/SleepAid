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

/* 全局 UI 对象：各屏控件句柄集中存放，外部模块通过 extern 访问 */
lv_ui guider_ui;

/* settings_p4_info_check_update：信息页"检查更新"按钮点击 → 检查是否有新版本（占位打印） */
void settings_p4_info_check_update(lv_event_t *e)
{
    (void)e;
    debugprintf("123456789\r\n");
    
    //主动查询是否有新固件可升级
    MQTTOTA_GetFirmware(1, DeviceName);
}

/* settings_p4_info_switch_page：CheckUpdate2"确认"按钮点击 → 切换到信息页（占位打印） */
void settings_p4_info_switch_page(lv_event_t *e)
{
    (void)e;
    debugprintf("hello LVGL\r\n");
}
