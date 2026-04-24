/**
 ****************************************************************************************************
 * @file        fdcan.h
 * @version     V1.0
 * @brief       FDCAN 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#ifndef __CAN_H
#define __CAN_H

#include "./SYSTEM/sys/sys.h"


/******************************************************************************************/
/* FDCAN 引脚 定义 */

#define FDCAN_RX_GPIO_PORT                GPIOB
#define FDCAN_RX_GPIO_PIN                 GPIO_PIN_8
#define FDCAN_RX_GPIO_AF                  GPIO_AF9_FDCAN1
#define FDCAN_RX_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)              /* PB口时钟使能 */

#define FDCAN_TX_GPIO_PORT                GPIOB
#define FDCAN_TX_GPIO_PIN                 GPIO_PIN_9
#define FDCAN_TX_GPIO_AF                  GPIO_AF9_FDCAN1
#define FDCAN_TX_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)              /* PB口时钟使能 */

/* FDCAN1接收RX0中断使能 */
#define FDCAN1_RX0_INT_ENABLE   0         /* 0,不使能;1,使能. */

/******************************************************************************************/

uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntseg1, uint8_t ntseg2, uint32_t mode);  /* FDCAN初始化 */
uint8_t fdcan_send_msg(uint8_t *msg, uint32_t len);                                                /* FDCAN发送数据 */
uint8_t fdcan_receive_msg(uint8_t *buf);                                                           /* FDCAN接收数据 */

#endif





