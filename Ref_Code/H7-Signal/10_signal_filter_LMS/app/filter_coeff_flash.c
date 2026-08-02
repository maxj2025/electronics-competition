#include "filter_coeff_flash.h"


/*
 * pCoeffs 存放运行时使用的滤波器系数。
 *
 * 这里把它放到 SDRAM 段 .RAM_SDRAM 中。
 * 你需要确认 Keil scatter 文件中已经配置了 .RAM_SDRAM 段。
 *
 * 例如：
 *
 * RW_IRAM5 0xC0000000 UNINIT 0x02000000  {
 *     *(.RAM_SDRAM)
 *     *(.RAM_SDRAM*)
 * }
 */
__attribute__((section(".RAM_SDRAM"), zero_init))
float32_t pCoeffs[FILTER_COEFF_NUM];


/*
 * pState 是 CMSIS-DSP DF1 滤波器的状态缓存。
 *
 * 注意：
 * DF1 状态数组长度是：
 *
 *     4 * numStages
 *
 * 你这里 numStages = 5，所以 pState 长度是 20。
 */
__attribute__((section(".RAM_SDRAM"), zero_init))
float32_t pState[4 * FILTER_NUM_STAGES];


/*
 * CMSIS-DSP DF1 滤波器实例
 *
 * 后续处理数据时使用：
 *
 *     arm_biquad_cascade_df1_f32(&S, input, output, blockSize);
 */
arm_biquad_casd_df1_inst_f32 S;


/*
 * 二阶节数量
 *
 * 这里固定为 5。
 */
uint8_t numStages = FILTER_NUM_STAGES;


/*
 * 计算 CRC32
 *
 * 用于检查 QFlash 中保存的滤波器系数是否损坏。
 *
 * 写入时：
 *     先把 record.crc32 = 0；
 *     然后对整个 FilterCoeffRecord_t 计算 CRC；
 *     最后把计算结果写入 record.crc32。
 *
 * 读取时：
 *     先保存 record.crc32；
 *     再临时把 record.crc32 = 0；
 *     重新计算 CRC；
 *     和保存的 crc32 比较。
 */
static uint32_t FilterCoeff_CalcCrc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFFUL;

    for (uint32_t i = 0; i < len; i++)
    {
        crc ^= data[i];

        for (uint32_t j = 0; j < 8; j++)
        {
            if (crc & 1U)
            {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1;
            }
        }
    }

    return ~crc;
}


/*
 * 检查滤波器类型和截止频率是否合法
 *
 * 合法条件：
 *
 * 1. type 必须是 FILTER_TYPE_LPF 或 FILTER_TYPE_HPF；
 * 2. cutoff_hz 必须在 1000 ~ 50000 之间；
 * 3. cutoff_hz 必须是 1000 的整数倍。
 */
static uint8_t FilterCoeff_CheckParam(FilterType_t type, uint32_t cutoff_hz)
{
    if ((type != FILTER_TYPE_LPF) && (type != FILTER_TYPE_HPF))
    {
        return FILTER_ERR_TYPE;
    }

    if ((cutoff_hz < FILTER_CUTOFF_MIN_HZ) ||
        (cutoff_hz > FILTER_CUTOFF_MAX_HZ) ||
        ((cutoff_hz % FILTER_CUTOFF_STEP_HZ) != 0U))
    {
        return FILTER_ERR_CUTOFF;
    }

    return FILTER_OK;
}

uint32_t FilterCoeff_GetSampleRateByCutoff(uint32_t cutoff_hz)
{
    if (cutoff_hz >= FILTER_SAMPLE_RATE_SWITCH_HZ)
    {
        return FILTER_SAMPLE_RATE_HIGH_HZ;
    }

    return FILTER_SAMPLE_RATE_LOW_HZ;
}

/*
 * 根据滤波器类型和截止频率计算槽位编号
 *
 * 截止频率索引：
 *
 * 1kHz  -> 0
 * 2kHz  -> 1
 * 3kHz  -> 2
 * ...
 * 50kHz -> 49
 *
 * QFlash 槽位分布：
 *
 * LPF 1kHz  ~ LPF 50kHz  : slot 0  ~ 49
 * HPF 1kHz  ~ HPF 50kHz  : slot 50 ~ 99
 */
uint32_t FilterCoeff_GetSlotIndex(FilterType_t type, uint32_t cutoff_hz)
{
    uint32_t freq_index;

    freq_index = (cutoff_hz - FILTER_CUTOFF_MIN_HZ) / FILTER_CUTOFF_STEP_HZ;

    return ((uint32_t)type * FILTER_FREQ_NUM) + freq_index;
}


/*
 * 根据滤波器类型和截止频率计算 QFlash 地址
 *
 * 地址计算公式：
 *
 *     address = FILTER_FLASH_BASE_ADDR + slot_index * FILTER_FLASH_SLOT_SIZE
 *
 * 例如：
 *
 *     LPF 1kHz:
 *         slot = 0
 *         addr = 0x010000
 *
 *     HPF 1kHz:
 *         slot = 50
 *         addr = 0x010000 + 50 * 256 = 0x013200
 */
uint32_t FilterCoeff_GetFlashAddr(FilterType_t type, uint32_t cutoff_hz)
{
    return FILTER_FLASH_BASE_ADDR +
           FilterCoeff_GetSlotIndex(type, cutoff_hz) * FILTER_FLASH_SLOT_SIZE;
}


/*
 * 保存一组滤波器系数到 QFlash
 *
 * 使用方法示例：
 *
 *     FilterCoeff_Save(FILTER_TYPE_LPF, 1000, lpf_1k_coeffs);
 *     FilterCoeff_Save(FILTER_TYPE_HPF, 5000, hpf_5k_coeffs);
 *
 * 注意：
 * 1. 调用本函数前必须先调用 norflash_init()；
 * 2. coeffs 必须指向 25 个 float32_t 系数；
 * 3. 系数顺序必须符合 CMSIS-DSP DF1：
 *
 *        b0, b1, b2, a1, a2
 *
 * 4. 本函数内部调用 norflash_write()，你的 norflash_write() 已经带擦除处理。
 */
uint8_t FilterCoeff_Save(FilterType_t type, uint32_t cutoff_hz, const float32_t *coeffs)
{
    FilterCoeffRecord_t record;
    uint8_t ret;

    ret = FilterCoeff_CheckParam(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    if (coeffs == NULL)
    {
        return FILTER_ERR_PARAM;
    }

    /*
     * 先把整个结构体填充为 0xFF。
     *
     * QFlash 擦除后的默认值也是 0xFF。
     * reserved 区域填 0xFF，后续 CRC 也会包含 reserved。
     */
    memset(&record, 0xFF, sizeof(record));

    /*
     * 填充记录头信息。
     *
     * 这些字段用于读取时判断：
     * 读取到的数据是否是目标滤波器系数。
     */
    record.magic          = FILTER_FLASH_MAGIC;
    record.version        = FILTER_FLASH_VERSION;
    record.filter_type    = (uint32_t)type;
    record.cutoff_hz      = cutoff_hz;
    record.sample_rate_hz = FilterCoeff_GetSampleRateByCutoff(cutoff_hz);

    record.numStages      = FILTER_NUM_STAGES;
    record.coeff_num      = FILTER_COEFF_NUM;

    /*
     * 复制 25 个滤波器系数到记录中。
     */
    memcpy(record.coeffs, coeffs, sizeof(record.coeffs));

    /*
     * 计算 CRC。
     *
     * 注意：
     * 计算 CRC 前必须先把 crc32 字段清零，
     * 否则每次计算结果都会受原 crc32 值影响。
     */
    record.crc32 = 0;
    record.crc32 = FilterCoeff_CalcCrc32((const uint8_t *)&record,
                                         sizeof(FilterCoeffRecord_t));

    /*
     * 写入 QFlash。
     *
     * 写入地址由 type 和 cutoff_hz 自动计算。
     */
    norflash_write((uint8_t *)&record,
                   FilterCoeff_GetFlashAddr(type, cutoff_hz),
                   sizeof(record));

    return FILTER_OK;
}


/*
 * 从 QFlash 读取一组完整滤波器记录
 *
 * 本函数会做完整校验：
 *
 * 1. 参数是否合法；
 * 2. magic 是否正确；
 * 3. filter_type 是否匹配；
 * 4. cutoff_hz 是否匹配；
 * 5. numStages 是否等于 5；
 * 6. coeff_num 是否等于 25；
 * 7. CRC32 是否正确。
 *
 * 如果任意一步失败，会返回对应错误码。
 */
uint8_t FilterCoeff_ReadRecord(FilterType_t type, uint32_t cutoff_hz, FilterCoeffRecord_t *record)
{
    uint8_t ret;
    uint32_t crc_read;
    uint32_t crc_calc;

    ret = FilterCoeff_CheckParam(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    if (record == NULL)
    {
        return FILTER_ERR_PARAM;
    }

    /*
     * 从 QFlash 固定槽位读取 256 字节记录。
     */
    norflash_read((uint8_t *)record,
                  FilterCoeff_GetFlashAddr(type, cutoff_hz),
                  sizeof(FilterCoeffRecord_t));

    /*
     * 检查 magic。
     *
     * 如果这里失败，通常说明：
     * 1. 这个槽位还没有写入过数据；
     * 2. 地址不对；
     * 3. QFlash 数据损坏；
     * 4. QFlash 读取失败。
     */
    if (record->magic != FILTER_FLASH_MAGIC)
    {
        return FILTER_ERR_MAGIC;
    }

    if (record->filter_type != (uint32_t)type)
    {
        return FILTER_ERR_TYPE;
    }

    if (record->cutoff_hz != cutoff_hz)
    {
        return FILTER_ERR_CUTOFF;
    }

    if (record->numStages != FILTER_NUM_STAGES)
    {
        return FILTER_ERR_NUMSTAGE;
    }

    if (record->coeff_num != FILTER_COEFF_NUM)
    {
        return FILTER_ERR_NUMSTAGE;
    }

    /*
     * 校验 CRC。
     *
     * 读取出来的 crc32 先保存。
     * 然后把 record->crc32 临时清零，
     * 再对整个结构体重新计算 CRC。
     */
    crc_read = record->crc32;
    record->crc32 = 0;

    crc_calc = FilterCoeff_CalcCrc32((const uint8_t *)record,
                                     sizeof(FilterCoeffRecord_t));

    record->crc32 = crc_read;

    if (crc_read != crc_calc)
    {
        return FILTER_ERR_CRC;
    }

    return FILTER_OK;
}


/*
 * 从 QFlash 加载滤波器系数到 SDRAM
 *
 * 本函数只负责：
 *
 *     QFlash -> 临时 record -> SDRAM pCoeffs
 *
 * 不初始化 CMSIS-DSP 滤波器实例。
 *
 * 如果你只想检查或打印 pCoeffs，可以调用这个函数。
 */
uint8_t FilterCoeff_LoadToSDRAM(FilterType_t type, uint32_t cutoff_hz)
{
    FilterCoeffRecord_t record;
    uint8_t ret;

    ret = FilterCoeff_ReadRecord(type, cutoff_hz, &record);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    /*
     * 把读取到的 25 个系数复制到 SDRAM 中的 pCoeffs。
     */
    memcpy(pCoeffs, record.coeffs, sizeof(pCoeffs));

    return FILTER_OK;
}


/*
 * 从 QFlash 加载滤波器系数，并初始化 CMSIS-DSP DF1 滤波器
 *
 * 正式运行时推荐调用这个函数。
 *
 * 例如：
 *
 *     ret = FilterCoeff_InitBiquadFromQFlash(FILTER_TYPE_LPF, 10000);
 *
 * 表示：
 *
 *     从 QFlash 读取低通 10kHz 的滤波器系数，
 *     放入 SDRAM，
 *     然后初始化 arm_biquad_cascade_df1_f32 使用的实例 S。
 */
uint8_t FilterCoeff_InitBiquadFromQFlash(FilterType_t type, uint32_t cutoff_hz)
{
    uint8_t ret;

    /*
     * 先把指定滤波器系数加载到 SDRAM 的 pCoeffs 中。
     */
    ret = FilterCoeff_LoadToSDRAM(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    /*
     * 每次切换滤波器时，必须清空状态数组。
     *
     * 否则上一个滤波器的历史状态会影响新滤波器输出。
     */
    memset(pState, 0, sizeof(pState));

    /*
     * 初始化 CMSIS-DSP DF1 滤波器实例。
     *
     * 初始化完成后，就可以调用：
     *
     *     arm_biquad_cascade_df1_f32(&S, input, output, blockSize);
     */
    arm_biquad_cascade_df1_init_f32(&S,
                                    FILTER_NUM_STAGES,
                                    pCoeffs,
                                    pState);

    return FILTER_OK;
}

uint8_t FilterCoeff_EraseByFreq(FilterType_t type, uint32_t cutoff_hz)
{
    uint8_t ret;
    uint32_t addr;
    uint32_t sector_index;

    ret = FilterCoeff_CheckParam(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    addr = FilterCoeff_GetFlashAddr(type, cutoff_hz);
    sector_index = addr / 4096U;

    norflash_erase_sector(sector_index);

    return FILTER_OK;
}

uint8_t FilterCoeff_Update(FilterType_t type, uint32_t cutoff_hz, const float32_t *coeffs)
{
    uint8_t ret;
    FilterCoeffRecord_t record;

    ret = FilterCoeff_CheckParam(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    if (coeffs == NULL)
    {
        return FILTER_ERR_PARAM;
    }

    /* 擦除当前频率对应的扇区 */
    ret = FilterCoeff_EraseByFreq(type, cutoff_hz);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    /* 写入新系数 */
    ret = FilterCoeff_Save(type, cutoff_hz, coeffs);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    /* 读取并校验 record，包括 magic、type、cutoff、CRC */
    ret = FilterCoeff_ReadRecord(type, cutoff_hz, &record);
    if (ret != FILTER_OK)
    {
        return ret;
    }

    /* 再额外比较系数内容 */
    if (memcmp(record.coeffs, coeffs, sizeof(record.coeffs)) != 0)
    {
        return FILTER_ERR_VERIFY;
    }

    return FILTER_OK;
}

