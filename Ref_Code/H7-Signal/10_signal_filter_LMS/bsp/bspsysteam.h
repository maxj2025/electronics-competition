#ifndef __BSP_SYSTEAM_H
#define __BSP_SYSTEAM_H


#include "stm32h7xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fmc.h"
#include "gpio.h"
#include "octospi.h"
#include <stdarg.h>
#include "arm_math.h"


#include "usart.h"
#include "adc.h"


#include "bsp_sdram.h"
#include "bsp.h"
#include "qspi.h"
#include "norflash.h"
#include "test_demo.h"
#include "scheduler.h"
#include "usart_app.h"
#include "adc_app.h"
#include "blue_app.h"
#include "fft_app.h"
#include "AD9910.h"
#include "filter_designer.h"
#include "filter_coeff_flash.h"

#include "nlms.h"
extern __attribute__((section (".RAM_D1"))) uint8_t uart_rx_dma_buffer[128];
extern __attribute__((section (".RAM_D1"))) char uart_rx_temp_buffer[128];

extern __attribute__((section (".RAM_D1"))) uint8_t uart3_rx_dma_buffer[128];
extern __attribute__((section (".RAM_D1"))) char uart3_rx_temp_buffer[128];



#endif
