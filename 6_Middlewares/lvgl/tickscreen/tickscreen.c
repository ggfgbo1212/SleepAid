/*
 * tickscreen.c
 * 每页 tick 循环函数 + 分发。
 *
 * 参考 EEZ Studio 生成的 screens.c 结构：
 *   - tick_screen_funcs[] 函数指针表按页面 ID 排列
 *   - ui_tick() 用 lv_scr_act() 判断当前页并分发
 * 与参考工程的差异：不维护 currentScreen 状态变量，
 * 用 lv_scr_act() 动态判断当前页，切页后自动生效。
 *
 * 注意：本文件所有函数都在 DispTask（UI 线程）里执行，可直接调 LVGL API。
 */

#include "tickscreen.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "printf.h"   /* debugprintf（调试用） */

/* ==================== 页间信号量 ==================== */

/* WiFi 已连接信号量（二值）：创建时无令牌（不可 Take），WiFi 任务连接成功后 Give 一次 */
SemaphoreHandle_t wifi_connected_sem = NULL;

/* 入云成功信号量（二值）：mqtt_task 连接阿里云成功后 Give，Info 页 Take → aliyun_status_led 变绿 */
SemaphoreHandle_t aliyun_connected_sem = NULL;

void tickscreen_init(void)
{
    if(NULL == wifi_connected_sem)
        wifi_connected_sem = xSemaphoreCreateBinary();
    if(NULL == wifi_connected_sem)
        debugprintf("wifi_connected_sem create failed\r\n");

    if(NULL == aliyun_connected_sem)
        aliyun_connected_sem = xSemaphoreCreateBinary();
    if(NULL == aliyun_connected_sem)
        debugprintf("aliyun_connected_sem create failed\r\n");
}

/* ==================== 各页面周期循环函数（在此填写页面逻辑） ==================== */

void tick_screen_SettingsPage4Info(void)
{
    /* WiFi 已连接信号量 → WiFi 状态 LED 变绿（非阻塞 Take，收到即处理一次） */
    if(NULL != wifi_connected_sem && pdTRUE == xSemaphoreTake(wifi_connected_sem, 0))
    {
        lv_led_set_color(guider_ui.SettingsPage4Info_wifi_status_led,
                         lv_palette_main(LV_PALETTE_GREEN));
    }

    /* 入云成功信号量 → 云平台状态 LED 变绿（非阻塞 Take，收到即处理一次） */
    if(NULL != aliyun_connected_sem && pdTRUE == xSemaphoreTake(aliyun_connected_sem, 0))
    {
        lv_led_set_color(guider_ui.SettingsPage4Info_aliyun_status_led,
                         lv_palette_main(LV_PALETTE_GREEN));
    }

    /* 两个状态 LED 都为绿色（WiFi 已连 + 入云成功）→ 检查更新按钮才使能，否则失能。
     * static 缓存上次使能状态，仅状态翻转时操作控件，避免每轮重复设置。 */
    static bool check_update_btn_enabled = true;   /* 初值按"使能"对齐 setup 生成的默认可点击状态 */
    lv_color_t green = lv_palette_main(LV_PALETTE_GREEN);
    bool wifi_ok  = (lv_color_to32(((lv_led_t *)guider_ui.SettingsPage4Info_wifi_status_led)->color) == lv_color_to32(green));
    bool aliyun_ok = (lv_color_to32(((lv_led_t *)guider_ui.SettingsPage4Info_aliyun_status_led)->color) == lv_color_to32(green));
    bool can_check = (wifi_ok && aliyun_ok);

    if(can_check && !check_update_btn_enabled)
    {
        /* 使能：去掉失能状态，恢复可点击 */
        lv_obj_clear_state(guider_ui.SettingsPage4Info_check_update_btn, LV_STATE_DISABLED);
        lv_obj_add_flag(guider_ui.SettingsPage4Info_check_update_btn, LV_OBJ_FLAG_CLICKABLE);
        check_update_btn_enabled = true;
    }
    else if(!can_check && check_update_btn_enabled)
    {
        /* 失能：加失能状态，去掉可点击标志 */
        lv_obj_add_state(guider_ui.SettingsPage4Info_check_update_btn, LV_STATE_DISABLED);
        lv_obj_clear_flag(guider_ui.SettingsPage4Info_check_update_btn, LV_OBJ_FLAG_CLICKABLE);
        check_update_btn_enabled = false;
    }

    /* 信息页周期逻辑：例如刷新 WiFi/云平台状态标签
     * lv_label_set_text(guider_ui.SettingsPage4Info_wifi_status_label, "已连接"); */
}

void tick_screen_SettingsPage4CheckUpdate1(void)
{
    /* 检查更新页1（正在检查中）：例如检查结果出来后跳转页面 */
}

void tick_screen_SettingsPage4CheckUpdate2(void)
{
    /* 检查更新页2（已是最新版本） */
}

void tick_screen_SettingsPage4CheckUpdate3(void)
{
    /* 检查更新页3（发现新版本，待确认） */
}

void tick_screen_SettingsPage4Updating(void)
{
    /* 更新中页：例如刷新 OTA 下载进度条
     * lv_bar_set_value(guider_ui.SettingsPage4Updating_settings_p4_updating_bar,
     *                 progress, LV_ANIM_OFF); */
}

void tick_screen_SettingsPage4UpdateCplt(void)
{
    /* 更新完成页 */
}

void tick_screen_SettingsPage4UpdateError(void)
{
    /* 更新失败页 */
}

/* ==================== 分发 ==================== */

typedef void (*tick_screen_func_t)(void);

/* 函数指针表：与 screen_id_t 一一对应（顺序不能乱） */
static tick_screen_func_t tick_screen_funcs[SCREEN_NUM] = {
    tick_screen_SettingsPage4Info,
    tick_screen_SettingsPage4CheckUpdate1,
    tick_screen_SettingsPage4CheckUpdate2,
    tick_screen_SettingsPage4CheckUpdate3,
    tick_screen_SettingsPage4Updating,
    tick_screen_SettingsPage4UpdateCplt,
    tick_screen_SettingsPage4UpdateError,
};

void tick_screen_by_id(screen_id_t id)
{
    if(id < SCREEN_NUM)
        tick_screen_funcs[id]();
}

void ui_tick(void)
{
    lv_obj_t *act = lv_scr_act();
    if(NULL == act)
        return;   /* 屏幕未创建/未加载，跳过（避免 NULL==NULL 误匹配） */

    if(act == guider_ui.SettingsPage4Info)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_INFO);
    else if(act == guider_ui.SettingsPage4CheckUpdate1)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_CHECKUPDATE1);
    else if(act == guider_ui.SettingsPage4CheckUpdate2)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_CHECKUPDATE2);
    else if(act == guider_ui.SettingsPage4CheckUpdate3)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_CHECKUPDATE3);
    else if(act == guider_ui.SettingsPage4Updating)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_UPDATING);
    else if(act == guider_ui.SettingsPage4UpdateCplt)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_UPDATECPLT);
    else if(act == guider_ui.SettingsPage4UpdateError)
        tick_screen_by_id(SCREEN_SETTINGSPAGE4_UPDATEERROR);
}
