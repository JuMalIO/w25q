#include "w25q.h"

uint16_t W25Q_ReadWord(uint32_t address)
{
	uint16_t word;
	W25Q_ReadBytes((uint8_t*)&word, address, 1);
	return word;
}

bool W25Q_IsEmptyBytes(uint32_t address, uint32_t bytesToCheck)
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
	}	while (bytesToCheck > 0)

	return true;
}

void W25Q_ReadLastEntry(uint8_t *buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToRead)
{
	uint16_t size;
	uint16_t lastSize = 0;
	uint32_t fromAddress = fromSector * W25Q_SECTOR_SIZE;
	uint32_t toAddress = toSector * W25Q_SECTOR_SIZE;
	
	do
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
		
		fromAddress += 2 + size;
		lastSize = size;
	} while (fromAddress + 2 + bytesToRead < toAddress);
}

void W25Q_IfNotEmptyErase(uint32_t address, uint16_t bytesToErase)
{
	
}

void W25Q_EraseWriteEntry(uint8_t *buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToWrite)
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
		
		fromAddress += 2 + size;
		
		if (fromAddress + 2 + bytesToWrite + 2 >= toAddress)
		{
			fromAddress = fromSector * W25Q_SECTOR_SIZE;
			break;
		}
	}

	W25Q_IfNotEmptyErase(fromAddress, 2 + bytesToWrite + 2);
	W25Q_WriteBytes((uint8_t*)&bytesToWrite, fromAddress, 2);
	W25Q_WriteBytes(buffer, fromAddress + 2, bytesToWrite);
}
