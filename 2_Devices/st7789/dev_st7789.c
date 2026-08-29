/*
 * ST7789 TFT-LCD driver (240x320, SPI3 + interrupt, RGB565)
 *
 * Adapted from the SPI+DMA reference driver. This version talks through
 * the project device framework: SPI3_Dev via SPIDev_Find("SPI3") and the
 * screen GPIO via GPIODev_Find("LCD_xxx"). The low-level SPI write uses
 * pSPI->Write() which is HAL_SPI_Transmit_DMA() + wait-flag (DMA driven
 * for SPI3), so it blocks until the transfer completes.
 */

#include "dev_st7789.h"
#include "dev_spi.h"
#include "dev_gpio.h"
#include "errno.h"
#include "ST7789_Font.h"


/* ==================== 屏幕参数（运行时可变） ==================== */

uint16_t st7789_width  = 240;
uint16_t st7789_height = 320;


/* ==================== 内部函数声明 ==================== */

static int ST7789Init(struct ST7789Dev *ptdev);
static int ST7789SetRotation(struct ST7789Dev *ptdev, uint8_t rotation);
static int ST7789SetWindow(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
static int ST7789WriteData(struct ST7789Dev *ptdev, const uint8_t *data, uint32_t len);
static int ST7789Clear(struct ST7789Dev *ptdev, uint16_t color);
static int ST7789DrawPoint(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t color);
static int ST7789FillRect(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
static int ST7789DrawImage(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *data);
static int ST7789WriteString(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, char *str, uint16_t color);


/* ==================== 设备对象（仿 W25Q64 单例） ==================== */

static ST7789Device gST7789 = {
    .name = "ST7789",
    .Init = ST7789Init,
    .SetRotation = ST7789SetRotation,
    .SetWindow = ST7789SetWindow,
    .WriteData = ST7789WriteData,
    .Clear = ST7789Clear,
    .DrawPoint = ST7789DrawPoint,
    .FillRect = ST7789FillRect,
    .DrawImage = ST7789DrawImage,
    .WriteString = ST7789WriteString
};

ST7789Device *GetST7789Device(void)
{
    return &gST7789;
}


/* ==================== 底层资源 ==================== */

static SPIDevice  *g_spi;
static GPIODevice *g_rst;
static GPIODevice *g_dc;
static GPIODevice *g_cs;
static GPIODevice *g_led;

/* 从设备链表获取 SPI3 与屏幕 GPIO（需先调用 SPI_DevRegis/GPIO_DevRegis） */
static int ST7789FindDevice(void)
{
    g_spi = SPIDev_Find("SPI3");
    if(NULL == g_spi)   return -ENODEV;

    g_rst = GPIODev_Find("LCD_RST");
    if(NULL == g_rst)   return -ENODEV;

    g_dc = GPIODev_Find("LCD_DC");
    if(NULL == g_dc)    return -ENODEV;

    g_cs = GPIODev_Find("LCD_CS");
    if(NULL == g_cs)    return -ENODEV;

    g_led = GPIODev_Find("LCD_LED");
    if(NULL == g_led)   return -ENODEV;

    return ESUCCESS;
}

static void lcd_cs(uint8_t level)   { g_cs->Write(g_cs, level); }
static void lcd_dc(uint8_t level)   { g_dc->Write(g_dc, level); }
static void lcd_rst(uint8_t level)  { g_rst->Write(g_rst, level); }
static void lcd_led(uint8_t level)  { g_led->Write(g_led, level); }


/* ==================== SPI 底层收发（SPI3 + 中断） ==================== */

#define LCD_SPI_CHUNK   65535   /* HAL SPI Size 为 uint16_t，DMA 单次传输上限，超出由驱动层再分块 */

/* 裸写 SPI，不控制 CS/DC（供整段 CS 低电平的操作复用） */
static int lcd_spi_write(const uint8_t *buf, uint32_t len)
{
    uint32_t sent = 0;

    while(sent < len)
    {
        uint32_t chunk = len - sent;
        if(chunk > LCD_SPI_CHUNK)   chunk = LCD_SPI_CHUNK;

        int ret = g_spi->Write(g_spi, (unsigned char *)(buf + sent), chunk);
        if(ret != (int)chunk)   return -EIO;

        sent += chunk;
    }
    return ESUCCESS;
}

/* 写命令（DC = 0） */
static int lcd_write_cmd(uint8_t cmd)
{
    lcd_cs(0);
    lcd_dc(0);
    int ret = lcd_spi_write(&cmd, 1);
    lcd_cs(1);
    return ret;
}

/* 写单字节数据（DC = 1） */
static int lcd_write_data8(uint8_t data)
{
    lcd_cs(0);
    lcd_dc(1);
    int ret = lcd_spi_write(&data, 1);
    lcd_cs(1);
    return ret;
}

/* 写双字节数据（DC = 1，RGB565 像素值，高位在前） */
static int lcd_write_data16(uint16_t data)
{
    uint8_t buf[2];
    buf[0] = (uint8_t)(data >> 8);
    buf[1] = (uint8_t)(data & 0xFF);

    lcd_cs(0);
    lcd_dc(1);
    int ret = lcd_spi_write(buf, 2);
    lcd_cs(1);
    return ret;
}

/* 批量写数据（DC = 1，分块发送） */
static int lcd_write_buf(const uint8_t *data, uint32_t len)
{
    lcd_cs(0);
    lcd_dc(1);
    int ret = lcd_spi_write(data, len);
    lcd_cs(1);
    return ret;
}


/* ==================== 初始化 ==================== */

/**
  * @brief  ST7789 初始化（硬件复位 + 寄存器配置）
  */
static int ST7789Init(struct ST7789Dev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;

    int ret = ST7789FindDevice();
    if(ESUCCESS != ret) return ret;

    /* 硬件复位 */
    lcd_rst(0);
    HAL_Delay(10);
    lcd_rst(1);
    HAL_Delay(120);

    /* 退出睡眠模式 */
    lcd_write_cmd(0x11);
    HAL_Delay(120);

    /* 设置像素格式：RGB565（16-bit） */
    lcd_write_cmd(0x3A);
    lcd_write_data8(0x55);

    /* 帧率控制 */
    lcd_write_cmd(0xB2);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x00);
    lcd_write_data8(0x33);
    lcd_write_data8(0x33);

    /* 显示反转关 */
    lcd_write_cmd(0x20);

    /* VCOM 设置 */
    lcd_write_cmd(0xBB);
    lcd_write_data8(0x3B);

    /* 电源控制 */
    lcd_write_cmd(0xC2);
    lcd_write_data8(0x01);

    lcd_write_cmd(0xC3);
    lcd_write_data8(0x1A);

    lcd_write_cmd(0xC4);
    lcd_write_data8(0x20);

    lcd_write_cmd(0xC6);
    lcd_write_data8(0x0F);

    lcd_write_cmd(0xD0);
    lcd_write_data8(0xA4);
    lcd_write_data8(0xA1);

    /* Gamma 校正 */
    lcd_write_cmd(0xE0);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x06);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x09);
    lcd_write_data8(0x31);
    lcd_write_data8(0x35);
    lcd_write_data8(0x2F);
    lcd_write_data8(0x31);
    lcd_write_data8(0x37);
    lcd_write_data8(0x3A);
    lcd_write_data8(0x1A);
    lcd_write_data8(0x11);
    lcd_write_data8(0x14);
    lcd_write_data8(0x05);

    lcd_write_cmd(0xE1);
    lcd_write_data8(0xD0);
    lcd_write_data8(0x06);
    lcd_write_data8(0x0C);
    lcd_write_data8(0x09);
    lcd_write_data8(0x31);
    lcd_write_data8(0x35);
    lcd_write_data8(0x2F);
    lcd_write_data8(0x31);
    lcd_write_data8(0x37);
    lcd_write_data8(0x3A);
    lcd_write_data8(0x1A);
    lcd_write_data8(0x11);
    lcd_write_data8(0x14);
    lcd_write_data8(0x05);

    /* 正常显示模式 */
    lcd_write_cmd(0x13);

    /* 显示开 */
    lcd_write_cmd(0x29);

    /* 背光开 */
    lcd_led(1);

    /* 默认竖屏 240×320，rotation 2 = 翻转 180°（按实测面板方向） */
    ST7789SetRotation(ptdev, 2);

    return ESUCCESS;
}


/* ==================== 显示方向 ==================== */

/**
  * @brief  设置屏幕旋转方向
  * @param  rotation 旋转模式：0=竖屏 1=横屏 2=竖屏翻转 3=横屏翻转
  */
static int ST7789SetRotation(struct ST7789Dev *ptdev, uint8_t rotation)
{
    if(NULL == ptdev)   return -EINVAL;

    static const uint8_t madctl_table[] = { 0x00, 0x60, 0xC0, 0xA0 };

    lcd_write_cmd(0x36);
    lcd_write_data8(madctl_table[rotation & 0x03]);

    if(rotation & 0x01)
    {
        st7789_width  = 320;
        st7789_height = 240;
    }
    else
    {
        st7789_width  = 240;
        st7789_height = 320;
    }
    return ESUCCESS;
}


/* ==================== 画图 API ==================== */

/**
  * @brief  设置像素写入窗口
  */
static int ST7789SetWindow(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if(NULL == ptdev)   return -EINVAL;

    uint16_t xe = x + w - 1;
    uint16_t ye = y + h - 1;

    lcd_write_cmd(0x2A);        /* CASET 列地址 */
    lcd_write_data16(x);
    lcd_write_data16(xe);

    lcd_write_cmd(0x2B);        /* RASET 行地址 */
    lcd_write_data16(y);
    lcd_write_data16(ye);

    lcd_write_cmd(0x2C);        /* RAMWR 写内存 */
    return ESUCCESS;
}

/**
  * @brief  批量写入像素数据（配合 ST7789_SetWindow 使用）
  */
static int ST7789WriteData(struct ST7789Dev *ptdev, const uint8_t *data, uint32_t len)
{
    if(NULL == ptdev || NULL == data || 0 == len)   return -EINVAL;

    return lcd_write_buf(data, len);
}

/**
  * @brief  全屏填充
  */
static int ST7789Clear(struct ST7789Dev *ptdev, uint16_t color)
{
    if(NULL == ptdev)   return -EINVAL;

    uint8_t  fill[1024];
    uint8_t  hi = (uint8_t)(color >> 8);
    uint8_t  lo = (uint8_t)(color);
    uint16_t xe = ST7789_WIDTH - 1;
    uint16_t ye = ST7789_HEIGHT - 1;
    uint32_t i, sent = 0;
    uint32_t total = (uint32_t)ST7789_WIDTH * ST7789_HEIGHT * 2;

    /* 预填颜色缓冲 */
    for(i = 0; i < sizeof(fill); i += 2)
    {
        fill[i]     = hi;
        fill[i + 1] = lo;
    }

    /* 一次 CS 周期内发完 窗口设置 + 全部像素数据 */
    lcd_cs(0);

    /* CASET */
    lcd_dc(0);
    uint8_t caset = 0x2A;
    lcd_spi_write(&caset, 1);
    lcd_dc(1);
    uint8_t caset_buf[4] = { 0, 0, (uint8_t)(xe >> 8), (uint8_t)(xe) };
    lcd_spi_write(caset_buf, 4);

    /* RASET */
    lcd_dc(0);
    uint8_t raset = 0x2B;
    lcd_spi_write(&raset, 1);
    lcd_dc(1);
    uint8_t raset_buf[4] = { 0, 0, (uint8_t)(ye >> 8), (uint8_t)(ye) };
    lcd_spi_write(raset_buf, 4);

    /* RAMWR + 像素数据 */
    lcd_dc(0);
    uint8_t ramwr = 0x2C;
    lcd_spi_write(&ramwr, 1);
    lcd_dc(1);
    while(sent < total)
    {
        uint32_t chunk = total - sent;
        if(chunk > sizeof(fill))    chunk = sizeof(fill);
        lcd_spi_write(fill, chunk);
        sent += chunk;
    }

    lcd_cs(1);
    return ESUCCESS;
}

/* 裸画单像素（无 NULL 检查，供 DrawPoint 与字符绘制复用） */
static void st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    uint8_t hi = (uint8_t)(color >> 8);
    uint8_t lo = (uint8_t)(color);

    lcd_cs(0);

    /* CASET */
    lcd_dc(0);
    uint8_t caset = 0x2A;
    lcd_spi_write(&caset, 1);
    lcd_dc(1);
    uint8_t caset_buf[4] = { (uint8_t)(x >> 8), (uint8_t)(x),
                             (uint8_t)(x >> 8), (uint8_t)(x) };
    lcd_spi_write(caset_buf, 4);

    /* RASET */
    lcd_dc(0);
    uint8_t raset = 0x2B;
    lcd_spi_write(&raset, 1);
    lcd_dc(1);
    uint8_t raset_buf[4] = { (uint8_t)(y >> 8), (uint8_t)(y),
                             (uint8_t)(y >> 8), (uint8_t)(y) };
    lcd_spi_write(raset_buf, 4);

    /* RAMWR + 像素 */
    lcd_dc(0);
    uint8_t ramwr = 0x2C;
    lcd_spi_write(&ramwr, 1);
    lcd_dc(1);
    uint8_t pixel[2] = { hi, lo };
    lcd_spi_write(pixel, 2);

    lcd_cs(1);
}

/**
  * @brief  画单个像素点
  */
static int ST7789DrawPoint(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t color)
{
    if(NULL == ptdev)   return -EINVAL;

    st7789_draw_pixel(x, y, color);
    return ESUCCESS;
}

/**
  * @brief  填充矩形区域
  */
static int ST7789FillRect(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if(NULL == ptdev)   return -EINVAL;

    uint8_t  fill[1024];
    uint8_t  hi = (uint8_t)(color >> 8);
    uint8_t  lo = (uint8_t)(color);
    uint16_t xe = x + w - 1;
    uint16_t ye = y + h - 1;
    uint32_t i, sent = 0;
    uint32_t total = (uint32_t)w * h * 2;

    for(i = 0; i < sizeof(fill); i += 2)
    {
        fill[i]     = hi;
        fill[i + 1] = lo;
    }

    lcd_cs(0);

    /* CASET */
    lcd_dc(0);
    uint8_t caset = 0x2A;
    lcd_spi_write(&caset, 1);
    lcd_dc(1);
    uint8_t caset_buf[4] = { (uint8_t)(x >> 8), (uint8_t)(x),
                             (uint8_t)(xe >> 8), (uint8_t)(xe) };
    lcd_spi_write(caset_buf, 4);

    /* RASET */
    lcd_dc(0);
    uint8_t raset = 0x2B;
    lcd_spi_write(&raset, 1);
    lcd_dc(1);
    uint8_t raset_buf[4] = { (uint8_t)(y >> 8), (uint8_t)(y),
                             (uint8_t)(ye >> 8), (uint8_t)(ye) };
    lcd_spi_write(raset_buf, 4);

    /* RAMWR + 像素数据 */
    lcd_dc(0);
    uint8_t ramwr = 0x2C;
    lcd_spi_write(&ramwr, 1);
    lcd_dc(1);
    while(sent < total)
    {
        uint32_t chunk = total - sent;
        if(chunk > sizeof(fill))    chunk = sizeof(fill);
        lcd_spi_write(fill, chunk);
        sent += chunk;
    }

    lcd_cs(1);
    return ESUCCESS;
}

/**
  * @brief  显示图片（16 位色 RGB565）
  */
static int ST7789DrawImage(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
                           const uint16_t *data)
{
    if(NULL == ptdev || NULL == data)   return -EINVAL;

    ST7789SetWindow(ptdev, x, y, w, h);
    return lcd_write_buf((const uint8_t *)data, (uint32_t)w * h * 2);
}


/* ==================== 字符显示 ==================== */

/**
  * @brief  在指定位置写一个字符（8x16 字模）
  */
static void st7789_show_char(uint16_t x, uint16_t y, char ch, uint16_t color)
{
    uint8_t chr = ch - ' ';
    uint8_t col, bit;

    if(chr > 94) return;

    for(col = 0; col < 8; col++)
    {
        uint8_t upper = ST7789_Font8x16[chr][col];
        uint8_t lower = ST7789_Font8x16[chr][col + 8];

        for(bit = 0; bit < 8; bit++)
        {
            if(upper & 0x01)
                st7789_draw_pixel(x + col, y + bit, color);
            upper >>= 1;
        }
        for(bit = 0; bit < 8; bit++)
        {
            if(lower & 0x01)
                st7789_draw_pixel(x + col, y + 8 + bit, color);
            lower >>= 1;
        }
    }
}

/**
  * @brief  在指定位置写字符串（默认 8x16 字体）
  */
static int ST7789WriteString(struct ST7789Dev *ptdev, uint16_t x, uint16_t y, char *str, uint16_t color)
{
    if(NULL == ptdev || NULL == str)   return -EINVAL;

    while(*str)
    {
        if(x + 8 > ST7789_WIDTH)
        {
            x = 0;
            y += 16;
        }
        if(y + 16 > ST7789_HEIGHT)
        {
            y = 0;
        }

        st7789_show_char(x, y, *str, color);
        x += 8;
        str++;
    }
    return ESUCCESS;
}
