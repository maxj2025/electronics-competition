/**
 ****************************************************************************************************
 * @file        camera.h
 * @version     V1.0
 * @brief       APP-照相机 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#ifndef __CAMERA_H
#define __CAMERA_H

#include "common.h"


#define OV2640_RGB565_MODE      0       /* rgb565模式 */
#define OV2640_JPEG_MODE        1       /* jpeg模式 */

extern volatile uint8_t hsync_int;      /* 帧中断标志 */
extern volatile uint8_t jpeg_size;      /* jpeg图片分辨率 */
extern volatile uint8_t ov2640_mode;    /* 工作模式:0,RGB565模式;1,JPEG模式 */

extern uint32_t *dcmi_line_buf[2];      /* JPEG数据 DMA双缓存buf指针 */
extern uint32_t *jpeg_data_buf;         /* JPEG数据缓存buf指针 */

extern volatile uint32_t jpeg_data_len; /* buf中的JPEG有效数据长度 */

extern volatile uint8_t jpeg_data_ok;   /* JPEG数据采集完成标志
                                         * 0,数据没有采集完;
                                         * 1,数据采集完了,但是还没处理;
                                         * 2,数据已经处理完成了,可以开始下一帧接收
                                         */
                                         

void jpeg_data_process(void);
void sw_ov2640_mode(void);
void sw_sdcard_mode(void);
void camera_new_pathname(uint8_t *pname, uint8_t mode);
uint8_t ov2640_jpg_photo(uint8_t *pname);
uint8_t camera_play(void);

#endif






