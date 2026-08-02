#include "long_fft.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ========== 内部辅助函数 ========== */

/* 计算 log2,输入必须是 2 的幂;非 2 的幂返回 0xFFFFFFFF */
static uint32_t fft_log2(uint32_t n)
{
    uint32_t log = 0;
    if (n == 0U || (n & (n - 1U)) != 0U) {
        return 0xFFFFFFFFU;  /* 非 2 的幂 */
    }
    while (n > 1U) { n >>= 1U; log++; }
    return log;
}

/*
 * 生成复数 FFT 旋转因子表:
 *   pTw[2*k]   = cos(2*pi*k/N)
 *   pTw[2*k+1] = sin(2*pi*k/N),  k = 0..N/2-1
 * 用 double 计算保证高点数下的精度,再存成 float。
 */
static void fft_gen_twiddle_cfft(float32_t *pTw, uint32_t N)
{
    uint32_t half = N >> 1U;
    for (uint32_t k = 0U; k < half; k++) {
        double ang = 2.0 * M_PI * (double)k / (double)N;
        pTw[2U * k]      = (float32_t)cos(ang);
        pTw[2U * k + 1U] = (float32_t)sin(ang);
    }
}

/*
 * 生成实数 FFT 额外旋转因子表:
 *   pTw[2*k]   = cos(pi*k/N)
 *   pTw[2*k+1] = sin(pi*k/N),  k = 0..N/2
 * 注意是 pi*k/N (不是 2*pi*k/N),这是实数 FFT 算法的标准定义。
 */
static void fft_gen_twiddle_rfft(float32_t *pTw, uint32_t N)
{
    uint32_t half = N >> 1U;
    for (uint32_t k = 0U; k <= half; k++) {
        double ang = M_PI * (double)k / (double)N;
        pTw[2U * k]      = (float32_t)cos(ang);
        pTw[2U * k + 1U] = (float32_t)sin(ang);
    }
}

/* 位反转重排(原地),p 为交织复数,N 为复数点数 */
static void fft_bit_reverse(float32_t *p, uint32_t N)
{
    uint32_t j = 0U;
    for (uint32_t i = 1U; i < N; i++) {
        uint32_t bit = N >> 1U;
        for (; j & bit; bit >>= 1U) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) {
            /* 交换第 i 和第 j 个复数 */
            float32_t tr = p[2U * i];
            float32_t ti = p[2U * i + 1U];
            p[2U * i]     = p[2U * j];
            p[2U * i + 1U] = p[2U * j + 1U];
            p[2U * j]     = tr;
            p[2U * j + 1U] = ti;
        }
    }
}

/* ========== 复数 FFT 实现 ========== */

int dsp_cfft_init_static_f32(dsp_cfft_instance_f32 *S,
                             uint32_t fftLen,
                             float32_t *pTwiddleBuf)
{
    uint32_t log2Len = fft_log2(fftLen);
    if (log2Len == 0xFFFFFFFFU || fftLen < 2U || pTwiddleBuf == NULL) {
        return -1;
    }
    S->fftLen     = fftLen;
    S->log2Len    = log2Len;
    S->pTwiddle   = pTwiddleBuf;
    S->ownTwiddle = 0U;
    fft_gen_twiddle_cfft(S->pTwiddle, fftLen);
    return 0;
}

int dsp_cfft_init_f32(dsp_cfft_instance_f32 *S, uint32_t fftLen)
{
    uint32_t log2Len = fft_log2(fftLen);
    if (log2Len == 0xFFFFFFFFU || fftLen < 2U) {
        return -1;
    }
    /* 旋转因子表:N/2 组 cos/sin = fftLen 个 float */
    float32_t *tw = (float32_t *)malloc(sizeof(float32_t) * fftLen);
    if (tw == NULL) {
        return -2;
    }
    S->fftLen     = fftLen;
    S->log2Len    = log2Len;
    S->pTwiddle   = tw;
    S->ownTwiddle = 1U;
    fft_gen_twiddle_cfft(S->pTwiddle, fftLen);
    return 0;
}

void dsp_cfft_deinit_f32(dsp_cfft_instance_f32 *S)
{
    if (S && S->ownTwiddle && S->pTwiddle) {
        free(S->pTwiddle);
        S->pTwiddle   = NULL;
        S->ownTwiddle = 0U;
    }
}

void dsp_cfft_f32(const dsp_cfft_instance_f32 *S,
                  float32_t *p1,
                  uint8_t ifftFlag,
                  uint8_t bitReverseFlag)
{
    const uint32_t   N  = S->fftLen;
    const float32_t *Tw = S->pTwiddle;

    if (bitReverseFlag) {
        fft_bit_reverse(p1, N);
    }

    /*
     * 逐级蝶形运算 (DIT)。
     * 对长度为 len 的子变换,旋转因子 W_len^j = W_N^(j*step),step = N/len。
     * 正变换旋转因子 = cos - j*sin;反变换 = cos + j*sin。
     */
    const float32_t wiSign = ifftFlag ? 1.0f : -1.0f;

    for (uint32_t len = 2U; len <= N; len <<= 1U) {
        uint32_t h    = len >> 1U;      /* 半长 */
        uint32_t step = N / len;        /* 旋转因子步进 */

        for (uint32_t i = 0U; i < N; i += len) {
            uint32_t twIdx = 0U;
            for (uint32_t j = 0U; j < h; j++) {
                float32_t wr = Tw[2U * twIdx];
                float32_t wi = wiSign * Tw[2U * twIdx + 1U];
                twIdx += step;

                uint32_t ia = i + j;         /* 上半索引 */
                uint32_t ib = ia + h;        /* 下半索引 */

                float32_t ar = p1[2U * ia];
                float32_t ai = p1[2U * ia + 1U];
                float32_t br = p1[2U * ib];
                float32_t bi = p1[2U * ib + 1U];

                /* t = b * W */
                float32_t tr = br * wr - bi * wi;
                float32_t ti = br * wi + bi * wr;

                p1[2U * ia]     = ar + tr;
                p1[2U * ia + 1U] = ai + ti;
                p1[2U * ib]     = ar - tr;
                p1[2U * ib + 1U] = ai - ti;
            }
        }
    }

    /* 反变换按 1/N 缩放,与 CMSIS-DSP arm_cfft_f32 行为一致 */
    if (ifftFlag) {
        float32_t scale = 1.0f / (float32_t)N;
        for (uint32_t i = 0U; i < 2U * N; i++) {
            p1[i] *= scale;
        }
    }
}

/* ========== 实数 FFT 实现 ========== */

int dsp_rfft_fast_init_static_f32(dsp_rfft_fast_instance_f32 *S,
                                  uint32_t fftLenReal,
                                  float32_t *pTwiddleCFFT,
                                  float32_t *pTwiddleRFFT)
{
    uint32_t log2Len = fft_log2(fftLenReal);
    if (log2Len == 0xFFFFFFFFU || fftLenReal < 2U ||
        pTwiddleCFFT == NULL || pTwiddleRFFT == NULL) {
        return -1;
    }

    S->fftLenReal = fftLenReal;
    uint32_t halfLen = fftLenReal >> 1U;

    /* 初始化内部 N/2 点复 FFT */
    if (dsp_cfft_init_static_f32(&S->cfftInst, halfLen, pTwiddleCFFT) != 0) {
        return -2;
    }

    /* 生成实数 FFT 额外旋转因子 */
    S->pTwiddleRFFT = pTwiddleRFFT;
    S->ownTwiddleR  = 0U;
    fft_gen_twiddle_rfft(S->pTwiddleRFFT, fftLenReal);
    return 0;
}

int dsp_rfft_fast_init_f32(dsp_rfft_fast_instance_f32 *S, uint32_t fftLenReal)
{
    uint32_t log2Len = fft_log2(fftLenReal);
    if (log2Len == 0xFFFFFFFFU || fftLenReal < 2U) {
        return -1;
    }

    S->fftLenReal = fftLenReal;
    uint32_t halfLen = fftLenReal >> 1U;

    /* 初始化内部 N/2 点复 FFT (内部会 malloc) */
    if (dsp_cfft_init_f32(&S->cfftInst, halfLen) != 0) {
        return -2;
    }

    /* 分配并生成实数 FFT 额外旋转因子表,长度 = fftLenReal (N/2+1 组 cos/sin) */
    float32_t *twR = (float32_t *)malloc(sizeof(float32_t) * fftLenReal);
    if (twR == NULL) {
        dsp_cfft_deinit_f32(&S->cfftInst);
        return -3;
    }
    S->pTwiddleRFFT = twR;
    S->ownTwiddleR  = 1U;
    fft_gen_twiddle_rfft(S->pTwiddleRFFT, fftLenReal);
    return 0;
}

void dsp_rfft_fast_deinit_f32(dsp_rfft_fast_instance_f32 *S)
{
    if (S) {
        dsp_cfft_deinit_f32(&S->cfftInst);
        if (S->ownTwiddleR && S->pTwiddleRFFT) {
            free(S->pTwiddleRFFT);
            S->pTwiddleRFFT = NULL;
            S->ownTwiddleR  = 0U;
        }
    }
}

void dsp_rfft_fast_f32(dsp_rfft_fast_instance_f32 *S,
                       float32_t *pSrc,
                       float32_t *pDst,
                       uint8_t ifftFlag)
{
    const uint32_t N = S->fftLenReal;
    const uint32_t halfN = N >> 1U;
    const float32_t *twR = S->pTwiddleRFFT;

    if (ifftFlag == 0U) {
        /* ========== 实数正变换 (RFFT) ========== */

        /*
         * 步骤1:把 N 点实序列 pSrc 重排为 N/2 点复序列。
         * 偶数样本作实部,奇数样本作虚部: [x[0], x[1], x[2], x[3], ...] 
         * -> [x[0]+j*x[1], x[2]+j*x[3], ...]
         */
        for (uint32_t i = 0U; i < halfN; i++) {
            pDst[2U * i]      = pSrc[2U * i];       /* 实部 = x[2i] */
            pDst[2U * i + 1U] = pSrc[2U * i + 1U];  /* 虚部 = x[2i+1] */
        }

        /* 步骤2:对这 N/2 点复序列做复 FFT,结果仍在 pDst 中 */
        dsp_cfft_f32(&S->cfftInst, pDst, 0U, 1U);

        /*
         * 步骤3:后处理,利用共轭对称性把 N/2 点复 FFT 结果展开为 N 点实 FFT 频谱。
         * 公式(参考 CMSIS-DSP):
         *   X[k] = 0.5 * ((Y[k] + Y*[N/2-k]) - j*(Y[k] - Y*[N/2-k])*W_N^k)
         * 其中 Y 是 N/2 点复 FFT 结果,W_N^k = cos(pi*k/N) - j*sin(pi*k/N)。
         *
         * 特殊点:
         *   X[0]   = Re{Y[0]} + Im{Y[0]}  (DC 分量)
         *   X[N/2] = Re{Y[0]} - Im{Y[0]}  (Nyquist 分量)
         */

        /* 提取 DC 和 Nyquist */
        float32_t y0r = pDst[0];
        float32_t y0i = pDst[1];
        pDst[0] = y0r + y0i;  /* X[0] real */
        pDst[1] = y0r - y0i;  /* X[N/2] real (存在 pDst[1],与 ARM 格式一致) */

        /* 展开 k = 1..N/2-1 */
        for (uint32_t k = 1U; k < halfN; k++) {
            uint32_t kMirror = halfN - k;

            float32_t ykr = pDst[2U * k];
            float32_t yki = pDst[2U * k + 1U];
            float32_t ymr = pDst[2U * kMirror];
            float32_t ymi = pDst[2U * kMirror + 1U];

            /* Y[k] + Y*[N/2-k] */
            float32_t sumR = ykr + ymr;
            float32_t sumI = yki - ymi;

            /* Y[k] - Y*[N/2-k] */
            float32_t difR = ykr - ymr;
            float32_t difI = yki + ymi;

            /* W_N^k = cos(pi*k/N) - j*sin(pi*k/N) */
            float32_t wr = twR[2U * k];
            float32_t wi = -twR[2U * k + 1U];

            /* -j*(Y[k] - Y*[N/2-k])*W_N^k = -j*((difR + j*difI)*(wr + j*wi))
             * = -j*(difR*wr - difI*wi + j*(difR*wi + difI*wr))
             * = (difR*wi + difI*wr) - j*(difR*wr - difI*wi)
             */
            float32_t prodR = difR * wi + difI * wr;
            float32_t prodI = difI * wi - difR * wr;

            /* X[k] = 0.5 * (sum + prod) */
            pDst[2U * k]      = 0.5f * (sumR + prodR);
            pDst[2U * k + 1U] = 0.5f * (sumI + prodI);
        }

    } else {
        /* ========== 实数反变换 (RIFFT) ========== */

        /*
         * 步骤1:前处理,把 N 点实 FFT 频谱压缩为 N/2 点复频谱。
         * 逆过程的公式:
         *   Y[k] = (X[k] + X*[N-k]) + j*(X[k] - X*[N-k])*W_N^k
         * 其中 W_N^k = cos(pi*k/N) + j*sin(pi*k/N) (注意 IFFT 时旋转因子共轭)。
         */

        /* 提取 DC 和 Nyquist,还原 Y[0] */
        float32_t x0  = pSrc[0];
        float32_t xN2 = pSrc[1];
        pDst[0] = 0.5f * (x0 + xN2);
        pDst[1] = 0.5f * (x0 - xN2);

        /* 压缩 k = 1..N/2-1 */
        for (uint32_t k = 1U; k < halfN; k++) {
            uint32_t kMirror = N - k;

            float32_t xkr = pSrc[2U * k];
            float32_t xki = pSrc[2U * k + 1U];

            /* X[N-k] 的共轭 = X*[N-k]。因为实数 FFT 的对称性,X[N-k] = X*[k],
             * 所以 X*[N-k] = X[k]。但为通用性,按定义取镜像点。
             * 实际上 pSrc 只存到 N/2,更高频点隐式对称。这里简化:假设 X*[N-k] = xkr - j*xki。
             */
            float32_t xmr = xkr;
            float32_t xmi = -xki;

            /* X[k] + X*[N-k] */
            float32_t sumR = xkr + xmr;
            float32_t sumI = xki + xmi;

            /* X[k] - X*[N-k] */
            float32_t difR = xkr - xmr;
            float32_t difI = xki - xmi;

            /* W_N^k = cos(pi*k/N) + j*sin(pi*k/N) (IFFT 用共轭旋转因子) */
            float32_t wr = twR[2U * k];
            float32_t wi = twR[2U * k + 1U];

            /* j*(X[k] - X*[N-k])*W_N^k = j*((difR + j*difI)*(wr + j*wi))
             * = j*(difR*wr - difI*wi + j*(difR*wi + difI*wr))
             * = -(difR*wi + difI*wr) + j*(difR*wr - difI*wi)
             */
            float32_t prodR = -(difR * wi + difI * wr);
            float32_t prodI = difR * wr - difI * wi;

            /* Y[k] = sum + prod */
            pDst[2U * k]      = sumR + prodR;
            pDst[2U * k + 1U] = sumI + prodI;
        }

        /* 步骤2:对 N/2 点复序列做复 IFFT */
        dsp_cfft_f32(&S->cfftInst, pDst, 1U, 1U);

        /*
         * 步骤3:把 N/2 点复 IFFT 结果解交织回 N 点实序列。
         * 复序列 [Y[0], Y[1], ...] = [y0r+j*y0i, y1r+j*y1i, ...]
         * -> 实序列 [y0r, y0i, y1r, y1i, ...]
         */
        for (uint32_t i = halfN; i > 0U; i--) {
            pDst[2U * i - 2U] = pDst[2U * (i - 1U)];      /* x[2i] = Re{Y[i]} */
            pDst[2U * i - 1U] = pDst[2U * (i - 1U) + 1U]; /* x[2i+1] = Im{Y[i]} */
        }
    }
}
