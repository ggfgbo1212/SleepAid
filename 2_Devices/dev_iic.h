#ifndef __DEV_IIC_H__
#define __DEV_IIC_H__

#include "stdint.h"

typedef struct I2CDev{
    char *name;
    unsigned char channel;
    unsigned short own_addr;  //我们作为从机的地址
    unsigned short slave_addr;//从机的地址
    
    int (*Init)(struct I2CDev *ptdev);
    int (*Write)(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
    int (*Read)(struct I2CDev *ptdev, unsigned char *buf, unsigned int length);
    struct I2CDev *next;
}I2CDevice;


//外部可调用
void IIC_DevRegis(void);
struct I2CDev* I2CDev_Find(char* name);
int I2CDev_Insert(struct I2CDev* Dev);

#endif /* __DEV_IIC_H__ */
