#include "flash.h"
#include "w25q.h"
#include "utils.h"

#define FLASH_BUFFER_SIZE   64
#define FLASH_UINT16_T_SIZE sizeof(uint16_t)

void FLASH_Init(void)
{
  W25Q_Init();
}
	
uint16_t FLASH_ReadWord(uint32_t address)
{
  uint16_t word;
  W25Q_ReadBytes((uint8_t*)&word, address, FLASH_UINT16_T_SIZE);
  return word;
}

void FLASH_WriteWord(uint16_t word, uint32_t address)
{
  W25Q_WriteBytes((uint8_t*)&word, address, FLASH_UINT16_T_SIZE);
}

void FLASH_ReadEntity(uint8_t* buffer, uint16_t fromSector, uint16_t toSector, uint16_t bytesToRead)
{
  uint16_t size;
  uint16_t lastSize = 0;
  uint32_t fromAddress = fromSector * W25Q_SECTOR_SIZE;
  uint32_t toAddress = toSector * W25Q_SECTOR_SIZE;

  while (1)
  {
    size = FLASH_ReadWord(fromAddress);

    if (size == 0xFFFF)
    {
      if (lastSize == bytesToRead)
      {
        W25Q_ReadBytes(buffer, fromAddress - bytesToRead, bytesToRead);
      }
      break;
    }

    fromAddress += FLASH_UINT16_T_SIZE + size;

    if (fromAddress + FLASH_UINT16_T_SIZE + bytesToRead + FLASH_UINT16_T_SIZE >= toAddress)
    {
      break;
    }

    lastSize = size;
  }
}

void FLASH_EraseBytes(uint32_t address, uint16_t bytesToErase)
{
  uint16_t fromSector = address / W25Q_SECTOR_SIZE;
  uint16_t toSector = (address + bytesToErase) / W25Q_SECTOR_SIZE;

  for (uint16_t i = fromSector; i <= toSector; i++)
  {
    W25Q_EraseSector(i);
  }
}

void FLASH_IfNotEmptyErase(uint32_t address, uint16_t bytesToCheck)
{
  uint8_t	buffer[FLASH_BUFFER_SIZE];
  uint16_t bytes;

  do
  {
		bytes = MIN(FLASH_BUFFER_SIZE, bytesToCheck);
    W25Q_ReadBytes(buffer, address, bytes);

    for (uint16_t i = 0; i < bytes; i++)
    {
      if (buffer[i] != 0xFF)
      {
        FLASH_EraseBytes(address + i, bytesToCheck - i);
        break;
      }
    }

    address += bytes;
    bytesToCheck -= bytes;
  } while (bytesToCheck > 0);
}

void FLASH_EraseWriteEntity(uint8_t* buffer, uint16_t fromSector, uint16_t toSector, uint16_t bytesToWrite)
{
  uint16_t size;
  uint32_t fromAddress = fromSector * W25Q_SECTOR_SIZE;
  uint32_t toAddress = toSector * W25Q_SECTOR_SIZE;

  while (1)
  {
    size = FLASH_ReadWord(fromAddress);

    if (size == 0xFFFF)
    {
      break;
    }

    fromAddress += FLASH_UINT16_T_SIZE + size;

    if (fromAddress + FLASH_UINT16_T_SIZE + bytesToWrite + FLASH_UINT16_T_SIZE >= toAddress)
    {
      fromAddress = fromSector * W25Q_SECTOR_SIZE;
      break;
    }
  }

  FLASH_IfNotEmptyErase(fromAddress, FLASH_UINT16_T_SIZE + bytesToWrite + FLASH_UINT16_T_SIZE);
  FLASH_WriteWord(bytesToWrite, fromAddress);
  W25Q_WriteBytes(buffer, fromAddress + FLASH_UINT16_T_SIZE, bytesToWrite);
}
