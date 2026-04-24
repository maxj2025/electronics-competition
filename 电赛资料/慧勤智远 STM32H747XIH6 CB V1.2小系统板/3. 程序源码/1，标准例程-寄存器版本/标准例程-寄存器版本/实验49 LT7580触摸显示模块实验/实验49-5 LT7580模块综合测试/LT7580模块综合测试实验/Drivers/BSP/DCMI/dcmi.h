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
#define FLASH_LED(x)         sys_gpio_pin_set(GPIOG, SYS_GPIO_PIN2, x)      

/* DCMI DMA接收回调函数,需要用户实现该函数 */
extern void (*dcmi_rx_callback)(void);

/******************************************************************************************/

/* 接口函数 */
void dcmi_init(void);      /* 初始化DCMI */
void dcmi_start(void);     /* 启动DCMI传输 */
void dcmi_stop(void);      /* 停止DCMI传输 */
void dcmi_dma_init(uint32_t mem0addr, uint32_t mem1addr, uint16_t memsize, uint8_t memblen, uint8_t meminc);   /* DCMI DMA配置 */

/* 测试用函数 */
void dcmi_cr_set(uint8_t pclk, uint8_t hsync, uint8_t vsync);
void dcmi_set_window(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height);

#endif






