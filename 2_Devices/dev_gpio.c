#include "dev_gpio.h"
#include "drv_gpio.h"
#include "errno.h"
#include <string.h>


static struct GPIODev* HeadDev = NULL;//头节点


void GPIO_DevRegis()
{
    GPIO_Regis();
}

struct GPIODev* GPIODev_Find(char* name)
{
    struct GPIODev* Dev = HeadDev;
    
    while(Dev != NULL)
    {
        if(strstr(Dev->name,  name))
        {
            return Dev;
        }
        Dev = Dev->next;
    }
    return NULL;
}

//链表头插法
int GPIODev_Insert(struct GPIODev* Dev)
{
    if(Dev == NULL)   return -1;//ENAVAIL
    
    if(HeadDev == NULL)
    {
        HeadDev = Dev;
    }
    else
    {
        Dev->next = HeadDev;
        HeadDev = Dev;
    }
    
    return ESUCCESS;
}
