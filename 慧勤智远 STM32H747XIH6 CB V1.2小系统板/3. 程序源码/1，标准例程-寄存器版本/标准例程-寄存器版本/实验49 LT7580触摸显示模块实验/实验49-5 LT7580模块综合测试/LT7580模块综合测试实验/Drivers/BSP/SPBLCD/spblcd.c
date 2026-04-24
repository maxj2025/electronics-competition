/**
 ****************************************************************************************************
 * @file        spblcd.c
 * @version     V1.0
 * @brief       SPB效果实现 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/SPBLCD/spblcd.h"
#include "./MALLOC/malloc.h"
#include "./BSP/LCD/ltdc.h"
#include "spb.h"
#include "ucos_ii.h"
#include "common.h"


uint16_t *g_sramlcdbuf; /* SRAM LCD BUFFER,背景图片显存区 */

/**
 * @brief       在指定位置画点
 * @param       x,y     : 点坐标
 * @param       color   : 点颜色
 * @retval      无
 */
void slcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    if (lcdltdc.pwidth && lcddev.dir == 1)
    {
       	g_sramlcdbuf[x + y * spbdev.spbwidth] = color;
    }
    else
    {
        if (lcdltdc.pwidth && lcddev.dir == 0) 
        {
            x = spbdev.spbwidth - x - 1;
        }

        g_sramlcdbuf[y + x * spbdev.spbheight] = color;
    }
}

/**
 * @brief       读取指定位置点的颜色值
 * @param       x,y     : 点坐标
 * @retval      颜色值
 */
uint16_t slcd_read_point(uint16_t x, uint16_t y)
{
    if (lcdltdc.pwidth && lcddev.dir == 1)
    {
	      return g_sramlcdbuf[x + y * spbdev.spbwidth];
    }
    else
    {
        if (lcdltdc.pwidth && lcddev.dir == 0) 
        {
            x = spbdev.spbwidth - x - 1;
        }

	      return g_sramlcdbuf[y + x * spbdev.spbheight];
    }
}

/**
 * @brief       填充颜色
 * @param       x,y     : 点坐标
 * @param       width   : 宽度
 * @param       height  : 高度
 * @param       *color  : 颜色数组
 * @retval      无
 */
void slcd_fill_color(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t *color)
{
    uint16_t i, j;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            slcd_draw_point(x + j, y + i, *color++);
        }
    }
}

/**
 * @brief       在指定区域内填充指定颜色
 * @param       x,y     : 点坐标
 * @param       width   : 宽度
 * @param       height  : 高度
 * @param       color   : 要填充的颜色
 * @retval      无
 */
void slcd_fill(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint16_t i, j;

    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            slcd_draw_point(x + j, y + i, color);
        }
    }
}

/**
 * @brief       SRAM到LCD_RAM DMA配置
 *              16位,外部SRAM传输到LCD GRAM
 * @param       无
 * @retval      无
 */
void slcd_dma_init(void)
{
#if STM32_FMC_16

    RCC->AHB1ENR |= 1 << 1;             /* DMA2时钟使能 */

    while (DMA2_Stream0->CR & 0X01);    /* 等待DMA2_Stream0可配置 */

    DMA2->LIFCR |= 0X3D << 6 * 0;       /* 清空通道0上所有中断标志 */
    DMA2_Stream0->FCR = 0X0000021;      /* 设置为默认值 */
 
    DMA2_Stream0->PAR = 0;              /* 暂不设置 */
    DMA2_Stream0->M0AR = (uint32_t)&LCD->LCD_RAM;   /* 目标地址为LCD_RAM */
    DMA2_Stream0->M1AR = 0;             /* 不用设置 */
    DMA2_Stream0->NDTR = 0;             /* 暂时设置长度为0 */
    DMA2_Stream0->CR = 0;               /* 先全部复位CR寄存器值 */
    DMA2_Stream0->CR |= 2 << 6;         /* 存储器到存储器模式 */
    DMA2_Stream0->CR |= 0 << 8;         /* 普通模式 */
    DMA2_Stream0->CR |= 1 << 9;         /* 外设增量模式 */
    DMA2_Stream0->CR |= 0 << 10;        /* 存储器非增量模式 */
    DMA2_Stream0->CR |= 1 << 11;        /* 外设数据长度:16位 */
    DMA2_Stream0->CR |= 1 << 13;        /* 存储器数据长度:16位 */
    DMA2_Stream0->CR |= 0 << 16;        /* 低优先级 */
    DMA2_Stream0->CR |= 0 << 18;        /* 单缓冲模式 */
    DMA2_Stream0->CR |= 0 << 21;        /* 外设突发单次传输 */
    DMA2_Stream0->CR |= 0 << 23;        /* 存储器突发单次传输 */

    DMA2_Stream0->FCR &= ~(1 << 2);     /* 不使用FIFO模式 */
    DMA2_Stream0->FCR &= ~(3 << 0);     /* 无FIFO 设置 */
    
#endif
}

/**
 * @brief       开启一次外部SRAM到LCD的DMA传输
 * @param       x       : 起始传输地址编号
 * @retval      无
 */
void slcd_dma_enable(uint32_t x)
{
    uint32_t lcdsize = spbdev.spbwidth * spbdev.spbheight;
    uint32_t dmatransfered = 0;

    while (lcdsize)
    {
        DMA2_Stream0->CR &= ~(1 << 0);                /* 关闭DMA传输 */

        while (DMA2_Stream0->CR & 0X01);              /* 等待DMA2_Stream0可配置 */

        DMA2->LIFCR |= 1 << 5;                        /* 清除上次的传输完成标记 */

        if (lcdsize > SLCD_DMA_MAX_TRANS)
        {
            lcdsize -= SLCD_DMA_MAX_TRANS;
            DMA2_Stream0->NDTR = SLCD_DMA_MAX_TRANS;  /* 设置传输长度 */
        }
        else
        {
            DMA2_Stream0->NDTR = lcdsize;             /* 设置传输长度 */
            lcdsize = 0;
        }

        DMA2_Stream0->PAR = (uint32_t)(g_sramlcdbuf + x * spbdev.spbheight + dmatransfered);   /* DMA2,改变外设基地址 */
        dmatransfered += SLCD_DMA_MAX_TRANS;
        DMA2_Stream0->CR |= 1 << 0;                   /* 开启DMA传输 */

        while ((DMA2->LISR & (1 << 5)) == 0);         /* 等待传输完成 */
    }

    DMA2_Stream0->CR &= ~(1 << 0);                    /* 关闭DMA传输 */
}

/**
 * @brief       显示一帧,即启动一次外部sram到lcd的显示.
 * @param       x       : 坐标偏移量
 * @retval      无
 */
void slcd_frame_show(uint32_t x)
{
    if (lcdltdc.pwidth)                  /* RGB屏 */
    {      
        ltdc_color_fill(0, spbdev.stabarheight, spbdev.spbwidth - 1, spbdev.stabarheight + spbdev.spbheight - 1, g_sramlcdbuf + x * spbdev.spbheight);    
    }
    else
    {
        if (STM32_FMC_16)
        {
            lcd_scan_dir(U2D_L2R);       /* 设置扫描方向 */

            lcd_set_window(0, spbdev.stabarheight, spbdev.spbwidth, spbdev.spbheight);  /* 设置窗口 */
            lcd_set_cursor(0, spbdev.stabarheight);                                     /* 设置光标位置 */

            lcd_write_ram_prepare();     /* 开始写入GRAM */
                    
            slcd_dma_enable(x);
            
            lcd_scan_dir(DFT_SCAN_DIR);                         /* 恢复默认方向 */
            lcd_set_window(0, 0, lcddev.width, lcddev.height);  /* 恢复默认窗口大小 */
        }
        else
        {
            LT758_DMA_24bit_Block(1, 0, 0, spbdev.stabarheight, spbdev.spbwidth, spbdev.spbheight, spbdev.spbwidth, 0);        
        }
    }
}






