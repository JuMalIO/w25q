#ifndef __w25q_H
#define __w25q_H


#include <stdint.h>

#define W25Q_PAGE_SIZE        0x0100                   /* 256 B */
#define W25Q_SECTOR_SIZE      0x1000                   /*  4KB */
#define W25Q_BLOCK_SIZE       (W25Q_SECTOR_SIZE * 16)  /* 64KB */

void W25Q_Init(void);
uint32_t W25Q_ReadId(void);
uint16_t ReadBlockCount(void);
void W25Q_EraseSector(uint16_t sector);
void W25Q_EraseBlock(uint16_t block);
void W25Q_EraseChip(void);
void W25Q_ReadBytes(uint8_t *buffer, uint32_t address, uint16_t bytesToRead);
void W25Q_WriteBytes(uint8_t *buffer, uint32_t address, uint16_t bytesToWrite);


#endif /* __w25q_H */
