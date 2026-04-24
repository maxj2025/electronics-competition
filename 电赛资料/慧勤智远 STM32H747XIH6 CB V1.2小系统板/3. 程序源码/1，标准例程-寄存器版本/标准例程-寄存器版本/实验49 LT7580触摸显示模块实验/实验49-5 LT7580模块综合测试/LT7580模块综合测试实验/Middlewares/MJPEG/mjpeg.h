/**
 ****************************************************************************************************
 * @file        mjpeg.h
 * @version     V1.0
 * @brief       MJPEG视频处理 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#ifndef __MJPEG_H
#define __MJPEG_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/JPEGCODEC/jpegcodec.h"


extern jpeg_codec_typedef mjpeg;

/******************************************************************************************/

/* 函数声明 */
uint8_t mjpeg_jpeg_core_init(jpeg_codec_typedef *tjpeg);
void mjpeg_jpeg_core_destroy(jpeg_codec_typedef *tjpeg); 
void mjpeg_dma_in_callback(void);
void mjpeg_dma_out_callback(void);
void mjpeg_endofcovert_callback(void);
void mjpeg_hdrover_callback(void);
uint8_t mjpeg_init(uint16_t offx, uint16_t offy, uint32_t width, uint32_t height);
void mjpeg_free(void);
void mjpeg_ltdc_dma2d_yuv2rgb_fill(uint16_t sx, uint16_t sy, jpeg_codec_typedef *tjpeg);
void mjpeg_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color);
uint8_t mjpeg_decode(uint8_t *buf, uint32_t bsize);

#endif




