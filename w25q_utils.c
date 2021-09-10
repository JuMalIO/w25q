#include "w25q_utils.h"
#include "w25q.h"
#include "utils.h"

#define W25Q_BUFFER_SIZE 64
#define UINT16_T_SIZE sizeof(unit16_t)

uint16_t W25Q_ReadWord(uint32_t address)
{
  uint16_t word;
  W25Q_ReadBytes((uint8_t*)&word, address, UINT16_T_SIZE);
  return word;
}

void W25Q_WriteWord(uint16_t word, uint32_t address)
{
  W25Q_WriteBytes((uint8_t*)&word, address, UINT16_T_SIZE);
}

void W25Q_ReadEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToRead)
{
  uint16_t size;
  uint16_t lastSize = 0;
  uint32_t fromAddress = fromSector * W25Q_SECTOR_SIZE;
  uint32_t toAddress = toSector * W25Q_SECTOR_SIZE;

  while (1)
  {
    size = W25Q_ReadWord(fromAddress);

    if (size == 0xFFFF)
    {
      if (lastSize == bytesToRead)
      {
        W25Q_ReadBytes(buffer, fromAddress - bytesToRead, bytesToRead);
      }
      break;
    }

    fromAddress += UINT16_T_SIZE + size;

    if (fromAddress + UINT16_T_SIZE + bytesToRead + UINT16_T_SIZE >= toAddress)
    {
      break;
    }

    lastSize = size;
  }
}

/*bool W25Q_IsEmptyBytes(uint32_t address, uint32_t bytesToCheck)
{
  uint8_t	buffer[MIN(W25Q_BUFFER_SIZE, bytesToCheck)];

  do
  {
    uint16_t bytes = MIN(W25Q_BUFFER_SIZE, bytesToCheck);

    W25Q_ReadBytes(buffer, address, bytes);

    for (uint16_t i = 0; i < bytes; i++)
    {
      if (buffer[i] != 0xFF)
      {
        return false;
      }
    }

    bytesToCheck -= bytes;
    address += bytes;
  } while (bytesToCheck > 0)

    return true;
}*/

void W25Q_EraseBytes(uint32_t address, uint16_t bytesToErase)
{
  uint32_t fromSector = address / W25Q_SECTOR_SIZE;
  uint32_t toSector = (address + bytesToErase) / W25Q_SECTOR_SIZE;

  for (uint32_t i = fromSector; i <= toSector; i++)
  {
    W25Q_EraseSector(i);
  }
}

void W25Q_IfNotEmptyErase(uint32_t address, uint16_t bytesToCheck)
{
  uint16_t bytes = MIN(W25Q_BUFFER_SIZE, bytesToCheck);
  uint8_t	buffer[bytes];

  do
  {
    W25Q_ReadBytes(buffer, address, bytes);

    for (uint16_t i = 0; i < bytes; i++)
    {
      if (buffer[i] != 0xFF)
      {
        W25Q_EraseBytes(address + i, bytesToCheck - i);
        break;
      }
    }

    address += bytes;
    bytesToCheck -= bytes;
    bytes = MIN(W25Q_BUFFER_SIZE, bytesToCheck);
  } while (bytesToCheck > 0);
}

void W25Q_EraseWriteEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToWrite)
{
  uint16_t size;
  uint32_t fromAddress = fromSector * W25Q_SECTOR_SIZE;
  uint32_t toAddress = toSector * W25Q_SECTOR_SIZE;

  while (1)
  {
    size = W25Q_ReadWord(fromAddress);

    if (size == 0xFFFF)
    {
      break;
    }

    fromAddress += UINT16_T_SIZE + size;

    if (fromAddress + UINT16_T_SIZE + bytesToWrite + UINT16_T_SIZE >= toAddress)
    {
      fromAddress = fromSector * W25Q_SECTOR_SIZE;
      break;
    }
  }

  W25Q_IfNotEmptyErase(fromAddress, UINT16_T_SIZE + bytesToWrite + UINT16_T_SIZE);
  W25Q_WriteWord(bytesToWrite, fromAddress);
  W25Q_WriteBytes(buffer, fromAddress + UINT16_T_SIZE, bytesToWrite);
}
