/*
 * lvgl_action.h
 * LVGL 控件事件回调模块（应用层 UI 行为独立成层，与 GUI Guider 生成的
 * events_init.c 分开，避免 GUI Guider 重新生成代码时被覆盖）。
 *
 * - 全局 UI 对象 guider_ui 由 lvgl_action.c 定义，本头文件 extern 声明，
 *   任何模块（如 gpio_test.c 的 DispTask）包含本头文件即可访问。
 * - 各控件事件回调函数在本文件声明、在 lvgl_action.c 实现；
 *   "绑定"这一行写在对应屏幕的 setup_scr_SettingsPage4Xxx() 函数里。
 */

#ifndef LVGL_ACTION_H
#define LVGL_ACTION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"   /* lv_ui 结构体类型 + 各屏控件句柄定义 */

/* 全局 UI 对象：外部模块用这个变量访问各屏控件（如 guider_ui.SettingsPage4CheckUpdate2） */
extern lv_ui guider_ui;

/* ==================== 控件事件回调函数 ==================== */
/* 在对应的 setup_scr_SettingsPage4Xxx() 末尾绑定到控件，例如：
 *   lv_obj_add_event_cb(ui->SettingsPage4Info_check_update_btn,
 *                       settings_p4_info_check_update, LV_EVENT_CLICKED, NULL);
 */

/* 信息页"检查更新"按钮点击 → 检查是否有新版本 */
void settings_p4_info_check_update(lv_event_t *e);

/* CheckUpdate2"确认"按钮点击 → 返回信息页 */
void settings_p4_checkupdate2_confirm_back(lv_event_t *e);

/* CheckUpdate3"立即升级"按钮点击 → 立即升级 */
void settings_p4_checkupdate3_confirm_upgrade(lv_event_t *e);

/* UpdateCplt"确认重启"按钮点击 → 确认重启跳转（内容待补充） */
void settings_updatecplt_confirm_btn_event(lv_event_t *e);

/* CheckUpdate2"确认"按钮点击 → 切换到信息页 */
void settings_p4_info_switch_page(lv_event_t *e);

#ifdef __cplusplus
}
#endif

#endif /* LVGL_ACTION_H */
