#ifndef __QSPI_H
#define __QSPI_H


#include "bspsysteam.h"
void ospi_send_cmd(uint8_t cmd, uint32_t addr, uint16_t mode, uint8_t dmcycle);
uint8_t ospi_receive(uint8_t *buf, uint32_t datalen);
uint8_t ospi_transmit(uint8_t *buf, uint32_t datalen);
#endif

