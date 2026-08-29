#include "dev_w25qx.h"
#include "dev_gpio.h"
#include "dev_spi.h"
#include "errno.h"
#include <string.h>
#include "printf.h"


static int W25Q64Init(struct W25QDev *ptdev);
static int W25Q64Erase(struct W25QDev *ptdev, unsigned int addr, unsigned int sectors);
static int W25Q64Write(struct W25QDev *ptdev, unsigned int addr, unsigned char *wbuf, unsigned int length);
static int W25Q64Read(struct W25QDev *ptdev, unsigned int addr, unsigned char *rbuf, unsigned int length);

static W25QDevice gW25Q64 = {
    .name = "W25Q64",
    .Init = W25Q64Init,
    .Erase = W25Q64Erase,
    .Write = W25Q64Write,
    .Read = W25Q64Read
};

W25QDevice *GetW25Q64Device(void)
{
    return &gW25Q64;
}

unsigned int ReadID(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return 0;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return 0;
    
    unsigned char cmd = 0x9F;   // W25X_JedecDeviceID
    unsigned char id[3] = {0};
    
    pCS->Write(pCS, 0);
    pSPI->Write(pSPI, &cmd, 1);
    pSPI->Read(pSPI, id, 3);
    pCS->Write(pCS, 1);
    
    unsigned int ID = (id[0] << 16) | (id[1] << 8) | (id[2]);
    return ID;
}

static int W25Q64Init(struct W25QDev *ptdev)
{
    if(NULL == ptdev)   return -EINVAL;
    
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    if(ESUCCESS != pCS->Init(pCS))      return -EIO;
    
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    if(ESUCCESS != pSPI->Init(pSPI))    return -EIO;
    
    unsigned int id = ReadID();
    if(W25Q64_ID != id)     return -EIO;
    debugprintf("W25Q64 ID: %d\r\n", id);
    debugprintf(",,,,,,\r\n");
    
    return ESUCCESS;
}

static int EnableWrite(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char cmd = 0x06;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, &cmd, 1);
    if(len != 1)
        return -EIO;
    pCS->Write(pCS, 1);
    
    return ESUCCESS;
}

static int WaitWrite(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char cmd = 0x05;
    unsigned char status = 0xFF;
    
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, &cmd, 1);
    if(len != 1)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    while((status & 0x01) == 1)
    {
        len = pSPI->Read(pSPI, &status, 1);
        if(len != 1)
        {
            pCS->Write(pCS, 1);
            return -EIO;
        }
    }
    pCS->Write(pCS, 1);
    return ESUCCESS;
}

static int PowerDown(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char cmd = 0xB9;
    
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, &cmd, 1);
    if(len != 1)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    return ESUCCESS;
}

static int WakeUp(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char cmd = 0xAB;
    
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, &cmd, 1);
    if(len != 1)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    return ESUCCESS;
}

static int SectorErase(unsigned int addr)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char data[4];
    data[0] = 0x20;
    data[1] = (addr &0xFF0000)>>16;
    data[2] = (addr &0x00FF00)>>8;
    data[3] = (addr &0x0000FF);
    
    if(EnableWrite() != ESUCCESS)   return -EIO;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, data, 4);
    if(len != 4)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    if(WaitWrite() != ESUCCESS)   return -EIO;
    
    return ESUCCESS;
}

static int BlockErase32K(unsigned int addr)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char data[4];
    data[0] = 0x52;
    data[1] = (addr &0xFF0000)>>16;
    data[2] = (addr &0x00FF00)>>8;
    data[3] = (addr &0x0000FF);
    
    if(EnableWrite() != ESUCCESS)   return -EIO;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, data, 4);
    if(len != 4)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    if(WaitWrite() != ESUCCESS)   return -EIO;
    
    return ESUCCESS;
}

static int BlockErase64K(unsigned int addr)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char data[4];
    data[0] = 0xD8;
    data[1] = (addr &0xFF0000)>>16;
    data[2] = (addr &0x00FF00)>>8;
    data[3] = (addr &0x0000FF);
    
    if(EnableWrite() != ESUCCESS)   return -EIO;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, data, 4);
    if(len != 4)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    if(WaitWrite() != ESUCCESS)   return -EIO;
    
    return ESUCCESS;
}

static int ChipErase(void)
{
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char cmd = 0xC7;
    
    if(EnableWrite() != ESUCCESS)   return -EIO;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, &cmd, 1);
    if(len != 1)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    if(WaitWrite() != ESUCCESS)   return -EIO;
    
    return ESUCCESS;
}

static int PageWrite(unsigned int addr, unsigned char *buf, unsigned short length)
{
    if(NULL == buf) return -EINVAL;
    if(0 == length) return -EINVAL;
    if(addr >= W25Q64_SIZE) return -EINVAL;
    if(256 <= length)
    {
        length = 256;
    }
    
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char data[4];
    data[0] = 0x02;
    data[1] = (addr &0xFF0000)>>16;
    data[2] = (addr &0x00FF00)>>8;
    data[3] = (addr &0x0000FF);
    
    if(EnableWrite() != ESUCCESS)   return -EIO;
    pCS->Write(pCS, 0);
    int len = pSPI->Write(pSPI, data, 4);
    if(len != 4)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    len = pSPI->Write(pSPI, buf, length);
    if(len != length)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 1);
    if(WaitWrite() != ESUCCESS)   return -EIO;
    return (int)length;
}


static int W25Q64Erase(struct W25QDev *ptdev, unsigned int addr, unsigned int sectors)
{
    if(NULL == ptdev)       return -EINVAL;
    if(addr >= W25Q64_SIZE) return -EINVAL;
    if(sectors > W25Q64_SECTOR_NUM) return -EINVAL;
    
    while(sectors--)
    {
        if(SectorErase(addr) != ESUCCESS)
            return -EIO;
        addr = addr + W25Q64_SECTOR_SIZE;
    }
    return ESUCCESS;
}


static int W25Q64Write(struct W25QDev *ptdev, unsigned int addr, unsigned char *wbuf, unsigned int length)
{
    if(NULL == ptdev)       return -EINVAL;
    if(addr >= W25Q64_SIZE) return -EINVAL;
    if(NULL == wbuf)        return -EINVAL;
    if(length > W25Q64_SIZE)
    {
        length = W25Q64_SIZE;
    }
    unsigned int init_len = length;
    
    // 计算擦除的扇区个数
//    unsigned int dwSectorCount = (length + addr%W25Q64_SECTOR_SIZE)/W25Q64_SECTOR_SIZE + 1;
//    if(W25Q64Erase(ptdev, addr, dwSectorCount) != ESUCCESS)
//        return -EIO;
    // 分页写
    // 计算要写的页的个数
    unsigned int dwPageCount = (length + addr%W25Q64_PAGE_SIZE)/W25Q64_PAGE_SIZE + 1;
    // 如果要写入的页只有1页，则从addr处写入length
    if(dwPageCount == 1)
    {
        if(PageWrite(addr, wbuf, length) != length)
            return -EIO;
    }
    // 如果要写的页的个数不止1页
    else
    {
        // 计算起始地址所在页还可以填充/写入多少个数据
        unsigned short dwFirstBytes = 256 - addr%256;
        // 填充起始页可以写的数据
        if(PageWrite(addr, wbuf, dwFirstBytes) != dwFirstBytes)
            return -EIO;
        // 偏移地址和写buf指针
        addr = addr + dwFirstBytes;
        wbuf = wbuf + dwFirstBytes;
        // 计算剩余需要写的数据个数
        length = length - dwFirstBytes;
        while(length)
        {
            // 如果剩余要写的数据个数超过了1页允许的最大个数256byte
            if(length > W25Q64_PAGE_SIZE)
            {
                // 那么就将从addr开始的这一页整页写满
                if(PageWrite(addr, wbuf, W25Q64_PAGE_SIZE) != W25Q64_PAGE_SIZE)
                    return -EIO;
                // 并且将地址偏移1页
                addr = addr + W25Q64_PAGE_SIZE;
                // 数据buf指针指向的地址也偏移256byte
                wbuf = wbuf + W25Q64_PAGE_SIZE;
                // 重新计算剩余个数
                length = length - W25Q64_PAGE_SIZE;
            }
            // 如果剩余要写的数据个数没有超过1页允许的最大个数256byte
            else
            {
                // 那么就将剩余的数据全部写入到addr开始的地址
                if(PageWrite(addr, wbuf, length) != length)
                    return -EIO;
                // 并且将剩余个数写为0表示写完了
                length = 0;
            }
        }
    }
    
    return (int)init_len;
}

static int W25Q64Read(struct W25QDev *ptdev, unsigned int addr, unsigned char *rbuf, unsigned int length)
{    
    if(NULL == ptdev)       return -EINVAL;
    if(addr >= W25Q64_SIZE) return -EINVAL;
    if(NULL == rbuf)        return -EINVAL;
    if(length > W25Q64_SIZE)
    {
        length = W25Q64_SIZE;
    }
    
    GPIODevice *pCS = GPIODev_Find("W25Q64 CS");
    if(NULL == pCS)     return -ENODEV;
    SPIDevice *pSPI = SPIDev_Find("SPI1");
    if(NULL == pSPI)    return -ENODEV;
    
    unsigned char data[4];
    data[0] = 0x03;
    data[1] = (addr &0xFF0000)>>16;
    data[2] = (addr &0x00FF00)>>8;
    data[3] = (addr &0x0000FF);
    
    pCS->Write(pCS, 0);
    if(pSPI->Write(pSPI, data, 4) != 4)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    if(pSPI->Read(pSPI, rbuf, length) != length)
    {
        pCS->Write(pCS, 1);
        return -EIO;
    }
    pCS->Write(pCS, 0);
    
    return (int)length;
}
