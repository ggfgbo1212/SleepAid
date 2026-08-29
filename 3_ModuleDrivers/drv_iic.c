#include "dev_iic.h"
#include "errno.h"
#include "drv_iic.h"
#include "i2c.h"

static int I2CDrvInit(struct I2CDev *ptdev);
static int I2CDrvWrite(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
static int I2CDrvRead(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);


static void I2C2WaitTxCplt(void);
static void I2C2WaitRxCplt(void);

static volatile unsigned char gI2C2TxCpltFlag = 0;
static volatile unsigned char gI2C2RxCpltFlag = 0;


/* IIC类对象定义 */
struct I2CDev IIC2 = {
    .name = "IIC2",
    .channel = 2,
    .own_addr = 0,
    .slave_addr = 0,
    .Init = I2CDrvInit,
    .Write = I2CDrvWrite,
    .Read = I2CDrvRead,
    .next = NULL
};


void IIC_Regis()
{
    I2CDev_Insert(&IIC2);
}

static int I2CDrvInit(struct I2CDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;
    switch(ptdev->channel)
    {
        case 1:
        {
            break;
        }
        case 2:
        {
            ptdev->own_addr = hi2c2.Init.OwnAddress1;//初始化自身iic地址
            break;
        }
        case 3:break;
        default:break;
    }
    
    return ESUCCESS;
}

static int I2CDrvWrite(struct I2CDev *ptdev, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)   return -EINVAL;
    if(0 == length)   return -EINVAL;
    
    unsigned int init_len = length;
    
    switch(ptdev->channel)
    {
        case 2://IIC2通道
        {
            unsigned char *pbuf = buf;
            if(length == 1)
            {
                HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_IT(&hi2c2, ptdev->slave_addr<<1, buf, length);
                if(HAL_OK != status)    return -EIO;
                I2C2WaitTxCplt();
                break;
            }
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(&hi2c2, ptdev->slave_addr<<1, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                I2C2WaitTxCplt();
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 1:case 3:break;
        default:break;
    }
    return (int)init_len;
}

static int I2CDrvRead(struct I2CDev *ptdev, unsigned char *buf, unsigned int length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == buf)   return -EINVAL;
    if(0 == length)   return -EINVAL;
    
    unsigned int init_len = length;
    
    switch(ptdev->channel)
    {
        case 2://IIC2通道
        {
            unsigned char *pbuf = buf;
            if(length == 1)
            {
                HAL_StatusTypeDef status = HAL_I2C_Master_Receive_IT(&hi2c2, ptdev->slave_addr<<1, buf, length);
                if(HAL_OK != status)    return -EIO;
                I2C2WaitRxCplt();
                break;
            }
            while(length)
            {
                unsigned int size = 0;
                if(length >= 65536)
                    size = 65535;
                else
                    size = length;
                HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(&hi2c2, ptdev->slave_addr<<1, pbuf, size);
                if(HAL_OK != status)    return -EIO;
                I2C2WaitRxCplt();
                pbuf += size;
                length -= size;
            }
            break;
        }
        case 1:case 3:break;
        default:break;
    }
    return (int)init_len;
}

static void I2C2WaitTxCplt(void)
{
    while(gI2C2TxCpltFlag != 1);
    gI2C2TxCpltFlag = 0;
}

//IIC发送完成中断回调函数
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C2)
    {
        gI2C2TxCpltFlag = 1;
    }
}

static void I2C2WaitRxCplt(void)
{
    while(gI2C2RxCpltFlag != 1);
    gI2C2RxCpltFlag = 0;
}

//IIC接收完成中断回调函数
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if(hi2c->Instance == I2C2)
    {
        gI2C2RxCpltFlag = 1;
    }
}


