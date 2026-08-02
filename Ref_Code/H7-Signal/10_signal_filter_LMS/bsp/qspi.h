#ifndef __QSPI_H
#define __QSPI_H


#include "bspsysteam.h"

void ospi_send_cmd(uint8_t cmd, uint32_t addr, uint16_t mode, uint8_t dmcycle);
uint8_t ospi_receive(uint8_t *buf, uint32_t datalen);
uint8_t ospi_transmit(uint8_t *buf, uint32_t datalen);

uint8_t W25Q128_ReadID(uint8_t *id);
void W25Q128_WriteEnable(void);
uint8_t W25Q128_ReadSR1(void);
void W25Q128_WaitBusy(void);
uint8_t W25Q128_EraseSector(uint32_t addr);
uint8_t W25Q128_PageProgram(uint32_t addr, uint8_t *buf, uint16_t len);
uint8_t W25Q128_Write(uint32_t addr, uint8_t *buf, uint32_t len);
uint8_t W25Q128_Read(uint32_t addr, uint8_t *buf, uint32_t len);
#endif

