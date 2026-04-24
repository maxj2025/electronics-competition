/******************************************************************************
  * Copyright(c) 2024 Levetop Semiconductor Co., Ltd. All rights reserved
  * @file      LT758_Draw_Font.h
  * @author    Levetop TFT Controller Application Team
  * @version   V1.0.0
  * @date      2024-12-30
  * @brief     LT758x Draw Font function 
*******************************************************************************/
#include "./BSP/LT758_Lib/LT758_Lib.h"
#include "./BSP/IF_PORT/if_port.h"

void LT758_Draw_Font
(
	uint16_t encode		// Encoding type, 0:GB2312  1:GBK  2:BIG5  3:UNICODE  4:ASCII 6:UNICODE
	,
	uint32_t FlashAddr	// Font source address(exSpiFlash)
	,
	uint32_t ShowAddr	// Displays the address of the layer
	,
	uint32_t BuffAddr	// Buff address of the layer
	,
	uint16_t width		// Display the width of the layer
	,
	uint8_t W_Size		// Font width
	,
	uint8_t H_Size		// Font height
	,
	uint8_t Alignment	// Alignment 0:Left	1:Mid	2:Right
	,
	uint32_t FontColor	// The foreground color of the font
	,
	uint8_t star		// Text display mode: 0:txt  1:encryption use '*' (0x2A)
	,
	uint16_t Xs			// Text box start x position
	,
	uint16_t Ys			// Text box start Y position
	,
	uint16_t Xe			// Text box end x position
	,
	uint16_t Ye			// Text box end y position
	,
	uint8_t dis_x		// The interval between two adjacent fonts on the x-axis
	,
	uint8_t dis_y		// The interval between two adjacent fonts on the y-axis
	,
	uint8_t *c			// Address of text
	,
	uint16_t len_max	// max lenght of text
);
