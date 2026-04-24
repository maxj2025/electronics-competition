/**
 ****************************************************************************************************
 * @file        myiic.h
 * @version     V1.0
 * @brief       IIC 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __MYIIC_H
#define __MYIIC_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* 引脚 定义 */

#define IIC_SCL_GPIO_PORT               GPIOD
#define IIC_SCL_GPIO_PIN                SYS_GPIO_PIN12
#define IIC_SCL_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 3; }while(0)   /* PD口时钟使能 */

#define IIC_SDA_GPIO_PORT               GPIOD
#define IIC_SDA_GPIO_PIN                SYS_GPIO_PIN13
#define IIC_SDA_GPIO_CLK_ENABLE()       do{ RCC->AHB4ENR |= 1 << 3; }while(0)   /* PD口时钟使能 */

/******************************************************************************************/

/* IO操作函数 */
#define IIC_SCL(x)      sys_gpio_pin_set(IIC_SCL_GPIO_PORT, IIC_SCL_GPIO_PIN, x)   /* SCL */
#define IIC_SDA(x)      sys_gpio_pin_set(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN, x)   /* SDA */
#define IIC_READ_SDA    sys_gpio_pin_get(IIC_SDA_GPIO_PORT, IIC_SDA_GPIO_PIN)      /* 读取SDA */

/******************************************************************************************/

/* IIC所有操作函数 */
void iic_init(void);                /* 初始化IIC的IO口 */
void iic_start(void);               /* 发送IIC开始信号 */
void iic_stop(void);                /* 发送IIC停止信号 */
void iic_ack(void);                 /* IIC发送ACK信号 */
void iic_nack(void);                /* IIC不发送ACK信号 */
uint8_t iic_wait_ack(void);         /* IIC等待ACK信号 */
void iic_send_byte(uint8_t data);   /* IIC发送一个字节 */
uint8_t iic_read_byte(uint8_t ack); /* IIC读取一个字节 */

#endif






