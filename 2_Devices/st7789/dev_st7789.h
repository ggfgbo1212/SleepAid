#ifndef __DEV_ST7789_H__
#define __DEV_ST7789_H__

#include "stdint.h"

/* ==================== 屏幕参数（运行时可变，配合旋转） ==================== */

extern uint16_t st7789_width;
extern uint16_t st7789_height;
#define ST7789_WIDTH            (st7789_width)
#define ST7789_HEIGHT           (st7789_height)

/* ==================== 颜色定义（RGB565） ==================== */

#define ST7789_COLOR_BLACK      0x0000
#define ST7789_COLOR_WHITE      0xFFFF
#define ST7789_COLOR_RED        0xF800
#define ST7789_COLOR_GREEN      0x07E0
#define ST7789_COLOR_BLUE       0x001F
#define ST7789_COLOR_YELLOW     0xFFE0
#define ST7789_COLOR_CYAN       0x07FF
#define ST7789_COLOR_MAGENTA    0xF81F

/* ==================== 设备对象 ==================== */

typedef struct ST7789Dev{
    char *name;
    int (*Init)(struct ST7789Dev *ptdev);
    int (*SetRotation)(struct ST7789Dev *ptdev, uint8_t rotation);
    int (*SetWindow)(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
    int (*WriteData)(struct ST7789Dev *ptdev, const uint8_t *data, uint32_t len);
    int (*Clear)(struct ST7789Dev *ptdev, uint16_t color);
    int (*DrawPoint)(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t color);
    int (*FillRect)(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    int (*DrawImage)(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
    int (*WriteString)(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, char *str, uint16_t color);
}ST7789Device;

/* 获取 ST7789 设备对象（仿 W25Q64 单例模式） */
ST7789Device *GetST7789Device(void);

#endif /* __DEV_ST7789_H__ */
