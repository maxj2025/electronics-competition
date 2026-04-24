/**
 ****************************************************************************************************
 * @file        sccb.h
 * @version     V1.0
 * @brief       SCCB 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __SCCB_H
#define __SCCB_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 引脚 定义 */

#define SCCB_SCL_GPIO_PORT               GPIOD
#define SCCB_SCL_GPIO_PIN                SYS_GPIO_PIN11
#define SCCB_SCL_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 3; }while(0)   /* PD口时钟使能 */

#define SCCB_SDA_GPIO_PORT               GPIOG
#define SCCB_SDA_GPIO_PIN                SYS_GPIO_PIN3
#define SCCB_SDA_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 6; }while(0)   /* PG口时钟使能 */

/******************************************************************************************/

/* IO操作函数 */
#define SCCB_SCL(x)         sys_gpio_pin_set(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, x)  /* SCL */
#define SCCB_SDA(x)         sys_gpio_pin_set(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, x)  /* SDA */
#define SCCB_READ_SDA       sys_gpio_pin_get(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN)     /* 读取SDA */

/******************************************************************************************/

/* 对外接口函数 */
void sccb_init(void);                   /* 初始化SCCB接口 */
void sccb_start(void);                  /* 发送SCCB起始信号 */
void sccb_stop(void);                   /* 发送SCCB停止信号 */

void sccb_nack(void);                   /* 不发送ACK应答 */
uint8_t sccb_send_byte(uint8_t data);   /* SCCB 发送一个字节 */
uint8_t sccb_read_byte(void);           /* SCCB 读取一个字节 */

#endif





