/**
 ****************************************************************************************************
 * @file        gyroscope.c
 * @version     V1.0
 * @brief       APP-陀螺仪测试 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __GYROSCOPE_H
#define __GYROSCOPE_H

#include "./SYSTEM/sys/sys.h"
#include "common.h"

extern uint8_t *const gyro_remind_tbl[2][GUI_LANGUAGE_NUM];

typedef int32_t s32;
typedef int16_t s16;
typedef int8_t  s8;

uint8_t gyro_play(void);

#endif







