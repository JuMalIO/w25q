#ifndef __flash_H
#define __flash_H


#include <stdint.h>

void FLASH_Init(void);
void FLASH_ReadEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToRead);
void FLASH_EraseWriteEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToWrite);


#endif /* __flash_H */
