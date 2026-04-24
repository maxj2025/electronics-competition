/**
 ****************************************************************************************************
 * @file        paint.h
 * @version     V1.0
 * @brief       APP-画板 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __PAINT_H
#define __PAINT_H

#include "common.h"


/* 各图标/图片路径 */
extern uint8_t *const PAINT_COLOR_TBL_PIC; /* 颜色表路径 */


/* RGB565位域定义 */
typedef struct
{
    uint16_t b: 5;
    uint16_t g: 6;
    uint16_t r: 5;
} PIX_RGB565;

void paint_new_pathname(uint8_t *pname);
void paint_show_colorval(uint16_t xr, uint16_t yr, uint16_t color);
uint8_t paint_pen_color_set(uint16_t x, uint16_t y, uint16_t *color, uint8_t *caption);
uint8_t paint_pen_size_set(uint16_t x, uint16_t y, uint16_t color, uint8_t *mode, uint8_t *caption);
void paint_draw_point(uint16_t x, uint16_t y, uint16_t color, uint8_t mode);
uint8_t paint_play(void);


#endif




