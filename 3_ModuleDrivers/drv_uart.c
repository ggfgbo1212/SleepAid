#include "dev_uart.h"
#include "errno.h"
#include "drv_uart.h"
#include "usart.h"
#include "ring_buffer.h"
#include "printf.h"

static int UART_Init(struct UARTDev* dev);
static int UART_Write(struct UARTDev* dev, const uint8_t *pData, uint16_t Size);
static int UART_Read(struct UARTDev* dev, uint8_t *pData, uint16_t Size);

static void WaitUart1Tx(void);
static void WaitUart3Tx(void);

static volatile int g_uart1_waitTx = 0;//等待串口1发送完成中断标志位
static volatile int guart3_waitTX = 0;//等待串口3发送完成中断标志位

unsigned char gUsart3RxData = 0;//串口3接收数据

struct RingBuffer *pRxBuffer = NULL;//串口3专用环形缓冲区


/* 串口类对象定义 */
struct UARTDev DEBUGUART = {
    .name = "DEBUG",
    .UART = &huart1,
    .channel = 1,
    .Init = UART_Init,
    .Write = UART_Write,
    .Read = UART_Read,
    .next = NULL
};

struct UARTDev WIFIUART = {
    .name = "WIFIUART",
    .UART = &huart3,
    .channel = 3,
    .Init = UART_Init,
    .Write = UART_Write,
    .Read = UART_Read,
    .next = NULL
};


void UART_Regis()
{
    UARTDev_Insert(&DEBUGUART);
    UARTDev_Insert(&WIFIUART);
}

static int UART_Init(struct UARTDev* dev)
{
    if(dev == NULL)  return -ENAVAIL;
    
    switch(dev->channel)
    {
        case 1://debug串口打印
        {
            break;
        }
        case 3:
        {
            pRxBuffer = RingBufferNew(sizeof(unsigned char)*100);
            if(NULL == pRxBuffer) {
                printf("RingBufferNew error\r\n");
                return -ENOMEM;
            }
            
            //开启串口3中断接收
            HAL_StatusTypeDef status = HAL_UART_Receive_IT(&huart3, (unsigned char*)&gUsart3RxData, 1);
            if(HAL_OK != status)        return -EIO;
            break;
        }
    }
    
    return ESUCCESS;
}

static int UART_Write(struct UARTDev* dev, const uint8_t *pData, uint16_t Size)
{
    if(dev == NULL || pData == NULL || Size == 0)  return -ENAVAIL;
    
    switch(dev->channel)
    {
        case 1://debug串口打印
        {
            while(HAL_UART_STATE_BUSY_TX == HAL_UART_GetState(&huart1));//检查串口状态
            HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart1, pData, Size);
            if(HAL_OK != status) return -EIO;
            WaitUart1Tx();//死等发送完成
            break;
        }
        case 3:
        {
            while(HAL_UART_STATE_BUSY_TX == HAL_UART_GetState(&huart3));//检查串口状态
            HAL_StatusTypeDef status = HAL_UART_Transmit_IT(&huart3, pData, Size);
            if(HAL_OK != status) return -EIO;
            WaitUart3Tx();//死等发送完成
            break;
        }
      
    }
    
    return Size;
}

static int UART_Read(struct UARTDev* dev, uint8_t *pData, uint16_t Size)
{
    if(dev == NULL || pData == NULL || Size == 0)  return -ENAVAIL;
    
    
    switch(dev->channel)
    {
        case 1://debug串口打印
        {
            break;
        }
        case 3:
        {
            break;//这里没用到我们使用的是中断接收
        }
    }
    
    return Size;
}

static void WaitUart1Tx()
{
    while(g_uart1_waitTx != 1);
    g_uart1_waitTx = 0;
}

/**
 * @brief 等待串口3发送完成
 * 
 */
static void WaitUart3Tx()
{
    while(guart3_waitTX != 1);
    guart3_waitTX = 0;
}


/* 串口发送完成中断 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        g_uart1_waitTx = 1;
    }
    else if(huart->Instance == USART3)
    {
        guart3_waitTX = 1;
    }
}

/* 串口接收完成中断 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART1)
    {
        
    }
    else if(huart->Instance == USART3)
    {
        HAL_UART_Receive_IT(&huart3, &gUsart3RxData, 1);//重新开启接收中断
        
        if(NULL != pRxBuffer)
            pRxBuffer->Write(pRxBuffer, &gUsart3RxData, 1);//将接收到的数据写入到环形缓冲区中
        
        
        //需要解析特定格式的数据，并且将其放进特定节点的一个环形缓冲区里面
        //格式：+EVENT:SocketDown,1,3,123
        extern void NetSocketDataAnalysis(unsigned char data);
        NetSocketDataAnalysis(gUsart3RxData);
    }
}

