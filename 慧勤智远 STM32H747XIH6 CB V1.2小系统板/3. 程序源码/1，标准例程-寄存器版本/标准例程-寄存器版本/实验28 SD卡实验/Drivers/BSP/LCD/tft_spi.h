/**
 ****************************************************************************************************
 * @file        tft_spi.h
 * @version     V1.0
 * @brief       TFTLCD(RGB屏) SPI接口 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __TFT_SPI_H
#define __TFT_SPI_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/ 
/* TFT_SPI 相关引脚 定义 */
/* 引脚对应关系: 
 *         TFT_SPI_CS  --> TP_MISO,    TFT_SPI_SCL --> LTDC_VSYNC
 *         TFT_SPI_SDA --> LTDC_HSYNC, TFT_SPI_RST --> LCD_RST 
 */

#define TFT_SPI_CS_GPIO_PORT                GPIOB
#define TFT_SPI_CS_GPIO_PIN                 SYS_GPIO_PIN13
#define TFT_SPI_CS_GPIO_CLK_ENABLE()        do{ RCC->AHB4ENR |= 1 << 1; }while(0)   /* PB口时钟使能 */

#define TFT_SPI_SCL_GPIO_PORT               GPIOI
#define TFT_SPI_SCL_GPIO_PIN                SYS_GPIO_PIN13
#define TFT_SPI_SCL_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* PI口时钟使能 */

#define TFT_SPI_SDA_GPIO_PORT               GPIOI
#define TFT_SPI_SDA_GPIO_PIN                SYS_GPIO_PIN12
#define TFT_SPI_SDA_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 8; }while(0)   /* PI口时钟使能 */

#define TFT_SPI_RST_GPIO_PORT               GPIOH
#define TFT_SPI_RST_GPIO_PIN                SYS_GPIO_PIN5
#define TFT_SPI_RST_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 7; }while(0)   /* PH口时钟使能 */

/******************************************************************************************/

/* TFTLCD SPI接口引脚IO操作函数 定义 */
#define TFT_SPI_CS(x)        sys_gpio_pin_set(TFT_SPI_CS_GPIO_PORT, TFT_SPI_CS_GPIO_PIN, x)     /* 片选引脚CS */   

#define TFT_SPI_SCL(x)       sys_gpio_pin_set(TFT_SPI_SCL_GPIO_PORT, TFT_SPI_SCL_GPIO_PIN, x)   /* 时钟信号引脚SCL */

#define TFT_SPI_SDA(x)       sys_gpio_pin_set(TFT_SPI_SDA_GPIO_PORT, TFT_SPI_SDA_GPIO_PIN, x)   /* 数据输出引脚SDA */
                       
/* LCD复位引脚 */
#define TFT_SPI_RST(x)       sys_gpio_pin_set(TFT_SPI_RST_GPIO_PORT, TFT_SPI_RST_GPIO_PIN, x)   

/******************************************************************************************/
/* 函数声明 */

void tft_spi_init(void);                             /* TFTLCD SPI接口初始化 */
                             
void tft_spi_write_byte(uint8_t buf);                /* SPI写入1字节数据 */
void tft_spi_write_cmd(uint8_t cmd);                 /* 向LCD驱动IC写命令 */
void tft_spi_write_data(uint8_t data);               /* 向LCD驱动IC写数据 */
void tft_spi_write_reg(uint8_t reg, uint8_t data);   /* 向LCD驱动IC写寄存器的值 */
                             
                             
#endif





