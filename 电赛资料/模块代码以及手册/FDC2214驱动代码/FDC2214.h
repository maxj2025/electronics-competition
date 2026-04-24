#ifndef __FDC2214_H__
#define __FDC2214_H__

/* 包含必要的底层库及配置文件 */
#include "stm32h7xx_hal.h"
#include "main.h"
#include "i2c.h"

/* --- 器件 I2C 地址定义 --- */
// FDC2214 默认 7 位地址为 0x2A，左移 1 位以符合 HAL 库的 8 位地址格式要求
#define FDC2214 0x2A << 1

/* --- 寄存器映射 (Register Map) --- */

// 数据寄存器：每个通道由 MSB (高位) 和 LSB (低位) 两个 16 位寄存器组成，共 28 位有效数据
#define DATA_MSB_CH0 0x00
#define DATA_LSB_CH0 0x01
#define DATA_MSB_CH1 0x02
#define DATA_LSB_CH1 0x03
#define DATA_MSB_CH2 0x04
#define DATA_LSB_CH2 0x05
#define DATA_MSB_CH3 0x06
#define DATA_LSB_CH3 0x07

// 参考计数器：用于设置每个通道的转换时间 (RCOUNT)
#define RCOUNT_CH0 0x08
#define RCOUNT_CH1 0x09
#define RCOUNT_CH2 0x0A
#define RCOUNT_CH3 0x0B

// 偏移量寄存器：用于电容测量的数值偏移校准 (OFFSET)
#define OFFSET_CH0 0x0C
#define OFFSET_CH1 0x0D
#define OFFSET_CH2 0x0E
#define OFFSET_CH3 0x0F

// 稳定时间计数器：设置传感器启动后的 LC 振荡稳定等待时间
#define SETTLECOUNT_CH0 0x10
#define SETTLECOUNT_CH1 0x11
#define SETTLECOUNT_CH2 0x12
#define SETTLECOUNT_CH3 0x13

// 时钟分频器：设置传感器驱动时钟的分频系数
#define CLOCK_DIVIDERS_C_CH0 0x14
#define CLOCK_DIVIDERS_C_CH1 0x15
#define CLOCK_DIVIDERS_C_CH2 0x16
#define CLOCK_DIVIDERS_C_CH3 0x17

// 状态与配置寄存器
#define STATUS 0x18         // 器件状态寄存器 (错误标志等)
#define ERROR_CONFIG 0x19   // 错误报告配置
#define CONFIG 0x1A         // 主配置寄存器 (时钟源、转换使能等)
#define MUX_CONFIG 0x1B     // 多路复用配置 (通道扫描顺序、带宽设置)
#define RESET_DEV 0x1C      // 设备复位寄存器

// 驱动电流设置：配置每个通道 LC 振荡电路的激励电流
#define DRIVE_CURRENT_CH0 0x1E
#define DRIVE_CURRENT_CH1 0x1F
#define DRIVE_CURRENT_CH2 0x20
#define DRIVE_CURRENT_CH3 0x21

// 器件标识寄存器
#define MANUFACTURER_ID 0x7E // 厂商 ID 寄存器地址
#define DEVICE_ID 0x7F       // 设备 ID 寄存器地址

/* --- 预设验证值与计算常量 --- */
#define MANUFACTURER_ID_val 0x5449 // TI 厂商 ID (十六进制为 'TI')
#define DEVICE_ID_val 0x3055       // FDC2214 的设备 ID

// 算法参数：需根据实际电路设计调整
#define F_REF 43400000.0   // 参考时钟频率 (Hz)，通常为外部晶振频率
#define L_SENSOR 0.000018  // 传感器端电感量 (H)，此处为 18uH
#define C_BOARD 33e-12     // 电路板及线路带来的寄生电容 (F)，此处为 33pF
#define PI    3.14159265358979f


uint8_t FDC2214_Init();
uint32_t FDC2214_GetCapacitanceData(uint8_t channel);
double FDC2214_pf(uint8_t channel);

#endif