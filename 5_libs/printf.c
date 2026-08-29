#include "dev_uart.h"
#include <stdio.h>

struct __FILE{
    int handle;
};

FILE __stdout;

int fputc(int ch, FILE *f)
{
    (void)f;
    
    UARTDevice *ptdev = UARTDev_Find("DEBUG");
    if(NULL == ptdev)   return 0;
    
    if(ptdev->Write(ptdev, (uint8_t*)&ch, 1) == 1)
        return ch;
    
    return 0;
}
