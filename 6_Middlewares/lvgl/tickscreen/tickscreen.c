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

/* WiFi 断开信号量（二值）：wifi_auto_connect_task 轮询发现掉线后 Give，Info 页 Take → 两个状态 LED 变红 */
SemaphoreHandle_t wifi_disconnected_sem = NULL;

/* 检查更新结果队列（队列项：uint8_t，1=可升级 / 2=不可升级）：
 * check_update_task（业务线程）检测到 isGetUpgrade 后 xQueueSend 发结果，
 * SettingsPage4CheckUpdate1 页 tick（UI 线程）xQueueReceive 接收并按值跳页。 */
QueueHandle_t g_update_result_q = NULL;

/* OTA 下载进度（0~100）：ota_upgrade_task 写、Updating 页 tick 读（见 tickscreen.h 注释） */
volatile int g_ota_progress = 0;

/* OTA 结果队列（队列项：uint8_t，1=完成 / 2=失败）：
 * ota_upgrade_task 发结果，SettingsPage4Updating 页 tick 接收并按值跳页。 */
QueueHandle_t g_ota_result_q = NULL;

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

    if(NULL == wifi_disconnected_sem)
        wifi_disconnected_sem = xSemaphoreCreateBinary();
    if(NULL == wifi_disconnected_sem)
        debugprintf("wifi_disconnected_sem create failed\r\n");

    if(NULL == g_update_result_q)
        g_update_result_q = xQueueCreate(4, sizeof(uint8_t));   /* 深度4足够：一次检查只发一条结果 */
    if(NULL == g_update_result_q)
        debugprintf("update_result_q create failed\r\n");

    if(NULL == g_ota_result_q)
        g_ota_result_q = xQueueCreate(4, sizeof(uint8_t));      /* 深度4足够：一次升级只发一条结果 */
    if(NULL == g_ota_result_q)
        debugprintf("ota_result_q create failed\r\n");
}

/* ==================== 各页面周期循环函数（在此填写页面逻辑） ==================== */

void tick_screen_SettingsPage4Info(void)
{
    /* WiFi 断开信号量 → 两个状态 LED 变红（WiFi 掉线，云平台随之失联；收到即处理一次）。
     * 放在"变绿"之前：若在别的页面期间先断开后重连，断开/连接两个信号量会同时挂起，
     * 先红后绿 → 最终以"当前已连接"为准，避免陈旧的断开信号把已恢复的 LED 又刷红。 */
    if(NULL != wifi_disconnected_sem && pdTRUE == xSemaphoreTake(wifi_disconnected_sem, 0))
    {
        lv_led_set_color(guider_ui.SettingsPage4Info_wifi_status_led,
                         lv_palette_main(LV_PALETTE_RED));
        lv_led_set_color(guider_ui.SettingsPage4Info_aliyun_status_led,
                         lv_palette_main(LV_PALETTE_RED));
    }

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
    /* 检查更新页1（正在检查中）：接收 check_update_task 发来的结果队列，按值跳转页面。
     * 本函数跑在 UI 线程（DispTask），此时调 lv_scr_load/setup_scr 不会与渲染冲突。 */
    uint8_t result = 0;
    if(NULL != g_update_result_q && pdTRUE == xQueueReceive(g_update_result_q, &result, 0))
    {
        if(1 == result)    /* 可升级 → 跳 CheckUpdate3（发现新版本） */
        {
            if(NULL == guider_ui.SettingsPage4CheckUpdate3)
                setup_scr_SettingsPage4CheckUpdate3(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4CheckUpdate3);
        }
        else if(2 == result)   /* 不可升级 → 跳 CheckUpdate2（已是最新版本） */
        {
            if(NULL == guider_ui.SettingsPage4CheckUpdate2)
                setup_scr_SettingsPage4CheckUpdate2(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4CheckUpdate2);
        }
    }
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
    /* 1) OTA 结果队列：1=完成→UpdateCplt，2=失败→UpdateError。
     *    本函数跑在 UI 线程（DispTask），此处 lv_scr_load/setup_scr 不会与渲染冲突。 */
    uint8_t result = 0;
    if(NULL != g_ota_result_q && pdTRUE == xQueueReceive(g_ota_result_q, &result, 0))
    {
        if(1 == result)    /* 升级完成 */
        {
            if(NULL == guider_ui.SettingsPage4UpdateCplt)
                setup_scr_SettingsPage4UpdateCplt(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4UpdateCplt);
        }
        else if(2 == result)   /* 升级失败 */
        {
            if(NULL == guider_ui.SettingsPage4UpdateError)
                setup_scr_SettingsPage4UpdateError(&guider_ui);
            lv_scr_load(guider_ui.SettingsPage4UpdateError);
        }
        return;   /* 已跳页，不再刷新进度条 */
    }

    /* 2) 读取业务线程（ota_upgrade_task）写入的 g_ota_progress 刷新进度条：
     *    static 缓存上次值，进度没变化就不重绘，避免每轮无谓刷屏。
     *    初值 -1 强制首次刷新；新一次 OTA 开始时 g_ota_progress 被清零也会触发刷新。 */
    static int last_progress = -1;
    if(g_ota_progress != last_progress)
    {
        last_progress = g_ota_progress;
        lv_bar_set_value(guider_ui.SettingsPage4Updating_settings_p4_updating_bar,
                         g_ota_progress, LV_ANIM_OFF);
    }
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
