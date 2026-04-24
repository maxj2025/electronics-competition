/**
 ****************************************************************************************************
 * @file        recorder.h
 * @version     V1.0
 * @brief       APP-录音机 代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    慧勤智远 STM32开发板
 *
 ****************************************************************************************************
 */
 
#ifndef __RECORDER_H
#define __RECORDER_H

#include "common.h"
#include "wavplay.h"


#define SAI_RX_DMA_BUF_SIZE     4096        /* 定义RX DMA 数组大小 */
#define SAI_RX_FIFO_SIZE        10          /* 定义接收FIFO大小 */

/* 各图标/图片路径 */
extern uint8_t *const RECORDER_DEMO_PIC;        /* demo图片路径 */
extern uint8_t *const RECORDER_RECR_PIC;        /* 录音 松开 */
extern uint8_t *const RECORDER_RECP_PIC;        /* 录音 按下 */
extern uint8_t *const RECORDER_PAUSER_PIC;      /* 暂停 松开 */
extern uint8_t *const RECORDER_PAUSEP_PIC;      /* 暂停 按下 */
extern uint8_t *const RECORDER_STOPR_PIC;       /* 停止 松开 */
extern uint8_t *const RECORDER_STOPP_PIC;       /* 停止 按下 */


uint8_t recoder_sai_fifo_read(uint8_t **buf);
uint8_t recoder_sia_fifo_write(uint8_t *buf);
uint8_t recorder_play(void);

#endif






