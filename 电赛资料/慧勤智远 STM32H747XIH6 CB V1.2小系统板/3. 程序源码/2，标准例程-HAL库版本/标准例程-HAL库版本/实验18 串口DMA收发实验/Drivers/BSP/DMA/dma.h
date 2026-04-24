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

extern DMA_HandleTypeDef g_usart_tx_dma_handle;  /* 串口TX DMA句柄 */
extern DMA_HandleTypeDef g_usart_rx_dma_handle;  /* 串口RX DMA句柄 */

/******************************************************************************************/

void dma_usart_tx_init(DMA_Stream_TypeDef *dma_streamx, uint32_t ch);   /* 串口TX DMA初始化函数 */
void dma_usart_rx_init(DMA_Stream_TypeDef *dma_streamx, uint32_t ch, uint32_t par, uint32_t mar);   /* 串口RX DMA初始化函数 */

#endif






