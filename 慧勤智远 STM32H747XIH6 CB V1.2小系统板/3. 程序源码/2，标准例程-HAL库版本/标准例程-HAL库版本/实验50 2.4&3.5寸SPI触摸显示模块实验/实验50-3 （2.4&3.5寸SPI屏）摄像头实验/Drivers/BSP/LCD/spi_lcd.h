/**
 ****************************************************************************************************
 * @file        spi_lcd.h
 * @version     V1.0
 * @brief       SPI串口屏 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __SPI_LCD_H
#define __SPI_LCD_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/ 
/* SPI_LCD 相关 引脚 定义 */

#define SPI_LCD_DC_GPIO_PORT                GPIOC
#define SPI_LCD_DC_GPIO_PIN                 GPIO_PIN_3
#define SPI_LCD_DC_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOC_CLK_ENABLE(); }while(0)   /* PC口时钟使能 */

#define SPI_LCD_BL_GPIO_PORT                GPIOB
#define SPI_LCD_BL_GPIO_PIN                 GPIO_PIN_0
#define SPI_LCD_BL_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* PB口时钟使能 */

#define SPI_LCD_RST_GPIO_PORT               GPIOH
#define SPI_LCD_RST_GPIO_PIN                GPIO_PIN_5
#define SPI_LCD_RST_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOH_CLK_ENABLE(); }while(0)   /* PH口时钟使能 */

/* SPI4 引脚 定义 */
#define SPI4_CS_GPIO_PORT                   GPIOE
#define SPI4_CS_GPIO_PIN                    GPIO_PIN_4
#define SPI4_CS_GPIO_CLK_ENABLE()           do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_SCK_GPIO_PORT                  GPIOE
#define SPI4_SCK_GPIO_PIN                   GPIO_PIN_2
#define SPI4_SCK_GPIO_CLK_ENABLE()          do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_MISO_GPIO_PORT                 GPIOE
#define SPI4_MISO_GPIO_PIN                  GPIO_PIN_5
#define SPI4_MISO_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_MOSI_GPIO_PORT                 GPIOE
#define SPI4_MOSI_GPIO_PIN                  GPIO_PIN_6
#define SPI4_MOSI_GPIO_CLK_ENABLE()         do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

/* SPI4相关定义 */
#define SPI4_SPI                            SPI4
#define SPI4_SPI_CLK_ENABLE()               do{ __HAL_RCC_SPI4_CLK_ENABLE(); }while(0)    /* SPI4时钟使能 */

extern SPI_HandleTypeDef g_spi4_handle;          /* SPI句柄 */
extern DMA_HandleTypeDef g_dma_spi_handle;       /* DMA句柄 */

/******************************************************************************************/

/* 片选引脚 */
#define SPI_LCD_CS(x)        do{ x ? \
                                 HAL_GPIO_WritePin(SPI4_CS_GPIO_PORT, SPI4_CS_GPIO_PIN, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(SPI4_CS_GPIO_PORT, SPI4_CS_GPIO_PIN, GPIO_PIN_RESET); \
                             }while(0)

/* 时钟信号引脚 */
#define SPI_LCD_SCL(x)       do{ x ? \
                                 HAL_GPIO_WritePin(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN, GPIO_PIN_RESET); \
                             }while(0)

/* 命令/数据引脚 */
#define SPI_LCD_DC(x)        do{ x ? \
                                 HAL_GPIO_WritePin(SPI_LCD_DC_GPIO_PORT, SPI_LCD_DC_GPIO_PIN, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(SPI_LCD_DC_GPIO_PORT, SPI_LCD_DC_GPIO_PIN, GPIO_PIN_RESET); \
                             }while(0)

/* LCD背光控制 */
#define SPI_LCD_BL(x)        do{ x ? \
                                 HAL_GPIO_WritePin(SPI_LCD_BL_GPIO_PORT, SPI_LCD_BL_GPIO_PIN, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(SPI_LCD_BL_GPIO_PORT, SPI_LCD_BL_GPIO_PIN, GPIO_PIN_RESET); \
                             }while(0)

/* LCD复位引脚 */
#define SPI_LCD_RST(x)       do{ x ? \
                                 HAL_GPIO_WritePin(SPI_LCD_RST_GPIO_PORT, SPI_LCD_RST_GPIO_PIN, GPIO_PIN_SET) : \
                                 HAL_GPIO_WritePin(SPI_LCD_RST_GPIO_PORT, SPI_LCD_RST_GPIO_PIN, GPIO_PIN_RESET); \
                             }while(0)

                             
/* SPI总线速度设置 */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7

#define SPI_DMA_MAX_TRANS   60 * 1024     /* DMA一次最多传输60K字节 */
                             
/******************************************************************************************/
/* 函数声明 */

void spi_lcd_init(void);                             /* SPI LCD接口初始化 */                             
void SPI_CmdWrite(uint8_t cmd);                      /* 向LCD驱动IC写命令 */
void SPI_DataWrite(uint8_t data);                    /* 向LCD驱动IC写数据 */
void SPI_DataWrite_Pixel(uint16_t data);             /* 写一个像素点的数据 */
void dummy_clock(void);                              /* 增加一个空时钟周期 */
uint16_t lcd_read_ram(uint8_t regno);                /* 读GRAM数据 */
uint16_t lcd_read_id(uint8_t regno);                 /* 读LCD驱动IC的ID */
                         
void spi_io_init(void);                              /* SPI初始化 */
void spi4_set_speed(uint8_t speed);                  /* 设置SPI4速度 */
uint8_t spi4_read_write_byte(uint8_t txdata);        /* SPI4读写一个字节 */
void lcd_spi4_mode(uint8_t mode);                    /* SPI4模式设置 */
void spi_dma_init(uint32_t meminc);                  /* SPI DMA配置 */
void spi_dma_lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color);          /* SPI DMA传输方式纯色填充矩形 */
void spi_dma_lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color);   /* SPI DMA传输方式彩色填充矩形 */


#endif





