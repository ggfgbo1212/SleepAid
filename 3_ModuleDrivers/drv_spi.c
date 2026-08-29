#include "dev_spi.h"
#include "errno.h"
#include "drv_spi.h"
#include "spi.h"


static volatile unsigned char gSPI1TxCplt = 0;  // 0-not complete;1-complete
static volatile unsigned char gSPI1RxCplt = 0;  // 0-not complete;1-complete
static volatile unsigned char gSPI1TxRxCplt = 0;
static volatile unsigned char gSPI3TxCplt = 0;  // 0-not complete;1-complete
static volatile unsigned char gSPI3RxCplt = 0;  // 0-not complete;1-complete
static volatile unsigned char gSPI3TxRxCplt = 0;  // 0-not complete;1-complete


static int SPIInit(struct SPIDev *dev);
static int SPIWrite(struct SPIDev *dev, unsigned char *wbuf, unsigned int length);
static int SPIRead(struct SPIDev *dev, unsigned char *rbuf, unsigned int length);
static int SPIWriteRead(struct SPIDev *dev, unsigned char *wbuf, unsigned char *rbuf, unsigned int length);

static void SPI1WaitTxCplt(void);
static void SPI1WaitRxCplt(void);
static void SPI1WaitTxRxCplt(void);
static void SPI3WaitTxCplt(void);
static void SPI3WaitRxCplt(void);
static void SPI3WaitTxRxCplt(void);


/* SPI设备对象 */
struct SPIDev SPI1_Dev = {
    .name = "SPI1",
    .channel = 1,
    .SPI = &hspi1,
    .Init = SPIInit,
    .Write = SPIWrite,
    .Read =SPIRead,
    .WriteRead = SPIWriteRead,
    .next = NULL
};

struct SPIDev SPI3_Dev = {
    .name = "SPI3",
    .channel = 3,
    .SPI = &hspi3,
    .Init = SPIInit,
    .Write = SPIWrite,
    .Read = SPIRead,
    .WriteRead = SPIWriteRead,
    .next = NULL
};


void SPI_Regis()
{
    SPIDev_Insert(&SPI1_Dev);
    SPIDev_Insert(&SPI3_Dev);
}

static int SPIInit(struct SPIDev *dev)
{
    if(NULL == dev)   return -EINVAL;
    switch(dev->channel)
    {
        case 1:// SPI1
        {
            break;
        }
        case 3:// SPI3
        {
            break;
        }
        default:
            break;
    }
    return ESUCCESS;
}


static int SPIWrite(struct SPIDev *dev, unsigned char *wbuf, unsigned int length)
{
    if(NULL == dev)   return -EINVAL;
    if(NULL == wbuf)  return -EINVAL;
    if(0 == length)   return -EINVAL;

    unsigned int init_len = length;

    switch(dev->channel)
    {
        case 1:// SPI1
        {
            HAL_StatusTypeDef status = HAL_SPI_Transmit_IT(dev->SPI, wbuf, length);
            if(HAL_OK != status)    return -EIO;
            SPI1WaitTxCplt();
            break;
        }
        case 3:// SPI3
        {
            unsigned char *pbuf = wbuf;
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_SPI_Transmit_DMA(dev->SPI, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                SPI3WaitTxCplt();
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 2:break;
        default:break;
    }
    return (int)init_len;
}

static int SPIRead(struct SPIDev *dev, unsigned char *rbuf, unsigned int length)
{
    if(NULL == dev)   return -EINVAL;
    if(NULL == rbuf)  return -EINVAL;
    if(0 == length)   return -EINVAL;

    unsigned int init_len = length;

    switch(dev->channel)
    {
        case 1:// SPI1
        {
            HAL_StatusTypeDef status = HAL_SPI_Receive_IT(dev->SPI, rbuf, length);
            if(HAL_OK != status)    return -EIO;
            SPI1WaitRxCplt();
            break;
        }
        case 3:// SPI3
        {
            unsigned char *pbuf = rbuf;
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_SPI_Receive_DMA(dev->SPI, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                SPI3WaitTxRxCplt();// 2LINES+MASTER: HAL 内部改调 TransmitReceive_DMA，完成回调为 TxRxCplt
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 2:break;
        default:break;
    }
    return (int)init_len;
}

static int SPIWriteRead(struct SPIDev *dev, unsigned char *wbuf, unsigned char *rbuf, unsigned int length)
{
    if(NULL == dev)   return -EINVAL;
    if(NULL == wbuf)  return -EINVAL;
    if(NULL == rbuf)  return -EINVAL;
    if(0 == length)   return -EINVAL;

    unsigned int init_len = length;

    switch(dev->channel)
    {
        case 1:// SPI1
        {
            HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_IT(dev->SPI, wbuf, rbuf, length);
            if(HAL_OK != status)    return -EIO;
            SPI1WaitTxRxCplt();
            break;
        }
        case 3:// SPI3
        {
            unsigned char *pbuf  = wbuf;
            unsigned char *prbuf = rbuf;
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_SPI_TransmitReceive_DMA(dev->SPI, pbuf, prbuf, size);
                if(HAL_OK != status)    return -EIO;
                SPI3WaitTxRxCplt();
                pbuf  += size;
                prbuf += size;
                length -= size;
            }
            break;
        }
        case 2:break;
        default:break;
    }
    return (int)init_len;
}



static void SPI1WaitTxCplt(void)
{
    while(gSPI1TxCplt != 1);
    gSPI1TxCplt = 0;
}

static void SPI1WaitRxCplt(void)
{
    while(gSPI1RxCplt != 1);
    gSPI1RxCplt = 0;
}

static void SPI1WaitTxRxCplt(void)
{
    while(gSPI1TxRxCplt != 1);
    gSPI1TxRxCplt = 0;
}

static void SPI3WaitTxCplt(void)
{
    while(gSPI3TxCplt != 1);
    gSPI3TxCplt = 0;
}

static void SPI3WaitRxCplt(void)
{
    while(gSPI3RxCplt != 1);
    gSPI3RxCplt = 0;
}

static void SPI3WaitTxRxCplt(void)
{
    while(gSPI3TxRxCplt != 1);
    gSPI3TxRxCplt = 0;
}

/* SPI传输完成中断 */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(SPI1 == hspi->Instance)
    {
        gSPI1TxCplt = 1;
    }
    else if(SPI3 == hspi->Instance)
    {
        gSPI3TxCplt = 1;
    }
}

/* SPI接收完成中断 */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(SPI1 == hspi->Instance)
    {
        gSPI1RxCplt = 1;
    }
    else if(SPI3 == hspi->Instance)
    {
        gSPI3RxCplt = 1;
    }
}

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if(SPI1 == hspi->Instance)
    {
        gSPI1TxRxCplt = 1;
    }
    else if(SPI3 == hspi->Instance)
    {
        gSPI3TxRxCplt = 1;
    }
}
