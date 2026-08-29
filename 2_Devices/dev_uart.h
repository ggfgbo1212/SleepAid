#ifndef __DEV_UART_H__
#define __DEV_UART_H__

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef struct UARTDev{
    
    char* name;
    UART_HandleTypeDef* UART;
    unsigned char channel;     //使用到了多个串口
    int(*Init)(struct UARTDev* dev);
    int(*Write)(struct UARTDev* dev, const uint8_t *pData, uint16_t Size);
    int(*Read)(struct UARTDev* dev, uint8_t *pData, uint16_t Size);

    struct UARTDev* next;
    
}UARTDevice;


//外部可调用
void UART_DevRegis(void);
struct UARTDev* UARTDev_Find(char* name);
int UARTDev_Insert(struct UARTDev* Dev);

#endif /* __DEV_UART_H__ */
