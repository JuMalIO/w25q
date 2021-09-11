#ifndef __flash_H
#define __flash_H


#include <stdint.h>

void FLASH_Init(void);
void FLASH_ReadEntity(uint8_t* buffer, uint16_t fromSector, uint16_t toSector, uint16_t bytesToRead);
void FLASH_EraseWriteEntity(uint8_t* buffer, uint16_t fromSector, uint16_t toSector, uint16_t bytesToWrite);


#endif /* __flash_H */
