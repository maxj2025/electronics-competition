/**
 * @file    freq_measure.c
 * @brief   频率测量（TIM2）+ 占空比测量（TIM15）+ HMI/UART 工具
 *
 * 硬件链路：
 *   路1：信号 → 比较器整形为 3.3V 方波 → PA5 (TIM2_CH1)  测频率
 *   路2：信号 → 比较器整形为 3.3V 方波 → PA2 (TIM15_CH1) 测占空比
 *   路3：信号 → 3 倍衰减放大器 → PC5 (ADC1)              测幅度/上升沿（见 adc_app.c）
 *
 * 时钟：HSE 25MHz→PLL=550MHz SYSCLK→AHB/2=275MHz HCLK
 *       APB1/APB2 分频=2，故定时器时钟 = 275MHz（代码中 tick_freq / TIM_SCK 均用此值）
 */

#include "freq_measure.h"


/* ---- 频率测量模式 ---- */
#define FMODE_PERIOD 0   // 测周法（低频，输入捕获周期）
#define FMODE_COUNT  1   // 测频法（高频，ETR 外部时钟计数 + 闸门）

/* ---- 频率测量默认配置 ---- */
#define TICK_FREQ_DEFAULT    55000000.0f  /* TIM2 计数时钟 (Hz) */
#define SWITCH_HIGH_DEFAULT  50000.0f      /* > 该值切测频法 */
#define SWITCH_LOW_DEFAULT   40000.0f      /* < 该值切回测周法 */
#define GATE_TIME_DEFAULT    100           /* 测频法闸门时间 (ms)，原 500ms 过慢，改 100ms */





void my_MX_TIM2_Init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef     sConfigIC = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 5-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 0xFFFFFFFF;                 // 32 位计数器
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK) Error_Handler();
  if (HAL_TIM_IC_Init(&htim2) != HAL_OK) Error_Handler();

  // 从模式：每个 CH1 上升沿复位计数器 → CCR1 直接得到“周期计数值”
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_RESET;
  sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
  sSlaveConfig.TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sSlaveConfig.TriggerFilter = 0;
  if (HAL_TIM_SlaveConfigSynchro(&htim2, &sSlaveConfig) != HAL_OK) Error_Handler();

  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) Error_Handler();

  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 0;
  if (HAL_TIM_IC_ConfigChannel(&htim2, &sConfigIC, TIM_CHANNEL_1) != HAL_OK) Error_Handler();
}

/* ============================================================
 *  频率测量硬件模式切换
 *  - FMODE_PERIOD：测周法（上）
 *  - FMODE_COUNT ：测频法，TIM2 外部时钟模式1（ETR 经 TI1FP1 计数），零中断
 * ============================================================ */
static void freq_hw_switch(FreqMeasure *self, uint8_t target_mode)
{
    GPIO_InitTypeDef gpio = {0};
    self->first_frame = 1;

    HAL_TIM_Base_Stop(self->htim);
    HAL_TIM_IC_Stop(self->htim, TIM_CHANNEL_1);
    HAL_TIM_Base_DeInit(self->htim);

    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpio.Pin       = GPIO_PIN_5;
    gpio.Mode      = GPIO_MODE_AF_PP;
    gpio.Pull      = GPIO_PULLDOWN;
    gpio.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    gpio.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init(GPIOA, &gpio);

    if (target_mode == FMODE_PERIOD) {
        my_MX_TIM2_Init();
        __HAL_TIM_CLEAR_FLAG(self->htim, TIM_FLAG_CC1 | TIM_FLAG_UPDATE);
        HAL_TIM_Base_Start(self->htim);
        HAL_TIM_IC_Start(self->htim, TIM_CHANNEL_1);
    } else {
        // 测频法：信号作为外部时钟驱动 TIM2 计数
        self->htim->Instance = TIM2;
        self->htim->Init.Prescaler = 5-1;
        self->htim->Init.CounterMode = TIM_COUNTERMODE_UP;
        self->htim->Init.Period = 0xFFFFFFFF;
        self->htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
        HAL_TIM_Base_Init(self->htim);

        TIM_SlaveConfigTypeDef sSlaveConfig = {0};
        sSlaveConfig.SlaveMode = TIM_SLAVEMODE_EXTERNAL1;
        sSlaveConfig.InputTrigger = TIM_TS_TI1FP1;
        sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_RISING;
        sSlaveConfig.TriggerFilter = 0;
        HAL_TIM_SlaveConfigSynchro(self->htim, &sSlaveConfig);

        __HAL_TIM_SET_COUNTER(self->htim, 0);
        HAL_TIM_Base_Start(self->htim);
    }
}

/* 测周法：读取 CCR1（周期计数值），换算频率 */
static float freq_read_period(FreqMeasure *self)
{
    if (__HAL_TIM_GET_FLAG(self->htim, TIM_FLAG_CC1) != RESET) {
        uint32_t cap = HAL_TIM_ReadCapturedValue(self->htim, TIM_CHANNEL_1);
        __HAL_TIM_CLEAR_FLAG(self->htim, TIM_FLAG_CC1);
        __HAL_TIM_CLEAR_FLAG(self->htim, TIM_FLAG_UPDATE);

        if (self->first_frame) { self->first_frame = 0; return -1.0f; } // 丢弃切换后首帧
        if (cap == 0) return 0.0f;
        // +1 修正：捕获值 = 周期-1（计数从 0 开始），且 prescaler=0
        return (float)((double)self->tick_freq / (double)(cap + 1));
    }
    return -1.0f;   // 无新捕获
}

/* ===================== 频率测量公共 API ===================== */
void FreqMeasure_Init(FreqMeasure *self, TIM_HandleTypeDef *htim)
{
    self->htim            = htim;
    self->tick_freq       = TICK_FREQ_DEFAULT;
    self->switch_high_thr = SWITCH_HIGH_DEFAULT;
    self->switch_low_thr  = SWITCH_LOW_DEFAULT;
    self->gate_time_ms    = GATE_TIME_DEFAULT;
    self->mode            = FMODE_PERIOD;
    freq_hw_switch(self, FMODE_PERIOD);
}


void FreqMeasure_Process(FreqMeasure *self, Wave_Struct *wave) {
    if (self->mode == FMODE_PERIOD) {
        float f = freq_read_period(self);
        if (f >= 0.0f) {
            wave->Freq = f;

            if (f > self->switch_high_thr) {
                self->mode = FMODE_COUNT;
                freq_hw_switch(self, FMODE_COUNT);
                self->measuring = 0;
            }
        }
    } else {
        if (!self->measuring) {
            __HAL_TIM_SET_COUNTER(self->htim, 0);
            self->gate_start_ms = HAL_GetTick();
            self->measuring     = 1;
        } else {
            uint32_t current_ms = HAL_GetTick();
            uint32_t delta_ms   = current_ms - self->gate_start_ms;

            if (delta_ms >= self->gate_time_ms) {
                uint32_t total_pulses = __HAL_TIM_GET_COUNTER(self->htim);

                if (delta_ms > 0) {
                    wave->Freq = (float)(((double)total_pulses * 1000.0) / (double)delta_ms);
                }

                self->measuring = 0;

                if (wave->Freq < self->switch_low_thr) {
                    self->mode = FMODE_PERIOD;
                    freq_hw_switch(self, FMODE_PERIOD);
                }
            }
        }
    }
		if(wave->Freq<100)wave->Freq=(float)(((uint32_t)(wave->Freq+0.5f)));
		else if(wave->Freq<1000)wave->Freq=(float)(((uint32_t)(wave->Freq+50)/50)*50);
		else if(wave->Freq<10000)wave->Freq=(float)(((uint32_t)(wave->Freq+50)/100)*100);
		else if(wave->Freq<110000)wave->Freq=(float)(((uint32_t)(wave->Freq+50)/100)*100);
		else if(wave->Freq<2500000)wave->Freq=(float)(((uint32_t)(wave->Freq+3000)/5000)*5000);
		
} 

