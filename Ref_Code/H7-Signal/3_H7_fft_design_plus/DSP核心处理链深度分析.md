# DSP 核心处理链（Process_DSP_Block）深度分析

> 分析日期：2026-06-01
> 基于文件：`Core/Src/main.c`
> 目标芯片：STM32H723ZGT6 (Cortex-M7, 480MHz)

---

## 目录

1. [系统宏观架构](#1-系统宏观架构)
2. [硬件 DMA 乒乓缓冲机制](#2-硬件-dma-乒乓缓冲机制)
3. [Process_DSP_Block 逐步骤全景分析](#3-process_dsp_block-逐步骤全景分析)
   - [步骤 0：Cache 一致性维护](#步骤-0cache-一致性维护)
   - [步骤 1：去直流分量](#步骤-1去直流分量)
   - [步骤 2：加窗与 FFT 输入准备](#步骤-2加窗与-fft-输入准备)
   - [步骤 3：FFT 与幅度谱计算](#步骤-3fft-与幅度谱计算)
   - [步骤 4：双峰搜索与抛物线频率插值](#步骤-4双峰搜索与抛物线频率插值)
   - [步骤 5：精准子箱 DFT 测相/测幅](#步骤-5精准子箱-dft-测相测幅)
   - [步骤 6：频率分配（低→A，高→B）](#步骤-6频率分配低a高b)
   - [步骤 7：频率跳变检测与 EMA 平滑](#步骤-7频率跳变检测与-ema-平滑)
   - [步骤 8：PLL 相位误差计算](#步骤-8pll-相位误差计算)
   - [步骤 9：PLL PI 控制器与积分抗饱和](#步骤-9pll-pi-控制器与积分抗饱和)
   - [步骤 10：NCO 数字振荡器与连续相位合成](#步骤-10nco-数字振荡器与连续相位合成)
4. [PLL 锁相环控制理论解析](#4-pll-锁相环控制理论解析)
5. [相位连续性：核心保证机制](#5-相位连续性核心保证机制)
6. [时序与实时性分析](#6-时序与实时性分析)
7. [参数调优指南](#7-参数调优指南)

---

## 1. 系统宏观架构

### 1.1 系统功能一句话总结

> 输入两个频率的模拟正弦信号，经过 ADC 采样→FFT 分析→双路 PLL 锁相→NCO 合成→DAC 输出两路与输入**同频**但**固定相位偏移**（A路+90°，B路-45°）的正弦波形。

### 1.2 完整的硬件-软件数据流

```
┌─────────────────┐
│   信号发生器      │  双频正弦信号 (如 1kHz + 3kHz)
│  (外部输入)      │
└────────┬────────┘
         │ 模拟电压 (叠加后)
         ▼
┌─────────────────┐
│  ADC1 (16-bit)   │  TIM4 触发 @ 100kHz (每 10μs 一次)
│  GPIO 模拟输入   │
└────────┬────────┘
         │ DMA 自动搬运到 adc_dma_buf[4096]（RAM_D1, 512KB AXI SRAM）
         │ 循环模式 (circular), 半满/全满各 2048 点
         ▼
┌─────────────────────────────────────────────────────┐
│                 main() 主循环                         │
│                                                     │
│   adc_proc()  ─── 检查 ADC/DAC 乒乓状态              │
│        │                                            │
│        ▼                                            │
│   Process_DSP_Block(offset=0 或 offset=2048)        │
│        │                                            │
│        ├─ Cache Invalidate (使 DMA 数据对 CPU 可见)  │
│        ├─ 去直流                                     │
│        ├─ Hann 窗                                    │
│        ├─ 2048 点 CFFT                               │
│        ├─ 双峰搜索 + 抛物线插值 + 精准 DFT            │
│        ├─ PLL 锁相 (PI 控制)                          │
│        ├─ NCO 合成 2048 点正弦波                      │
│        └─ Cache Clean (使 CPU 数据对 DMA 可见)        │
└────────────────────┬────────────────────────────────┘
                     │ DMA 自动从 dac_A_dma_buf / dac_B_dma_buf 搬运
                     ▼
┌─────────────────────────────────────┐
│  DAC1 CH1 → 输出 A 路 (同频 +90°)   │
│  DAC1 CH2 → 输出 B 路 (同频 -45°)   │
│  12-bit, 中心值 2048                │
└─────────────────────────────────────┘
```

### 1.3 关键参数速查

| 参数 | 值 | 含义 |
|------|-----|------|
| `SAMPLE_RATE` | 100000.0 Hz | ADC/DAC 采样率 |
| `FFT_LENGTH` | 2048 | FFT 点数 |
| 频率分辨率 Δf | 100000/2048 ≈ 48.83 Hz | FFT bin 宽度 |
| 时间窗口 | 2048/100000 = 20.48 ms | 每块数据的时间长度 |
| `DAC_CENTER` | 2048 | DAC 12bit 中点电压 (~1.65V) |
| `PHASE_SHIFT_A` | +π/2 (+90°) | A 路输出相对于输入的固定相移 |
| `PHASE_SHIFT_B` | -π/4 (-45°) | B 路输出相对于输入的固定相移 |
| `PLL_KP` | 0.08 | 比例增益 |
| `PLL_KI` | 0.0005 | 积分增益 |
| `FREQ_ALPHA` | 0.03 | 频率 EMA 平滑系数 |
| `AMP_ALPHA` | 0.05 | 幅值 EMA 平滑系数 |
| `SIGNAL_THRESHOLD` | 20.0 | 断线/噪声检测门限 |
| `OUT_GAIN` | 0.8 | 输出增益（防削顶） |
| `PEAK_GUARD_BINS` | 16 | 寻峰保护区宽度（约 781Hz） |
| `DC_IGNORE_BINS` | 4 | 直流盲区（约 0~195Hz） |

---

## 2. 硬件 DMA 乒乓缓冲机制

### 2.1 为什么需要乒乓缓冲

DMA 在**循环模式**下会不断把 ADC 数据写入同一个缓冲区。CPU 在处理前一半数据时，DMA 正在写入后一半——如果 CPU 去读 DMA 正在写的位置，就会读到**半新半旧**的撕裂数据。乒乓缓冲解决了这个问题：

```
DMA 循环缓冲区 adc_dma_buf[4096]:

  [0 ───────── 2047] [2048 ───────── 4095]
       ↑ 前半 (half)       ↑ 后半 (full)

时间线：
  DMA: 写入 [0..2047] → 触发半满中断 → 写入 [2048..4095] → 触发全满中断 → 回到 [0..2047] → ...
  CPU:                   处理 [0..2047]                      处理 [2048..4095]
```

### 2.2 对应的中断回调

```c
/* ADC 半满中断：DMA 刚写完前 2048 个点 */
HAL_ADC_ConvHalfCpltCallback()  →  adc_half_ready = 1

/* ADC 全满中断：DMA 刚写完后 2048 个点 */
HAL_ADC_ConvCpltCallback()      →  adc_full_ready = 1

/* DAC CH1 半满/全满完成：DMA 刚发送完前/后半数据 */
HAL_DAC_ConvHalfCpltCallbackCh1() →  dac_half_free = 1
HAL_DAC_ConvCpltCallbackCh1()    →  dac_full_free = 1
```

### 2.3 调度逻辑（adc_proc）

```c
void adc_proc(void)
{
    // 关中断原子操作：检查 ADC 数据就绪 AND DAC 缓冲区空闲
    __disable_irq();
    if (adc_half_ready && dac_half_free) {
        adc_half_ready = 0; dac_half_free = 0; do_half = 1;
    }
    if (adc_full_ready && dac_full_free) {
        adc_full_ready = 0; dac_full_free = 0; do_full = 1;
    }
    __enable_irq();

    if (do_half) Process_DSP_Block(0);           // 处理前半
    if (do_full) Process_DSP_Block(FFT_LENGTH);  // 处理后半 (offset=2048)
}
```

CPU 必须在一个半区周期（20.48ms）内完成 `Process_DSP_Block` 的全部计算，否则就会丢帧。丢帧计数器 `adc_half_drop_count` / `adc_full_drop_count` 用于监控这一点。

---

## 3. Process_DSP_Block 逐步骤全景分析

```c
void Process_DSP_Block(uint32_t offset)
```
`offset` 为 0（处理前半）或 2048（处理后半）。

---

### 步骤 0：Cache 一致性维护

```c
DCache_Invalidate_By_Addr(&adc_dma_buf[offset], 2048 * sizeof(uint16_t));
```

**为什么需要：** Cortex-M7 有 D-Cache。DMA 直接把 ADC 数据写到物理内存（RAM_D1），但 CPU 的 Cache 里可能还保留着这块内存的**旧副本**。`Invalidate` 强制 CPU 下次读取时从物理内存重新加载。

**函数实现的精巧之处：**
```c
static void DCache_Invalidate_By_Addr(void *addr, uint32_t size)
{
    uint32_t start = (uint32_t)addr;
    uint32_t end   = start + size;
    start &= ~(32 - 1);                        // 向下对齐到 32 字节边界
    end = (end + 32 - 1) & ~(32 - 1);          // 向上对齐到 32 字节边界
    SCB_InvalidateDCache_by_Addr((uint32_t *)start, end - start);
}
```
Cache 操作必须以 **32 字节（Cache Line）** 为单位，所以函数自动做了对齐扩展。

---

### 步骤 1：去直流分量

```c
float32_t mean = 0.0f;
for (uint32_t i = 0; i < 2048; i++) {
    mean += (float32_t)adc_dma_buf[offset + i];
}
mean /= 2048.0f;
```

**原理：** ADC 采集的原始数据是 unsigned 16-bit，中心值约 32768（16-bit ADC）。`mean` 就是直流偏置的估计值。后续减去 `mean` 后，信号变成对称于 0 的有符号信号。

**数学本质：** 直流分量在频域对应 0Hz（DC bin），如果不消除，FFT 后 DC bin 会有一个巨大的峰值，干扰后续寻峰算法（尤其是低频信号的识别）。虽然代码中也用 `DC_IGNORE_BINS = 4` 强制清零了前 4 个 bin，但去直流从根本上减少了低频泄漏。

---

### 步骤 2：加窗与 FFT 输入准备

```c
for (uint32_t i = 0; i < 2048; i++) {
    float32_t x = (float32_t)adc_dma_buf[offset + i] - mean;
    fft_buffer[i * 2]     = x * window_buf[i];  // 实部 = 去直流数据 × Hann 窗
    fft_buffer[i * 2 + 1] = 0.0f;               // 虚部 = 0 (纯实数输入)
}
```

**为什么要加窗：** 2048 点截断一个连续的正弦波，相当于在时域乘以一个矩形窗。矩形窗在频域的旁瓣很高（仅比主瓣低 13dB），会导致**频谱泄漏**——一个强的正弦信号的能量会"污染"相邻频率 bin，假扮成另一个信号。

**Hann 窗（汉宁窗）：**
```
w[n] = 0.5 - 0.5 * cos(2πn / (N-1)),  n = 0, 1, ..., N-1
```

Hann 窗的第一旁瓣比主瓣低约 **31.5dB**，远优于矩形窗。代价是主瓣宽度加倍（频率分辨率略降）。窗函数的相干增益（coherent gain）在初始化时预计算：

```c
window_coherent_gain = (1/N) * Σ w[n]   // Hann 窗约 0.5
```

这个系数在后续 DFT 的幅度还原中用来补偿窗函数对幅值的衰减。

---

### 步骤 3：FFT 与幅度谱计算

```c
arm_cfft_f32(&fft_inst, fft_buffer, 0, 1);      // 正向 FFT
arm_cmplx_mag_f32(fft_buffer, mag_buffer, 1024); // 计算幅度谱
```

- `arm_cfft_f32`：CMSIS-DSP 的复数 FFT，原地计算（输入/输出共用 `fft_buffer`）
  - 参数 `0` = 正向 FFT，`1` = 位反转标志
  - 输出格式：`fft_buffer[0]` = DC 实部，`fft_buffer[1]` = DC 虚部，...
- `arm_cmplx_mag_f32`：计算 `sqrt(real² + imag²)` 得到 1024 个幅度值（只到 Nyquist 频率 = 50kHz）
- `mag_buffer[0..3]` 强制清零：屏蔽 DC~195Hz 的低频盲区

**FFT 输出解读：**
- `mag_buffer[k]` 代表频率 `k * 100000/2048 ≈ k * 48.83 Hz` 处的幅度
- `mag_buffer[20]` ≈ 977Hz 附近
- `mag_buffer[61]` ≈ 2979Hz 附近

---

### 步骤 4：双峰搜索与抛物线频率插值

#### 4.1 第一次寻峰

```c
arm_max_f32(mag_buffer, 1024, &max1_val, &idx1);
freq1 = estimate_freq_parabolic(idx1);
```

`arm_max_f32` 找到幅度谱中最大的值及其索引。注意此时 DC 附近已被清零。

#### 4.2 抛物线插值求精确频率

FFT 的频率分辨率只有 48.83Hz，直接使用 bin 索引计算频率精度不够。**抛物线插值**利用峰值 bin 及其左右邻居的幅度，估计真实的峰值位置（子箱精度）：

```c
static float32_t estimate_freq_parabolic(uint32_t idx)
{
    // 边界保护
    if (idx <= 1 || idx >= 1022) {
        return idx * 100000.0f / 2048.0f;  // 退化到 bin 中心频率
    }

    ym1 = mag_buffer[idx - 1];   // 左边幅度
    y0  = mag_buffer[idx];       // 峰值幅度
    yp1 = mag_buffer[idx + 1];   // 右边幅度

    denom = ym1 - 2*y0 + yp1;
    if (|denom| < 1e-20) delta = 0.0;
    else delta = 0.5 * (ym1 - yp1) / denom;

    // 钳制 delta 在 [-0.5, 0.5]
    if (delta > 0.5)  delta = 0.5;
    if (delta < -0.5) delta = -0.5;

    bin = idx + delta;  // 例如 bin 20.32
    return bin * 100000.0f / 2048.0f;  // 转换为 Hz
}
```

**数学原理：** 假设峰值附近三个点的幅度在 log 坐标下近似一条抛物线 `y = a(x - x_peak)² + c`。三个数据点 `(idx-1, ym1)`, `(idx, y0)`, `(idx+1, yp1)` 可以解得峰值偏移：
```
delta = 0.5 * (ym1 - yp1) / (ym1 - 2*y0 + yp1)
```
这就是抛物线顶点相对于 `idx` 的偏移量。

#### 4.3 频谱清零保护区

```c
// 将 idx1 左右各 16 个 bin 清零，防止频谱泄漏被当成第二个频率
for (int32_t i = idx1 - 16; i <= idx1 + 16; i++) {
    if (i > 0 && i < 1024) mag_buffer[i] = 0.0f;
}
```

保护区宽度 `16 * 48.83 = 781.3 Hz`。这意味着两个输入频率必须相距至少约 781Hz 才能被正确区分。如果两个频率太近（比如 1kHz 和 1.5kHz 只差 500Hz < 781Hz），第二个频率会被清零，导致 B 路锁不到信号。

#### 4.4 第二次寻峰

```c
arm_max_f32(mag_buffer, 1024, &max2_val, &idx2);
freq2 = estimate_freq_parabolic(idx2);
```

在清零后的幅度谱上寻找第二高的峰值。

---

### 步骤 5：精准子箱 DFT 测相/测幅

FFT 在频域给出的是离散频率点的复数结果。虽然我们已经用抛物线插值得到了比 bin 更精确的频率，但 FFT 并不能直接给出**任意频率**处的相位和幅度。所以这里改用**单点滑动 DFT（Goertzel 算法的简化版）**：

```c
// 先恢复 fft_buffer 为去直流但未加窗的原始数据
for (uint32_t i = 0; i < 2048; i++) {
    fft_buffer[i] = (float32_t)adc_dma_buf[offset + i] - mean;
}

// 对 freq1 做单点 DFT
measure_tone_by_dft(fft_buffer, freq1, &amp1, &ph1);
// 对 freq2 做单点 DFT
measure_tone_by_dft(fft_buffer, freq2, &amp2, &ph2);
```

**单点 DFT 的实现：**
```c
static void measure_tone_by_dft(const float32_t *x,   // 去直流数据
                                 float32_t freq,       // 感兴趣频率（Hz）
                                 float32_t *amp_out,   // 输出幅度
                                 float32_t *phase_out) // 输出相位
{
    float32_t omega = 2π * freq / SAMPLE_RATE;  // 数字角频率
    float32_t c_step = cos(omega);              // 每步 cos 旋转量
    float32_t s_step = sin(omega);              // 每步 sin 旋转量

    // 用递推生成 cos/sin 序列（避免每点都调用三角函数）
    float32_t c = 1.0f, s = 0.0f;  // 从 n=0 开始：cos(0)=1, sin(0)=0
    float32_t re = 0.0f, im = 0.0f;

    for (uint32_t n = 0; n < 2048; n++) {
        float32_t xn = x[n] * window_buf[n];     // 加窗后的采样值
        re += xn * c;                             // 与 cos(ωn) 相关
        im -= xn * s;                             // 与 -sin(ωn) 相关（注意负号）
        // 递推：cos(ω(n+1)) = cos(ωn)cos(ω) - sin(ωn)sin(ω)
        //       sin(ω(n+1)) = sin(ωn)cos(ω) + cos(ωn)sin(ω)
        float32_t next_c = c * c_step - s * s_step;
        float32_t next_s = s * c_step + c * s_step;
        c = next_c;
        s = next_s;
    }

    // 幅度归一化：除以 N × coherent_gain，乘以 2（双边谱→单边幅度）
    *amp_out = 2.0f * sqrt(re² + im²) / (2048 * window_coherent_gain);
    // 相位：atan2(im, re) 是余弦相位，加 π/2 转换为正弦相位
    *phase_out = wrap_to_pi(atan2f(im, re) + π/2);
}
```

**关键理解：**

1. **DFT 本质**：对于频率 `f`，计算采样序列与该频率的复指数 `e^{-jωn}` 的内积。`re` 对应与 cos 的相关，`im` 对应与 sin 的相关（注意代码中 `im -= xn * s` 正好对应 `e^{-jωn} = cos(ωn) - j*sin(ωn)`）。

2. **递推生成正弦/余弦**：利用三角恒等式 `cos(a+b) = cos(a)cos(b) - sin(a)sin(b)`，每步只需 2 次乘法 + 2 次加法，避免每点调用 `cosf()`/`sinf()`（在 2048 点 × 2 频率下能节省约 4096 次三角函数调用）。

3. **为什么返回的是正弦相位**：`atan2(im, re)` 给出的是相对于余弦波的相位（cos 相位为 0 对应信号在 n=0 时达到正峰值）。加 `π/2` 转换为正弦波相位（sin 相位为 0 对应信号在 n=0 时从 0 开始上升）。

4. **幅度还原**：除以 `N * coherent_gain` 补偿了窗函数对幅值的衰减，乘以 2 是因为只取了正频率（双边频谱的一半能量）。

**为什么需要 DFT 而不用 FFT 结果：**
- FFT 只能给出 bin 中心频率（48.83Hz 的整数倍）处的复数结果
- 如果真实频率是 1003.7Hz（落在 bin 20 和 21 之间），直接用 FFT bin 20 的结果会有显著的幅度和相位误差
- 单点 DFT 允许以**任意精度**的频率（已由抛物线插值得到）进行测量，给出准确的幅值和相位

---

### 步骤 6：频率分配（低→A，高→B）

```c
if (freq1 <= freq2) {
    freq_L = freq1; amp_L = amp1; ph_L = ph1;  // 低频 → A 通道
    freq_H = freq2; amp_H = amp2; ph_H = ph2;  // 高频 → B 通道
} else {
    freq_L = freq2; amp_L = amp2; ph_L = ph2;
    freq_H = freq1; amp_H = amp1; ph_H = ph1;
}
```

简单地将频率较低的分量分配给通道 A，较高的分配给通道 B。这个策略保证了即使信号发生器改变了频率组合，通道分配也始终保持一致。

---

### 步骤 7：频率跳变检测与 EMA 平滑

```c
// 通道 A（低频）
if (|freq_L - center_freq_A| > 100.0f) {
    center_freq_A = freq_L;      // 瞬间锁定：检测到频率跳变
    pll_int_A = 0.0f;            // 清积分历史（防止跑飞）
} else {
    center_freq_A += 0.03 * (freq_L - center_freq_A);  // EMA 平滑
}

// 通道 B（高频）同理
if (|freq_H - center_freq_B| > 100.0f) {
    center_freq_B = freq_H;
    pll_int_B = 0.0f;
} else {
    center_freq_B += 0.03 * (freq_H - center_freq_B);
}
```

**EMA（指数移动平均）原理：**
```
new_center = old_center + α * (measured - old_center)
           = α * measured + (1-α) * old_center
```
α = 0.03 意味着新测量值权重仅 3%，历史值权重 97%——这是一个**极强**的平滑，适合稳定运行中避免 FFT 噪声干扰。但当频率突变（>100Hz），瞬间锁定机制绕过平滑，快速响应。

**为什么需要清空积分（`pll_int = 0`）：** 积分项 `pll_int` 是对**历史相位误差的累积**。如果频率从 1kHz 跳变到 5kHz，旧的积分值还在基于旧的频率偏差计算，不清零会导致 PLL 失控。

**幅值平滑同理：**
```c
amp_A += 0.05 * (amp_L - amp_A);
amp_B += 0.05 * (amp_H - amp_B);
```
α = 0.05，比频率平滑快一些，因为幅度测量本身就比频率测量稳定。

---

### 步骤 8：PLL 相位误差计算

这是整个锁相环的**核心传感器**：

```c
err_A = wrap_to_pi( wrap_to_pi(ph_L + PHASE_SHIFT_A) - phase_A );
err_B = wrap_to_pi( wrap_to_pi(ph_H + PHASE_SHIFT_B) - phase_B );
```

**逐步解剖：**

1. **`ph_L`** 是 DFT 测得的输入信号当前相位（相对于 n=0 的正弦参考）

2. **`ph_L + PHASE_SHIFT_A`** = 输入相位 + 90°：这就是**目标输出相位**。我们想要 A 路输出比输入超前 90°

3. **`phase_A`** 是 NCO 当前的瞬时相位（即上一个 block 结束时输出相位停留的位置）

4. **差值** = 目标相位 - 当前相位 = "我还需要转多少才能对齐"

5. **双重 `wrap_to_pi`**：第一次将目标相位归一化到 [-π, π]，第二次将误差归一化到 [-π, π]。两个 wrap 是安全的——即使目标相位被 wrap 了，误差在 ±π 范围内的物理意义仍然是正确的。

**`wrap_to_pi` 实现：**
```c
static float32_t wrap_to_pi(float32_t x) {
    while (x >  π) x -= 2π;
    while (x < -π) x += 2π;
    return x;
}
```
将任意角度折叠到 [-π, π] 范围内。这是相位运算中最重要的工具函数——没有它，相位差可能被错误解读（比如差 350° 应该被理解为差 -10°）。

---

### 步骤 9：PLL PI 控制器与积分抗饱和

```c
/* 仅在信号足够强（非噪声）时驱动 PLL */
if (amp_L > 20.0f) {
    phase_A += PLL_KP * err_A;                              // 比例项：瞬时相位修正
    pll_int_A += PLL_KI * err_A * SAMPLE_RATE / (2π);       // 积分项：频率修正累积
} else {
    pll_int_A = 0.0f;  // 断线保护：清零积分
}

// 积分抗饱和 (Anti-windup)：限幅 ±50Hz
if (pll_int_A > 50.0f)  pll_int_A = 50.0f;
if (pll_int_A < -50.0f) pll_int_A = -50.0f;
```

**PI 控制器的数学形式：**

PLL 的输出 = ① FFT 追踪频率 + ② PI 修正

① `center_freq_A`：FFT 粗测 + EMA 平滑后的频率（大范围但可能有稳态误差）

② PI 控制器的输出 `pll_int_A`：
```
pll_int = KI * Σ(err) * Ts          (离散积分)
         └── KI * err * fs * Ts ──┘
         其中 Ts = 1/fs = 1/100000 = 10μs（采样周期）
```

实际代码中 `pll_int_A += KI * err * SAMPLE_RATE / (2π)`：
```
KI = 0.0005
err 单位：弧度
每个采样点的积分增量 = 0.0005 * err * 100000 / (2π) ≈ 7.96 * err
```

这意味着每个采样周期（10μs），积分项累积约 8×相位误差，2048 个采样周期（一个 block）约等于 20.48ms，可以累积出有意义的频率修正。

**比例项 `phase_A += KP * err_A`：**
- 直接在 NCO 相位上施加瞬时修正
- KP = 0.08 意味着 1 弧度的相位误差会立即让 NCO 相位跳 0.08 弧度
- 这个修正**不做频率调整**，是一次性的相位跳变
- 效果：快速对齐瞬时相位

**为什么 P 项直接改 phase 而 I 项存到 pll_int：**
- P 负责"快速拉回"，处理相位偏差的即刻纠正——像一个弹簧
- I 负责"持久纠偏"，慢慢把 NCO 运行频率微调到与输入完全一致——像一个积分器，补偿晶振频率误差

**积分抗饱和（Anti-windup）：**
- 如果长时间没有输入信号，积分项可能累积到极大值
- 一旦信号恢复，巨大的积分值会让 NCO 频率严重偏离，难以收敛
- 限幅 ±50Hz 确保 PLL 最多修正 50Hz 的误差（远超正常晶振 ppm 级别的偏差）
- 结合断线保护（amp < 20 时清零积分），双重安全

---

### 步骤 10：NCO 数字振荡器与连续相位合成

这是最终生成输出波形的环节，也是**相位连续性**的关键。

#### 10.1 NCO 运行频率的确定

```c
float32_t nco_freq_A = center_freq_A + pll_int_A;  // FFT粗频 + PLL微调
float32_t nco_freq_B = center_freq_B + pll_int_B;

step_A = 2π * nco_freq_A / SAMPLE_RATE;   // 每个采样点相位增量
step_B = 2π * nco_freq_B / SAMPLE_RATE;
```

例如：`center_freq_A = 1000Hz`, `pll_int_A = 0.5Hz` → `nco_freq_A = 1000.5Hz` → `step_A = 2π × 1000.5 / 100000 ≈ 0.06286 弧度/采样`

#### 10.2 逐点波形生成

```c
for (uint32_t i = 0; i < 2048; i++) {
    // 根据当前相位计算正弦值
    float32_t valA = 2048 + 0.8 * amp_A * sin(phase_A);
    float32_t valB = 2048 + 0.8 * amp_B * sin(phase_B);

    // 写入 DAC DMA 缓冲区（12-bit 饱和处理）
    dac_A_dma_buf[offset + i] = (uint16_t)__USAT((int32_t)valA, 12);
    dac_B_dma_buf[offset + i] = (uint16_t)__USAT((int32_t)valB, 12);

    // 相位累加（DDS 核心）
    phase_A += step_A;
    phase_B += step_B;

    // 相位归一化到 [0, 2π)
    if (phase_A >= 2π) phase_A -= 2π;
    if (phase_A < 0)   phase_A += 2π;
    // B 路同理
}
```

**这是 NCO（Numerically Controlled Oscillator）的经典 DDS（Direct Digital Synthesis）实现：**

```
phase[n+1] = phase[n] + step    (模 2π)
output[n]  = DAC_CENTER + GAIN * amplitude * sin(phase[n])
```

- `step = 2π * frequency / sample_rate`：每采样点的相位增量
- `phase` 是**持续累加的**，不会在 block 之间重置
- `phase` 是 `static` 全局变量，跨 block 保持

#### 10.3 相位连续性的保证机制

这是整个系统设计中最精妙的部分。让我们逐步分析：

**关键问题：如果 block 之间 NCO 频率变了（因为 PLL 做了修正），输出波形会不会出现相位跳变（断点）？**

**答案是：不会。原因如下：**

1. **`phase_A` 是持久状态变量，永不重置**
   ```c
   static float32_t phase_A = 0.0f;   // 全局 static，跨 block
   ```
   它不是每个 block 从头开始算的，而是从上一个 block 的最后一个采样点继续。

2. **频率变化通过改变步长而非重置相位来实现**
   当 PLL 修正了 `nco_freq_A`，只改变了 `step_A`。下一个采样点的相位是：
   ```
   phase_A = 上一次的phase_A + 新的step_A
   ```
   这是一个**一阶连续**的过程。相位永远不会跳变。

3. **P 项修正的含义再审视**
   ```c
   phase_A += PLL_KP * err_A;   // 在进入 for 循环之前执行一次
   ```
   这行代码在进入 2048 点生成循环**之前**对 phase_A 做了一个**微小的瞬时调整**。这个调整的量 `KP * err_A = 0.08 * err_A` 即使 err_A 达到最大值 π（180°），也就跳 0.08π ≈ 14.4°。这个微调在数学上等价于"PLL 锁相环的相位校正带宽内的平滑过渡"，听觉（或在示波器上）是不可感知的。

4. **频率缓慢变化而非突变**
   `center_freq` 经 EMA 平滑（α=0.03），`pll_int` 经积分缓慢累积（KI=0.0005），所以 `nco_freq` 的变化极其平缓。步长 `step` 在两个相邻采样点之间的变化微乎其微。

**数学证明：**

设 block N-1 结束时 phase = φ₀，block N 的步长从 s₁ 变为 s₂（因为 PLL 修正了频率）：
- block N-1 最后一点：`val = sin(φ₀)`，phase 变为 `φ₀ + s₁（但模2π后可能 wrap）`
- block N 第一点：`val = sin(φ₀')` 其中 `φ₀'` 就是 block N-1 结束时的 phase 值
- 两个 block 之间没有 phase 的"重置"或"跳变"

**但注意：** block N 开始前有一个 P 项修正：
```
φ₀' = φ₀_final + KP * err    （约 0~14° 的微调）
```
这是有意的、受控的修正——正是这个微调，让输出相位慢慢"追上"变化的输入相位。

**图示：**
```
时间 →
Block 0: [sin(φ₀) sin(φ₁) sin(φ₂) ... sin(φ₂₀₄₇)]
                                       ↓ phase 继续累加，不清零
Block 1: [sin(φ₂₀₄₈) sin(φ₂₀₄₉) ... sin(φ₄₀₉₅)]
                                       ↓
Block 2: [sin(φ₄₀₉₆) ...                    ]
```

phase 从 0 一直累加，经过成百上千个 block，形成一个**理论上永远连续**的正弦波相位轨迹。

#### 10.4 相位溢出处理

```c
if (phase_A >= 2π) phase_A -= 2π;
if (phase_A < 0)   phase_A += 2π;
```
`float32_t` 的精度在 2π × 10¹² 个周期后才会出现 ULP > 步长的问题，在 100kHz 采样率下需要数百年，根本不是问题。简单的条件减法足够。

#### 10.5 输出保护

```c
#define OUT_GAIN  0.8f
val = DAC_CENTER + OUT_GAIN * amp * sin(phase);
dac_buf[i] = (uint16_t)__USAT((int32_t)val, 12);
```

- `OUT_GAIN = 0.8`：即使输入信号峰值达到 ADC 满量程，输出也不会削顶
- `__USAT(val, 12)`：ARM 内建的无符号饱和指令，保证输出在 [0, 4095] 范围内
- `DAC_CENTER = 2048`：12-bit DAC 的中点值（约 1.65V）

---

## 4. PLL 锁相环控制理论解析

### 4.1 系统可以理解为两级 PLL

```
输入信号相位 θ_in
      │
      ▼
┌─────────────┐      ┌──────────────┐      ┌────────────┐
│ FFT + DFT   │─────▶│ 锁频环 (FLL)  │─────▶│ 锁相环 (PLL)│
│ 测频测相     │      │ freq→center  │      │ phase→NCO  │
└─────────────┘      └──────────────┘      └────────────┘
                           │                      │
                    粗大范围跟踪            精细相位锁定
                    频差 < 100Hz 慢跟        PI 控制器
                    频差 > 100Hz 瞬锁        ±50Hz 积分修偏
```

**FLL（锁频环）部分：** `center_freq` 跟踪 FFT 测量的大致频率，提供粗粒度的频率跟踪（精度约 48.8Hz bin + 抛物线插值 + EMA 平滑）

**PLL（锁相环）部分：** `pll_int` 提供细粒度的频率微调（±50Hz 范围），加上 `phase_A += KP*err` 的瞬时相位修正，完成精细的相位锁定

### 4.2 PLL 的传递函数（连续时间近似）

PLL 的相位修正量：
```
Δphase = KP * err + KI * ∫ err dt * (fs / 2π)
```

令 s-domain 中：
```
H(s) = θ_out / θ_in = (KP*s + KI*fs/(2π)) / (s² + KP*s + KI*fs/(2π))
```

这是一个二阶锁相环（Type II），有：
- **带宽** ≈ KP/2 = 0.04 rad/sample ≈ 637 Hz（相对 100kHz 采样率）
- **阻尼** ≈ KP / (2 * sqrt(KI * fs/(2π))) ≈ 1.0（临界阻尼，兼顾速度与稳定）

### 4.3 固定相移的实现

```
目标输出相位 = 输入相位 + PHASE_SHIFT

PHASE_SHIFT_A = +π/2  →  输出超前输入 90°
PHASE_SHIFT_B = -π/4  →  输出滞后输入 45°
```

误差计算：
```
err = (θ_in + PHASE_SHIFT) - θ_out
    = (θ_in + PHASE_SHIFT) - θ_out
```

当 PLL 锁定（err ≈ 0）：
```
θ_out ≈ θ_in + PHASE_SHIFT  ✓
```

---

## 5. 相位连续性：核心保证机制

### 5.1 跨 block 相位连续

一个常见误解是："每次 Process_DSP_Block 重新生成 2048 个点，相位会不会从头开始？"

**不会。** 原因已经在 3.10.3 节详细说明了，这里再强调核心点：

```c
static float32_t phase_A = 0.0f;   // ⬅ 全局持久状态
```

`phase_A` 和 `phase_B` 是文件作用域的 static 变量，**在整个程序的生命周期中只初始化一次**（在 `main()` 之前由 C 运行时初始化为 0）。每个 `Process_DSP_Block` 调用都从上次调用结束时的相位继续累加。

### 5.2 跨频率变化的相位连续

当输入信号从 1kHz 变为 5kHz 时：

1. FFT 在下一次测量时检测到 `|freq_L - center_freq_A| > 100Hz`
2. `center_freq_A` 瞬间更新为 5000Hz
3. `pll_int_A` 清零
4. `step_A` 从 `2π*1000/100000` 变为 `2π*5000/100000`
5. `phase_A` **不变**，仍然保持上一刻的值
6. 从下一采样点开始，波形以新的频率继续——

这个过渡在示波器上看不到任何"咔嚓"声或跳变。唯一的代价是**相位偏移可能短暂偏离目标的 +90°**，但 PLL 会在几个 block 内重新锁定。

### 5.3 P 项瞬时修正的影响

```c
phase_A += PLL_KP * err_A;   // 在 for 循环之前执行
```

这确实是相位的一次**不连续跳变**。但这个跳变的量级：

- `KP = 0.08`
- `err_A` 的最大有效范围经 `wrap_to_pi` 后为 [-π, π]（±180°）
- 单次最大跳变 = `0.08 × π = 0.251 弧度 ≈ 14.4°`

在实际正常运行中（信号稳定，PLL 接近锁定），`err_A` 只有几度甚至不到一度，跳变几乎为零。

此外，这个微跳变发生在**两个 block 之间的边界**，不在 block 内部的连续采样点之间。

---

## 6. 时序与实时性分析

### 6.1 关键时间参数

| 参数 | 值 | 计算 |
|------|-----|------|
| 采样周期 | 10 μs | 1/100kHz |
| 每个 block 时间 | 20.48 ms | 2048/100000 |
| DMA 传输时间 | ≈ 20.48 ms | 无需 CPU 介入 |
| CPU 处理时间 | **必须 < 20.48ms** | 否则丢帧 |
| PLL 积分时间常数 | ~2 秒 | 1/(KI×fs/2π) 数量级 |

### 6.2 计算量估算

`Process_DSP_Block` 中主要的计算开销：

| 操作 | 次数 | 每次开销 |
|------|------|---------|
| 去直流（加法） | 2048 | ~10ns (480MHz) |
| 加窗（乘法+赋值） | 2048×2 | ~20ns |
| 2048 点 CFFT | 1 | 约 100μs (CMSIS-DSP 优化) |
| 幅度谱 | 1024 | ~50μs (sqrt 较慢) |
| 抛物线插值 | 2 | ~100ns |
| 单点 DFT | 2048×2=4096 点 | ~150μs (三角递推) |
| NCO 生成 | 2048×2 路 | ~200μs |
| Cache 操作 | 2 次 | ~5μs |

**总计约 700-900 μs**，远小于 20.48ms 的时限。STM32H723 @ 480MHz 处理这个任务绰绰有余。

---

## 7. 参数调优指南

### 7.1 快速参考：症状 → 对策

| 症状 | 调什么 | 方向 |
|------|--------|------|
| 波形高频抖动 | `PLL_KP` | **减小** (如 0.04) |
| 频率变化后很久才对齐 | `FREQ_ALPHA` | **增大** (如 0.1) |
| 波形缓慢来回摆动 | `PLL_KI` | **减小** (如 0.0001) |
| 频率漂移长时间无法消除 | `PLL_KI` | **增大** (如 0.002) |
| 两个信号太近分不开 | `PEAK_GUARD_BINS` | **减小** (如 8) |
| 频谱泄漏导致误识别 | `PEAK_GUARD_BINS` | **增大** (如 24) |
| 极低频信号锁不住 | `DC_IGNORE_BINS` | **减小** (如 1) |
| 拔线后波形乱飞 | `SIGNAL_THRESHOLD` | **增大** (如 50) |
| 弱信号锁不住 | `SIGNAL_THRESHOLD` | **减小** (如 5) |
| 频率突变后长时间不稳定 | `PLL_KI` 积分限幅 | **减小**限幅值 |
| 波形幅度波动大 | `AMP_ALPHA` | **减小** (如 0.02) |
| 启动时幅度上升太慢 | `AMP_ALPHA` | **增大** (如 0.1) |

### 7.2 参数之间的关系

```
快速响应 ←─────────────────────────→ 稳定抗噪
   大 KP      小 KP
   大 KI      小 KI
   大 ALPHA   小 ALPHA
   小 guard   大 guard
```

实际调参时，建议一次只改一个参数，观察效果后再调下一个。

---

## 附录 A：内存布局总结

```
STM32H723ZG 内存域：

┌──────────────────────────────┐
│ ITCM (64KB @ 0x00000000)     │ ← Process_DSP_Block, estimate_freq_parabolic,
│                              │   measure_tone_by_dft, DCache_xxx, wrap_to_pi
├──────────────────────────────┤
│ DTCM (128KB @ 0x20000000)    │ ← fft_buffer[4096], mag_buffer[1024],
│                              │   window_buf[2048], FIR/IIR状态
├──────────────────────────────┤
│ RAM_D1 (512KB AXI @ 0x24000000)│ ← adc_dma_buf[4096], dac_A_dma_buf[4096],
│                              │   dac_B_dma_buf[4096], uart buffer
├──────────────────────────────┤
│ RAM_D2 (288KB @ 0x30000000)  │ ← 测试用
├──────────────────────────────┤
│ RAM_D3 (64KB @ 0x38000000)   │ ← blue_en, blue_PIN
├──────────────────────────────┤
│ SDRAM (32MB @ 0xC0000000)    │ ← 大缓冲（未在 DSP 中使用）
└──────────────────────────────┘
```

**设计思想：**
- 热代码 → ITCM（0 等待，双发射）
- 热数据（FFT 计算）→ DTCM（0 等待）
- DMA 数据 → RAM_D1（AXI，高带宽，可 Cache）
- 这样 DMA 和 CPU 计算各用各的总线，互不阻塞

---

## 附录 B：DSP 初始化

```c
void DSP_Process_Init(void)
{
    arm_cfft_init_f32(&fft_inst, 2048);    // 初始化 FFT 实例（查表）

    // 预计算 Hann 窗
    window_coherent_gain = 0.0f;
    for (uint32_t n = 0; n < 2048; n++) {
        window_buf[n] = 0.5 - 0.5 * cos(2π * n / 2047);
        window_coherent_gain += window_buf[n];
    }
    window_coherent_gain /= 2048;  // ≈ 0.5
}
```

Hann 窗只计算一次（上电初始化时），后续每个 block 直接查表使用，避免了重复计算三角函数的开销。

---

> **文档结束**
>
> 本文档旨在对 `Process_DSP_Block` 的每一行代码提供深度理解。如有疑问或需要进一步分析某个环节，可以在此基础上继续讨论。
