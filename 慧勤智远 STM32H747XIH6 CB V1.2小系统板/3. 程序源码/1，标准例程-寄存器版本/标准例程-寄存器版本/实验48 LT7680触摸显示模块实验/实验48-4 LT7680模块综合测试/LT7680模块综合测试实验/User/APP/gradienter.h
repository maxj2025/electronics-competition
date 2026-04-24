/**
 ****************************************************************************************************
 * @file        gradienter.h
 * @version     V1.0
 * @brief       APP-水平仪 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __GRADIENTER_H
#define __GRADIENTER_H

#include "common.h"


extern uint8_t*const gyro_remind_tbl[2][GUI_LANGUAGE_NUM];


void grad_draw_hline(short x0,short y0,uint16_t len,uint16_t color);
void grad_fill_circle(short x0,short y0,uint16_t r,uint16_t color);
uint8_t grad_load_font(void);
void grad_delete_font(void);

uint8_t grad_play(void); 

#endif



