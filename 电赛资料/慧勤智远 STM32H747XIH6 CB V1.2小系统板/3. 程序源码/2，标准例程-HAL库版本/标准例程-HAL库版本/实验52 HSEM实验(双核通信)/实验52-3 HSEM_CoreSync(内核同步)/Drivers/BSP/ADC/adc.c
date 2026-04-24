/**
 ****************************************************************************************************
 * @file        adc.c
 * @version     V1.0
 * @brief       ADC 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/ADC/adc.h"
#include "./SYSTEM/delay/delay.h"


ADC_HandleTypeDef g_adc_handle;           /* ADC句柄 */

/**
 * @brief       ADC初始化函数
 * @note        本函数支持ADC1/ADC2任意通道,但是不支持ADC3
 *              我们使用16位精度, ADC采样时钟=32M, 转换时间为:采样周期 + 8.5个ADC周期
 *              设置最大采样周期: 810.5, 则转换时间 = 819个ADC周期 = 25.6us
 * @param       无
 * @retval      无
 */
void adc_init(void)
{ 
    g_adc_handle.Instance = ADC_ADCX;                                       /* 选择ADC */
    g_adc_handle.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV2;                /* 输入ADC时钟2分频，ADCCLK=per_ck/2=64/2=32Mhz(不能超过50Mhz) */
    g_adc_handle.Init.Resolution = ADC_RESOLUTION_16B;                      /* 16位分辨率 */
    g_adc_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;                      /* 非扫描模式，仅转换一个通道 */
    g_adc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;                   /* 轮询转换或中断使用EOC标志 */
    g_adc_handle.Init.LowPowerAutoWait = DISABLE;                           /* 自动低功耗关闭 */
    g_adc_handle.Init.ContinuousConvMode = DISABLE;                         /* 关闭连续转换模式 */
    g_adc_handle.Init.NbrOfConversion = 1;                                  /* 1个转换在规则序列中，也就是只转换规则序列1 */
    g_adc_handle.Init.DiscontinuousConvMode = DISABLE;                      /* 禁止常规通道的不连续采样模式 */
    g_adc_handle.Init.NbrOfDiscConversion = 0;                              /* 不连续采样通道数为0，禁止常规通道的不连续采样模式后，此参数忽略 */
    g_adc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;                /* 软件触发 */
    g_adc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE; /* 禁止硬件触发检测，即使用软件触发 */
    g_adc_handle.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;     /* 规则通道的数据仅仅保存在DR寄存器里面 */
    g_adc_handle.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;                   /* 复写模式，有新的数据后直接覆盖掉旧数据 */
    g_adc_handle.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;                 /* 设置ADC转换结果不左移,数据右对齐 */
    g_adc_handle.Init.OversamplingMode = DISABLE;                           /* 过采样关闭 */
    HAL_ADC_Init(&g_adc_handle);                                            /* 初始化ADC */

    HAL_ADCEx_Calibration_Start(&g_adc_handle, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED); /* 单端输入模式ADC校准 */
}

/**
 * @brief       ADC底层驱动，引脚配置，时钟使能
 * @note        此函数会被HAL_ADC_Init()调用
 * @param       hadc:ADC句柄
 * @retval      无
 */
void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc)
{
    GPIO_InitTypeDef gpio_init_struct;

    ADC_ADCX_CHY_CLK_ENABLE();                                 /* ADC时钟使能 */
    ADC_ADCX_CHY_GPIO_CLK_ENABLE();                            /* ADC IO口时钟使能 */
    __HAL_RCC_ADC_CONFIG(RCC_ADCCLKSOURCE_CLKP);               /* ADC时钟源选择，将per_ck选作为ADC时钟源，默认选择hsi_ker_ck作为per_ck，频率:64Mhz */
    
    gpio_init_struct.Pin = ADC_ADCX_CHY_GPIO_PIN;              /* ADC采集引脚 */
    gpio_init_struct.Mode = GPIO_MODE_ANALOG;                  /* 模拟模式 */
    gpio_init_struct.Pull = GPIO_NOPULL;                       /* 不带上下拉 */
    HAL_GPIO_Init(ADC_ADCX_CHY_GPIO_PORT, &gpio_init_struct);  /* 初始化ADC采集引脚 */
}

/**
 * @brief       获得ADC转换后的结果
 * @param       ch: 通道号 0~19，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_19
 * @retval      转换结果
 */
uint32_t adc_get_result(uint32_t ch)   
{
    ADC_ChannelConfTypeDef adc_ch_conf;

    adc_ch_conf.Channel = ch;                               /* 通道 */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_1;                  /* 常规序列1 */
    adc_ch_conf.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;  /* 采样时间，设置最大采样周期: 810.5个ADC时钟周期 */
    adc_ch_conf.SingleDiff = ADC_SINGLE_ENDED;              /* 单端采集 */
    adc_ch_conf.OffsetNumber = ADC_OFFSET_NONE;             /* 不使用偏移量的通道 */
    adc_ch_conf.Offset = 0;                                 /* 偏移量为0 */
    HAL_ADC_ConfigChannel(&g_adc_handle, &adc_ch_conf);     /* 通道配置 */

    HAL_ADC_Start(&g_adc_handle);                           /* 开启ADC常规通道转换 */

    HAL_ADC_PollForConversion(&g_adc_handle, 10);           /* 轮询转换 */
    return HAL_ADC_GetValue(&g_adc_handle);                 /* 返回最近一次ADC规则通道的转换结果 */
}

/**
 * @brief       获取通道ch的转换值，取times次,然后平均
 * @param       ch    : 通道号 0~19，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_19
 * @param       times : 获取次数
 * @retval      通道ch的times次转换结果平均值
 */
uint32_t adc_get_result_average(uint32_t ch, uint8_t times)
{
    uint32_t temp_val = 0;
    uint8_t t;

    for (t = 0; t < times; t++)         /* 获取times次数据 */
    {
        temp_val += adc_get_result(ch);
        delay_ms(5);
    }

    return temp_val / times;            /* 返回平均值 */
} 

/**
 * @brief       ADC转换常规通道
 * @param       ch: 通道号 0~19，取值范围为：ADC_CHANNEL_0~ADC_CHANNEL_19
 * @retval      无
 */
void adc_conversion(uint32_t ch)   
{
    ADC_ChannelConfTypeDef adc_ch_conf;

    adc_ch_conf.Channel = ch;                               /* 通道 */
    adc_ch_conf.Rank = ADC_REGULAR_RANK_1;                  /* 常规序列1 */
    adc_ch_conf.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;  /* 采样时间，设置最大采样周期: 810.5个ADC时钟周期 */
    adc_ch_conf.SingleDiff = ADC_SINGLE_ENDED;              /* 单端采集 */
    adc_ch_conf.OffsetNumber = ADC_OFFSET_NONE;             /* 不使用偏移量的通道 */
    adc_ch_conf.Offset = 0;                                 /* 偏移量为0 */
    HAL_ADC_ConfigChannel(&g_adc_handle, &adc_ch_conf);     /* 通道配置 */

    HAL_ADC_Start(&g_adc_handle);                           /* 开启ADC常规通道转换 */

    HAL_ADC_PollForConversion(&g_adc_handle, 10);           /* 轮询转换 */
}







