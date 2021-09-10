#ifndef __w25q_utils_H
#define __w25q_utils_H


#include <stdint.h>

void W25Q_ReadEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToRead);
void W25Q_EraseWriteEntity(uint8_t* buffer, uint32_t fromSector, uint32_t toSector, uint16_t bytesToWrite);


#endif /* __w25q_utils_H */
