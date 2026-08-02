#ifndef __BSP_SDRAM_H
#define __BSP_SDRAM_H


#include "bspsysteam.h"

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram, FMC_SDRAM_CommandTypeDef *Command);
void SRAM_Clock_Enable(void);

#endif

