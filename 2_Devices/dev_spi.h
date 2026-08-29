#ifndef __DEV_SPI_H__
#define __DEV_SPI_H__

#include "stdint.h"
#include "stm32f4xx_hal.h"

typedef struct SPIDev{
    char *name;
    unsigned char channel;
    SPI_HandleTypeDef* SPI;
    int (*Init)(struct SPIDev *dev);
    int (*Write)(struct SPIDev *dev, unsigned char *wbuf, unsigned int length);
    int (*Read)(struct SPIDev *dev, unsigned char *rbuf, unsigned int length);
    int (*WriteRead)(struct SPIDev *dev, unsigned char *wbuf, unsigned char *rbuf, unsigned int length);
    struct SPIDev *next;
}SPIDevice;


//外部可调用
void SPI_DevRegis(void);
struct SPIDev* SPIDev_Find(char* name);
int SPIDev_Insert(struct SPIDev* Dev);


#endif /* __DEV_SPI_H__ */
