#include "dev_gpio.h"
#include "errno.h"
#include "drv_gpio.h"

static int GPIO_Init(struct GPIODev *ptdev);
static int GPIO_Write(struct GPIODev *ptdev, unsigned char status);
static int GPIO_Read(struct GPIODev *ptdev);


/* GPIO类对象定义 */
struct GPIODev LED1 = {
    .name = "LED1",
    .port = GPIOD,
    .pin = GPIO_PIN_10,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LED2 = {
    .name = "LED2",
    .port = GPIOD,
    .pin = GPIO_PIN_11,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LED3 = {
    .name = "LED3",
    .port = GPIOD,
    .pin = GPIO_PIN_12,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev SPICS = {
    .name = "W25Q64 CS",
    .port = GPIOA,
    .pin = GPIO_PIN_4,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LCD_RST = {
    .name = "LCD_RST",
    .port = GPIOC,
    .pin = GPIO_PIN_7,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LCD_DC = {
    .name = "LCD_DC",
    .port = GPIOB,
    .pin = GPIO_PIN_0,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LCD_CS = {
    .name = "LCD_CS",
    .port = GPIOA,
    .pin = GPIO_PIN_15,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};

struct GPIODev LCD_LED = {
    .name = "LCD_LED",
    .port = GPIOB,
    .pin = GPIO_PIN_1,
    .Init = GPIO_Init,
    .Write = GPIO_Write,
    .Read = GPIO_Read,
    .next = NULL
};


void GPIO_Regis()
{
    GPIODev_Insert(&LED1);
    GPIODev_Insert(&LED2);
    GPIODev_Insert(&LED3);
    GPIODev_Insert(&SPICS);
    GPIODev_Insert(&LCD_RST);
    GPIODev_Insert(&LCD_DC);
    GPIODev_Insert(&LCD_CS);
    GPIODev_Insert(&LCD_LED);
}


static int GPIO_Init(struct GPIODev *ptdev)
{
    return ESUCCESS;
}

static int GPIO_Write(struct GPIODev *ptdev, unsigned char status)
{
    HAL_GPIO_WritePin(ptdev->port, ptdev->pin,status);
    return ESUCCESS;
}

static int GPIO_Read(struct GPIODev *ptdev)
{
    GPIO_PinState status = HAL_GPIO_ReadPin(ptdev->port, ptdev->pin);
    return ESUCCESS;
}

