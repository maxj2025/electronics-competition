/**
 ****************************************************************************************************
 * @file        wwdg.c
 * @version     V1.0
 * @brief       窗口看门狗 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/WWDG/wwdg.h"
#include "./BSP/LED/led.h"


/* 保存WWDG计数器的设置值,默认为最大. */
uint8_t g_wwdg_cnt = 0x7f;

/**
 * @brief       初始化窗口看门狗
 * @param       tr    : T[6:0],计数器值
 * @param       wr    : W[6:0],窗口值
 * @param       fprer :分频系数（WDGTB）,范围:0~7,表示2^WDGTB分频
 * @note        Fwwdg = PCLK3 / (4096 * 2^fprer). 一般PCLK3 = 120Mhz
 *              溢出时间 = (4096 * 2^fprer) * (tr - 0X3F) / PCLK3
 *              假设fprer = 4, tr = 7f, PCLK3 = 120Mhz
 *              则溢出时间 = 4096 * 16 * 64 / 120Mhz = 34.96ms
 * @retval      无
 */
void wwdg_init(uint8_t tr, uint8_t wr, uint8_t fprer)
{
    RCC->APB3ENR |= 1 << 6;            /* 使能wwdg时钟 */
    g_wwdg_cnt = tr & g_wwdg_cnt;      /* 初始化WWDG_CNT设置值 */
    RCC->GCR |= 1 << 0;                /* WW1RSC=1,复位WWDG1,硬件清零 */
    WWDG1->CFR |= fprer << 11;         /* 设置分频系数，窗口看门狗定时器时基为PCLK3 / (4096 * 2^fprer) */
    WWDG1->CFR &= 0XFF80;
    WWDG1->CFR |= wr;                  /* 设置窗口值 */
    WWDG1->CR |= g_wwdg_cnt;           /* 设置计数器值 */
    WWDG1->CR |= 1 << 7;               /* 开启看门狗 */
    sys_nvic_init(2, 3, WWDG_IRQn, 2); /* 抢占优先级2，子优先级3，组2 */
    WWDG1->SR = 0X00;                  /* 清除提前唤醒中断标志位 */
    WWDG1->CFR |= 1 << 9;              /* 使能窗口看门狗提前唤醒中断 */
}

/**
 * @brief       重设置WWDG计数器的值
 * @param       无
 * @retval      无
 */
void wwdg_set_counter(uint8_t cnt)
{
    WWDG1->CR = (cnt & 0x7F);          /* 重设置7位计数器 */
}

/**
 * @brief       窗口看门狗中断服务程序
 * @param       无
 * @retval      无
 */
void WWDG_IRQHandler(void)
{  
    if (WWDG1->SR & 0X01)              /* 先判断是否发生了WWDG提前唤醒中断 */
    {
        WWDG1->SR = 0X00;              /* 清除提前唤醒中断标志位 */
        wwdg_set_counter(g_wwdg_cnt);  /* 重设窗口看门狗计数器的值! */
        LED1_TOGGLE();                 /* LED1(蓝灯)闪烁 */
    }
}








