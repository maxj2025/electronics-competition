/**
 ****************************************************************************************************
 * @file        dcmi.h
 * @version     V1.0
 * @brief       DCMI 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#ifndef _DCMI_H
#define _DCMI_H

#include "./SYSTEM/sys/sys.h"


/* 闪光灯控制引脚 */
#define FLASH_LED(x)         do{ x ? \
                                 HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET); \
                             }while(0)  

/* DCMI DMA接收回调函数,需要用户实现该函数 */
extern void (*dcmi_rx_callback)(void);

extern DCMI_HandleTypeDef g_dcmi_handle;          /* DCMI句柄 */
extern DMA_HandleTypeDef g_dma_dcmi_handle;       /* DMA句柄 */

/******************************************************************************************/

/* 接口函数 */
void dcmi_init(void);      /* 初始化DCMI */
void dcmi_start(void);     /* 启动DCMI传输 */
void dcmi_stop(void);      /* 停止DCMI传输 */
void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint32_t memblen, uint32_t meminc);   /* DCMI DMA配置 */

/* 测试用函数 */
void dcmi_cr_set(uint8_t pclk, uint8_t hsync, uint8_t vsync);
void dcmi_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);

#endif





