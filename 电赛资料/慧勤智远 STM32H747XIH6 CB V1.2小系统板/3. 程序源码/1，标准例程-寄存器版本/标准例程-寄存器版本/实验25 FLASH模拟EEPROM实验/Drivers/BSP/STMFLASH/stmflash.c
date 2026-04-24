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
 * @brief       解锁STM32的FLASH
 * @param       无
 * @retval      无
 */
static void stmflash_unlock(void)
{
    FLASH->KEYR1 = STM32_FLASH_KEY1;    /* Bank1 写入解锁序列. */
    FLASH->KEYR1 = STM32_FLASH_KEY2;
    
    FLASH->KEYR2 = STM32_FLASH_KEY1;    /* Bank2 写入解锁序列. */
    FLASH->KEYR2 = STM32_FLASH_KEY2;
}

/**
 * @brief       flash上锁
 * @param       无
 * @retval      无
 */
static void stmflash_lock(void)
{
    FLASH->CR1 |= 1 << 0;           /* Bank1,上锁 */
    FLASH->CR2 |= 1 << 0;           /* Bank2,上锁 */
}

/**
 * @brief       得到FLASH的错误状态
 * @param       bankx   : 0,获取bank1的状态
 *                        1,获取bank2的状态
 * @retval      执行结果
 *   @arg       0    : 操作已完成
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_get_error_status(uint8_t bankx)
{
    uint32_t res = 0;
    
    if (bankx == 0)
    {
        res = FLASH->SR1;           /* 读取Bank1状态寄存器 */
    }
    else
    {
        res = FLASH->SR2;           /* 读取Bank2状态寄存器 */
    }
    
    if (res & (1 << 17)) return 1;  /* WRPERR=1,写保护错误 */
    if (res & (1 << 18)) return 2;  /* PGSERR=1,编程序列错误 */
    if (res & (1 << 19)) return 3;  /* STRBERR=1,复写错误 */
    if (res & (1 << 21)) return 4;  /* INCERR=1,数据不一致错误 */
    if (res & (1 << 22)) return 5;  /* OPERR=1,写入/擦除错误 */
    if (res & (1 << 23)) return 6;  /* RDPERR=1,读保护错误 */
    if (res & (1 << 24)) return 7;  /* RDSERR=1,非法访问安全保护字错误 */
    if (res & (1 << 25)) return 8;  /* SNECCERR=1,ECC单校正错误 */
    if (res & (1 << 26)) return 9;  /* DBECCERR=1,ECC双重检测错误 */

    return 0;                       /* 没有任何错误状态/操作完成 */
}

/**
 * @brief       等待操作完成
 * @param       bankx   : 0,bank1; 1,bank2
 * @param       time    : 要等待的延时大小
 * @retval      执行结果
 *   @arg       0    : 已完成
 *   @arg       0XFF : 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_wait_done(uint8_t bankx, uint32_t time)
{
    uint8_t res = 0;
    uint32_t tempreg = 0;

    while (1)
    {
        if (bankx == 0) 
        {
            tempreg = FLASH->SR1;
        }
        else 
        {
            tempreg = FLASH->SR2;
        }
        
        if ((tempreg & 0X07) == 0)
        {
            break;  /* BSY=0,WBNE=0,QW=0,则操作完成 */
        }
        
        time--;

        if (time == 0)return 0XFF;  /* 等待超时 */
    }

    res = stmflash_get_error_status(bankx);

    if (res)
    {
        if (bankx == 0)  
        {
            FLASH->CCR1 = 0X07EE0000;  /* 清bank1所有错误标志位 */
        }
        else 
        {
            FLASH->CCR2 = 0X07EE0000;  /* 清bank2所有错误标志位 */
        }
    }

    return res;
}

/**
 * @brief       擦除扇区
 * @param       saddr   : 扇区地址所在的扇区编号,范围是:0~15.
 *                        0~7 ,addr所在的bank1扇区编号
 *                        8~15,addr所在的bank2扇区编号+8,需要减去8,才得到bank2扇区编号
 * @retval      执行结果
 *   @arg       0    : 已完成
 *   @arg       0XFF : 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_erase_sector(uint32_t saddr)
{
    uint8_t res = 0;
    res = stmflash_wait_done(saddr / 8, 0XFFFFFFFF);    /* 等待上次操作结束 */

    if (res == 0)
    {
        if (saddr < 8)  /* BANK1 擦除 */
        {
            FLASH->CR1 &= ~(7 << 8);            /* SNB1[2:0]=0,清除原来的设置 */
            FLASH->CR1 &= ~(3 << 4);            /* PSIZE1[1:0]=0,清除原来的设置 */
            FLASH->CR1 |= (uint32_t)saddr << 8; /* 设置要擦除的扇区编号,0~7 */
            FLASH->CR1 |= 2 << 4;               /* 设置编程使用的并行位数为32位 */
            FLASH->CR1 |= 1 << 2;               /* SER1=1,请求扇区擦除 */
            FLASH->CR1 |= 1 << 7;               /* START1=1,开始扇区擦除 */
        }
        else            /* BANK2 擦除 */
        {
            FLASH->CR2 &= ~(7 << 8);            /* SNB2[2:0]=0,清除原来的设置 */
            FLASH->CR2 &= ~(3 << 4);            /* PSIZE2[1:0]=0,清除原来的设置 */
            FLASH->CR2 |= (uint32_t)(saddr - 8) << 8;   /* 设置要擦除的扇区编号,0~7 */
            FLASH->CR2 |= 2 << 4;               /* 设置编程使用的并行位数为32位 */
            FLASH->CR2 |= 1 << 2;               /* SER2=1,请求扇区擦除 */
            FLASH->CR2 |= 1 << 7;               /* START2=1,开始扇区擦除 */
        }

        res = stmflash_wait_done(saddr / 8, 0XFFFFFFFF);    /* 等待操作结束 */

        if (saddr < 8)FLASH->CR1 &= ~(1 << 2);  /* SER1=0,清除扇区擦除标志 */
        else FLASH->CR2 &= ~(1 << 2);           /* SER2=0,清除扇区擦除标志 */
    }

    return res;
}

/**
 * @brief       在FLASH指定地址写8个字 (256位数据)
 * @note        必须以256bit为单位编程
 * @param       faddr   : 写入地址 (此地址必须为4的倍数!!)
 * @param       pdata   : 要写入的数据首地址（共8个字）
 * @retval      执行结果
 *   @arg       0    : 已完成
 *   @arg       0XFF : 超时
 *   @arg       其他 : 错误编号
 */
static uint8_t stmflash_write_8word(uint32_t faddr, uint32_t *pdata)
{
    uint8_t nword = 8;  /* 每次写8个字,256bit */
    uint8_t res;
    uint8_t bankx = 0;

    if (faddr < BANK2_FLASH_SECTOR_0)
    {
        bankx = 0;      /* 判断地址是在bank0,还是在bank1 */
    }
    else 
    {
        bankx = 1;
    }

    res = stmflash_wait_done(bankx, 0XFFFF);  /* 等待上次操作完成 */

    if (res == 0)       /* OK */
    {
        if (bankx == 0) /* BANK1 编程 */
        {
            FLASH->CR1 &= ~(3 << 4);    /* PSIZE1[1:0]=0,清除原来的设置 */
            FLASH->CR1 |= 2 << 4;       /* 设置编程使用的并行位数为32位 */
            FLASH->CR1 |= 1 << 1;       /* PG1=1,编程使能 */
        }
        else            /* BANK2 编程 */
        {
            FLASH->CR2 &= ~(3 << 4);    /* PSIZE2[1:0]=0,清除原来的设置 */
            FLASH->CR2 |= 2 << 4;       /* 设置编程使用的并行位数为32位 */
            FLASH->CR2 |= 1 << 1;       /* PG2=1,编程使能 */
        }

        while (nword)
        {
            *(volatile uint32_t *)faddr = *pdata;   /* 写入数据 */
            faddr += 4;                 /* 写地址偏移4个字节 */
            pdata++;                    /* 偏移到下一个数据首地址 */
            nword--;
        }

        __DSB();        /* 写操作完成后,屏蔽数据同步,使CPU重新执行指令序列 */
        res = stmflash_wait_done(bankx, 0XFFFF);/* 等待操作完成,一个字编程,最多100us. */

        if (bankx == 0)FLASH->CR1 &= ~(1 << 1); /* PG1=0,清除编程使能位 */
        else FLASH->CR2 &= ~(1 << 1);           /* PG2=0,清除编程使能位 */
    }

    return res;
}

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
 * @brief       获取某个地址所在的flash扇区
 * @param       faddr   : flash地址
 * @retval      0~7 ,addr所在的bank1扇区编号
 *              8~15,addr所在的bank2扇区编号+8,需要减去8,才得到bank2扇区编号
 */
static uint8_t stmflash_get_flash_sector(uint32_t faddr)
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
    uint8_t status = 0;
    uint32_t addrx = 0;
    uint32_t endaddr = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 32 ||       /* 写入地址小于 STM32_FLASH_BASE, 或不是32的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE))  /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }
    
    stmflash_unlock();              /* FLASH解锁 */
    
    addrx = waddr;                  /* 写入的起始地址 */
    endaddr = waddr + length * 4;   /* 写入的结束地址 */

    if (addrx < 0X1FF00000)         /* 只有主存储区,才需要执行擦除操作!! */
    {
        while (addrx < endaddr)     /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF)    /* 有非0XFFFFFFFF的地方,要擦除这个扇区 */
            {
                status = stmflash_erase_sector(stmflash_get_flash_sector(addrx));

                if (status)break;   /* 发生错误了 */
                
                SCB_CleanInvalidateDCache();   /* 清除无效的D-Cache */
            }
            else
            {
                addrx += 4;
            }
        }
    }

    if (status == 0)
    {
        while (waddr < endaddr)     /* 写入地址小于结束地址，还未写完 */
        {
            if (stmflash_write_8word(waddr, pbuf))  /* 写入数据 */
            {
                break;              /* 写入出错 */
            }

            waddr += 32;
            pbuf += 8;
        }
    }

    stmflash_lock();                /* FLASH上锁 */
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









