#include "dev_spi.h"
#include "drv_spi.h"
#include "errno.h"
#include <string.h>


static struct SPIDev* HeadDev = NULL;

void SPI_DevRegis(void)
{
    SPI_Regis();
}

struct SPIDev* SPIDev_Find(char* name)
{
    struct SPIDev* Dev = HeadDev;
    
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

int SPIDev_Insert(struct SPIDev* Dev)
{
    if(Dev == NULL)   return -ENAVAIL;
    
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
