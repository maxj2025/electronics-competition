/**
 ****************************************************************************************************
 * @file        dma.h
 * @version     V1.0
 * @brief       DMA 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __DMA_H
#define	__DMA_H

#include "./SYSTEM/sys/sys.h"


#define USART_RX_DMA_TRANS        10             /* 串口接收DMA一次传输字节数 */

extern uint8_t g_transfer_complete;              /* DMA传输完成标志 */

/******************************************************************************************/

void dma_usart_tx_config(DMA_Stream_TypeDef *dma_streamx, uint8_t ch, uint32_t par, uint32_t mar);  /* 串口TX DMA初始化函数 */
void dma_mux_init(DMA_Stream_TypeDef *dma_streamx, uint8_t ch);     /* DMA请求复用器初始化 */
void dma_enable(DMA_Stream_TypeDef *dma_streamx, uint16_t ndtr);    /* 使能一次DMA传输 */
void dma_usart_rx_config(DMA_Stream_TypeDef *dma_streamx, uint8_t ch, uint32_t par, uint32_t mar);  /* 串口RX DMA初始化函数 */

#endif








