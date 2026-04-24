#include "FDC2214.h"

// 引用外部定义的 I2C 句柄，用于硬件通信
extern I2C_HandleTypeDef hi2c1;

/**
 * @brief  FDC2214 芯片初始化
 * @return uint8_t 0: 初始化成功; 1: 初始化失败 (ID读取错误)
 */
uint8_t FDC2214_Init(void)
{
    uint8_t i = 0;
    uint8_t cof[2];          // 用于存储准备写入寄存器的 16 位数据（2个字节）
    volatile uint16_t check[2]; // 用于存储读取到的 ID 进行校验
    
    // --- 配置各通道的 RCOUNT (参考计数器) ---
    // 设置转换时间，0xFFFF 为最大值，转换时间越长精度越高
    cof[0] = 0xFF; cof[1] = 0xFF;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, RCOUNT_CH0, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, RCOUNT_CH1, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, RCOUNT_CH2, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, RCOUNT_CH3, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // --- 配置各通道的 OFFSET (偏移量) ---
    // 初始设置为 0
    cof[0] = 0x00; cof[1] = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, OFFSET_CH0, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, OFFSET_CH1, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, OFFSET_CH2, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, OFFSET_CH3, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // --- 配置各通道的 SETTLECOUNT (稳定计数器) ---
    // 设置 LC 振荡器稳定时间
    cof[0] = 0x04; cof[1] = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, SETTLECOUNT_CH0, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, SETTLECOUNT_CH1, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, SETTLECOUNT_CH2, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, SETTLECOUNT_CH3, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // --- 配置各通道的 CLOCK_DIVIDERS (时钟分频器) ---
    // 设置传感器时钟分频
    cof[0] = 0x10; cof[1] = 0x01;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, CLOCK_DIVIDERS_C_CH0, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, CLOCK_DIVIDERS_C_CH1, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, CLOCK_DIVIDERS_C_CH2, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, CLOCK_DIVIDERS_C_CH3, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // 设置错误配置寄存器
    cof[0] = 0x00; cof[1] = 0x01;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, ERROR_CONFIG, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // 设置主配置寄存器 (选择时钟源、使能通道等)
    cof[0] = 0x1e; cof[1] = 0x01;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, CONFIG, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // 设置多路复用配置 (通道扫描序列及带宽)
    cof[0] = 0xC2; cof[1] = 0x0C;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, MUX_CONFIG, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // 软件复位设备
    cof[0] = 0x00; cof[1] = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, RESET_DEV, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // --- 设置各通道的驱动电流 ---
    // 0x8C40 和 0x8800 用于初始化各通道的传感器激励电流
    cof[0] = 0x8c; cof[1] = 0x40;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, DRIVE_CURRENT_CH0, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, DRIVE_CURRENT_CH1, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    cof[0] = 0x88; cof[1] = 0x00;
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, DRIVE_CURRENT_CH2, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    HAL_I2C_Mem_Write(&hi2c1, FDC2214, DRIVE_CURRENT_CH3, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);

    // 读取厂家 ID 并进行字节组合 (高位左移8位 | 低位)
    HAL_I2C_Mem_Read(&hi2c1, FDC2214, MANUFACTURER_ID, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    check[0] = cof[1] | cof[0] << 8;

    // 读取设备 ID 并进行字节组合
    HAL_I2C_Mem_Read(&hi2c1, FDC2214, DEVICE_ID, I2C_MEMADD_SIZE_8BIT, cof, 2, 100);
    HAL_Delay(1);
    check[1] = cof[1] | cof[0] << 8;

    // 校验厂家 ID 和设备 ID 是否匹配预设值
    if ((check[0] == MANUFACTURER_ID_val) && (check[1] == DEVICE_ID_val))
        return 0;   // 成功
    else
        return 1;   // 失败
}

/**
 * @brief  获取指定通道的原始电容转换数据 (28位)
 * @param  channel: 通道号 (0-3)
 * @return uint32_t: 28位原始数据
 */
uint32_t FDC2214_GetCapacitanceData(uint8_t channel)
{
    volatile uint32_t data;
    uint8_t i = 0;
    uint8_t pdata[4]; // 存储 MSB(2字节) 和 LSB(2字节)
    
    switch (channel)
    {
    case 0:
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_MSB_CH0, I2C_MEMADD_SIZE_8BIT, pdata, 2, 100);
        HAL_Delay(1);
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_LSB_CH0, I2C_MEMADD_SIZE_8BIT, pdata + 2, 2, 100);
        HAL_Delay(1);
        // 拼接4字节数据并取低28位
        data = ((pdata[0] << 24 | pdata[1] << 16 | pdata[2] << 8 | pdata[3]) & 0x0fffffff);
        return data;
    case 1:
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_MSB_CH1, I2C_MEMADD_SIZE_8BIT, pdata, 2, 100);
        HAL_Delay(1);
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_LSB_CH1, I2C_MEMADD_SIZE_8BIT, pdata + 2, 2, 100);
        HAL_Delay(1);
        data = ((pdata[0] << 24 | pdata[1] << 16 | pdata[2] << 8 | pdata[3]) & 0x0fffffff);
        return data;
    case 2:
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_MSB_CH2, I2C_MEMADD_SIZE_8BIT, pdata, 2, 100);
        HAL_Delay(1);
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_LSB_CH2, I2C_MEMADD_SIZE_8BIT, pdata + 2, 2, 100);
        HAL_Delay(1);
        data = ((pdata[0] << 24 | pdata[1] << 16 | pdata[2] << 8 | pdata[3]) & 0x0fffffff);
        return data;
    case 3:
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_MSB_CH3, I2C_MEMADD_SIZE_8BIT, pdata, 2, 100);
        HAL_Delay(1);
        HAL_I2C_Mem_Read(&hi2c1, FDC2214, DATA_LSB_CH3, I2C_MEMADD_SIZE_8BIT, pdata + 2, 2, 100);
        HAL_Delay(1);
        data = ((pdata[0] << 24 | pdata[1] << 16 | pdata[2] << 8 | pdata[3]) & 0x0fffffff);
        return data;
    default:
        return 0;
    }
}

/**
 * @brief  将原始数据计算为实际电容值 (pF)
 * @param  channel: 通道号 (0-3)
 * @return double: 计算后的电容值 (pF)
 */
double FDC2214_pf(uint8_t channel) {
    // 获取 28 位原始频率转换数据
    uint32_t raw_data = FDC2214_GetCapacitanceData(channel);
    
    // 计算传感器振荡频率 f_sensor = (Data * F_REF) / 2^28
    double f_sensor = ((double)raw_data * F_REF) / pow(2, 28);
    
    // 根据 LC 谐振公式 C = 1 / (L * (2 * pi * f)^2) 计算总电容
    double c_total = 1.0 / (L_SENSOR * pow(2 * PI * f_sensor, 2));
    
    // 减去电路板寄生电容 C_BOARD，转换为 pF 并进行微调偏移 (-17.5f)
    double c_pf = (c_total - C_BOARD) * 1e12 - 17.5f;
    
    return c_pf;
}