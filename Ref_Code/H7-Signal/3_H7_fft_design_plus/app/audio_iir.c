#include "audio_iir.h"
#include <math.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static float hpf_clamp_f32(float x, float min_value, float max_value)
{
    if (x < min_value)
    {
        return min_value;
    }

    if (x > max_value)
    {
        return max_value;
    }

    return x;
}

static uint16_t hpf_clamp_u16_from_float(float x,
                                         uint16_t min_value,
                                         uint16_t max_value)
{
    if (x < (float)min_value)
    {
        return min_value;
    }

    if (x > (float)max_value)
    {
        return max_value;
    }

    return (uint16_t)(x + 0.5f);
}

static void hpf_make_biquad_highpass(STM32_Biquad *bq,
                                     float fs,
                                     float fc,
                                     float q)
{
    double w0;
    double cos_w0;
    double sin_w0;
    double alpha;

    double b0;
    double b1;
    double b2;
    double a0;
    double a1;
    double a2;

    w0 = 2.0 * M_PI * (double)fc / (double)fs;
    cos_w0 = cos(w0);
    sin_w0 = sin(w0);
    alpha = sin_w0 / (2.0 * (double)q);

    b0 = (1.0 + cos_w0) * 0.5;
    b1 = -(1.0 + cos_w0);
    b2 = (1.0 + cos_w0) * 0.5;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cos_w0;
    a2 = 1.0 - alpha;

    bq->b0 = (float)(b0 / a0);
    bq->b1 = (float)(b1 / a0);
    bq->b2 = (float)(b2 / a0);
    bq->a1 = (float)(a1 / a0);
    bq->a2 = (float)(a2 / a0);

    bq->z1 = 0.0f;
    bq->z2 = 0.0f;
}

int STM32_StableHPF_Init(STM32_StableHPF *hpf,
                         float fs,
                         float fc,
                         uint32_t order)
{
    if (hpf == 0)
    {
        return -1;
    }

    if (fs <= 0.0f)
    {
        return -2;
    }

    if (fc <= 0.0f)
    {
        return -3;
    }

    if (fc >= fs * 0.45f)
    {
        return -4;
    }

    memset(hpf, 0, sizeof(STM32_StableHPF));

    hpf->fs = fs;
    hpf->fc = fc;

    /*
     * 目前推荐 4 阶 Butterworth 高通。
     * 4 阶 = 两个二阶节，Q 分别约为 0.5411961 和 1.3065630。
     */
    if (order <= 2U)
    {
        hpf->stage_num = 1U;
        hpf_make_biquad_highpass(&hpf->stage[0], fs, fc, 0.70710678f);
    }
    else
    {
        hpf->stage_num = 2U;
        hpf_make_biquad_highpass(&hpf->stage[0], fs, fc, 0.54119610f);
        hpf_make_biquad_highpass(&hpf->stage[1], fs, fc, 1.30656300f);
    }

    STM32_StableHPF_SetAdcDacRange(hpf,
                                   (float)STM32_ADC_MID_VALUE,
                                   (float)STM32_ADC_SCALE_VALUE,
                                   (float)STM32_DAC_MID_VALUE,
                                   (float)STM32_DAC_SCALE_VALUE);

    STM32_StableHPF_Reset(hpf);

    return 0;
}

void STM32_StableHPF_SetAdcDacRange(STM32_StableHPF *hpf,
                                    float adc_mid,
                                    float adc_scale,
                                    float dac_mid,
                                    float dac_scale)
{
    if (hpf == 0)
    {
        return;
    }

    if (adc_scale <= 0.0f)
    {
        adc_scale = (float)STM32_ADC_SCALE_VALUE;
    }

    if (dac_scale <= 0.0f)
    {
        dac_scale = (float)STM32_DAC_SCALE_VALUE;
    }

    hpf->adc_mid = adc_mid;
    hpf->adc_scale = adc_scale;
    hpf->dac_mid = dac_mid;
    hpf->dac_scale = dac_scale;
}

void STM32_StableHPF_Reset(STM32_StableHPF *hpf)
{
    uint32_t i;

    if (hpf == 0)
    {
        return;
    }

    for (i = 0U; i < STM32_HPF_MAX_BIQUAD_STAGE; i++)
    {
        hpf->stage[i].z1 = 0.0f;
        hpf->stage[i].z2 = 0.0f;
    }
}

float STM32_StableHPF_ProcessFloat(STM32_StableHPF *hpf, float x)
{
    uint32_t i;
    float y;
    STM32_Biquad *bq;

    if (hpf == 0)
    {
        return 0.0f;
    }

    y = x;

    for (i = 0U; i < hpf->stage_num; i++)
    {
        bq = &hpf->stage[i];

        /*
         * Direct Form II Transposed:
         * y = b0*x + z1
         * z1 = b1*x - a1*y + z2
         * z2 = b2*x - a2*y
         */
        float out = bq->b0 * y + bq->z1;
        bq->z1 = bq->b1 * y - bq->a1 * out + bq->z2;
        bq->z2 = bq->b2 * y - bq->a2 * out;

        bq->z1 = hpf_clamp_f32(bq->z1, -8.0f, 8.0f);
        bq->z2 = hpf_clamp_f32(bq->z2, -8.0f, 8.0f);

        y = hpf_clamp_f32(out, -4.0f, 4.0f);
    }

    return y;
}

uint16_t STM32_StableHPF_ProcessAdc(STM32_StableHPF *hpf, uint16_t adc_value)
{
    float x;
    float y;
    float dac_value;

    if (hpf == 0)
    {
        return STM32_DAC_MID_VALUE;
    }

    x = ((float)adc_value - hpf->adc_mid) / hpf->adc_scale;
    x = hpf_clamp_f32(x, -1.2f, 1.2f);

    y = STM32_StableHPF_ProcessFloat(hpf, x);

    y = hpf_clamp_f32(y, -1.0f, 1.0f);

    dac_value = y * hpf->dac_scale + hpf->dac_mid;

    return hpf_clamp_u16_from_float(dac_value,
                                    0U,
                                    (uint16_t)STM32_DAC_MAX_VALUE);
}

void STM32_StableHPF_ProcessAdcBlock(STM32_StableHPF *hpf,
                                     const uint16_t *adc_input,
                                     uint16_t *dac_output,
                                     uint32_t block_size)
{
    uint32_t i;

    if ((hpf == 0) || (adc_input == 0) || (dac_output == 0))
    {
        return;
    }

    for (i = 0U; i < block_size; i++)
    {
        dac_output[i] = STM32_StableHPF_ProcessAdc(hpf, adc_input[i]);
    }
}
