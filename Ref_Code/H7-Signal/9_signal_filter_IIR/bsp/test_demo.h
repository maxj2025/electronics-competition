#ifndef __TEST_DEMO_H
#define __TEST_DEMO_H


#include "bspsysteam.h"
void Mem_test(void);
void SDRAM_Speed_Test(void);
void norflash_test(void);
static void norflash_dump_buf(const char *name, uint8_t *buf, uint32_t len);
void norflash_speed_test(void);
#endif
