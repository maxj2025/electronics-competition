/**
 ****************************************************************************************************
 * @file        nesplay.h
 * @version     V1.0
 * @brief       APP-NES模拟器 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */

#ifndef __NESPLAY_H
#define __NESPLAY_H

#include "common.h"


extern uint8_t nesruning ;  /* 退出NES的标志 */
extern uint8_t frame_cnt;   /* 统计帧数 */
 
void nes_clock_set(uint8_t PLL);
void load_nes(uint8_t* path);
uint8_t nes_play(void);

#endif







