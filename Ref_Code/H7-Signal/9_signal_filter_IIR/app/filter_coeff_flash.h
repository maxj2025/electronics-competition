#ifndef __FILTER_COEFF_FLASH_H
#define __FILTER_COEFF_FLASH_H

#include "main.h"
#include "arm_math.h"
#include "norflash.h"
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 滤波器类型定义
 *
 * FILTER_TYPE_LPF : Low Pass Filter，低通滤波器
 * FILTER_TYPE_HPF : High Pass Filter，高通滤波器
 *
 * 后续存储到 QFlash 时，会根据 type 区分低通和高通。
 */
typedef enum
{
    FILTER_TYPE_LPF = 0,
    FILTER_TYPE_HPF = 1
} FilterType_t;


/*
 * 滤波器基本参数
 *
 * 采样率固定为 1MHz。
 * 截止频率范围为 1kHz ~ 50kHz。
 * 步进为 1kHz。
 *
 * 也就是说每种滤波器类型有 50 组系数：
 * 1kHz, 2kHz, 3kHz, ... , 50kHz。
 */
#define FILTER_SAMPLE_RATE_LOW_HZ       1000000UL
#define FILTER_SAMPLE_RATE_HIGH_HZ      2200000UL
#define FILTER_SAMPLE_RATE_SWITCH_HZ    20000UL

#define FILTER_CUTOFF_MIN_HZ            1000UL
#define FILTER_CUTOFF_MAX_HZ            100000UL
#define FILTER_CUTOFF_STEP_HZ           1000UL
#define FILTER_SAMPLE_RATE_HZ           FILTER_SAMPLE_RATE_LOW_HZ



/*
 * CMSIS-DSP Biquad 级联滤波器参数
 *
 * numStages = 5，表示 5 个二阶节。
 *
 * 对于 arm_biquad_cascade_df1_f32：
 * 每个二阶节需要 5 个系数：
 *
 *     b0, b1, b2, a1, a2
 *
 * 所以总系数数量为：
 *
 *     5 stages * 5 coeffs/stage = 25 个 float32_t
 */
#define FILTER_NUM_STAGES              5U
#define FILTER_COEFF_PER_STAGE         5U
#define FILTER_COEFF_NUM               (FILTER_NUM_STAGES * FILTER_COEFF_PER_STAGE)


/*
 * 总滤波器数量
 *
 * 低通 50 组，高通 50 组。
 * 总共 100 组滤波器系数。
 */
#define FILTER_TYPE_NUM                2U
#define FILTER_FREQ_NUM                100U
#define FILTER_TOTAL_NUM               (FILTER_TYPE_NUM * FILTER_FREQ_NUM)


/*
 * QFlash 中滤波器系数表的起始地址
 *
 * 这里从 0x010000 开始存储。
 * W25Q128 容量是 16MB，这个地址一般足够安全。
 *
 * 注意：
 * 如果你的 QFlash 其他地方也存了参数、字库、图片、文件系统等数据，
 * 要避免地址冲突。
 */
#define FILTER_FLASH_BASE_ADDR         0x010000UL


/*
 * 每一组滤波器系数占用 256 字节
 *
 * 好处：
 * 1. 地址计算简单；
 * 2. 一组数据刚好按固定槽位存储；
 * 3. 后续扩展字段也方便；
 * 4. 写入时和 Flash Page 256 字节大小匹配。
 */
#define FILTER_FLASH_SLOT_SIZE         4096U


/*
 * magic 用于判断 QFlash 中该位置是否存放了有效滤波器系数。
 *
 * 0x46434F46 对应 ASCII：
 * 'F' 'C' 'O' 'F'
 *
 * 读取时如果 magic 不匹配，说明该槽位没有有效数据，
 * 或者数据已经损坏。
 */
#define FILTER_FLASH_MAGIC             0x46434F46UL


/*
 * 数据结构版本号
 *
 * 后续如果你修改了 FilterCoeffRecord_t 的格式，
 * 可以修改这个版本号，用于兼容旧数据。
 */
#define FILTER_FLASH_VERSION           0x00010000UL


/*
 * 函数返回值
 *
 * FILTER_OK            : 成功
 * FILTER_ERR_PARAM     : 参数错误，比如指针为空
 * FILTER_ERR_MAGIC     : magic 不匹配，说明 QFlash 中没有有效数据
 * FILTER_ERR_TYPE      : 滤波器类型错误
 * FILTER_ERR_CUTOFF    : 截止频率错误
 * FILTER_ERR_NUMSTAGE  : 二阶节数量或系数数量错误
 * FILTER_ERR_CRC       : CRC 校验失败，说明数据可能损坏
 */
#define FILTER_OK                      0U
#define FILTER_ERR_PARAM               1U
#define FILTER_ERR_MAGIC               2U
#define FILTER_ERR_TYPE                3U
#define FILTER_ERR_CUTOFF              4U
#define FILTER_ERR_NUMSTAGE            5U
#define FILTER_ERR_CRC                 6U
#define FILTER_ERR_VERIFY              10U
#define FILTER_ERR_ERASE               11U
#define FILTER_ERR_SAMPLE_RATE         12U



/*
 * QFlash 中每一组滤波器系数的数据结构
 *
 * 每个截止频率对应一个 FilterCoeffRecord_t。
 *
 * 数据结构内容：
 *
 * magic          : 数据有效标志
 * version        : 数据结构版本
 * filter_type    : 低通或高通
 * cutoff_hz      : 截止频率，例如 1000、2000、50000
 * sample_rate_hz : 采样率，这里固定为 1000000
 * numStages      : 二阶节数量，这里固定为 5
 * coeff_num      : 系数数量，这里固定为 25
 * coeffs         : CMSIS-DSP 使用的滤波器系数
 * crc32          : CRC32 校验值
 * reserved       : 预留空间，使整个结构体刚好 256 字节
 *
 * 注意：
 * coeffs 的顺序必须是 CMSIS-DSP DF1 格式：
 *
 *     b0, b1, b2, a1, a2
 *     b0, b1, b2, a1, a2
 *     ...
 *
 * 共 5 组二阶节。
 */
typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t filter_type;
    uint32_t cutoff_hz;
    uint32_t sample_rate_hz;
    uint32_t numStages;
    uint32_t coeff_num;
    float32_t coeffs[FILTER_COEFF_NUM];
    uint32_t crc32;
    uint8_t reserved[3964];
} FilterCoeffRecord_t;


/*
 * 编译期检查结构体大小
 *
 * 如果 FilterCoeffRecord_t 不是 4096 字节，编译会直接报错。
 * 这样可以避免 QFlash 地址槽位错乱。
 */
typedef char FilterCoeffRecord_Size_Check[
    (sizeof(FilterCoeffRecord_t) == FILTER_FLASH_SLOT_SIZE) ? 1 : -1
];


/*
 * SDRAM 中运行时使用的滤波器变量
 *
 * 注意：
 * 这里必须使用 extern。
 * 真正的变量定义放在 filter_coeff_flash.c 里面。
 *
 * pCoeffs:
 *     从 QFlash 读取出来的 25 个滤波器系数。
 *
 * pState:
 *     DF1 滤波器状态缓存。
 *     arm_biquad_cascade_df1_f32 要求状态数组长度为：
 *
 *         4 * numStages
 *
 *     所以这里是 4 * 5 = 20 个 float32_t。
 *
 * S:
 *     CMSIS-DSP 的 DF1 滤波器实例。
 *
 * numStages:
 *     二阶节数量，这里固定为 5。
 */
extern float32_t pCoeffs[FILTER_COEFF_NUM];
extern float32_t pState[4 * FILTER_NUM_STAGES];
extern arm_biquad_casd_df1_inst_f32 S;
extern uint8_t numStages;


/*
 * 根据滤波器类型和截止频率计算槽位编号
 *
 * 例如：
 * LPF 1kHz  -> slot 0
 * LPF 50kHz -> slot 49
 * HPF 1kHz  -> slot 50
 * HPF 50kHz -> slot 99
 */
//uint32_t FilterCoeff_GetSlotIndex(FilterType_t type, uint32_t cutoff_hz);


///*
// * 根据滤波器类型和截止频率计算 QFlash 地址
// */
//uint32_t FilterCoeff_GetFlashAddr(FilterType_t type, uint32_t cutoff_hz);

uint32_t FilterCoeff_GetSampleRateByCutoff(uint32_t cutoff_hz);

uint32_t FilterCoeff_GetSlotIndex(FilterType_t type, uint32_t cutoff_hz);

uint32_t FilterCoeff_GetFlashAddr(FilterType_t type, uint32_t cutoff_hz);


/**************************************************************************************************/
/*
 * 保存一组滤波器系数到 QFlash
 *
 * type:
 *     FILTER_TYPE_LPF 或 FILTER_TYPE_HPF
 *
 * cutoff_hz:
 *     1000 ~ 50000，必须是 1000 的整数倍
 *
 * coeffs:
 *     25 个 float32_t 系数，顺序为 CMSIS-DSP DF1 格式
 */
uint8_t FilterCoeff_Save(FilterType_t type, uint32_t cutoff_hz, const float32_t *coeffs);


/*
 * 从 QFlash 读取完整记录
 *
 * 这个函数会读取 FilterCoeffRecord_t，并检查：
 * magic、type、cutoff、numStages、coeff_num、CRC。
 */
uint8_t FilterCoeff_ReadRecord(FilterType_t type, uint32_t cutoff_hz, FilterCoeffRecord_t *record);


/*
 * 从 QFlash 读取滤波器系数，并复制到 SDRAM 中的 pCoeffs
 *
 * 这里只加载系数，不初始化 CMSIS-DSP 滤波器实例。
 */
uint8_t FilterCoeff_LoadToSDRAM(FilterType_t type, uint32_t cutoff_hz);


/*
 * 从 QFlash 读取系数到 SDRAM，并初始化 CMSIS-DSP DF1 滤波器
 *
 * 这个函数内部会执行：
 *
 * 1. 从 QFlash 读取指定滤波器系数；
 * 2. 校验数据；
 * 3. 拷贝到 pCoeffs；
 * 4. 清空 pState；
 * 5. 调用 arm_biquad_cascade_df1_init_f32 初始化滤波器。
 *
 * 正式运行时，推荐直接调用这个函数。
 */
uint8_t FilterCoeff_InitBiquadFromQFlash(FilterType_t type, uint32_t cutoff_hz);

uint8_t FilterCoeff_EraseByFreq(FilterType_t type, uint32_t cutoff_hz);
uint8_t FilterCoeff_Update(FilterType_t type, uint32_t cutoff_hz, const float32_t *coeffs);


#ifdef __cplusplus
}
#endif

#endif
