/**
 ****************************************************************************************************
 * @file        vmeterplay.h
 * @version     V1.0
 * @brief       APP-电压表测试 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __VMETERPLAY_H
#define __VMETERPLAY_H

#include "common.h"

/* 定义ADC采集结果的最大值: 2^位数 - 1 */
#define VMETER_ADC_MAX_VAL      65536


void vmeter_show_7seg(uint16_t x,uint16_t y,uint16_t xend,uint16_t yend,uint16_t offset,uint16_t color,uint16_t size,uint8_t chr,uint8_t mode);
void vmeter_show_num(uint16_t x,uint16_t y,uint8_t len,uint16_t color,uint8_t size,long long num,uint8_t mode);
uint8_t vmeter_play(void);

#endif





