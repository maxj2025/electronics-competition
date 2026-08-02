#ifndef __LONG_FFT_H
#define __LONG_FFT_H

#include <stdint.h>

/* 与 CMSIS-DSP 保持一致的类型别名,若已包含 arm_math.h 可删掉这一行 */
typedef float float32_t;

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 复数 FFT 实例结构,类比 arm_cfft_instance_f32。
 * pTwiddle 保存 N/2 个旋转因子 (cos, sin) 交织存放。
 */
typedef struct {
    uint32_t   fftLen;      /* 变换长度,必须是 2 的幂 (如 8192、16384、32768) */
    uint32_t   log2Len;     /* log2(fftLen) */
    float32_t *pTwiddle;    /* 旋转因子表,长度 = fftLen (N/2 组 cos/sin) */
    uint8_t    ownTwiddle;  /* 是否由 init 内部 malloc,销毁时用 */
} dsp_cfft_instance_f32;

/*
 * 实数 FFT 实例结构,类比 arm_rfft_fast_instance_f32。
 * 内部持有一个 N/2 点的复 FFT 实例,外加实数 FFT 所需的额外旋转因子表。
 */
typedef struct {
    uint32_t               fftLenReal;  /* 实数序列长度 N,必须是 2 的幂 */
    dsp_cfft_instance_f32  cfftInst;    /* 内部 N/2 点复 FFT 实例 */
    float32_t             *pTwiddleRFFT;/* 实数 FFT 额外旋转因子,长度 = fftLenReal */
    uint8_t                ownTwiddleR; /* 是否由 init 内部 malloc */
} dsp_rfft_fast_instance_f32;

/* ========== 复数 FFT 接口 ========== */

/*
 * 初始化复数 FFT (内部 malloc 旋转因子表)。
 * 返回 0 成功,非 0 失败(长度非法或内存不足)。
 */
int dsp_cfft_init_f32(dsp_cfft_instance_f32 *S, uint32_t fftLen);

/*
 * 用外部提供的缓冲区初始化复数 FFT (不使用 malloc,适合无堆环境)。
 * pTwiddleBuf 长度必须 >= fftLen 个 float32_t。
 */
int dsp_cfft_init_static_f32(dsp_cfft_instance_f32 *S,
                             uint32_t fftLen,
                             float32_t *pTwiddleBuf);

/* 释放 init 分配的资源 */
void dsp_cfft_deinit_f32(dsp_cfft_instance_f32 *S);

/*
 * 复数 FFT/IFFT,原地运算,与 arm_cfft_f32 用法一致:
 *   p1            : 交织复数缓冲区,长度 2*fftLen 个 float32_t
 *   ifftFlag      : 0 = 正变换,1 = 反变换(反变换按 1/N 缩放)
 *   bitReverseFlag: 1 = 输出为自然顺序(推荐);0 = 保留位反转顺序
 */
void dsp_cfft_f32(const dsp_cfft_instance_f32 *S,
                  float32_t *p1,
                  uint8_t ifftFlag,
                  uint8_t bitReverseFlag);

/* ========== 实数 FFT 接口 ========== */

/*
 * 初始化实数 FFT (内部 malloc 旋转因子表)。
 * fftLenReal 为实数序列长度,必须是 2 的幂 (如 8192、16384、32768)。
 * 返回 0 成功,非 0 失败。
 */
int dsp_rfft_fast_init_f32(dsp_rfft_fast_instance_f32 *S, uint32_t fftLenReal);

/*
 * 用外部提供的缓冲区初始化实数 FFT (不使用 malloc)。
 * pTwiddleCFFT 用于内部 N/2 点复 FFT,长度 >= fftLenReal/2。
 * pTwiddleRFFT 用于实数 FFT 后处理,长度 >= fftLenReal。
 */
int dsp_rfft_fast_init_static_f32(dsp_rfft_fast_instance_f32 *S,
                                  uint32_t fftLenReal,
                                  float32_t *pTwiddleCFFT,
                                  float32_t *pTwiddleRFFT);

/* 释放 init 分配的资源 */
void dsp_rfft_fast_deinit_f32(dsp_rfft_fast_instance_f32 *S);

/*
 * 实数正变换 (R FFT),类比 arm_rfft_fast_f32。
 * pSrc: 输入实数序列,长度 fftLenReal。
 * pDst: 输出复频谱,长度 fftLenReal,交织格式 [DC_real, Nyq_real, X[1]_real, X[1]_imag, ...],
 *       与 ARM CMSIS-DSP 的 arm_rfft_fast_f32 输出格式一致。
 * 注意:pSrc 和 pDst 可以是同一缓冲区(原地运算)。
 */
void dsp_rfft_fast_f32(dsp_rfft_fast_instance_f32 *S,
                       float32_t *pSrc,
                       float32_t *pDst,
                       uint8_t ifftFlag);

#ifdef __cplusplus
}
#endif

#endif /* __DSP_FFT_H */




//#define N 16384U

//static dsp_cfft_instance_f32 S;
//static float32_t fftData[2U * N];
//static float32_t twiddle[N];

//void FFT_Init(void)
//{
//    if (dsp_cfft_init_static_f32(&S, N, twiddle) != 0) {
//        Error_Handler();
//    }
//}

//void FFT_Process(void)
//{
//    dsp_cfft_f32(&S, fftData, 0U, 1U);
//}

//#define N 16384U

//static dsp_rfft_fast_instance_f32 S;

//static float32_t input[N];
//static float32_t spectrum[N];

//static float32_t cfftTwiddle[N / 2U];
//static float32_t rfftTwiddle[N + 2U];

//void RFFT_Init(void)
//{
//    if (dsp_rfft_fast_init_static_f32(
//            &S,
//            N,
//            cfftTwiddle,
//            rfftTwiddle) != 0) {
//        Error_Handler();
//    }
//}

//void RFFT_Process(void)
//{
//    dsp_rfft_fast_f32(
//        &S,
//        input,
//        spectrum,
//        0U
//    );
//}
