/**
 * @file lv_port_disp_templ.c
 *
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp_template.h"
#include <stdbool.h>

#include "dev_st7789.h"
#include "errno.h"

static ST7789Device *pDisp = NULL;

/* ST7789 驱动 rotation 0 = 竖屏 240×320（SquareLine 界面按竖屏设计），与 LVGL 分辨率保持一致 */
#define MY_DISP_HOR_RES  240
#define MY_DISP_VER_RES  320

/*********************
 *      DEFINES
 *********************/
#ifndef MY_DISP_HOR_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
    #define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
    #warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen height, default value 240 is used for now.
    #define MY_DISP_VER_RES    240
#endif

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void disp_init(void);

static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//        const lv_area_t * fill_area, lv_color_t color);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*-----------------------------
     * Create a buffer for drawing
     *----------------------------*/

    /* 单缓冲模式：lv_disp_draw_buf_init 的第二个缓冲传 NULL 即单缓冲。
     * LVGL 先在 buf_1 里画好一块，再整块交给 disp_flush() 刷到屏幕。
     * 缓冲区 = 320 × 10 行 = 6.4KB（要求 ≥ 1 行）。
     * 若 RAM 富余可加大行数（如 50 行 = 32KB），减少 flush 次数。 */
    static lv_disp_draw_buf_t draw_buf;
    static lv_color_t buf_1[MY_DISP_HOR_RES * 10];                          /*A buffer for 10 rows*/
    lv_disp_draw_buf_init(&draw_buf, buf_1, NULL, MY_DISP_HOR_RES * 10);

    /*-----------------------------------
     * Register the display in LVGL
     *----------------------------------*/

    static lv_disp_drv_t disp_drv;                         /*Descriptor of a display driver*/
    lv_disp_drv_init(&disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;

    /*Used to copy the buffer's content to the display*/
    disp_drv.flush_cb = disp_flush;

    /*Set a display buffer*/
    disp_drv.draw_buf = &draw_buf;

    /*Required for Example 3)*/
    //disp_drv.full_refresh = 1;

    /* Fill a memory array with a color if you have GPU.
     * Note that, in lv_conf.h you can enable GPUs that has built-in support in LVGL.
     * But if you have a different GPU you can use with this callback.*/
    //disp_drv.gpu_fill_cb = gpu_fill;

    /*Finally register the driver*/
    lv_disp_drv_register(&disp_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /* 获取 ST7789 设备对象并初始化（含硬件复位、寄存器配置，rotation 3 = 横屏 320×240） */
    pDisp = GetST7789Device();
    if(NULL == pDisp)   return;

    if(ESUCCESS != pDisp->Init(pDisp))   return;

    /* 清屏为黑色，避免上电残留花屏 */
    pDisp->Clear(pDisp, ST7789_COLOR_BLACK);
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_disp_flush_ready()' has to be called when finished.*/
static void disp_flush(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
    if(disp_flush_enabled && pDisp) {
        /* LVGL 传来的 color_p 是该矩形区域的 RGB565 像素（LV_COLOR_16_SWAP=1，
         * 内存中高位字节在前，与驱动字节序一致）。DrawImage 内部会 SetWindow +
         * SPI DMA 发送并阻塞等待完成，单缓冲模式下无需额外同步，不会撕裂。 */
        pDisp->DrawImage(pDisp,
                         area->x1, area->y1,
                         area->x2 - area->x1 + 1,
                         area->y2 - area->y1 + 1,
                         (const uint16_t *)color_p);
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_disp_flush_ready(disp_drv);
}

/*OPTIONAL: GPU INTERFACE*/

/*If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color*/
//static void gpu_fill(lv_disp_drv_t * disp_drv, lv_color_t * dest_buf, lv_coord_t dest_width,
//                    const lv_area_t * fill_area, lv_color_t color)
//{
//    /*It's an example code which should be done by your GPU*/
//    int32_t x, y;
//    dest_buf += dest_width * fill_area->y1; /*Go to the first line*/
//
//    for(y = fill_area->y1; y <= fill_area->y2; y++) {
//        for(x = fill_area->x1; x <= fill_area->x2; x++) {
//            dest_buf[x] = color;
//        }
//        dest_buf+=dest_width;    /*Go to the next line*/
//    }
//}


#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
