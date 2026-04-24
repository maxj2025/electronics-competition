/**
 ****************************************************************************************************
 * @file        LT768_w25qxx.h
 * @version     V1.0
 * @brief       LT7680模块 SPI FLASH(W25QXX) 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */	

#ifndef __W25QXX_H
#define __W25QXX_H

#include "./SYSTEM/sys/sys.h"
#include "./BSP/NORFLASH/norflash.h"


extern uint16_t g_spi_flash_type;    /* 定义FLASH芯片型号 */
 
/* 指令表 */
#define W25X_WriteEnable           0x06 
#define W25X_WriteDisable          0x04 
#define W25X_ReadStatusReg1        0x05 
#define W25X_ReadStatusReg2        0x35 
#define W25X_ReadStatusReg3        0x15 
#define W25X_WriteStatusReg1       0x01 
#define W25X_WriteStatusReg2       0x31 
#define W25X_WriteStatusReg3       0x11 
#define W25X_ReadData              0x03 
#define W25X_FastReadData          0x0B 
#define W25X_FastReadDual          0x3B 
#define W25X_FastReadQuad          0x6B  
#define W25X_FastReadDual_IO       0xBB 
#define W25X_FastReadQuad_IO       0xEB 
#define W25X_PageProgram           0x02 
#define W25X_PageProgramQuad       0x32 
#define W25X_BlockErase            0xD8 
#define W25X_SectorErase           0x20 
#define W25X_ChipErase             0xC7 
#define W25X_PowerDown             0xB9 
#define W25X_ReleasePowerDown      0xAB 
#define W25X_DeviceID              0xAB 
#define W25X_ManufactDeviceID      0x90 
#define W25X_JedecDeviceID         0x9F 
#define W25X_Enable4ByteAddr       0xB7
#define W25X_Exit4ByteAddr         0xE9
#define W25X_SetReadParam          0xC0 
#define W25X_EnterQPIMode          0x38
#define W25X_ExitQPIMode           0xFF

/******************************************************************************************/

/* 普通函数 */
void LT768_SPI_Init(uint8_t cs, uint8_t div);                                 /* 初始化LT768与SPI FLASH的连接配置 */

void spi_flash_init(void);                                                    /* 初始化25QXX */
uint16_t spi_flash_read_id(void);                                             /* 读取FLASH ID */
void spi_flash_write_enable(void);                                            /* 写使能 */
void spi_flash_write_disable(void);                                           /* 写保护 */
uint8_t spi_flash_read_sr(uint8_t regno);                                     /* 读取状态寄存器 */
void spi_flash_write_sr(uint8_t regno, uint8_t sr);                           /* 写状态寄存器 */

void spi_flash_wait_busy(void);                                               /* 等待空闲 */
void spi_flash_send_address(uint32_t address);                                /* 发送地址 */
void spi_flash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen);    /* 写入page */
void spi_flash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint32_t datalen); /* 写flash,不带擦除 */

void spi_flash_erase_chip(void);                                              /* 整片擦除 */
void spi_flash_erase_sector(uint32_t saddr);                                  /* 扇区擦除 */
void spi_flash_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen);          /* 读取flash */
void spi_flash_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen);         /* 写入flash */

#endif






