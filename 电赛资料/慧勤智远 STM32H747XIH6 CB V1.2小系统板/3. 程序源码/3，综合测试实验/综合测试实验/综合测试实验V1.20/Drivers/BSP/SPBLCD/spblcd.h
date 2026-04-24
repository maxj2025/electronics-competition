/**
 ****************************************************************************************************
 * @file        spblcd.h
 * @version     V1.0
 * @brief       SPB效果实现 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __SPBLCD_H
#define	__SPBLCD_H

#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"


#define SLCD_DMA_MAX_TRANS  60 * 1024      /* DMA一次最多传输60K字节 */
extern uint16_t *g_sramlcdbuf;             /* SRAMLCD缓存,先在SRAM里面将图片解码,并加入图标以及文字等信息 */

  
void slcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
uint16_t slcd_read_point(uint16_t x, uint16_t y);
void slcd_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color);
void slcd_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

void slcd_dma_init(void);
void slcd_dma_enable(uint32_t x);
void slcd_frame_show(uint32_t x);

#endif





