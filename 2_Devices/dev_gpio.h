#ifndef __DEV_GPIO_H__
#define __DEV_GPIO_H__

#include "stdint.h"

typedef struct GPIODev{
    char *name;      //用于查找对象
    void *port;
    uint16_t pin;//unsigned char
    int (*Init)(struct GPIODev *ptdev);
    int (*Write)(struct GPIODev *ptdev, unsigned char status);
    int (*Read)(struct GPIODev *ptdev);
    struct GPIODev *next;
}GPIODevice;


//外部可调用
struct GPIODev* GPIODev_Find(char* name);
int GPIODev_Insert(struct GPIODev* Dev);
void GPIO_DevRegis(void);

#endif /* __DEV_GPIO_H__ */
