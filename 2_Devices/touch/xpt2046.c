/*
 * XPT2046 电阻触摸驱动（软件 SPI 位操作）
 *
 * 引脚（CubeMX 配置，见 main.h/gpio.c）：
 *   TP_CLK = PE10  GPIO_Output PP   SPEED_HIGH
 *   TP_CS  = PE12  GPIO_Output PP   SPEED_HIGH
 *   TP_DIN = PE13  GPIO_Output PP   SPEED_HIGH
 *   TP_DO  = PE14  GPIO_Input  PULLUP
 *   TP_IRQ = PE15  GPIO_EXTI15 下降沿 + PULLUP（低电平 = 有触摸）
 *
 * 协议：发 8 位控制字节选通道（S/A2-A0/12bit/差分），随后 2 字节读出 12 位 ADC，
 *      再按校准区间线性映射到 240×320 像素坐标（带触摸阈值判断与去抖）。
 *      结构体封装仿 ST7789 单例模式，用 GetXPT2046Device() 获取。
 */

#include "xpt2046.h"
#include "errno.h"


/* ==================== 触摸分辨率（默认竖屏，与 ST7789 保持一致） ==================== */

uint16_t xpt2046_width  = 240;
uint16_t xpt2046_height = 320;


/* ==================== 指令定义 ==================== */

#define XPT2046_CMD_X           0x90  /* 读 X 坐标（12 位，差分） */
#define XPT2046_CMD_Y           0xD0  /* 读 Y 坐标（12 位，差分） */


/* ==================== 内部函数声明 ==================== */

static int    XPT2046Init(struct XPT2046Dev *ptdev);
static int    XPT2046SetRotation(struct XPT2046Dev *ptdev, uint8_t rotation);
static uint8_t XPT2046IsPressed(struct XPT2046Dev *ptdev);
static uint8_t XPT2046ReadXY(struct XPT2046Dev *ptdev, uint16_t *x, uint16_t *y);


/* ==================== 设备对象（仿 W25Q64 单例） ==================== */

static XPT2046Device gXPT2046 = {
    .name = "XPT2046",
    .Init = XPT2046Init,
    .SetRotation = XPT2046SetRotation,
    .IsPressed = XPT2046IsPressed,
    .ReadXY = XPT2046ReadXY
};

XPT2046Device *GetXPT2046Device(void)
{
    return &gXPT2046;
}


/* ==================== 软件 SPI 底层 ==================== */

/**
  * @brief  SPI 写一个字节并读回一个字节（MSB 先行）
  * @param  tx 发送的字节
  * @retval 读回的字节
  */
static uint8_t xpt2046_spi_rw(uint8_t tx)
{
    uint8_t rx = 0;
    uint8_t i;
    volatile uint32_t d;

    for (i = 0; i < 8; i++)
    {
        XPT2046_DIN(tx & 0x80);
        tx <<= 1;

        XPT2046_CLK(1);
        for (d = 0; d < (SystemCoreClock / 2000000); d++) { }
        XPT2046_CLK(0);

        rx <<= 1;
        if (XPT2046_DO)
        {
            rx |= 0x01;
        }
    }

    return rx;
}

/**
  * @brief  读取 ADC 原始值（12 位）
  * @param  cmd 通道选择命令
  * @retval 12 位 ADC 值
  */
static uint16_t xpt2046_read_adc(uint8_t cmd)
{
    uint16_t val;

    XPT2046_CS(0);

    xpt2046_spi_rw(cmd);                        /* 发送命令，丢弃首个返回字节 */
    val  = (uint16_t)xpt2046_spi_rw(0x00) << 4; /* D11 ~ D4               */
    val |= (uint16_t)xpt2046_spi_rw(0x00) >> 4; /* D3 ~ D0（高4位有效）     */

    XPT2046_CS(1);

    return val;     /* 12 位 ADC 结果 */
}


/* ==================== API 实现 ==================== */

/**
  * @brief  XPT2046 初始化
  * @retval ESUCCESS 成功
  */
static int XPT2046Init(struct XPT2046Dev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;

    XPT2046_CS(1);
    XPT2046_CLK(0);
    return ESUCCESS;
}

/**
  * @brief  设置触摸分辨率（与 ST7789_SetRotation 保持同步）
  * @param  rotation 旋转模式：0=竖屏 1=横屏 2=竖屏翻转 3=横屏翻转
  * @retval ESUCCESS 成功
  */
static int XPT2046SetRotation(struct XPT2046Dev *ptdev, uint8_t rotation)
{
    if(NULL == ptdev)   return -EINVAL;

    if (rotation & 0x01)
    {
        xpt2046_width  = 320;
        xpt2046_height = 240;
    }
    else
    {
        xpt2046_width  = 240;
        xpt2046_height = 320;
    }
    return ESUCCESS;
}

/**
  * @brief  判断是否有触摸
  * @retval 1 = 按下, 0 = 未按下
  */
static uint8_t XPT2046IsPressed(struct XPT2046Dev *ptdev)
{
    if(NULL == ptdev)   return 0;

    return (XPT2046_IRQ == 0) ? 1 : 0;
}

/**
  * @brief  读取触摸坐标（已映射到屏幕分辨率）
  * @param  x  X 坐标指针（未按下时保持原值）
  * @param  y  Y 坐标指针（未按下时保持原值）
  * @retval 1 = 有触摸, 0 = 未按下
  */
static uint8_t XPT2046ReadXY(struct XPT2046Dev *ptdev, uint16_t *x, uint16_t *y)
{
    uint16_t raw_x, raw_y;
    int32_t  tmp;

    if(NULL == ptdev || NULL == x || NULL == y)   return 0;

    if (!XPT2046IsPressed(ptdev))
        return 0;

    /* 多次采样取平均（去抖） */
    raw_x = xpt2046_read_adc(XPT2046_CMD_X);
    raw_x += xpt2046_read_adc(XPT2046_CMD_X);
    raw_x >>= 1;

    raw_y = xpt2046_read_adc(XPT2046_CMD_Y);
    raw_y += xpt2046_read_adc(XPT2046_CMD_Y);
    raw_y >>= 1;

    /* 阈值判断：原始值必须在校准区间内，否则视为无效触摸 */
    if (raw_x < XPT2046_X_MIN || raw_x > XPT2046_X_MAX ||
        raw_y < XPT2046_Y_MIN || raw_y > XPT2046_Y_MAX)
    {
        return 0;
    }

    /* 线性映射到像素坐标（带钳位） */
    tmp = ((int32_t)(raw_x - XPT2046_X_MIN) * (XPT2046_WIDTH - 1))
        / (XPT2046_X_MAX - XPT2046_X_MIN);
    if (tmp < 0)                    tmp = 0;
    if (tmp > XPT2046_WIDTH - 1)   tmp = XPT2046_WIDTH - 1;
    *x = (uint16_t)tmp;

    tmp = ((int32_t)(raw_y - XPT2046_Y_MIN) * (XPT2046_HEIGHT - 1))
        / (XPT2046_Y_MAX - XPT2046_Y_MIN);
    if (tmp < 0)                     tmp = 0;
    if (tmp > XPT2046_HEIGHT - 1)   tmp = XPT2046_HEIGHT - 1;
    *y = (uint16_t)tmp;

    return 1;
}
