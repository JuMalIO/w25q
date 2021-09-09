#ifndef __w25q_H
#define __w25q_H


#include <stdint.h>

void W25Q_Init(void);
uint32_t W25Q_ReadId(void);
uint32_t ReadBlockCount(void);
void W25Q_EraseSector(uint32_t sector);
void W25Q_EraseBlock(uint32_t block);
void W25Q_EraseChip(void);
void W25Q_ReadBytes(uint8_t *buffer, uint32_t address, uint16_t bytesToRead);
void W25Q_WriteBytes(uint8_t *buffer, uint32_t address, uint32_t bytesToWrite);


#endif /* __w25q_H */
