/**
 ****************************************************************************************************
 * @file        qrplay.h
 * @version     V1.0
 * @brief       APP-二维码识别&编码 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    GD32H759IMT6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __QRPLAY_H
#define __QRPLAY_H

#include "common.h"


#define QR_BACK_COLOR           0XA599      /* 背景色 */

/* 在qrplay.c里面定义 */
extern volatile uint8_t qr_dcmi_rgbbuf_sta; /* RGB BUF状态 */
extern uint16_t qr_dcmi_curline;            /* 摄像头输出数据,当前行编号 */

void qr_cursor_show(uint8_t csize);
void qr_dcmi_rx_callback(void);
void qr_decode_show_result(uint8_t* result);
void qr_decode_play(void);
void qr_encode_play(void);
void qr_play(void);

#endif























