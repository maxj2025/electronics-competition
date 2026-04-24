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


extern uint16_t tpflag;
extern uint16_t tptime;


/**
 * @brief       定时器TIMX中断服务函数
 * @param       无
 * @retval      无
 */
void TIMX_INT_IRQHandler(void)
{ 
    if (TIMX_INT->SR & 0X0001)     /* 溢出中断 */
    {
        if (tpflag == 1)
        {
            if (tptime < 600)
            {
                tptime++;
            }
        }
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






