/**
 * @file lv_port_indev_templ.c
 * XPT2046 电阻触摸接入 LVGL（软件 SPI）
 *
 * 流程：lv_port_indev_init() 注册一个 LV_INDEV_TYPE_POINTER 输入设备，
 * LVGL 每 LV_INDEV_DEF_READ_PERIOD（30ms）在 lv_timer_handler() 里自动
 * 调用 touchpad_read_cb() 轮询触摸，命中检测由 LVGL 内部完成并向控件发事件。
 */

/*Copy this file as "lv_port_indev.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_indev_template.h"
#include <stdbool.h>

#include "xpt2046.h"
#include "errno.h"

/*********************
 *      DEFINES
 *********************/
/* 屏幕分辨率（与 lv_port_disp_template.c 保持一致：竖屏 240×320） */
#define MY_INDEV_HOR_RES  240
#define MY_INDEV_VER_RES  320

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void touchpad_init(void);

static void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data);

/**********************
 *  STATIC VARIABLES
 **********************/
static XPT2046Device *pTouch = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_indev_init(void)
{
    touchpad_init();

    /* 注册触摸输入设备（LVGL 按 LV_INDEV_DEF_READ_PERIOD=30ms 周期自动调 read_cb） */
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read_cb;

    lv_indev_drv_register(&indev_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/* 获取 XPT2046 设备对象并初始化（含 SetRotation 与 ST7789 保持同步） */
static void touchpad_init(void)
{
    pTouch = GetXPT2046Device();
    if(NULL == pTouch)   return;

    if(ESUCCESS != pTouch->Init(pTouch))   return;
    pTouch->SetRotation(pTouch, 2);   /* 与 ST7789 rotation 2 一致：竖屏 240×320 */
}

/* 坐标变换（实测校正）：
 * 本屏 XPT2046 的 X 通道对应屏幕"垂直"方向（顶部小、底部大）；
 * Y 通道对应屏幕"水平"方向（左大、右小，反相）。
 * 驱动输出 x∈[0,239]（垂直压缩）、y∈[0,319]（水平反相），
 * 线性还原为显示像素坐标（0,0 = 屏幕左上角）：
 *   dy = x × 320 / 240
 *   dx = 239 − y × 240 / 320
 */
static void touchpad_read_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    (void)indev_drv;

    uint16_t tx = 0, ty = 0;

    if(pTouch && pTouch->IsPressed(pTouch) && pTouch->ReadXY(pTouch, &tx, &ty))
    {
        data->point.x = (lv_coord_t)(239 - (uint32_t)ty * 240 / 320);
        data->point.y = (lv_coord_t)((uint32_t)tx * 320 / 240);
        data->state   = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
