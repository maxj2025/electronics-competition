/**
 ****************************************************************************************************
 * @file        stmflash.c
 * @version     V1.0
 * @brief       STM32内部FLASH读写 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/STMFLASH/stmflash.h"


/**
 * @brief       从指定地址读取一个字 (32位数据)
 * @param       faddr   : 读取地址 (此地址必须为4的倍数!!)
 * @retval      读取到的数据 (32位)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的字(32位)数,即4个字节的整数倍
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;
  
    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr);    /* 读取4个字节(1个字) */
        raddr += 4;                             /* 偏移4个字节 */
    }
}

/**
 * @brief       获取某个地址所在的flash扇区
 * @param       faddr   : flash地址
 * @retval      0~7 ,addr所在的bank1扇区编号
 *              8~15,addr所在的bank2扇区编号+8,需要减去8,才得到bank2扇区编号
 */
uint8_t stmflash_get_flash_sector(uint32_t faddr)
{
    if (faddr < BANK1_FLASH_SECTOR_1)return 0;
    else if (faddr < BANK1_FLASH_SECTOR_2)return 1;
    else if (faddr < BANK1_FLASH_SECTOR_3)return 2;
    else if (faddr < BANK1_FLASH_SECTOR_4)return 3;
    else if (faddr < BANK1_FLASH_SECTOR_5)return 4;
    else if (faddr < BANK1_FLASH_SECTOR_6)return 5;
    else if (faddr < BANK1_FLASH_SECTOR_7)return 6;
    else if (faddr < BANK2_FLASH_SECTOR_0)return 7;
    else if (faddr < BANK2_FLASH_SECTOR_1)return 8;
    else if (faddr < BANK2_FLASH_SECTOR_2)return 9;
    else if (faddr < BANK2_FLASH_SECTOR_3)return 10;
    else if (faddr < BANK2_FLASH_SECTOR_4)return 11;
    else if (faddr < BANK2_FLASH_SECTOR_5)return 12;
    else if (faddr < BANK2_FLASH_SECTOR_6)return 13;
    else if (faddr < BANK2_FLASH_SECTOR_7)return 14;

    return 15;
}

/**
 * @brief       在FLASH 指定位置, 写入指定长度的数据(自动擦除)
 * @note        因为STM32H7的扇区实在太大,没办法本地保存扇区数据,所以本函数写地址如果非0XFF
 *              ,那么会先擦除整个扇区且不保存扇区数据.所以写非0XFF的地址,将导致整个扇区数据丢失.
 *              建议写之前确保扇区里没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
 * @param       waddr   : 起始地址 (此地址必须为32的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 字(32位)数(就是要写入的32位数据的个数,一次至少写入32字节,即8个字)
 * @retval      无
 */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{
    FLASH_EraseInitTypeDef FlashEraseInit;
    HAL_StatusTypeDef FlashStatus = HAL_OK;
    uint32_t SectorError = 0;
    uint32_t  addrx = 0;
    uint32_t  endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 32 ||       /* 写入地址小于 STM32_FLASH_BASE, 或不是32的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE))  /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }

    HAL_FLASH_Unlock();                         /* FLASH解锁 */
    addrx = waddr;                              /* 写入的起始地址 */
    endaddr = waddr + length * 4;               /* 写入的结束地址 */

    if (addrx < 0X1FF00000)                     /* 只有主存储区,才需要执行擦除操作!! */
    {
        while (addrx < endaddr)                 /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF)                        /* 有非0XFFFFFFFF的地方,要擦除这个扇区 */
            {
                FlashEraseInit.Banks = FLASH_BANK_1;
                FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;             /* 擦除类型，扇区擦除 */
                FlashEraseInit.Sector = stmflash_get_flash_sector(addrx);       /* 要擦除的扇区 */
                FlashEraseInit.NbSectors = 1;                                   /* 一次只擦除一个扇区 */
                FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;            /* 电压范围，设置Flash写/擦除操作使用32位并行位数 */
              
                if (HAL_FLASHEx_Erase(&FlashEraseInit, &SectorError) != HAL_OK)
                {
                    break;                      /* 发生错误了 */
                }

                SCB_CleanInvalidateDCache();    /* 清除无效的D-Cache */
            }
            else
            {
               addrx += 4;
            }
            
            FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1);         /* 等待上次操作完成 */
        }
    }
    
    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME, FLASH_BANK_1);   /* 等待上次操作完成 */
    
    if (FlashStatus == HAL_OK)
    {
        while (waddr < endaddr)                 /* 写入地址小于结束地址，还未写完 */
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, waddr, (uint64_t)pbuf) != HAL_OK)   /* 写入数据 */
            {
                break;                          /* 写入出错 */
            }
            
            waddr += 32;
            pbuf += 8;
        }
    }
    
    HAL_FLASH_Lock();                           /* FLASH上锁 */
}

/******************************************************************************************/
/* 测试用代码 */

/**
 * @brief       测试写数据(写1个字)
 * @param       waddr : 起始地址
 * @param       wdata : 要写入的数据
 * @retval      无
 */
void test_write(uint32_t waddr, uint32_t wdata)
{
    stmflash_write(waddr, &wdata, 1);       /* 写入一个字 */
}








