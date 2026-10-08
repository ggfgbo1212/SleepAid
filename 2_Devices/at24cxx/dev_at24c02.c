#include "dev_at24cxx.h"
#include "dev_iic.h"
#include "errno.h"
#include <string.h>
#include "stm32f4xx_hal.h"

static int AT24C02Init(struct AT24CXXDev *ptdev);
static int AT24C02Erase(struct AT24CXXDev *ptdev);
static int AT24C02Write(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *wbuf, unsigned short length);
static int AT24C02Read(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *rbuf, unsigned short length);

static AT24CXXDevice gAT24C02 = {
    .name = "AT24C02",
    .Init = AT24C02Init,
    .Erase = AT24C02Erase,
    .Write = AT24C02Write,
    .Read = AT24C02Read
};

AT24CXXDevice *GetAT24C02Device(void)
{
    return &gAT24C02;
}

/* 获取 I2C2 总线对象，并绑定 AT24C02 的 7bit 从机地址。
   本层基于现有 IIC 驱动(drv_iic 实现的 I2CDev)完成收发。 */
static struct I2CDev *GetIICBus(void)
{
    struct I2CDev *pIIC = I2CDev_Find("IIC2");
    if(NULL == pIIC)    return NULL;
    pIIC->slave_addr = AT24C02_SLAVE_ADDR;   /* drv_iic 内部 <<1 后得 0xA0 */
    return pIIC;
}

static int AT24C02Init(struct AT24CXXDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;

    struct I2CDev *pIIC = GetIICBus();
    if(NULL == pIIC)    return -ENODEV;

    /* 探测器件是否应答：仅发一个内存地址字节(无数据，不触发写周期)。
       总线返回 1 = 从机 ACK；返回 -EIO = 无应答(未接/地址错)。 */
    unsigned char dummy = 0x00;
    if(pIIC->Write(pIIC, &dummy, 1) != 1)
        return -EIO;

    return ESUCCESS;
}

static int AT24C02Erase(struct AT24CXXDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;

    unsigned char buf[AT24C02_PAGE_SIZE];
    unsigned short i;
    unsigned short addr;

    for(i = 0; i < AT24C02_PAGE_SIZE; i++)
    {
        buf[i] = 0xFF;   /* 全片擦除 = 所有字节写 0xFF */
    }

    for(addr = 0; addr < AT24C02_MEM_SIZE; addr += AT24C02_PAGE_SIZE)
    {
        if(AT24C02Write(ptdev, addr, buf, AT24C02_PAGE_SIZE) != AT24C02_PAGE_SIZE)
            return -EIO;
    }
    return ESUCCESS;
}

static int AT24C02Write(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *wbuf, unsigned short length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == wbuf)    return -EINVAL;
    if(0 == length)     return -EINVAL;
    if((unsigned int)addr + length > AT24C02_MEM_SIZE)   return -EINVAL;   /* 越界 */

    struct I2CDev *pIIC = GetIICBus();
    if(NULL == pIIC)    return -ENODEV;

    unsigned short remain = length;
    unsigned short offset = 0;
    unsigned short cur = addr;

    /* 页写：一次最多 8 字节且不可跨页，按页边界拆分后逐页写 */
    while(remain)
    {
        unsigned char txbuf[1 + AT24C02_PAGE_SIZE];   /* [0]=内存地址，后随数据 */
        unsigned short page_remain = AT24C02_PAGE_SIZE - (cur % AT24C02_PAGE_SIZE);
        unsigned short chunk = (remain < page_remain) ? remain : page_remain;

        txbuf[0] = (unsigned char)cur;
        memcpy(&txbuf[1], wbuf + offset, chunk);

        if(pIIC->Write(pIIC, txbuf, 1 + chunk) != 1 + (int)chunk)
            return -EIO;

        HAL_Delay(5);   /* tWR<=5ms：等待内部写周期完成 */

        cur    += chunk;
        offset += chunk;
        remain -= chunk;
    }
    return (int)length;
}

static int AT24C02Read(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *rbuf, unsigned short length)
{
    if(NULL == ptdev)   return -EINVAL;
    if(NULL == rbuf)    return -EINVAL;
    if(0 == length)     return -EINVAL;
    if((unsigned int)addr + length > AT24C02_MEM_SIZE)   return -EINVAL;   /* 越界 */

    struct I2CDev *pIIC = GetIICBus();
    if(NULL == pIIC)    return -ENODEV;

    /* 随机读：先"伪写"内存地址(仅地址字节、不写数据)装入内部指针，
       再按"当前地址读"连续读 length 字节(读无页限制)。 */
    unsigned char dummy = (unsigned char)addr;
    if(pIIC->Write(pIIC, &dummy, 1) != 1)
        return -EIO;

    if(pIIC->Read(pIIC, rbuf, length) != (int)length)
        return -EIO;

    return (int)length;
}
