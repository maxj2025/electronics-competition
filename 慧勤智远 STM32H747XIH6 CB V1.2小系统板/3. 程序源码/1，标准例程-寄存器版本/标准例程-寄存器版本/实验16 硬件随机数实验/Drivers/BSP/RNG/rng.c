/**
 ****************************************************************************************************
 * @file        rng.c
 * @version     V1.0
 * @brief       RNG(随机数发生器) 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/RNG/rng.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       初始化RNG
 * @param       无
 * @retval      0,成功;1,失败
 */
uint8_t rng_init(void)
{
    uint16_t retry = 0;
    RCC->AHB2ENR = 1 << 6;       /* 使能RNG时钟 */
    RCC->D2CCIP2R &= ~(3 << 8);  /* RNGSEL[1:0]清零 */
    RCC->D2CCIP2R |= 1 << 8;     /* RNGSEL[1:0]=1,选择pll1_q_ck作为RNG时钟源(240Mhz) */
    RNG->CR |= 1 << 2;           /* 使能RNG */

    while ((RNG->SR & 0X01) == 0 && retry < 10000)  /* 等待随机数准备就绪 */
    {
        retry++;
        delay_us(100);
    }

    if (retry >= 10000)
    {
        return 1;                /* 随机数产生器工作不正常 */
    }

    return 0;
}

/**
 * @brief       得到随机数
 * @param       无
 * @retval      获取到的随机数(32bit)
 */
uint32_t rng_get_random_num(void)
{
    while ((RNG->SR & 0X01) == 0);  /* 等待随机数就绪 */

    return RNG->DR;
}

/**
 * @brief       得到某个范围内的随机数
 * @param       min,max: 区间最小,最大值.
 * @retval      得到的随机数(rval),满足:min<=rval<=max
 */
int rng_get_random_range(int min, int max)
{
    return rng_get_random_num() % (max - min + 1) + min;
}









