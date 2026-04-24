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

#include "./BSP/CAPTURE/capture.h"
#include "./BSP/LED/led.h"


TIM_HandleTypeDef g_timx_pwm_chy_handle;     /* 定时器TIMX句柄 */

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
    TIM_OC_InitTypeDef timx_oc_pwm_chy = {0};                                            /* 定时器输出句柄 */
    
    g_timx_pwm_chy_handle.Instance = TIMX_PWM;                                           /* 定时器TIMX */
    g_timx_pwm_chy_handle.Init.Prescaler = psc;                                          /* 设置预分频系数 */
    g_timx_pwm_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;                         /* 递增计数模式 */
    g_timx_pwm_chy_handle.Init.Period = arr;                                             /* 设置自动重装载值 */
    HAL_TIM_PWM_Init(&g_timx_pwm_chy_handle);                                            /* 初始化PWM */

    timx_oc_pwm_chy.OCMode = TIM_OCMODE_PWM1;                                            /* 输出比较模式选择PWM模式1 */
    timx_oc_pwm_chy.Pulse = arr / 2;                                                     /* 设置比较值,此值用来确定占空比 
                                                                                          * 这里默认设置比较值为自动重装载值的一半,即占空比为50% 
                                                                                          */
    timx_oc_pwm_chy.OCPolarity = TIM_OCPOLARITY_LOW;                                     /* 设置捕获比较输出极性为低电平有效 */
    HAL_TIM_PWM_ConfigChannel(&g_timx_pwm_chy_handle, &timx_oc_pwm_chy, TIMX_PWM_CHY);   /* 配置TIMX通道Y */
    HAL_TIM_PWM_Start(&g_timx_pwm_chy_handle, TIMX_PWM_CHY);                             /* 开启TIMX通道Y PWM输出 */
}

/**
 * @brief       定时器底层驱动，时钟使能，引脚配置
 * @note        此函数会被HAL_TIM_PWM_Init()函数调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIMX_PWM)                                 /* PWM输出定时器 */
    {
        GPIO_InitTypeDef gpio_init_struct;
      
        TIMX_PWM_CHY_GPIO_CLK_ENABLE();                             /* TIMX 通道Y IO口时钟使能 */
        TIMX_PWM_CHY_CLK_ENABLE();                                  /* TIMX 时钟使能 */

        gpio_init_struct.Pin = TIMX_PWM_CHY_GPIO_PIN;               /* 通道Y的GPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                        /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 快速 */
        gpio_init_struct.Alternate = TIMX_PWM_CHY_GPIO_AF;          /* 定时器TIMX通道Y的GPIO口复用功能选择 */
        HAL_GPIO_Init(TIMX_PWM_CHY_GPIO_PORT, &gpio_init_struct);   /* 初始化通道Y引脚 */
    }
}

/*********************************以下是定时器输入捕获实验程序*************************************/

TIM_HandleTypeDef g_timx_cap_chy_handle;   /* 定时器TIMX句柄 */

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
    TIM_IC_InitTypeDef timx_ic_cap_chy = {0};
    
    g_timx_cap_chy_handle.Instance = TIMX_CAP;                                          /* 定时器TIMX */
    g_timx_cap_chy_handle.Init.Prescaler = psc;                                         /* 设置预分频系数 */
    g_timx_cap_chy_handle.Init.CounterMode = TIM_COUNTERMODE_UP;                        /* 递增计数模式 */
    g_timx_cap_chy_handle.Init.Period = arr;                                            /* 设置自动重装载值 */
    HAL_TIM_IC_Init(&g_timx_cap_chy_handle);                                            /* 初始化定时器 */
    
    timx_ic_cap_chy.ICPolarity = TIM_ICPOLARITY_RISING;                                 /* 上升沿捕获 */
    timx_ic_cap_chy.ICSelection = TIM_ICSELECTION_DIRECTTI;                             /* ICx映射到TIx上 */
    timx_ic_cap_chy.ICPrescaler = TIM_ICPSC_DIV1;                                       /* 配置输入捕获不分频，全捕获 */
    timx_ic_cap_chy.ICFilter = 0;                                                       /* 配置输入捕获滤波器，不滤波 */
    HAL_TIM_IC_ConfigChannel(&g_timx_cap_chy_handle, &timx_ic_cap_chy, TIMX_CAP_CHY);   /* 配置TIMX通道Y */

    __HAL_TIM_ENABLE_IT(&g_timx_cap_chy_handle, TIM_IT_UPDATE);                         /* 使能更新中断 */
    HAL_TIM_IC_Start_IT(&g_timx_cap_chy_handle, TIMX_CAP_CHY);                          /* 使能捕获中断，开始捕获TIMX的通道Y */
}

/**
 * @brief       初始化定时器输入捕获接口
 * @note        HAL库调用的接口，用于配置不同的输入捕获
 *              此函数会被HAL_TIM_IC_Init()调用
 * @param       htim:定时器句柄
 * @retval      无
 */
void HAL_TIM_IC_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIMX_CAP)                                 /* 输入捕获定时器 */
    {
        GPIO_InitTypeDef gpio_init_struct;
      
        TIMX_CAP_CHY_CLK_ENABLE();                                  /* TIMX 时钟使能 */
        TIMX_CAP_CHY_GPIO_CLK_ENABLE();                             /* TIMX 通道Y IO口时钟使能 */

        gpio_init_struct.Pin = TIMX_CAP_CHY_GPIO_PIN;               /* 输入捕获的GPIO口 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;                    /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLDOWN;                      /* 下拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;              /* 快速 */
        gpio_init_struct.Alternate = TIMX_CAP_CHY_GPIO_AF;          /* 输入捕获的GPIO口复用功能选择 */
        HAL_GPIO_Init(TIMX_CAP_CHY_GPIO_PORT, &gpio_init_struct);   /* 初始化输入捕获IO引脚 */

        HAL_NVIC_SetPriority(TIMX_CAP_IRQn, 1, 3);                  /* 抢占优先级1，子优先级3 */
        HAL_NVIC_EnableIRQ(TIMX_CAP_IRQn);                          /* 使能TIMX中断 */
    }
}

/**
 * 输入捕获状态(g_timxchy_cap_sta)
 * [7]  :0,没有成功的捕获;1,成功捕获到一次.
 * [6]  :0,还没捕获到高电平;1,已经捕获到高电平了.
 * [5:0]:捕获高电平后溢出的次数,最多溢出63次,所以最长捕获值 = 63 * 65536 + 65535 = 4194303
 *       注意:为了通用,我们默认ARR和CCRy都是16位寄存器,对于32位的定时器(如:TIM5),也只按16位使用
 *       按1us的计数频率,最长溢出时间为:4194303 us, 约4.19秒
 *
 *      (说明一下：正常32位定时器来说,1us计数器加1,溢出时间:4294秒)
 */
uint8_t g_timxchy_cap_sta = 0;                          /* 输入捕获状态 */
uint32_t g_timxchy_cap_val = 0;                         /* 输入捕获值 */

/**
 * @brief       定时器TIMX 输入捕获 中断服务函数
 * @param       无
 * @retval      无
 */
void TIMX_CAP_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&g_timx_cap_chy_handle);         /* 定时器HAL库共用中断处理函数 */
}

/**
 * @brief       定时器输入捕获中断处理回调函数
 * @param       htim:定时器句柄指针
 * @note        该函数在HAL_TIM_IRQHandler中会被调用
 * @retval      无
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if ((g_timxchy_cap_sta & 0x80) == 0)                                                             /* 还未成功捕获 */
    {
        if (g_timxchy_cap_sta & 0x40)                                                                /* 捕获到一个下降沿 */
        {
            g_timxchy_cap_sta |= 0x80;                                                               /* 标记成功捕获到一次高电平脉宽 */
            g_timxchy_cap_val = HAL_TIM_ReadCapturedValue(&g_timx_cap_chy_handle, TIMX_CAP_CHY);     /* 获取当前的捕获值 */
            TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY);                         /* 一定要先清除原来的设置 */
            TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY, TIM_ICPOLARITY_RISING);    /* 配置TIMX通道Y为上升沿捕获，为下次捕获做准备 */
        }
        else                                                                                         /* 还未开始,第一次捕获上升沿 */
        {
            g_timxchy_cap_sta = 0;                                                                   /* 清空 */
            g_timxchy_cap_val = 0;                                                                   /* 捕获值清零 */
            g_timxchy_cap_sta |= 0x40;                                                               /* 标记捕获到了上升沿 */
            __HAL_TIM_SET_COUNTER(&g_timx_cap_chy_handle, 0);                                        /* 定时器计数器清零 */
            TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY);                         /* 一定要先清除原来的设置 */
            TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY, TIM_ICPOLARITY_FALLING);   /* TIMX通道Y设置为下降沿捕获 */
        }
    }
}

/**
 * @brief       定时器更新中断回调函数
 * @param       htim:定时器句柄指针
 * @note        此函数会被定时器共用中断处理函数调用
 * @retval      无
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIMX_CAP)
    {
        if ((g_timxchy_cap_sta & 0x80) == 0)                                                                /* 还未成功捕获 */
        {
            if (g_timxchy_cap_sta & 0x40)                                                                   /* 已经捕获到高电平了 */
            {
                if ((g_timxchy_cap_sta & 0x3F) == 0x3F)                                                     /* 溢出多次，高电平太长了 */
                {
                    TIM_RESET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY);                        /* 一定要先清除原来的设置 */
                    TIM_SET_CAPTUREPOLARITY(&g_timx_cap_chy_handle, TIMX_CAP_CHY, TIM_ICPOLARITY_RISING);   /* 配置TIMX通道Y为上升沿捕获 */
                    g_timxchy_cap_sta |= 0x80;                                                              /* 强制标记捕获了一次 */
                    g_timxchy_cap_val = 0xFFFF;                                                             /* 设置捕获值为最大 */
                }
                else                                                                                        /* 还可以累加高电平长度 */
                {
                    g_timxchy_cap_sta++;                                                                    /* 累计定时器溢出次数 */
                }
            }
        }
    }
}





