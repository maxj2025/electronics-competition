#include "audio_iir.h"
#include <math.h>
#include <string.h>


#ifndef M_PI
#define M_PI                    3.14159265358979323846
#endif

#define AUDIO_IIR_EPSILON       1.0e-12
#define AUDIO_IIR_Q15_MAX       32767
#define AUDIO_IIR_Q15_MIN       (-32768)

/*
 * IIR 系数使用 Q28。
 * 原因：IIR 的 a1 系数经常超过 Q15 的 [-1, 1) 范围。
 */
#define AUDIO_IIR_COEFF_Q       28
#define AUDIO_IIR_COEFF_SCALE   268435456.0

typedef struct
{
    int32_t b0;
    int32_t b1;
    int32_t b2;
    int32_t a1;
    int32_t a2;
} AudioIIR_BiquadCoeffQ28;

typedef struct
{
    q15_t x1;
    q15_t x2;
    q15_t y1;
    q15_t y2;
} AudioIIR_BiquadStateQ15;

__attribute__((section(".DTCM_DATA"))) static q15_t s_iir_input_q15[AUDIO_IIR_BLOCK_SIZE];
__attribute__((section(".DTCM_DATA"))) static q15_t s_iir_output_q15[AUDIO_IIR_BLOCK_SIZE];

__attribute__((section(".DTCM_DATA"))) static AudioIIR_BiquadCoeffQ28 s_iir_coeff;
__attribute__((section(".DTCM_DATA"))) static AudioIIR_BiquadStateQ15 s_iir_state;

__attribute__((section(".DTCM_DATA"))) static uint8_t s_iir_ready = 0U;

static int32_t AudioIIR_DoubleToQ28(double x)
{
    double y;

    y = x * AUDIO_IIR_COEFF_SCALE;

    if (y > 2147483647.0)
    {
        return 2147483647;
    }

    if (y < -2147483648.0)
    {
        return (int32_t)-2147483647 - 1;
    }

    if (y >= 0.0)
    {
        y += 0.5;
    }
    else
    {
        y -= 0.5;
    }

    return (int32_t)y;
}

static q15_t AudioIIR_SatQ15(int64_t x)
{
    if (x > AUDIO_IIR_Q15_MAX)
    {
        return (q15_t)AUDIO_IIR_Q15_MAX;
    }

    if (x < AUDIO_IIR_Q15_MIN)
    {
        return (q15_t)AUDIO_IIR_Q15_MIN;
    }

    return (q15_t)x;
}

static void AudioIIR_ADC12ToQ15(const uint16_t *src, q15_t *dst, uint32_t len)
{
    uint32_t i;
    int32_t x;
    int32_t y;

    for (i = 0; i < len; i++)
    {
        x = (int32_t)src[i] - (int32_t)AUDIO_IIR_ADC_MID;
        y = x << 4;

        if (y > AUDIO_IIR_Q15_MAX)
        {
            y = AUDIO_IIR_Q15_MAX;
        }
        else if (y < AUDIO_IIR_Q15_MIN)
        {
            y = AUDIO_IIR_Q15_MIN;
        }

        dst[i] = (q15_t)y;
    }
}

static void AudioIIR_Q15ToDAC12(const q15_t *src, uint16_t *dst, uint32_t len)
{
    uint32_t i;
    int32_t x;
    int32_t y;

    for (i = 0; i < len; i++)
    {
        x = ((int32_t)src[i]) >> 4;
        y = x + (int32_t)AUDIO_IIR_DAC_MID;

        if (y < 0)
        {
            y = 0;
        }
        else if (y > AUDIO_IIR_DAC_MAX)
        {
            y = AUDIO_IIR_DAC_MAX;
        }

        dst[i] = (uint16_t)y;
    }
}

/*
 * y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2]
 *        - a1*y[n-1] - a2*y[n-2]
 */
__attribute__((section(".ITCM_CODE")))
static void AudioIIR_ComputeQ15(const q15_t *src, q15_t *dst, uint32_t blockSize)
{
    uint32_t n;
    q15_t x0;
    q15_t y0;
    int64_t acc;

    if (src == NULL || dst == NULL)
    {
        return;
    }

    if (s_iir_ready == 0U)
    {
        return;
    }

    for (n = 0; n < blockSize; n++)
    {
        x0 = src[n];

        acc = 0;
        acc += (int64_t)s_iir_coeff.b0 * (int64_t)x0;
        acc += (int64_t)s_iir_coeff.b1 * (int64_t)s_iir_state.x1;
        acc += (int64_t)s_iir_coeff.b2 * (int64_t)s_iir_state.x2;
        acc -= (int64_t)s_iir_coeff.a1 * (int64_t)s_iir_state.y1;
        acc -= (int64_t)s_iir_coeff.a2 * (int64_t)s_iir_state.y2;

        acc >>= AUDIO_IIR_COEFF_Q;

        y0 = AudioIIR_SatQ15(acc);
        dst[n] = y0;

        s_iir_state.x2 = s_iir_state.x1;
        s_iir_state.x1 = x0;

        s_iir_state.y2 = s_iir_state.y1;
        s_iir_state.y1 = y0;
    }
}

int AudioIIR_RedesignLowPass(const AudioIIR_LowPassParam *param)
{
    double fs;
    double fc;
    double omega;
    double sn;
    double cs;
    double alpha;
    double q;
    double a0;
    double b0;
    double b1;
    double b2;
    double a1;
    double a2;

    if (param == NULL)
    {
        return 0;
    }

    fs = param->fs;
    fc = param->fc;

    if (fs <= 0.0)
    {
        return 0;
    }

    if (!(fc > 0.0 && fc < (fs * 0.5)))
    {
        return 0;
    }

    q = 0.7071067811865476;

    omega = 2.0 * M_PI * fc / fs;
    sn = sin(omega);
    cs = cos(omega);
    alpha = sn / (2.0 * q);

    b0 = (1.0 - cs) * 0.5;
    b1 = 1.0 - cs;
    b2 = (1.0 - cs) * 0.5;
    a0 = 1.0 + alpha;
    a1 = -2.0 * cs;
    a2 = 1.0 - alpha;

    if (fabs(a0) < AUDIO_IIR_EPSILON)
    {
        return 0;
    }

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    s_iir_coeff.b0 = AudioIIR_DoubleToQ28(b0);
    s_iir_coeff.b1 = AudioIIR_DoubleToQ28(b1);
    s_iir_coeff.b2 = AudioIIR_DoubleToQ28(b2);
    s_iir_coeff.a1 = AudioIIR_DoubleToQ28(a1);
    s_iir_coeff.a2 = AudioIIR_DoubleToQ28(a2);

    s_iir_ready = 1U;

    AudioIIR_ResetState();

    return 1;
}

int AudioIIR_InitLowPass(const AudioIIR_LowPassParam *param)
{
    s_iir_ready = 0U;

    memset(s_iir_input_q15, 0, sizeof(s_iir_input_q15));
    memset(s_iir_output_q15, 0, sizeof(s_iir_output_q15));
    memset(&s_iir_coeff, 0, sizeof(s_iir_coeff));
    memset(&s_iir_state, 0, sizeof(s_iir_state));

    return AudioIIR_RedesignLowPass(param);
}

__attribute__((section(".ITCM_CODE")))
int AudioIIR_ProcessBlock_U16(const uint16_t *adc_src,
                              uint16_t *dac_dst,
                              uint32_t len)
{
    if (adc_src == NULL || dac_dst == NULL)
    {
        return 0;
    }

    if (s_iir_ready == 0U)
    {
        return 0;
    }

    if (len == 0U || len > AUDIO_IIR_BLOCK_SIZE)
    {
        return 0;
    }

    AudioIIR_ADC12ToQ15(adc_src, s_iir_input_q15, len);
    AudioIIR_ComputeQ15(s_iir_input_q15, s_iir_output_q15, len);
    AudioIIR_Q15ToDAC12(s_iir_output_q15, dac_dst, len);

    return 1;
}

int AudioIIR_ProcessBlock_Q15(const q15_t *input_q15,
                              q15_t *output_q15,
                              uint32_t len)
{
    if (input_q15 == NULL || output_q15 == NULL)
    {
        return 0;
    }

    if (s_iir_ready == 0U)
    {
        return 0;
    }

    if (len == 0U || len > AUDIO_IIR_BLOCK_SIZE)
    {
        return 0;
    }

    AudioIIR_ComputeQ15(input_q15, output_q15, len);

    return 1;
}

void AudioIIR_ResetState(void)
{
    memset(&s_iir_state, 0, sizeof(s_iir_state));
}

uint8_t AudioIIR_IsReady(void)
{
    return s_iir_ready;
}
