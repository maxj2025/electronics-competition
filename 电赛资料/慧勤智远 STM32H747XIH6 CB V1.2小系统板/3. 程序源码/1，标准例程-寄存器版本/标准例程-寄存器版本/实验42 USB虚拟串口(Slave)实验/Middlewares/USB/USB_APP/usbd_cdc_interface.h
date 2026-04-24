/**
 ****************************************************************************************************
 * @file        usbd_cdc_interface.h
 * @version     V1.0
 * @brief       USB CDC 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __USBD_CDC_IF_H
#define __USBD_CDC_IF_H

#include "usbd_cdc.h"


#define USB_USART_REC_LEN       200     /* USB串口接收缓冲区最大字节数 */

/* 轮询周期，最大65ms，最小1ms */
#define CDC_POLLING_INTERVAL    1       /* 轮询周期，最大65ms，最小1ms */

extern uint8_t  g_usb_usart_rx_buffer[USB_USART_REC_LEN];   /* 接收缓冲,最大USB_USART_REC_LEN个字节.末字节为换行符 */
extern uint16_t g_usb_usart_rx_sta;                         /* 接收状态标记 */

extern USBD_CDC_ItfTypeDef  USBD_CDC_fops;


void cdc_vcp_data_tx(uint8_t *buf, uint32_t len);
void cdc_vcp_data_rx(uint8_t *buf, uint32_t len);
void usb_printf(char* fmt,...); 

#endif 




