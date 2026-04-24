/**
 ****************************************************************************************************
 * @file        capture.c
 * @version     V1.0
 * @brief       输入捕获 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/LED/led.h"
#include "./BSP/CAPTURE/capture.h"


/**
 * @brief       定时器TIMX 通道Y PWM输出 初始化函数（使用PWM模式1）
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
void timx_pwm_chy_init(uint32_t arr, uint16_t psc)
{
    uint8_t chy = TIMX_PWM_CHY;
    TIMX_PWM_CHY_GPIO_CLK_ENABLE();    /* TIMX 通道Y IO口时钟使能 */
    TIMX_PWM_CHY_CLK_ENABLE();         /* TIMX 时钟使能 */

    sys_gpio_set(TIMX_PWM_CHY_GPIO_PORT, TIMX_PWM_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* TIMX PWM CHY 引脚模式设置 */

    sys_gpio_af_set(TIMX_PWM_CHY_GPIO_PORT, TIMX_PWM_CHY_GPIO_PIN, TIMX_PWM_CHY_GPIO_AF);      /* IO口复用功能选择 必须设置对!! */

    TIMX_PWM->ARR = arr;       /* 设置计数器自动重载值 */
    TIMX_PWM->PSC = psc;       /* 设置预分频器  */
    TIMX_PWM->BDTR |= 1 << 15; /* 使能MOE位(仅TIM1/8/15/16/17 有此寄存器,必须设置MOE才能输出PWM), 其他定时器,
                                * 这个寄存器是无效的, 所以设置/不设置并不影响结果, 为了兼容这里统一设置MOE位
                                */

    if (chy <= 2)
    {
        TIMX_PWM->CCMR1 |= 6 << (4 + 8 * (chy - 1));   /* CH1/2 PWM模式1 */
        TIMX_PWM->CCMR1 |= 1 << (3 + 8 * (chy - 1));   /* CH1/2 预装载使能 */
    }
    else if (chy <= 4)
    {
        TIMX_PWM->CCMR2 |= 6 << (4 + 8 * (chy - 3));   /* CH3/4 PWM模式1 */
        TIMX_PWM->CCMR2 |= 1 << (3 + 8 * (chy - 3));   /* CH3/4 预装载使能 */
    }

    TIMX_PWM->CCER |= 1 << (4 * (chy - 1));            /* OCy 输出使能 */
    TIMX_PWM->CCER |= 1 << (1 + 4 * (chy - 1));        /* OCy 低电平有效 */
    TIMX_PWM->CR1 |= 1 << 7;   /* ARPE使能 */
    TIMX_PWM->CR1 |= 1 << 0;   /* 使能定时器TIMX */
}

/*********************************以下是定时器输入捕获实验程序*************************************/

/* 输入捕获状态(g_timxchy_cap_sta)
 * [7]  :0,没有成功的捕获;1,成功捕获到一次.
 * [6]  :0,还没捕获到高电平;1,已经捕获到高电平了.
 * [5:0]:捕获高电平后溢出的次数,最多溢出63次,所以最长捕获值 = 63 * 65536 + 65535 = 4194303
 *       注意:为了通用,我们默认ARR和CCRy都是16位寄存器,对于32位的定时器(如:TIM5),也只按16位使用
 *       按1us的计数频率,最长溢出时间为:4194303 us, 约4.19秒
 *
 *      (说明一下：正常32位定时器来说,1us计数器加1,溢出时间:4294秒)
 */
uint8_t g_timxchy_cap_sta = 0;   /* 输入捕获状态 */
uint32_t g_timxchy_cap_val = 0;  /* 输入捕获值 */

/**
 * @brief       定时器TIMX 输入捕获 中断服务函数
 * @param       无
 * @retval      无
 */
void TIMX_CAP_IRQHandler(void)
{
    uint16_t tsr = TIMX_CAP->SR;           /* 获取中断状态 */
    uint8_t chy = TIMX_CAP_CHY;            /* 需要捕获的通道 */

    if ((g_timxchy_cap_sta & 0X80) == 0)   /* 还未成功捕获 */
    {
        if (tsr & (1 << 0))                /* 溢出中断 */
        {
            if (g_timxchy_cap_sta & 0X40)  /* 已经捕获到高电平了 */
            {
                if ((g_timxchy_cap_sta & 0X3F) == 0X3F) /* 溢出多次，高电平太长了 */
                {
                    TIMX_CAP->CCER &= ~(1 << (1 + 4 * (chy - 1))); /* CCyP = 0 设置为上升沿捕获 */
                    g_timxchy_cap_sta |= 0X80;  /* 强制标记捕获了一次 */
                    g_timxchy_cap_val = 0XFFFF; /* 设置捕获值为最大 */
                }
                else                            /* 还可以累加高电平长度 */
                {
                    g_timxchy_cap_sta++;        /* 累计定时器溢出次数 */
                }
            }
        }

        if (tsr & (1 << chy))   /* 通道y 发生了捕获事件 */
        {
            if (g_timxchy_cap_sta & 0X40)   /* 捕获到一个下降沿 */
            {
                g_timxchy_cap_sta |= 0X80;  /* 标记成功捕获到一次高电平脉宽 */
                g_timxchy_cap_val = TIMX_CAP_CHY_CCRX; /* 获取当前的捕获值 */
                TIMX_CAP->CCER &= ~(1 << (1 + 4 * (chy - 1)));/* CCyP = 0 设置为上升沿捕获，为下次捕获做准备 */
            }
            else                            /* 还未开始,第一次捕获上升沿 */
            {
                g_timxchy_cap_val = 0;      /* 捕获值清零 */
                g_timxchy_cap_sta = 0X40;   /* 标记捕获到了上升沿 */
                TIMX_CAP->CNT = 0;          /* 计数器清空 */ 
                TIMX_CAP->CCER |= 1 << (1 + 4 * (chy - 1));   /* CCyP = 1 设置为下降沿捕获 */ 
            }
        }
    }

    TIMX_CAP->SR = 0;     /* 清除所有中断标志位 */
}

/**
 * @brief       定时器TIMX 通道Y 输入捕获 初始化函数
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
void timx_cap_chy_init(uint32_t arr, uint16_t psc)
{
    uint8_t chy = TIMX_CAP_CHY;
    TIMX_CAP_CHY_GPIO_CLK_ENABLE();   /* TIMX 通道Y IO口时钟使能 */
    TIMX_CAP_CHY_CLK_ENABLE();        /* TIMX 时钟使能 */

    sys_gpio_set(TIMX_CAP_CHY_GPIO_PORT, TIMX_CAP_CHY_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PD);   /* TIMX CAP CHY 引脚复用功能模式 下拉 */

    sys_gpio_af_set(TIMX_CAP_CHY_GPIO_PORT, TIMX_CAP_CHY_GPIO_PIN, TIMX_CAP_CHY_GPIO_AF);      /* IO口复用功能选择 必须设置对!! */

    TIMX_CAP->ARR = arr;       /* 设置计数器自动重载值 */
    TIMX_CAP->PSC = psc;       /* 设置预分频器值 */

    if (chy <= 2)
    {
        TIMX_CAP->CCMR1 |= 1 << 8 * (chy - 1);        /* CCyS[1:0]   = 01 选择输入端 IC1/2映射到TI1/2上 */
        TIMX_CAP->CCMR1 |= 0 << (2 + 8 * (chy - 1));  /* ICyPSC[1:0] = 00 输入捕获不分频,全捕获 */
        TIMX_CAP->CCMR1 |= 0 << (4 + 8 * (chy - 1));  /* ICyF[3:0]   = 00 输入端滤波 不滤波 */
    }
    else if (chy <= 4)
    {
        TIMX_CAP->CCMR2 |= 1 << 8 * (chy - 3);        /* CCyS[1:0]   = 01 选择输入端 IC3/4映射到TI3/4上 */
        TIMX_CAP->CCMR2 |= 0 << (2 + 8 * (chy - 3));  /* ICyPSC[1:0] = 00 输入捕获不分频,全捕获 */
        TIMX_CAP->CCMR2 |= 0 << (4 + 8 * (chy - 3));  /* ICyF[3:0]   = 00 输入端滤波 不滤波 */
    }

    TIMX_CAP->CCER |= 1 << (4 * (chy - 1));       /* CCyE = 1 输入捕获使能 */
    TIMX_CAP->CCER |= 0 << (1 + 4 * (chy - 1));   /* CCyP = 0 捕获上升沿,注意:CCyNP使用默认值0 */

    TIMX_CAP->EGR  |= 1 << 0;  /* 软件控制产生更新事件,使写入PSC的值立即生效,否则将会要等到定时器溢出才会生效 */
    TIMX_CAP->DIER |= 1 << 1;  /* 使能捕获1中断 */
    TIMX_CAP->DIER |= 1 << 0;  /* 使能更新中断 */
    TIMX_CAP->CR1  |= 1 << 0;  /* 使能定时器TIMX */

    sys_nvic_init(1, 3, TIMX_CAP_IRQn, 2);/* 抢占优先级1，子优先级3，组2 */
}









