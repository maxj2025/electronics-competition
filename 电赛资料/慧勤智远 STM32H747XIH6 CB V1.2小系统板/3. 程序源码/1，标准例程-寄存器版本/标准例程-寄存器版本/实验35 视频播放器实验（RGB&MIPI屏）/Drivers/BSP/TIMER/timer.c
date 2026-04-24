/**
 ****************************************************************************************************
 * @file        timer.c
 * @version     V1.0
 * @brief       定时器中断 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/LED/led.h"
#include "./BSP/TIMER/timer.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"


/* 视频播放帧率控制全局变量 */
uint16_t g_avi_frame;                /* 播放帧率 */
volatile uint8_t g_avi_frameup;      /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */


/**
 * @brief       定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void TIMX_INT_IRQHandler(void)
{ 
    if (TIMX_INT->SR & 0X0001)     /* 溢出中断 */
    {
        printf("frame:%dfps\r\n", g_avi_frame);  /* 打印帧率 */
        g_avi_frame = 0;
        LED0_TOGGLE();             /* LED0(绿灯)翻转 */
    }

    TIMX_INT->SR &= ~(1 << 0);     /* 清除更新中断标志 */
} 

/**
 * @brief       定时器TIMX定时中断初始化函数
 * @note
 *              定时器的时钟来自APB1或APB2,如果D2PPREx对应于1或2分频，
 *              定时器的时钟为rcc_hclk1, 而rcc_hclk1为240M, 所以定时器时钟 = 240Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void timx_int_init(uint32_t arr, uint16_t psc)
{
    TIMX_INT_CLK_ENABLE();                  /* 使能TIMX时钟 */
  
    /* 延时一点点时间, STM32H7不能在使能时钟后立即操作寄存器, 需等待时钟正常 */
    delay_us(1);
  
    TIMX_INT->ARR = arr;                    /* 设定计数器自动重载值 */
    TIMX_INT->PSC = psc;                    /* 设置预分频器 */
    TIMX_INT->DIER |= 1 << 0;               /* 允许更新中断 */
    TIMX_INT->CR1 |= 1 << 0;                /* 使能定时器TIMX */
    sys_nvic_init(1, 3, TIMX_INT_IRQn, 2);  /* 抢占优先级1，子优先级3，组2 */
}

/**
 * @brief       基本定时器TIM7中断服务函数
 * @param       无
 * @retval      无
 */
void TIM7_IRQHandler(void)
{ 
    if (TIM7->SR & 0X0001)         /* 溢出中断 */
    {
        g_avi_frameup = 1;         /* 标记时间到 */
    }

    TIM7->SR &= ~(1 << 0);         /* 清除更新中断标志 */
} 

/**
 * @brief       基本定时器TIM7定时中断初始化函数
 * @note
 *              定时器的时钟来自APB1或APB2,如果D2PPREx对应于1或2分频，
 *              定时器的时钟为rcc_hclk1, 而rcc_hclk1为240M, 所以定时器时钟 = 240Mhz
 *              定时器溢出时间计算方法: Tout = ((arr + 1) * (psc + 1)) / Ft us.
 *              Ft=定时器工作频率,单位:Mhz
 *
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void btim_tim7_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 5;            /* 使能TIM7时钟 */
  
    /* 延时一点点时间, STM32H7不能在使能时钟后立即操作寄存器, 需等待时钟正常 */
    delay_us(1);

    TIM7->ARR = arr;                    /* 设定计数器自动重载值 */
    TIM7->PSC = psc;                    /* 设置预分频器  */
    TIM7->DIER |= 1 << 0;               /* 允许更新中断 */
    TIM7->CR1 |= 1 << 0;                /* 使能定时器TIM7 */
    sys_nvic_init(0, 3, TIM7_IRQn, 2);  /* 抢占优先级0，子优先级3，组2 */
}







