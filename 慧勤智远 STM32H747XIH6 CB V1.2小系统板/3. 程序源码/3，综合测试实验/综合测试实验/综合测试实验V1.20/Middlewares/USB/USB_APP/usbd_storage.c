/**
 ****************************************************************************************************
 * @file        usbd_storage.c
 * @version     V1.0
 * @brief       usbd_storage 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "usbd_storage.h"
#include "./BSP/NORFLASH/norflash.h"
#include "./BSP/SDMMC/sdmmc_sdcard.h"
#include "./BSP/SDNAND/sdmmc_sdnand.h"
#include "usb_app.h"


/* 文件系统在外部FLASH的起始地址
 * 我们定义SPI FLASH前25M给文件系统用, 所以地址从0开始
 */
#define USB_STORAGE_FLASH_BASE  0


/* 自己定义的一个标记USB状态的寄存器, 方便判断USB状态
 * bit0 : 表示电脑正在向磁盘写入数据
 * bit1 : 表示电脑正在从磁盘读出数据
 * bit2 : 磁盘写数据错误标志位
 * bit3 : 磁盘读数据错误标志位
 * bit4 : 1,表示电脑有轮询操作(表明连接还保持着)
 */
//volatile uint8_t g_usb_state_reg = 0;


/* USB Mass storage 标准查询数据(每个lun占36字节) */
const int8_t  STORAGE_Inquirydata[] =
{
    /* LUN 0 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    ' ', ' ', ' ', 'W', 'K', 'S', ' ', ' ', ' ', /* 9字节 */
    /* Product Identification */
    'N', 'O', 'R', ' ', 'F', 'l', 'a', 's', 'h', /* 15字节 */
    ' ', 'D', 'i', 's', 'k', ' ',
    /* Product Revision Level */
    '1', '.', '0', '0',                          /* 4字节 */

    /* LUN 1 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    ' ', ' ', ' ', 'W', 'K', 'S', ' ', ' ', ' ', /* 9字节 */
    /* Product Identification */
    'S', 'D', ' ', 'N', 'A', 'N', 'D', ' ',      /* 15字节 */
    'D', 'i', 's', 'k', ' ', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', '0',                          /* 4字节 */
    
    /* LUN 2 */
    0x00,
    0x80,
    0x02,
    0x02,
    (STANDARD_INQUIRY_DATA_LEN - 4),
    0x00,
    0x00,
    0x00,
    /* Vendor Identification */
    ' ', ' ', ' ', 'W', 'K', 'S', ' ', ' ', ' ', /* 9字节 */
    /* Product Identification */
    'S', 'D', ' ', 'C', 'a', 'r', 'd', ' ',      /* 15字节 */
    'D', 'i', 's', 'k', ' ', ' ', ' ',
    /* Product Revision Level */
    '1', '.', '0', '0',                          /* 4字节 */
};

/* Private function prototypes ----------------------------------------------- */
int8_t STORAGE_Init(uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);

USBD_StorageTypeDef USBD_DISK_fops =
{
    STORAGE_Init,
    STORAGE_GetCapacity,
    STORAGE_IsReady,
    STORAGE_IsWriteProtected,
    STORAGE_Read,
    STORAGE_Write,
    STORAGE_GetMaxLun,
    (int8_t *)STORAGE_Inquirydata,
};

/**
 * @brief       初始化存储设备
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @retval      操作结果
 *   @arg       0    , 成功
 *   @arg       其他 , 失败
 */
int8_t STORAGE_Init (uint8_t lun)
{
    uint8_t res = 0;

    switch (lun)
    {
        case 0: /* SPI FLASH */
            norflash_init();
            break;
  
        case 1: /* SD NAND */
            res = sdnand_init();
            break;
        
        case 2: /* SD卡 */
            res = sd_init();
            break;
    }

    return res;
}

/**
 * @brief       获取存储设备的容量和块大小
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @param       block_num  : 块数量(扇区数)
 * @param       block_size : 块大小(扇区大小)
 * @retval      操作结果
 *   @arg       0    , 成功
 *   @arg       其他 , 失败
 */
int8_t STORAGE_GetCapacity (uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    switch (lun)
    {
        case 0: /* SPI FLASH */
            *block_size = 512;
            *block_num = (25 * 1024 * 1024) / 512;  /* SPI FLASH的前25M字节,文件系统用 */
            break;

        case 1: /* SD NAND */
            *block_size = 512;
            *block_num = g_sdnand_info.CardCapacity / 512;
            break;
        
        case 2: /* SD卡 */
            *block_size = 512;
            *block_num = g_sd_card_info.CardCapacity / 512;
            break;
    }

    return 0;
}

/**
 * @brief       查看存储设备是否就绪
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @retval      就绪状态
 *   @arg       0    , 就绪
 *   @arg       其他 , 未就绪
 */
int8_t  STORAGE_IsReady (uint8_t lun)
{
    usbx.bDeviceState |= 0X10;     /* 标记轮询 */
  
    return 0;
}

/**
 * @brief       查看存储设备是否写保护
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @retval      写保护状态
 *   @arg       0    , 没有写保护
 *   @arg       其他 , 有写保护
 */
int8_t  STORAGE_IsWriteProtected (uint8_t lun)
{
    return  0;
}

/**
 * @brief       从存储设备读取数据
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @param       buf        : 数据存储区首地址指针
 * @param       blk_addr   : 要读取的地址(扇区地址)
 * @param       blk_len    : 要读取的块数(扇区数)
 * @retval      读取结果
 *   @arg       0    , 成功
 *   @arg       其他 , 失败
 */
int8_t STORAGE_Read (uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = 0;
    usbx.bDeviceState |= 0X02;     /* 标记正在读数据 */

    switch (lun)
    {
        case 0: /* SPI FLASH */
            norflash_read(buf, USB_STORAGE_FLASH_BASE + blk_addr * 512, blk_len * 512);
            break;

        case 1: /* SD NAND */
            res = sdnand_read_disk(buf, blk_addr, blk_len);
            break;
        
        case 2: /* SD卡 */
            res = sd_read_disk(buf, blk_addr, blk_len);
            break;
    }

    if (res)
    {
        printf("rerr:%d,%d", lun, res);
        usbx.bDeviceState |= 0X08;     /* 读错误! */
    }

    return res;
}

/**
 * @brief       向存储设备写数据
 * @param       lun        : 逻辑单元编号
 *   @arg                  0, SPI FLASH
 *   @arg                  1, SD NAND
 *   @arg                  2, SD卡
 * @param       buf        : 数据存储区首地址指针
 * @param       blk_addr   : 要写入的地址(扇区地址)
 * @param       blk_len    : 要写入的块数(扇区数)
 * @retval      写入结果
 *   @arg       0    , 成功
 *   @arg       其他 , 失败
 */
int8_t STORAGE_Write (uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    int8_t res = 0;
    usbx.bDeviceState |= 0X01;     /* 标记正在写数据 */

    switch (lun)
    {
        case 0: /* SPI FLASH */
            norflash_write(buf, USB_STORAGE_FLASH_BASE + blk_addr * 512, blk_len * 512);
            break;

        case 1: /* SD NAND */
            res = sdnand_write_disk(buf, blk_addr, blk_len);
            break;
        
        case 2: /* SD卡 */
            res = sd_write_disk(buf, blk_addr, blk_len);
            break;
    }

    if (res)
    {
        usbx.bDeviceState |= 0X04;     /* 写错误! */
        printf("werr:%d,%d", lun, res);
    }

    return res;
}

/**
 * @brief       获取支持的最大逻辑单元个数
 * @note        注意, 这里返回的逻辑单元个数是减去了1的.
 *              0, 就表示1个; 1, 表示2个; 以此类推
 * @param       无
 * @retval      支持的逻辑单元个数 - 1
 */
int8_t STORAGE_GetMaxLun (void)
{
    /* STORAGE_LUN_NBR 在usbd_conf.h里面定义, 表示支持的逻辑单元个数 */
    /* 判断SD卡是否正常 */
    if (g_sd_card_info.CardCapacity)   /* 如果SD卡正常, 则支持3个磁盘(NOR Flash、SD NAND、SD Card) */
    {
        return STORAGE_LUN_NBR - 1;
    }
    else                               /* SD卡不正常, 则支持2个磁盘(NOR Flash、SD NAND) */
    {
        return STORAGE_LUN_NBR - 2;
    }
}




