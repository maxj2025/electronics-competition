/**
 ****************************************************************************************************
 * @file        Cortex-M4的main.c
 * @version     V1.0
 * @brief       HSEM内核通知 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"


#define HSEM_ID_0  0                        /* 使用信号量0 */
volatile uint32_t notif_recieved;           /* 接收到信号量通知标志 */


int main(void)
{
    __HAL_RCC_HSEM_CLK_ENABLE();            /* HSEM时钟使能 */

    /* Cortex-M4激活信号量释放通知(使能信号量中断) */
    HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

    /* 让D2域进入停止模式(Cortex-M4处于深度睡眠模式)，等待Cortex-M7完成系统初始化 */
    HAL_PWREx_ClearPendingEvent();
    HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE, PWR_D2_DOMAIN);
  
    HAL_Init();                             /* 初始化HAL库 */
    delay_init(240);                        /* Cortex-M4的延时初始化 */
  
    HAL_NVIC_SetPriority(HSEM2_IRQn, 3, 0); /* 抢占优先级3，子优先级0 */
    HAL_NVIC_EnableIRQ(HSEM2_IRQn);         /* 使能HSEM2中断 */
  
    led_init();                             /* 初始化LED */
    usart_init(115200);                     /* 初始化USART */    

    while (1)
    {
        notif_recieved = 0;
        
        /* 激活信号量释放通知(使能信号量中断) */
        HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
        
        /* 等待来自CPU1的信号量通知 */
        while (notif_recieved == 0);
        
        printf("CortexM4\r\n");
        LED1_TOGGLE();                      /* LED1(蓝灯)闪烁 */
    }
}

/**
 * @brief       HSEM2中断服务函数
 * @param       无
 * @retval      无
 */
void HSEM2_IRQHandler(void)
{
    HAL_HSEM_IRQHandler();	  
}

/**
 * @brief       信号量释放回调函数
 * @param       SemMask: 释放信号量的屏蔽状态
 * @retval      无
 */
void HAL_HSEM_FreeCallback(uint32_t SemMask)
{
    notif_recieved = 1;
} 




