/*
 * tickscreen.h
 * 每个页面一个 tick 循环函数（模仿 EEZ Studio 的 tick_screen 机制）。
 *
 * 工作机制：
 *   DispTask 主循环每轮调用 ui_tick()：
 *     1. 用 lv_scr_act() 判断当前显示的是哪个页面（不维护状态变量，切页自动生效）
 *     2. 分发到对应页面的 tick_screen_xxx() 周期函数
 *   各页面的周期逻辑（刷新状态、更新控件）写在对应的 tick_screen_xxx() 里。
 *
 * 注意：本模块所有函数都在 DispTask（UI 线程）里执行，可直接调 LVGL API。
 */

#ifndef TICKSCREEN_H
#define TICKSCREEN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"   /* lv_ui 类型 + guider_ui（各屏控件句柄） */
#include "FreeRTOS.h"     /* SemaphoreHandle_t：页间/WiFi→UI 信号量 */
#include "semphr.h"

/* 页面 ID：与 tick_screen_funcs[] 数组一一对应，顺序对齐 guider_ui 字段 */
typedef enum {
    SCREEN_SETTINGSPAGE4_INFO = 0,
    SCREEN_SETTINGSPAGE4_CHECKUPDATE1,
    SCREEN_SETTINGSPAGE4_CHECKUPDATE2,
    SCREEN_SETTINGSPAGE4_CHECKUPDATE3,
    SCREEN_SETTINGSPAGE4_UPDATING,
    SCREEN_SETTINGSPAGE4_UPDATECPLT,
    SCREEN_SETTINGSPAGE4_UPDATEERROR,
    SCREEN_NUM   /* 页面总数 = tick_screen_funcs[] 长度 */
} screen_id_t;

/* ==================== 每页一个周期循环函数 ==================== */
/* 写各页面周期逻辑的地方（在 tickscreen.c 里实现） */
void tick_screen_SettingsPage4Info(void);
void tick_screen_SettingsPage4CheckUpdate1(void);
void tick_screen_SettingsPage4CheckUpdate2(void);
void tick_screen_SettingsPage4CheckUpdate3(void);
void tick_screen_SettingsPage4Updating(void);
void tick_screen_SettingsPage4UpdateCplt(void);
void tick_screen_SettingsPage4UpdateError(void);

/* 按页面 ID 分发到对应 tick 函数（ui_tick 内部调用，外部一般不用） */
void tick_screen_by_id(screen_id_t id);

/* 每轮 UI 循环调用：tick 当前显示的页面 */
void ui_tick(void);

/* ==================== 页间信号量 ==================== */
/* WiFi 已连接信号量（二值）：wifi_auto_connect_task 连接成功后 Give，
 * SettingsPage4Info 页 tick 函数 Take → wifi_status_led 变绿。
 * WiFi 断开信号量（二值）：wifi_auto_connect_task 轮询发现掉线后 Give，
 * SettingsPage4Info 页 tick 函数 Take → wifi/aliyun 两个状态 LED 变红。
 * 在 DispTask（UI 线程）里初始化，业务任务仅 Give、UI tick 仅 Take，
 * 所有 lv_* 调用始终保持在 UI 线程。 */
void tickscreen_init(void);                    /* 创建各信号量（DispTask 里 setup 前调用） */
extern SemaphoreHandle_t wifi_connected_sem;     /* WiFi 连接成功信号量 */
extern SemaphoreHandle_t aliyun_connected_sem;   /* 入云（阿里云 MQTT）连接成功信号量 */
extern SemaphoreHandle_t wifi_disconnected_sem;  /* WiFi 断开信号量 */

#ifdef __cplusplus
}
#endif

#endif /* TICKSCREEN_H */
