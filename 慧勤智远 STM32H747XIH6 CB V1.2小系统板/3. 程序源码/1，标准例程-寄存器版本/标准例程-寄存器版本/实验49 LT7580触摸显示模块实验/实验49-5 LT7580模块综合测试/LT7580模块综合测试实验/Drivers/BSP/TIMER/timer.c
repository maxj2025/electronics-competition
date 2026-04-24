/**
 ****************************************************************************************************
 * @file        timer.c
 * @version     V1.0
 * @brief       定时器 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/TIMER/timer.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "ucos_ii.h"
#include "os.h"


volatile uint8_t g_framecnt;    /* 帧计数器 */
volatile uint8_t g_framecntout; /* 统一的帧计数器输出变量 */

extern void usbapp_pulling(void);


/**
 * @brief       定时器TIM8中断服务程序
 * @param       无
 * @retval      无
 */
void TIM8_UP_TIM13_IRQHandler(void)
{
    OSIntEnter();

    if (TIM8->SR & 0X0001)         /* 是更新中断 */
    {        
        if (OSRunning != OS_TRUE)  /* OS还没运行,借TIM8的中断,10ms一次,来扫描USB */
        {
            usbapp_pulling();
        }
        else
        {
            g_framecntout = g_framecnt;
            printf("frame:%d\r\n", g_framecnt); /* 打印帧率 */
            g_framecnt = 0;
        }
    }

    TIM8->SR &= ~(1 << 0);         /* 清除更新中断标志 */
    
    OSIntExit();
}

/**
 * @brief       定时器TIM8定时中断初始化函数
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim8_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB2ENR |= 1 << 1;     /* TIM8时钟使能 */
  
    while ((RCC->APB2ENR & (1 << 1)) == 0); /* 等待时钟使能成功（仅STM32H7XX需要） */
  
    TIM8->ARR = arr;            /* 设定计数器自动重载值 */
    TIM8->PSC = psc;            /* 设置预分频器 */
    TIM8->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM8->CR1 |= 1 << 0;        /* 使能定时器TIM8 */
    
    sys_nvic_init(1, 3, TIM8_UP_TIM13_IRQn, 2);  /* 抢占优先级1，子优先级3，组2 */
}

/* 视频播放帧率控制全局变量 */
extern volatile uint8_t g_avi_frameup;  /* 视频播放时隙控制变量,当等于1的时候,可以更新下一帧视频 */

/**
 * @brief       定时器TIM6中断服务程序
 * @param       无
 * @retval      无
 */
void TIM6_DAC_IRQHandler(void)
{
    OSIntEnter();

    if (TIM6->SR & 0X01)    /* 是更新中断 */
    {
        g_avi_frameup = 1;  /* 标记时间到 */
    }

    TIM6->SR &= ~(1 << 0);  /* 清除更新中断标志 */
    
    OSIntExit();
}

/**
 * @brief       基本定时器TIM6定时中断初始化
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim6_int_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 4;     /* TIM6时钟使能 */
    
    while ((RCC->APB1LENR & (1 << 4)) == 0); /* 等待时钟使能成功（仅STM32H7XX需要） */
    
    TIM6->ARR = arr;            /* 设定计数器自动重载值 */
    TIM6->PSC = psc;            /* 设置预分频器 */
    TIM6->DIER |= 1 << 0;       /* 允许更新中断 */
    TIM6->CR1 |= 1 << 0;        /* 使能定时器TIM6 */
    
    sys_nvic_init(3, 0, TIM6_DAC_IRQn, 2);  /* 抢占优先级3，子优先级0，组2 */
}

/**
 * @brief       定时器TIM3 CH3 PWM输出 初始化函数（使用PWM模式1）
 * @note        PWM输出初始化，用于LCD屏背光亮度控制
 * @param       arr: 自动重装值
 * @param       psc: 时钟预分频数
 * @retval      无
 */
void tim3_ch3_pwm_init(uint16_t arr, uint16_t psc)
{
    RCC->APB1LENR |= 1 << 1;    /* TIM3时钟使能 */
    RCC->AHB4ENR |= 1 << 1;     /* 使能PORTB时钟 */

    sys_gpio_set(GPIOB, SYS_GPIO_PIN0,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* PB0 引脚模式设置 */

    sys_gpio_af_set(GPIOB, SYS_GPIO_PIN0, 2);  /* PB0,AF2 */

    while ((RCC->APB1LENR & (1 << 1)) == 0);   /* 等待时钟使能成功（仅STM32H7XX需要） */

    TIM3->ARR = arr;        /* 设定计数器自动重装值 */
    TIM3->PSC = psc;        /* 设置预分频器  */
 
    TIM3->CCMR2 |= 6 << 4;  /* CH3 PWM1模式 */
    TIM3->CCMR2 |= 1 << 3;  /* CH3 预装载使能 */
    
    TIM3->CCER |= 1 << 8;   /* OC3 输出使能 */
    TIM3->CCER |= 0 << 9;   /* OC3 高电平有效 */
    TIM3->CR1 |= 1 << 7;    /* ARPE使能 */
    TIM3->CR1 |= 1 << 0;    /* 使能定时器TIM3 */
}







