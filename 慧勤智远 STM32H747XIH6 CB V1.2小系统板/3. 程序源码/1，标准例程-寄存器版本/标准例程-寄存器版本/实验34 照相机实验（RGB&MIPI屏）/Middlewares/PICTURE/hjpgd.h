/**
 ****************************************************************************************************
 * @file        hjpgd.h
 * @version     V1.0
 * @brief       图片解码-jpeg硬件解码部分 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __HJPGD_H
#define __HJPGD_H

#include "./BSP/JPEGCODEC/jpegcodec.h"


extern jpeg_codec_typedef hjpgd;  

/******************************************************************************************/ 
/* 接口函数 */

void jpeg_dma_in_callback(void);
void jpeg_dma_out_callback(void);
void jpeg_endofcovert_callback(void);
void jpeg_hdrover_callback(void);
uint8_t hjpgd_decode(char* pname);

#endif






