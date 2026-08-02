#include "dac_dds.h"



#define DDS_DAC_HANDLE          hdac1
#define DDS_TIM_HANDLE          htim4
#define DDS_DAC_CHANNEL         DAC_CHANNEL_1

/*
 * STM32H7 注意：
 * 1. DMA Buffer 不要放 DTCM，因为多数 DMA 访问不到 DTCM。
 * 2. 推荐放 AXI SRAM / SRAM1 / SRAM2 / SRAM3，具体看你的 DMA 控制器可访问区域。
 * 3. 32 字节对齐，方便 DCache Clean。
 *
 * 如果你有自己的 linker section，可以把 .dma_buffer 放到 DMA 可访问 RAM。
 */
__attribute__((section(".RAM_D1"), aligned(32)))uint16_t dds_dac_buf[DDS_DMA_BUF_SIZE];

/*
 * 正弦表，有符号格式。
 * 范围：-32767 ~ +32767。
 * 这里直接使用整数常量表，避免运行时 sinf。
 * 1024 点表太长，为了代码完整性，这里用初始化函数生成四分之一波形近似不方便。
 * 所以下面采用 256 点表插值会增加复杂度。
 *
 * 为了工程实用，下面仍然在 DDS_InitSineLUT() 里用整数递推/或者使用 math 初始化。
 * 但是 DDS_GenerateOneSample() 运行时是纯整数。
 * 如果你不想链接 libm，可以用后面附带的 Python 脚本生成静态 sine_lut。
 */
static int16_t sine_lut[DDS_LUT_SIZE];

/* DDS 状态变量 */
static volatile uint32_t phase_acc = 0;
static volatile uint32_t phase_inc = 0;
static volatile uint32_t phase_offset = 0;

/* 幅度 Q15：0 ~ 32767，对应 0% ~ 100% */
static volatile uint16_t dds_amp_q15 = 32767;

/* DAC 偏置，默认中点 */
static volatile uint16_t dds_offset = DDS_DAC_MID;

/* ================= 内部函数声明 ================= */

static void DDS_InitSineLUT(void);
static uint16_t DDS_GenerateOneSample(void);
static void DDS_CleanBuffer(uint16_t *buf, uint32_t len);

/* ================= DDS 初始化 ================= */

void DDS_Init(void)
{
    phase_acc = 0;
    phase_inc = 0;
    phase_offset = 0;
    dds_amp_q15 = 32767;
    dds_offset = DDS_DAC_MID;

    DDS_InitSineLUT();

    DDS_FillBuffer(dds_dac_buf, DDS_DMA_BUF_SIZE);
}

/* ================= 启动 DDS ================= */

void DDS_Start(void)
{
    /* 启动前确保整个 DMA Buffer 已经 Clean 到 RAM */
    DDS_CleanBuffer(dds_dac_buf, DDS_DMA_BUF_SIZE);

    HAL_DAC_Start_DMA(&DDS_DAC_HANDLE,
                      DDS_DAC_CHANNEL,
                      (uint32_t *)dds_dac_buf,
                      DDS_DMA_BUF_SIZE,
                      DAC_ALIGN_12B_R);

    HAL_TIM_Base_Start(&DDS_TIM_HANDLE);
}

/* ================= 停止 DDS ================= */

void DDS_Stop(void)
{
    HAL_TIM_Base_Stop(&DDS_TIM_HANDLE);
    HAL_DAC_Stop_DMA(&DDS_DAC_HANDLE, DDS_DAC_CHANNEL);
}

/* ================= 设置频率 =================
 * freq_hz：输出频率，单位 Hz。
 * 公式：phase_inc = fout * 2^32 / Fs
 */
void DDS_SetFrequency(uint32_t freq_hz)
{
    uint64_t temp;

    if (freq_hz > (DDS_SAMPLE_RATE_HZ / 2U))
    {
        freq_hz = DDS_SAMPLE_RATE_HZ / 2U;
    }

    temp = (uint64_t)freq_hz * DDS_PHASE_MAX_U64;
    temp = temp / DDS_SAMPLE_RATE_HZ;

    __disable_irq();
    phase_inc = (uint32_t)temp;
    __enable_irq();
}

/* ================= 设置相位，单位：度 ================= */
void DDS_SetPhaseDeg(uint32_t phase_deg)
{
    uint64_t temp;

    phase_deg %= 360U;

    temp = (uint64_t)phase_deg * DDS_PHASE_MAX_U64;
    temp = temp / 360U;

    __disable_irq();
    phase_offset = (uint32_t)temp;
    __enable_irq();
}

/* ================= 设置幅度，Q15 格式 =================
 * amp_q15 = 0      ：0%
 * amp_q15 = 16384  ：约 50%
 * amp_q15 = 32767  ：100%
 */
void DDS_SetAmplitudeQ15(uint16_t amp_q15)
{
    if (amp_q15 > 32767U)
    {
        amp_q15 = 32767U;
    }

    __disable_irq();
    dds_amp_q15 = amp_q15;
    __enable_irq();
}

/* ================= 设置幅度，千分比 =================
 * amp_permille = 0    ：0%
 * amp_permille = 500  ：50%
 * amp_permille = 1000 ：100%
 */
void DDS_SetAmplitudePermille(uint16_t amp_permille)
{
    uint32_t amp;

    if (amp_permille > 1000U)
    {
        amp_permille = 1000U;
    }

    amp = ((uint32_t)amp_permille * 32767U) / 1000U;

    DDS_SetAmplitudeQ15((uint16_t)amp);
}

/* ================= 设置 DAC 直流偏置 ================= */
void DDS_SetOffset(uint16_t offset)
{
    if (offset > DDS_DAC_MAX)
    {
        offset = DDS_DAC_MAX;
    }

    __disable_irq();
    dds_offset = offset;
    __enable_irq();
}

/* ================= 重置相位累加器 ================= */
void DDS_ResetPhase(void)
{
    __disable_irq();
    phase_acc = 0;
    __enable_irq();
}

/* ================= 初始化正弦 LUT =================
 * 注意：这里为了方便使用了浮点和 sinf，但只在初始化时执行一次。
 * DDS 实时生成采样点时是纯整数。
 */
#include <math.h>

static void DDS_InitSineLUT(void)
{
    for (uint32_t i = 0; i < DDS_LUT_SIZE; i++)
    {
        float theta = 2.0f * 3.14159265358979323846f * (float)i / (float)DDS_LUT_SIZE;
        sine_lut[i] = (int16_t)(sinf(theta) * 32767.0f);
    }
}

/* ================= 纯整数 DDS 生成一个采样点 =================
 * 无 float。
 * 运行时只有：
 * 1. 相位累加
 * 2. 查表
 * 3. 整数幅度缩放
 * 4. 映射到 DAC
 */
static uint16_t DDS_GenerateOneSample(void)
{
    uint32_t phase;
    uint32_t index;
    int32_t y;
    int32_t dac_value;
    uint16_t amp_q15_local;
    uint16_t offset_local;

    /* 读取 volatile 参数到局部变量，避免循环中多次访问 volatile */
    amp_q15_local = dds_amp_q15;
    offset_local = dds_offset;

    /* 相位累加 */
    phase_acc += phase_inc;

    /* 加相位偏移 */
    phase = phase_acc + phase_offset;

    /* 取相位高位作为 LUT 索引 */
    index = phase >> (DDS_PHASE_BITS - DDS_LUT_BITS);

    /* 查表：-32767 ~ +32767 */
    y = sine_lut[index];

    /* 幅度控制：仍然是 -32767 ~ +32767 */
    y = (y * (int32_t)amp_q15_local) / 32767;

    /* 映射到 DAC 半幅度：-2047 ~ +2047 */
    y = (y * DDS_DAC_HALF_SCALE) / 32767;

    /* 加直流偏置 */
    dac_value = (int32_t)offset_local + y;

    /* 限幅 */
    if (dac_value < 0)
    {
        dac_value = 0;
    }
    else if (dac_value > (int32_t)DDS_DAC_MAX)
    {
        dac_value = DDS_DAC_MAX;
    }

    return (uint16_t)dac_value;
}

/* ================= DCache Clean Buffer ================= */
static void DDS_CleanBuffer(uint16_t *buf, uint32_t len)
{
#if (__DCACHE_PRESENT == 1U)
    DCache_Clean_By_Addr((void *)buf, len * sizeof(uint16_t));
#else
    (void)buf;
    (void)len;
#endif
}

/* ================= 填充 DMA Buffer =================
 * 注意：填完后必须 Clean DCache。
 */
void DDS_FillBuffer(uint16_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        buf[i] = DDS_GenerateOneSample();
    }

    /* CPU 写完 Buffer 后，DMA 要读取，所以需要 Clean DCache */
    DDS_CleanBuffer(buf, len);
}

/* ================= DAC DMA 半传输完成回调 =================
 * DMA 正在输出后半区，此时 CPU 填前半区。
 */
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DDS_DAC_HANDLE.Instance)
    {
        DDS_FillBuffer(&dds_dac_buf[0], DDS_DMA_BUF_SIZE / 2U);
    }
}

/* ================= DAC DMA 全传输完成回调 =================
 * DMA 正在回到前半区，此时 CPU 填后半区。
 */
void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DDS_DAC_HANDLE.Instance)
    {
        DDS_FillBuffer(&dds_dac_buf[DDS_DMA_BUF_SIZE / 2U], DDS_DMA_BUF_SIZE / 2U);
    }
}

/* ================= DAC DMA 错误回调 ================= */
void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
    if (hdac->Instance == DDS_DAC_HANDLE.Instance)
    {
        /* 用户可以在这里加错误处理 */
    }
}
