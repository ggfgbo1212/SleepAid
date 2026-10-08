#ifndef __DEV_AT24CXX_H__
#define __DEV_AT24CXX_H__

/* ==================== AT24C02 芯片参数 ==================== */
#define AT24C02_MEM_SIZE        256U    /* 2Kbit = 256 字节，地址 0 ~ 255 */
#define AT24C02_PAGE_SIZE       8U      /* 页写最大 8 字节，且不能跨页 */
#define AT24C02_SLAVE_ADDR      0x50U   /* 7bit 器件地址(8bit 0xA0 >> 1) */

/* 设备对象：上层通过 GetAT24C02Device() 获取句柄后调用其方法 */
typedef struct AT24CXXDev{
    char *name;
    int (*Init)(struct AT24CXXDev *ptdev);
    int (*Erase)(struct AT24CXXDev *ptdev);
    int (*Write)(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *wbuf, unsigned short length);
    int (*Read)(struct AT24CXXDev *ptdev, unsigned short addr, unsigned char *rbuf, unsigned short length);
}AT24CXXDevice;

AT24CXXDevice *GetAT24C02Device(void);

#endif /* __DEV_AT24CXX_H__ */
