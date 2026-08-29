#ifndef __DEV_XPT2046_H__
#define __DEV_XPT2046_H__

#include "stdint.h"
#include "main.h"       /* TP_CLK/TP_CS/TP_DIN/TP_DO/TP_IRQ 引脚宏（CubeMX 生成，见 main.h） */

/* ==================== 触摸分辨率（运行时可变，配合 ST7789 旋转） ==================== */

extern uint16_t xpt2046_width;
extern uint16_t xpt2046_height;
#define XPT2046_WIDTH            (xpt2046_width)
#define XPT2046_HEIGHT           (xpt2046_height)

/* ==================== 软件 SPI 引脚宏（参考 config.h 写法，直接用 HAL） ==================== */

#define XPT2046_CLK(x)   HAL_GPIO_WritePin(TP_CLK_GPIO_Port,  TP_CLK_Pin,  ((x) ? GPIO_PIN_SET   : GPIO_PIN_RESET))
#define XPT2046_CS(x)    HAL_GPIO_WritePin(TP_CS_GPIO_Port,   TP_CS_Pin,   ((x) ? GPIO_PIN_SET   : GPIO_PIN_RESET))
#define XPT2046_DIN(x)   HAL_GPIO_WritePin(TP_DIN_GPIO_Port,  TP_DIN_Pin,  ((x) ? GPIO_PIN_SET   : GPIO_PIN_RESET))
#define XPT2046_DO       (HAL_GPIO_ReadPin(TP_DO_GPIO_Port,   TP_DO_Pin)   == GPIO_PIN_SET)   /* 读回 */
#define XPT2046_IRQ      (HAL_GPIO_ReadPin(TP_IRQ_GPIO_Port,  TP_IRQ_Pin)  == GPIO_PIN_SET)   /* 低=按下 */

/* ==================== 校准参数（按实际屏幕微调，见 README 说明） ==================== */

#define XPT2046_X_MIN       300
#define XPT2046_X_MAX       3800
#define XPT2046_Y_MIN       300
#define XPT2046_Y_MAX       3800

/* ==================== 设备对象 ==================== */

typedef struct XPT2046Dev{
    char *name;
    int    (*Init)(struct XPT2046Dev *ptdev);
    int    (*SetRotation)(struct XPT2046Dev *ptdev, uint8_t rotation);
    uint8_t (*IsPressed)(struct XPT2046Dev *ptdev);
    uint8_t (*ReadXY)(struct XPT2046Dev *ptdev, uint16_t *x, uint16_t *y);
}XPT2046Device;

/* 获取 XPT2046 设备对象（仿 ST7789 单例模式） */
XPT2046Device *GetXPT2046Device(void);

#endif /* __DEV_XPT2046_H__ */
