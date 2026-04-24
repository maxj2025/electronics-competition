/**
 ****************************************************************************************************
 * @file        tomcatplay.h
 * @version     V1.0
 * @brief       APP-TOM猫 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __TOMCATPLAY_H
#define __TOMCATPLAY_H

#include "common.h"


/* 各图标/图片路径 */
extern uint8_t*const TOMCAT_DEMO_PIC;//demo图片路径 	      


void tomcat_load_ui(void);
void tomcat_show_spd(uint16_t x,uint16_t y,uint16_t spd);
uint8_t tomcat_agcspd_set(uint16_t x,uint16_t y,uint8_t *agc,uint16_t *speed,uint8_t*caption);
void tomcat_rec_mode(uint8_t agc);
void tomcat_play_wav(uint8_t *buf,uint32_t len);
void tomcat_data_move(uint8_t* buf,uint16_t size,uint16_t dx);
uint8_t tomcat_play(void);

#endif























