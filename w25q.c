#include "w25q.h"
#include "w25q_config.h"

#define W25Q_CMD_WSR1           0x01  /* Write Status Register-1 */
#define W25Q_CMD_PP             0x02  /* Page Program */
#define W25Q_CMD_R              0x03  /* Read Data */
#define W25Q_CMD_RSR1           0x05  /* Read Status Register-1 */
#define W25Q_CMD_WE             0x06  /* Write Enable */
#define W25Q_CMD_FR             0x0B  /* Fast Read */
#define W25Q_CMD_SE             0x20  /* Sector Erase (4KB) */
#define W25Q_CMD_BE32           0x52  /* Block Erase (32KB) */
#define W25Q_CMD_RID            0x9F  /* JEDEC ID */
#define W25Q_CMD_RPD            0xAB  /* Release Power-down */
#define W25Q_CMD_PD             0xB9  /* Power-down */
#define W25Q_CMD_CE             0xC7  /* Chip Erase */
#define W25Q_CMD_BE64           0xD8  /* Block Erase (64KB) */

#define W25Q_WIP_FLAG           0x01  /* Write In Progress */
#define W25Q_DUMMY_BYTE         0xA5

#define W25Q_CS_LOW()       HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_RESET)
#define W25Q_CS_HIGH()      HAL_GPIO_WritePin(W25Q_CS_PORT, W25Q_CS_PIN, GPIO_PIN_SET)

extern SPI_HandleTypeDef W25Q_SPI;

static uint16_t BlockCount;

uint8_t W25Q_SPI_TransmitReceive(uint8_t data)
{
  uint8_t result;
  HAL_SPI_TransmitReceive(&W25Q_SPI, &data, &result, 1, 100);
  return result;
}

void W25Q_SPI_Receive(uint8_t *buffer, uint16_t bytesToRead)
{
  HAL_SPI_Receive(&W25Q_SPI, buffer, bytesToRead, 100 * (bytesToRead / W25Q_PAGE_SIZE + 1));
}

void W25Q_SPI_Transmit(uint8_t *buffer, uint16_t bytesToWrite)
{
  HAL_SPI_Transmit(&W25Q_SPI, buffer, bytesToWrite, 100);
}

void W25Q_SPI_TransmitCommandAddress(uint8_t cmd, uint32_t address)
{
  W25Q_SPI_TransmitReceive(cmd);
  if (BlockCount > 256)
  {
    W25Q_SPI_TransmitReceive((address & 0xFF000000) >> 24);
  }
  W25Q_SPI_TransmitReceive((address & 0xFF0000) >> 16);
  W25Q_SPI_TransmitReceive((address & 0xFF00) >> 8);
  W25Q_SPI_TransmitReceive(address & 0xFF);
}

void W25Q_WriteEnable(void)
{
  W25Q_CS_LOW();

  W25Q_SPI_TransmitReceive(W25Q_CMD_WE);

  W25Q_CS_HIGH();
}

void W25Q_WaitForWriteEnd(void)
{
  uint8_t flashStatus;

  W25Q_CS_LOW();

  W25Q_SPI_TransmitReceive(W25Q_CMD_RSR1);

  do
  {
    flashStatus = W25Q_SPI_TransmitReceive(W25Q_DUMMY_BYTE);
  }
  while (flashStatus & W25Q_WIP_FLAG);

  W25Q_CS_HIGH();
}

void W25Q_Init(void)
{
  W25Q_CS_HIGH();

  BlockCount = ReadBlockCount();
}

uint32_t W25Q_ReadId(void)
{
  W25Q_CS_LOW();

  W25Q_SPI_TransmitReceive(W25Q_CMD_RID);
  uint32_t id0 = W25Q_SPI_TransmitReceive(W25Q_DUMMY_BYTE);
  uint32_t id1 = W25Q_SPI_TransmitReceive(W25Q_DUMMY_BYTE);
  uint32_t id2 = W25Q_SPI_TransmitReceive(W25Q_DUMMY_BYTE);

  W25Q_CS_HIGH();

  return (id0 << 16) | (id1 << 8) | id2;
}

uint16_t ReadBlockCount(void)
{
  uint32_t id = W25Q_ReadId();

  switch (id & 0x000000FF)
  {
    case 0x20: // w25q512
      return 1024;
    case 0x19: // w25q256
      return 512;
    case 0x18: // w25q128
      return 256;
    case 0x17: // w25q64
      return 128;
    case 0x16: // w25q32
      return 64;
    case 0x15: // w25q16
      return 32;
    case 0x14: // w25q80
      return 16;
    case 0x13: // w25q40
      return 8;
    case 0x12: // w25q20
      return 4;
    case 0x11: // w25q10
      return 2;
    default:
      return 0;
  }
}

void W25Q_EraseSector(uint16_t sector)
{
  W25Q_WriteEnable();

  W25Q_CS_LOW();

  W25Q_SPI_TransmitCommandAddress(W25Q_CMD_SE, sector * W25Q_SECTOR_SIZE);

  W25Q_CS_HIGH();

  W25Q_WaitForWriteEnd();
}

void W25Q_EraseBlock(uint16_t block)
{
  W25Q_WriteEnable();

  W25Q_CS_LOW();

  W25Q_SPI_TransmitCommandAddress(W25Q_CMD_BE64, block * W25Q_BLOCK_SIZE);

  W25Q_CS_HIGH();

  W25Q_WaitForWriteEnd();
}

void W25Q_EraseChip(void)
{
  W25Q_WriteEnable();

  W25Q_CS_LOW();

  W25Q_SPI_TransmitReceive(W25Q_CMD_CE);

  W25Q_CS_HIGH();

  W25Q_WaitForWriteEnd();
}

void W25Q_ReadBytes(uint8_t *buffer, uint32_t address, uint16_t bytesToRead)
{
  W25Q_CS_LOW();

  W25Q_SPI_TransmitCommandAddress(W25Q_CMD_FR, address);
  W25Q_SPI_TransmitReceive(0);
  W25Q_SPI_Receive(buffer, bytesToRead);

  W25Q_CS_HIGH();
}

uint16_t W25Q_WritePage(uint8_t *buffer, uint32_t page, uint16_t offsetBytes, uint16_t bytesToWrite)
{
  if (bytesToWrite + offsetBytes > W25Q_PAGE_SIZE)
  {
    bytesToWrite = W25Q_PAGE_SIZE - offsetBytes;
  }

  W25Q_WriteEnable();

  W25Q_CS_LOW();

  W25Q_SPI_TransmitCommandAddress(W25Q_CMD_PP, page * W25Q_PAGE_SIZE + offsetBytes);
  W25Q_SPI_Transmit(buffer, bytesToWrite);

  W25Q_CS_HIGH();

  W25Q_WaitForWriteEnd();

  return bytesToWrite;
}

void W25Q_WriteBytes(uint8_t *buffer, uint32_t address, uint16_t bytesToWrite)
{
  uint16_t bytesWrote;
  uint16_t offset = address % W25Q_PAGE_SIZE;
  uint32_t page = address / W25Q_PAGE_SIZE;

  do
  {
    bytesWrote = W25Q_WritePage(buffer, page, offset, bytesToWrite);
    page++;
    bytesToWrite -= bytesWrote;
    buffer += bytesWrote;
    offset = 0;
  } while (bytesToWrite > 0);
}
