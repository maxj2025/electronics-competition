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
#define SCCB_SCL_GPIO_PIN                GPIO_PIN_11
#define SCCB_SCL_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)  /* PD口时钟使能 */

#define SCCB_SDA_GPIO_PORT               GPIOG
#define SCCB_SDA_GPIO_PIN                GPIO_PIN_3
#define SCCB_SDA_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOG_CLK_ENABLE(); }while(0)  /* PG口时钟使能 */

/******************************************************************************************/

/* IO操作函数 */
#define SCCB_SCL(x)         do{ x ? \
                                HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_SET) : \
                                HAL_GPIO_WritePin(SCCB_SCL_GPIO_PORT, SCCB_SCL_GPIO_PIN, GPIO_PIN_RESET); \
                            }while(0)                                                 /* SCL */

#define SCCB_SDA(x)         do{ x ? \
                                HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_SET) : \
                                HAL_GPIO_WritePin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN, GPIO_PIN_RESET); \
                            }while(0)                                                 /* SDA */

                      
#define SCCB_READ_SDA       HAL_GPIO_ReadPin(SCCB_SDA_GPIO_PORT, SCCB_SDA_GPIO_PIN)   /* 读取SDA */
                      
/******************************************************************************************/

/* 对外接口函数 */
void sccb_init(void);                   /* 初始化SCCB接口 */
void sccb_start(void);                  /* 发送SCCB起始信号 */     
void sccb_stop(void);                   /* 发送SCCB停止信号 */

void sccb_nack(void);                   /* 不发送ACK应答 */
uint8_t sccb_send_byte(uint8_t data);   /* SCCB 发送一个字节 */
uint8_t sccb_read_byte(void);           /* SCCB 读取一个字节 */

#endif






