#ifndef __DEV_W25QX_H
#define __DEV_W25QX_H

#define USE_W25Q64_MODULE

#ifdef USE_W25Q16_MODULE
    #define W25Q16_ID               (0xEF4015)
    #define W25Q16_PAGE_SIZE        (256)
    #define W25Q16_PAGE_NUM         (8192)
    #define W25Q16_SIZE             (W25Q16_PAGE_NUM * W25Q16_PAGE_SIZE)
    #define W25Q16_SECTOR_SIZE      (4096)
    #define W25Q16_SECTOR_NUM       (512)
#endif /* USE_W25Q16_MODULE */

#ifdef USE_W25Q32_MODULE
    #define W25Q32_ID               (0xEF4016)
    #define W25Q32_PAGE_SIZE        (256)
    #define W25Q32_PAGE_NUM         (16384)
    #define W25Q32_SIZE             (W25Q32_PAGE_NUM * W25Q32_PAGE_SIZE)
    #define W25Q32_SECTOR_SIZE      (4096)
    #define W25Q32_SECTOR_NUM       (1024)
#endif /* USE_W25Q32_MODULE */

#ifdef USE_W25Q64_MODULE
    #define W25Q64_ID               (0xEF4017)
    #define W25Q64_PAGE_SIZE        (256)
    #define W25Q64_PAGE_NUM         (32768)
    #define W25Q64_SIZE             (W25Q64_PAGE_NUM * W25Q64_PAGE_SIZE)
    #define W25Q64_SECTOR_SIZE      (4096)
    #define W25Q64_SECTOR_NUM       (2048)
#endif /* USE_W25Q64_MODULE */

#define W25X_WriteEnable		        (0x06) 
#define W25X_WriteDisable		        (0x04) 
#define W25X_ReadStatusReg		        (0x05) 
#define W25X_WriteStatusReg		        (0x01) 
#define W25X_ReadData			        (0x03) 
#define W25X_FastReadData		        (0x0B) 
#define W25X_FastReadDual		        (0x3B) 
#define W25X_PageProgram		        (0x02) 
#define W25X_BlockErase			        (0xD8) 
#define W25X_SectorErase		        (0x20) 
#define W25X_ChipErase			        (0xC7) 
#define W25X_PowerDown			        (0xB9) 
#define W25X_ReleasePowerDown	        (0xAB) 
#define W25X_DeviceID			        (0xAB) 
#define W25X_ManufactDeviceID   	    (0x90) 
#define W25X_JedecDeviceID		        (0x9F)

#define WIP_Flag                        (0x01)  /* Write In Progress (WIP) flag */
#define Dummy_Byte                      (0x55)

typedef struct W25QDev{
    char *name;
    int (*Init)(struct W25QDev *ptdev);
    int (*Erase)(struct W25QDev *ptdev, unsigned int addr, unsigned int sectors);
    int (*Write)(struct W25QDev *ptdev, unsigned int addr, unsigned char *wbuf, unsigned int length);
    int (*Read)(struct W25QDev *ptdev, unsigned int addr, unsigned char *rbuf, unsigned int length);
}W25QDevice;

W25QDevice *GetW25Q64Device(void);

#endif /* __DEV_W25QX_H */
