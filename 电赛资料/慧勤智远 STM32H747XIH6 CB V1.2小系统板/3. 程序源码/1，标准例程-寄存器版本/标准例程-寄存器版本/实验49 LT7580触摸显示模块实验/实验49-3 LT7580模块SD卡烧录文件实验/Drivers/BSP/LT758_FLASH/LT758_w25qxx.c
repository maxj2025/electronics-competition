/**
 ****************************************************************************************************
 * @file        LT758_w25qxx.c
 * @version     V1.0
 * @brief       LT7580模块 SPI FLASH(W25QXX) 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */	
 
#include "./BSP/LT758_FLASH/LT758_w25qxx.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/IF_PORT/if_port.h"
#include "./BSP/LT758_Lib/LT758_Lib.h"
#include "./BSP/LT758/LT758.h"


uint16_t g_spi_flash_type = W25Q128;    /* 默认是W25Q128 */

/* SPI FLASH 地址位宽 */
volatile uint8_t g_spiflash_addrw = 2;  /* SPI FLASH地址位宽, 在spi_flash_read_id函数里面被修改
                                         * 2, 表示24bit地址宽度
                                         * 3, 表示32bit地址宽度
                                         */

/**
 * @brief       初始化LT7580模块的SPI FLASH
 * @param       无
 * @retval      无
 */
void spi_flash_init(void)
{
    uint8_t temp;

    LT758_SPI_Init(1, 1);                /* LT758 SPI FLASH初始化 */       
    
    g_spi_flash_type = spi_flash_read_id();   /* 读取FLASH ID. */
    
    if (g_spi_flash_type == W25Q256)     /* SPI FLASH为W25Q256, 必须使能4字节地址模式 */
    {
        temp = spi_flash_read_sr(3);     /* 读取状态寄存器3，判断地址模式 */

        if ((temp & 0X01) == 0)          /* 如果不是4字节地址模式,则进入4字节地址模式 */
        {
            spi_flash_write_enable();    /* 写使能 */
            temp |= 1 << 1;              /* ADP=1, 上电4位地址模式 */
            spi_flash_write_sr(3, temp); /* 写SR3 */
            delay_ms(20);                /* 不加延时第一次上电会出错 */                                        	      
        }
       
        spi_flash_write_enable();        /* 写使能 */
        
        nSS_Active();	                                        
        SPI_Master_FIFO_Data_Put(W25X_Enable4ByteAddr);   /* 使能4字节地址指令 */
        nSS_Inactive(); 
    }

    printf("ID:%x\r\n", g_spi_flash_type);
}

/**
 * @brief       等待空闲
 * @param       无
 * @retval      无
 */
void spi_flash_wait_busy(void)
{
    while ((spi_flash_read_sr(1) & 0x01) == 0x01);   /*  等待BUSY位清空 */
}

/**
 * @brief       25QXX写使能
 * @note        将S1寄存器的WEL置位
 * @param       无
 * @retval      无
 */
void spi_flash_write_enable(void)
{
    nSS_Active();	                                /* 使能器件 */   
    SPI_Master_FIFO_Data_Put(W25X_WriteEnable);   /* 发送写使能指令 */
	  nSS_Inactive();                               /* 取消片选 */     	      
}

/**
 * @brief       25QXX写禁止
 * @note        将S1寄存器的WEL清零
 * @param       无
 * @retval      无
 */
void spi_flash_write_disable(void)
{
    nSS_Active();	                                  
    SPI_Master_FIFO_Data_Put(W25X_WriteDisable);  /* 发送写禁止指令 */
	  nSS_Inactive();                                  
}

/**
 * @brief       25QXX发送地址
 * @note        根据芯片型号的不同, 发送24ibt / 32bit地址
 * @param       address : 要发送的地址
 * @retval      无
 */
void spi_flash_send_address(uint32_t address)
{
    if (g_spi_flash_type == W25Q256) /*  只有W25Q256支持4字节地址模式 */
    {
        SPI_Master_FIFO_Data_Put((uint8_t)((address)>>24)); /* 发送 bit31 ~ bit24 地址 */
    } 
    
    SPI_Master_FIFO_Data_Put((uint8_t)((address)>>16));     /* 发送 bit23 ~ bit16 地址 */
    SPI_Master_FIFO_Data_Put((uint8_t)((address)>>8));      /* 发送 bit15 ~ bit8  地址 */
    SPI_Master_FIFO_Data_Put((uint8_t)address);             /* 发送 bit7  ~ bit0  地址 */
}

/**
 * @brief       读取25QXX的状态寄存器，25QXX一共有3个状态寄存器
 * @note        状态寄存器1：
 *              BIT7  6   5   4   3   2   1   0
 *              SPR   RV  TB BP2 BP1 BP0 WEL BUSY
 *              SPR:默认0,状态寄存器保护位,配合WP使用
 *              TB,BP2,BP1,BP0:FLASH区域写保护设置
 *              WEL:写使能锁定
 *              BUSY:忙标记位(1,忙;0,空闲)
 *              默认:0x00
 *
 *              状态寄存器2：
 *              BIT7  6   5   4   3   2   1   0
 *              SUS   CMP LB3 LB2 LB1 (R) QE  SRP1
 *
 *              状态寄存器3：
 *              BIT7      6    5    4   3   2   1   0
 *              HOLD/RST  DRV1 DRV0 (R) (R) WPS ADP ADS
 *
 * @param       regno: 状态寄存器号，范围:1~3
 * @retval      状态寄存器值
 */
uint8_t spi_flash_read_sr(uint8_t regno)
{
    uint8_t byte = 0, command = 0;

    switch (regno)
    {
        case 1:
            command = W25X_ReadStatusReg1;  /* 读状态寄存器1指令 */
            break;

        case 2:
            command = W25X_ReadStatusReg2;  /* 读状态寄存器2指令 */
            break;

        case 3:
            command = W25X_ReadStatusReg3;  /* 读状态寄存器3指令 */
            break;

        default:
            command = W25X_ReadStatusReg1;
            break;
    }

    nSS_Active();
    SPI_Master_FIFO_Data_Put(command);      /* 发送读寄存器命令 */
    byte = SPI_Master_FIFO_Data_Put(0Xff);  /* 读取一个字节 */
    nSS_Inactive();

    return byte;
}

/**
 * @brief       写25QXX状态寄存器
 * @note        寄存器说明见spi_flash_read_sr函数说明
 * @param       regno: 状态寄存器号，范围:1~3
 * @param       sr   : 要写入状态寄存器的值
 * @retval      无
 */
void spi_flash_write_sr(uint8_t regno, uint8_t sr)
{
    uint8_t command = 0;

    switch (regno)
    {
        case 1:
            command = W25X_WriteStatusReg1;  /* 写状态寄存器1指令 */
            break;

        case 2:
            command = W25X_WriteStatusReg2;  /* 写状态寄存器2指令 */
            break;

        case 3:
            command = W25X_WriteStatusReg3;  /* 写状态寄存器3指令 */
            break;

        default:
            command = W25X_WriteStatusReg1;
            break;
    }

    nSS_Active();
    SPI_Master_FIFO_Data_Put(command);  /* 发送写状态寄存器命令 */
    SPI_Master_FIFO_Data_Put(sr);       /* 写入一个字节 */
    nSS_Inactive();
}

/**
 * @brief       读取芯片ID
 * @param       无
 * @retval      FLASH芯片ID
 * @note        芯片ID列表见: LT758_w25qxx.h, 芯片列表部分
 */
uint16_t spi_flash_read_id(void)
{
    uint16_t deviceid;

    nSS_Active();
    SPI_Master_FIFO_Data_Put(W25X_ManufactDeviceID);   /* 发送读ID 命令 */
    SPI_Master_FIFO_Data_Put(0);                       /* 写入一个字节 */
    SPI_Master_FIFO_Data_Put(0);
    SPI_Master_FIFO_Data_Put(0);
    deviceid = SPI_Master_FIFO_Data_Put(0xFF) << 8;    /* 读取高8位字节 */
    deviceid |= SPI_Master_FIFO_Data_Put(0xFF);        /* 读取低8位字节 */
    nSS_Inactive();

    if (deviceid == W25Q256)
    {
        g_spiflash_addrw = 3;   /* 如果是W25Q256, 标记32bit地址宽度 */
    }
    
   return deviceid;
}

/**
 * @brief       读取SPI FLASH
 * @note        在指定地址开始读取指定长度的数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始读取的地址(最大32bit)
 * @param       datalen : 要读取的字节数
 * @retval      无
 */
void spi_flash_read(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    uint32_t i;

    nSS_Active();
    SPI_Master_FIFO_Data_Put(W25X_ReadData);        /* 发送读取命令 */
    spi_flash_send_address(addr);                   /* 发送地址 */
    
    for (i = 0; i < datalen; i++)
    {
        pbuf[i] = SPI_Master_FIFO_Data_Put(0XFF);   /* 循环读取 */
    }
    
    nSS_Inactive();
}

/**
 * @brief       SPI在一页(0~65535)内写入少于256个字节的数据
 * @note        在指定地址开始写入最大256字节的数据
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写入的地址(最大32bit)
 * @param       datalen : 要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
 * @retval      无
 */
void spi_flash_write_page(uint8_t *pbuf, uint32_t addr, uint16_t datalen)
{
    uint16_t i;

    spi_flash_write_enable();                     /* 写使能 */

    nSS_Active();
    SPI_Master_FIFO_Data_Put(W25X_PageProgram);   /* 发送写页命令 */
    spi_flash_send_address(addr);                 /* 发送地址 */

#if STM32_FMC_16
    for (i = 0; i < datalen; i++)
    {
        SPI_Master_FIFO_Data_Put(pbuf[i]);        /* 循环写入数据 */       
    }
#endif

#if STM32_SPI
    LCD_CmdWrite(0xB8);
    SPI_CS(0);	
    spi4_read_write_byte(0x80);    

    for (i = 0; i < datalen; i++)
    {
        spi4_read_write_byte(pbuf[i]);            /* 循环写入数据 */
    }

    SPI_CS(1);  
#endif
  
    nSS_Inactive();
    spi_flash_wait_busy();                        /* 等待写入结束 */
}

/**
 * @brief       无检验写SPI FLASH
 * @note        必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
 *              具有自动换页功能
 *              在指定地址开始写入指定长度的数据,但是要确保地址不越界!
 *
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写入的地址(最大32bit)
 * @param       datalen : 要写入的字节数
 * @retval      无
 */
void spi_flash_write_nocheck(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    uint16_t pageremain;
    pageremain = 256 - addr % 256;  /* 单页剩余的字节数 */

    if (datalen <= pageremain)      /* 不大于256个字节 */
    {
        pageremain = datalen;
    }

    while (1)
    {
        /* 当写入字节比页内剩余地址还少的时候, 一次性写完
         * 当写入直接比页内剩余地址还多的时候, 先写完整个页内剩余地址, 然后根据剩余长度进行不同处理
         */
        spi_flash_write_page(pbuf, addr, pageremain);

        if (datalen == pageremain)      /* 写入结束了 */
        {
            break;
        }
        else                            /* datalen > pageremain */
        {
            pbuf += pageremain;         /* pbuf指针地址偏移,前面已经写了pageremain字节 */
            addr += pageremain;         /* 写地址偏移,前面已经写了pageremain字节 */
            datalen -= pageremain;      /* 写入总长度减去已经写入了的字节数 */

            if (datalen > 256)          /* 剩余数据还大于一页,可以一次写一页 */
            {
                pageremain = 256;       /* 一次可以写入256个字节 */
            }
            else                        /* 剩余数据小于一页,可以一次写完 */
            {
                pageremain = datalen;   /* 不够256个字节了 */
            }
        }
    }
}

/**
 * @brief       写SPI FLASH
 * @note        在指定地址开始写入指定长度的数据 , 该函数带擦除操作!
 *              SPI FLASH 一般是: 256个字节为一个Page, 4Kbytes为一个Sector, 16个扇区为1个Block
 *              擦除的最小单位为Sector.
 *
 * @param       pbuf    : 数据存储区
 * @param       addr    : 开始写入的地址(最大32bit)
 * @param       datalen : 要写入的字节数
 * @retval      无
 */
uint8_t g_spi_flash_buf[4096];   /* 扇区缓存 */

void spi_flash_write(uint8_t *pbuf, uint32_t addr, uint32_t datalen)
{
    uint32_t secpos;
    uint16_t secoff;
    uint32_t secremain;
    uint16_t i;
    uint8_t *spi_flash_buf;

    spi_flash_buf = g_spi_flash_buf;
    secpos = addr / 4096;                                     /* 扇区地址 */
    secoff = addr % 4096;                                     /* 在扇区内的偏移 */
    secremain = 4096 - secoff;                                /* 扇区剩余空间大小 */

    //printf("ad:%X,nb:%X\r\n", addr, datalen);               /* 测试用 */
    if (datalen <= secremain)
    {
        secremain = datalen;                                  /* 不大于4096个字节 */
    }

    while (1)
    {
        spi_flash_read(spi_flash_buf, secpos * 4096, 4096);   /* 读出整个扇区的内容 */

        for (i = 0; i < secremain; i++)                       /* 校验数据 */
        {
            if (spi_flash_buf[secoff + i] != 0XFF)
            {
                break;                        /* 需要擦除, 直接退出for循环 */
            } 
        }

        if (i < secremain)                    /* 需要擦除 */
        {
            spi_flash_erase_sector(secpos);   /* 擦除这个扇区 */

            for (i = 0; i < secremain; i++)   /* 复制 */
            {
                spi_flash_buf[i + secoff] = pbuf[i];
            }

            spi_flash_write_nocheck(spi_flash_buf, secpos * 4096, 4096);  /* 写入整个扇区 */
        }
        else        /* 写已经擦除了的,直接写入扇区剩余区间. */
        {
            spi_flash_write_nocheck(pbuf, addr, secremain);  /* 直接写扇区 */
        }

        if (datalen == secremain)
        {
            break;                    /* 写入结束了 */
        }
        else                          /* 写入未结束 */
        {
            secpos++;                 /* 扇区地址增1 */
            secoff = 0;               /* 偏移位置为0 */

            pbuf += secremain;        /* 指针偏移 */
            addr += secremain;        /* 写地址偏移 */
            datalen -= secremain;     /* 字节数递减 */

            if (datalen > 4096)
            {
                secremain = 4096;     /* 下一个扇区还是写不完 */
            }
            else
            {
                secremain = datalen;  /* 下一个扇区可以写完了 */
            }
        }
    }
}

/**
 * @brief       擦除整个芯片
 * @note        等待时间超长...
 * @param       无
 * @retval      无
 */
void spi_flash_erase_chip(void)
{
    spi_flash_write_enable();    /* 写使能 */
    spi_flash_wait_busy();       /* 等待空闲 */
    nSS_Active();
    SPI_Master_FIFO_Data_Put(W25X_ChipErase);  /* 发送全片擦除命令 */ 
    nSS_Inactive();
    spi_flash_wait_busy();       /* 等待芯片擦除结束 */
}

/**
 * @brief       擦除一个扇区
 * @note        注意,这里是扇区地址,不是字节地址!!
 *              擦除一个扇区的最少时间:150ms
 *
 * @param       saddr : 扇区地址 根据实际容量设置
 * @retval      无
 */
void spi_flash_erase_sector(uint32_t saddr)
{
    //printf("fe:%x\r\n", saddr);    /* 监视flash擦除情况,测试用 */
    saddr *= 4096;
    spi_flash_write_enable();        /* 写使能 */
    spi_flash_wait_busy();           /* 等待空闲 */

    nSS_Active();
    SPI_Master_FIFO_Data_Put(W25X_SectorErase);    /* 发送扇区擦除指令 */
    spi_flash_send_address(saddr);   /* 发送地址 */
    nSS_Inactive();
    spi_flash_wait_busy();           /* 等待扇区擦除完成 */
}


 




