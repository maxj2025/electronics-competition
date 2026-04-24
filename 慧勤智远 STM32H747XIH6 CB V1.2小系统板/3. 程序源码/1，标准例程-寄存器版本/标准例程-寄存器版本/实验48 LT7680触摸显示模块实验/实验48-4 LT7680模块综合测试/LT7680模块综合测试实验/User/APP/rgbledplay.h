/**
 ****************************************************************************************************
 * @file        rgbledplay.h
 * @version     V1.0
 * @brief       APP-RGB彩灯测试 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __RGBLEDPLAY_H
#define __RGBLEDPLAY_H

#include "common.h"
#include "./BSP/LED/led.h"


#define LEDR(X)     LED0(X)
#define LEDG(X)     LED1(X)
#define LEDB(X)     LED2(X)



uint8_t rgbled_palette_draw(uint16_t x,uint16_t y,uint16_t width,uint16_t height);
void rgbled_show_colorval(uint16_t x,uint16_t y,uint8_t size,uint16_t color);
void rgbled_io_config(uint8_t af);
void rgbled_pwm_set(uint16_t color);
uint8_t rgbled_play(void); 

#endif










