#include "drv_flash.h"
#include "errno.h"
#include "stm32f4xx_hal.h"
#include "printf.h"

volatile static unsigned int tmp_value = 0;

static uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;  
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;  
  }

  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
static uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;

  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) || (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if(Sector == FLASH_SECTOR_4)
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }  
  return sectorsize;
}

int FlashDrvInit(void)
{
    HAL_NVIC_SetPriority(FLASH_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(FLASH_IRQn);
}

void FLASH_IRQHandler(void)
{
    HAL_FLASH_IRQHandler();
}

void HAL_FLASH_EndOfOperationCallback(uint32_t ReturnValue)
{
    tmp_value = ReturnValue;
}

//死等flash写入完成
static void FlashDrvWaitOptCplt(unsigned int value)
{
    while(tmp_value != value);
    tmp_value = 0;
}




int FlashDrvErase(unsigned int addr_start, unsigned int addr_end)
{
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t SectorError = 0;
    uint32_t FirstSector = 0, NbOfSectors = 0;
    /* Unlock the Flash to enable the flash control register access *************/ 
    HAL_FLASH_Unlock();
    
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR 
                           | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR | FLASH_FLAG_BSY);
    
    /* Get the 1st sector to erase */
    FirstSector = GetSector(addr_start);
    /* Get the number of sector to erase from 1st sector*/
    NbOfSectors = GetSector(addr_end) - FirstSector + 1;
    
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    EraseInitStruct.Sector = FirstSector;
    EraseInitStruct.NbSectors = NbOfSectors;
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);
    HAL_FLASH_Lock();
    if(status == HAL_OK)
        return ESUCCESS;
    return -EIO;
}

int FlashDrvWrite(unsigned int addr, unsigned char *buf, unsigned int length)
{
    unsigned int type = FLASH_TYPEPROGRAM_BYTE;
    HAL_FLASH_Unlock();
    
    volatile uint64_t data = 0;
    unsigned int len = length;
    unsigned char offset = 0;
    while(length != 0)
    {
//        if(length > 8)
//        {
//            type = FLASH_TYPEPROGRAM_DOUBLEWORD;
//            uint64_t *pdata = (uint64_t*)buf;
//            data = (uint64_t)pdata[0];
//            offset = 8;
//        }
        if(length > 4)
        {
            type = FLASH_TYPEPROGRAM_WORD;
            uint32_t *pdata = (uint32_t*)buf;
            data = (uint32_t)pdata[0];
            offset = 4;
        }
        else if(length > 2)
        {
            type = FLASH_TYPEPROGRAM_HALFWORD;
            uint16_t *pdata = (uint16_t*)buf;
            data = (uint16_t)pdata[0];
            offset = 2;
        }
        else
        {
            type = FLASH_TYPEPROGRAM_BYTE;
            uint8_t *pdata = (uint8_t*)buf;
            data = (uint8_t)pdata[0];
            offset = 1;
        }
        HAL_StatusTypeDef status = HAL_FLASH_Program_IT(type, addr, data);
        if(status != HAL_OK)
            break;
        FlashDrvWaitOptCplt(addr);
        length = length - offset;
        buf = buf + offset;
        addr = addr + offset;
    }
    
    HAL_FLASH_Lock();
    
    if(length == 0) return len;
    return (len - length);
}

int FlashDrvRead(unsigned int addr, unsigned char *buf, unsigned int length)
{
    unsigned int len = length;
    unsigned char offset = 0;
    while(length != 0)
    {
        if(length > 8)
        {
            uint64_t data = *(__IO uint64_t*)addr;
            uint64_t *pdata = (uint64_t*)buf;
            *pdata = data;
            offset = 8;
        }
        else if(length > 4)
        {
            uint32_t data = *(__IO uint32_t*)addr;
            uint32_t *pdata = (uint32_t*)buf;
            *pdata = data;
            offset = 4;
        }
        else if(length > 2)
        {
            uint16_t data = *(__IO uint16_t*)addr;
            uint16_t *pdata = (uint16_t*)buf;
            *pdata = data;
            offset = 2;
        }
        else
        {
            uint8_t data = *(__IO uint8_t*)addr;
            uint8_t *pdata = (uint8_t*)buf;
            *pdata = data;
            offset = 1;
        }
        length = length - offset;
        buf = buf + offset;
        addr = addr + offset;
    }
    
    return len;
}
