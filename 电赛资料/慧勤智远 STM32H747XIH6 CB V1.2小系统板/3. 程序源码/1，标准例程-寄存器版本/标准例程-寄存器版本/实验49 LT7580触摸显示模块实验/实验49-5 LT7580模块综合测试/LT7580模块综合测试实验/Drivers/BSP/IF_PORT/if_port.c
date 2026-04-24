/**
 ****************************************************************************************************
 * @file        if_port.c
 * @version     V1.0
 * @brief       LT7580模块 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/IF_PORT/if_port.h"
#include "./SYSTEM/delay/delay.h"


void Delay_us(uint32_t time)
{    
    delay_us(time);
}

void Delay_ms(uint32_t time)
{    
    delay_ms(time);
}


// -------------------------------------------------------- FMC总线驱动 -------------------------------------------------------------

#if STM32_FMC_16

/**
 * @brief       初始化FMC
 * @param       无
 * @retval      无
 */
void fmc_init_16(void)
{ 	
    LCD_CS_GPIO_CLK_ENABLE();   /* LCD_CS脚时钟使能 */
    LCD_WR_GPIO_CLK_ENABLE();   /* LCD_WR脚时钟使能 */
    LCD_RD_GPIO_CLK_ENABLE();   /* LCD_RD脚时钟使能 */
    LCD_RS_GPIO_CLK_ENABLE();   /* LCD_RS脚时钟使能 */

    RCC->AHB4ENR |= 3 << 3;     /* 使能PD,PE时钟 */
    RCC->AHB3ENR |= 1 << 12;    /* 使能FMC时钟 */

    sys_gpio_set(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* LCD_CS引脚模式设置 */

    sys_gpio_set(LCD_WR_GPIO_PORT, LCD_WR_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* LCD_WR引脚模式设置 */

    sys_gpio_set(LCD_RD_GPIO_PORT, LCD_RD_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* LCD_RD引脚模式设置 */

    sys_gpio_set(LCD_RS_GPIO_PORT, LCD_RS_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* LCD_RS引脚模式设置 */
                 
    sys_gpio_af_set(LCD_CS_GPIO_PORT, LCD_CS_GPIO_PIN, 12); /* LCD_CS脚, AF12 */
    sys_gpio_af_set(LCD_WR_GPIO_PORT, LCD_WR_GPIO_PIN, 12); /* LCD_WR脚, AF12 */
    sys_gpio_af_set(LCD_RD_GPIO_PORT, LCD_RD_GPIO_PIN, 12); /* LCD_RD脚, AF12 */
    sys_gpio_af_set(LCD_RS_GPIO_PORT, LCD_RS_GPIO_PIN, 12); /* LCD_RS脚, AF12 */

    /* LCD_D0~LCD_D15 IO口初始化 */
    sys_gpio_set(GPIOD, (3 << 0) | (7 << 8) | (3 << 14), SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* PD0,1,8,9,10,14,15   AF OUT */
    sys_gpio_set(GPIOE, (0X1FF << 7), SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);                      /* PE7~15  AF OUT */

    sys_gpio_af_set(GPIOD, (3 << 0) | (7 << 8) | (3 << 14), 12);/* PD0,1,8,9,10,14,15   AF12 */
    sys_gpio_af_set(GPIOE, (0X1FF << 7), 12);                   /* PE7~15  AF12 */

    /* fmc_ker_ck来自pll2_r_ck = 220Mhz
     * 寄存器清零
     * bank1有NE1~4,每一个有一个BCR+TCR，所以总共八个寄存器。
     * 这里我们使用NE4 ，也就对应BTCR[6], [7]
     */
    LCD_FMC_BCRX = 0X00000000;  /* BCR寄存器清零 */
    LCD_FMC_BTRX = 0X00000000;  /* BTR寄存器清零 */
    LCD_FMC_BWTRX = 0X00000000; /* BWTR寄存器清零 */

    /* 设置BCR寄存器 使用异步模式 */
    LCD_FMC_BCRX |= 1 << 12;    /* 存储器写使能 */
    LCD_FMC_BCRX |= 1 << 14;    /* 读写使用不同的时序 */
    LCD_FMC_BCRX |= 1 << 4;     /* 存储器数据宽度为16bit */

    /* 设置BTR寄存器, 读时序控制寄存器 */
    LCD_FMC_BTRX |= 0 << 28;    /* 模式A */
    LCD_FMC_BTRX |= 15 << 0;    /* 地址建立时间(ADDSET)为15个fmc_ker_ck 1/220M = 4.5ns * 15 = 67.5ns */

    /* 因为液晶驱动IC的读数据的时候，速度不能太快 */
    LCD_FMC_BTRX |= 78 << 8;    /* 数据保存时间(DATAST)为78个fmc_ker_ck = 4.5ns * 78 = 351ns */

    /* 设置BWTR寄存器, 写时序控制寄存器 */
    LCD_FMC_BWTRX |= 0 << 28;   /* 模式A */
    LCD_FMC_BWTRX |= 15 << 0;   /* 地址建立时间(ADDSET)为15个fmc_ker_ck = 4.5ns * 15 = 67.5ns */

    /* 某些液晶驱动IC的写信号脉宽，初始化的时候需要设置大一些 */
    LCD_FMC_BWTRX |= 15 << 8;   /* 数据保存时间(DATAST)为15个fmc_ker_ck = 4.5ns * 15 = 67.5ns */

    /* 使能BANK,区域x */
    LCD_FMC_BCRX |= 1 << 0;     /* 使能存储区域x */
    LCD_FMC_BCRX |= (uint32_t)1 << 31;  /* 使能FMC */
}

void FMC_16_CmdWrite(uint8_t cmd)
{
    cmd = cmd;
    *(volatile uint16_t *)(LCD_BASE0) = (cmd);
}

void FMC_16_DataWrite(uint8_t data)
{
  	data=data;
  	*(volatile uint16_t *)(LCD_BASE1) = (data);
}

void FMC_16_DataWrite_Pixel(uint16_t data)
{
    data = data;
    *(volatile uint16_t *)(LCD_BASE1) = (data);
}

uint8_t FMC_16_StatusRead(void)
{
    uint16_t temp = 0;
    temp = *(volatile uint16_t *)(LCD_BASE0);
    return temp;
}

uint16_t FMC_16_DataRead(void)
{
    uint16_t temp = 0;
    temp = *(volatile uint16_t *)(LCD_BASE1);
    return temp;
}

#endif

// -------------------------------------------------------- SPI驱动 -------------------------------------------------------------

#if STM32_SPI


/**
 * @brief       SPI接口初始化
 * @param       无
 * @retval      无
 */
void spi_io_init(void)
{    
    uint32_t tempreg = 0;
    
    SPI4_SPI_CLK_ENABLE();          /* SPI4时钟使能 */
    SPI4_CS_GPIO_CLK_ENABLE();      /* SPI4_CS引脚时钟使能 */
    SPI4_SCK_GPIO_CLK_ENABLE();     /* SPI4_SCK引脚时钟使能 */
    SPI4_MISO_GPIO_CLK_ENABLE();    /* SPI4_MISO引脚时钟使能 */
    SPI4_MOSI_GPIO_CLK_ENABLE();    /* SPI4_MOSI引脚时钟使能 */

    sys_gpio_set(SPI4_CS_GPIO_PORT, SPI4_CS_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* CS引脚模式设置(推挽输出) */
                 
    sys_gpio_set(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SCK引脚模式设置(复用推挽输出) */

    sys_gpio_set(SPI4_MISO_GPIO_PORT, SPI4_MISO_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MISO引脚模式设置(复用推挽输出) */

    sys_gpio_set(SPI4_MOSI_GPIO_PORT, SPI4_MOSI_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* MOSI引脚模式设置(复用推挽输出) */

    sys_gpio_af_set(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN, SPI4_SCK_GPIO_AF);        /* SCK引脚, AF功能设置 */
    sys_gpio_af_set(SPI4_MISO_GPIO_PORT, SPI4_MISO_GPIO_PIN, SPI4_MISO_GPIO_AF);     /* MISO引脚, AF功能设置 */
    sys_gpio_af_set(SPI4_MOSI_GPIO_PORT, SPI4_MOSI_GPIO_PIN, SPI4_MOSI_GPIO_AF);     /* MOSI引脚, AF功能设置 */

    /* 配置SPI4/5的内核时钟源, 选择rcc_pclk2时钟作为时钟源, 120Mhz */
    RCC->D2CCIP1R &= ~(7 << 16);    /* SPI45SEL[2:0]=0,清除原来的设置 */
    RCC->D2CCIP1R |= 0 << 16;       /* SPI45SEL[2:0]=0,选择rcc_pclk2时钟作为SPI4/5的时钟源,一般为120Mhz */
 
    /* 这里只针对SPI4_SPI初始化 */
    RCC->APB2RSTR |= 1 << 13;       /* 复位SPI4_SPI */
    RCC->APB2RSTR &= ~(1 << 13);    /* 停止复位SPI4_SPI */

    SPI4_SPI->CR1 |= 1 << 12;       /* SSI=1,设置内部SS信号为高电平 */
    SPI4_SPI->CFG1 = 1 << 28;       /* MBR[2:0]=1,设置spi_ker_ck为4分频. */
    SPI4_SPI->CFG1 |= 7 << 0;       /* DSIZE[4:0]=7,设置SPI帧格式为8位,即字节传输 */
    tempreg = (uint32_t)1 << 31;    /* AFCNTR=1,SPI保持对IO口的控制 */
    tempreg |= 0 << 29;             /* SSOE=0,禁止硬件NSS输出 */
    tempreg |= 1 << 26;             /* SSM=1,软件管理NSS脚 */
    tempreg |= 1 << 25;             /* CPOL=1,空闲状态下,SCK为高电平 */
    tempreg |= 1 << 24;             /* CPHA=1,数据采样从第2个时间边沿开始 */
    tempreg |= 0 << 23;             /* LSBFRST=0,MSB先传输 */
    tempreg |= 1 << 22;             /* MASTER=1,主机模式 */
    tempreg |= 0 << 19;             /* SP[2:0]=0,摩托罗拉格式 */
    tempreg |= 0 << 17;             /* COMM[1:0]=0,全双工通信 */
    SPI4_SPI->CFG2 = tempreg;       /* 设置CFG2寄存器 */
    SPI4_SPI->I2SCFGR &= ~(1 << 0); /* 选择SPI模式 */
    SPI4_SPI->CR1 |= 1 << 0;        /* SPE=1,使能SPI4_SPI */

    spi4_read_write_byte(0xff);     /* 启动传输 */
}

/**
 * @brief       SPI4速度设置函数
 * @note        SPI4时钟选择来自rcc_pclk2, 为120Mhz
 *              SPI速度 = spi_ker_ck / 2^(speed + 1)
 * @param       speed: SPI时钟分频系数
 * @retval      无
 */
void spi4_set_speed(uint8_t speed)
{
    speed &= 0X07;                          /* 限制范围 */
    SPI4_SPI->CR1 &= ~(1 << 0);             /* SPE=0,SPI设备失能 */
    SPI4_SPI->CFG1 &= ~(7 << 28);           /* MBR[2:0]=0,清除原来的分频设置 */
    SPI4_SPI->CFG1 |= (uint32_t)speed << 28;/* MBR[2:0]=speed,设置SPI时钟分频系数 */
    SPI4_SPI->CR1 |= 1 << 0;                /* SPE=1,SPI设备使能 */
}

/**
 * @brief       SPI4读写一个字节数据
 * @param       txdata: 要发送的数据(1字节)
 * @retval      接收到的数据(1字节)
 */
uint8_t spi4_read_write_byte(uint8_t txdata)
{   
    uint8_t rxdata = 0;
    SPI4_SPI->CR1 |= 1 << 0;    /* SPE=1,使能SPI4_SPI */
    SPI4_SPI->CR1 |= 1 << 9;    /* CSTART=1,启动传输 */

    while ((SPI4_SPI->SR & 1 << 1) == 0);   /* 等待发送区空 */

    /* 发送一个byte,以传输长度访问TXDR寄存器 */
    *(volatile uint8_t *)&SPI4_SPI->TXDR = txdata;  

    while ((SPI4_SPI->SR & 1 << 0) == 0);   /* 等待接收完一个byte */

    /* 接收一个byte,以传输长度访问RXDR寄存器 */
    rxdata = *(volatile uint8_t *)&SPI4_SPI->RXDR;  

    SPI4_SPI->IFCR |= 3 << 3;   /* EOTC和TXTFC置1,清零EOT和TXTF标志位 */
    SPI4_SPI->CR1 &= ~(1 << 0); /* SPE=0,关闭SPI4_SPI,会执行状态机复位/FIFO重置等操作 */
  
    return rxdata;              /* 返回收到的数据 */
}

void SPI_CmdWrite(uint8_t cmd)
{  
    SPI_CS(0);		
    spi4_read_write_byte(0x00);      /* 代表MCU要写入指令寄存器地址 */
    spi4_read_write_byte(cmd);
    SPI_CS(1);		
}

void SPI_DataWrite(uint8_t data)
{ 

    SPI_CS(0);	
    spi4_read_write_byte(0x80);      /* 代表MCU要写入数据到寄存器或是显示内存中 */
    spi4_read_write_byte(data);
    SPI_CS(1);
}

void SPI_DataWrite_Pixel(uint16_t data)
{ 

    SPI_CS(0);	
    spi4_read_write_byte(0x80);
    spi4_read_write_byte(data);	
    spi4_read_write_byte(data >> 8);	
    SPI_CS(1);
}

uint8_t SPI_StatusRead(void)
{
    uint8_t temp = 0;	
  
    SPI_CS(0);	
    spi4_read_write_byte(0x40);      /* 代表MCU要读取状态寄存器的数据 */
    temp = spi4_read_write_byte(0xff);
    SPI_CS(1);
  
    return temp;
}

uint16_t SPI_DataRead(void)
{
    uint16_t temp = 0;	
    
    SPI_CS(0);	
    spi4_read_write_byte(0xc0);      /* 代表MCU要读取指令寄存器的数据 */
    temp = spi4_read_write_byte(0xff);
    SPI_CS(1);
  
    return temp;
}

#endif



//-----------------------------------------------------------------------------------------------------------------------------------
/* 不管使用哪种接口，这些接口主要要实现以下5个函数的功能，
 * 而LT7580的其他的功能都是基于这5个函数来实现的。
 */

/**
 * @brief       向LT7580写命令 
 * @param       cmd: 要写入的命令
 * @retval      无
 */
void LCD_CmdWrite(uint8_t cmd)
{
#if STM32_FMC_16
    FMC_16_CmdWrite(cmd);
#endif
    
#if STM32_SPI
    SPI_CmdWrite(cmd);
#endif
}

/**
 * @brief       向LT7580写数据
 * @param       data: 要写入的数据
 * @retval      无
 */
void LCD_DataWrite(uint8_t data)
{
#if STM32_FMC_16
    FMC_16_DataWrite(data);
#endif
    
#if STM32_SPI
    SPI_DataWrite(data);
#endif
}

/**
 * @brief       向LT7580写像素点的数据  
 * @param       data: 像素点的颜色值
 * @retval      无
 */
void LCD_DataWrite_Pixel(uint16_t data)
{	
#if STM32_FMC_16
    FMC_16_DataWrite_Pixel(data);
#endif
    
#if STM32_SPI
    SPI_DataWrite_Pixel(data);
#endif
}

/**
 * @brief       读状态寄存器 
 * @param       无
 * @retval      读取到的状态寄存器值
 */
uint8_t LCD_StatusRead(void)
{
    uint8_t temp = 0;
      
#if STM32_FMC_16
    temp = FMC_16_StatusRead();
#endif
    
#if STM32_SPI
    temp = SPI_StatusRead();
#endif
    
    return temp;
}

/**
 * @brief       读寄存器数据
 * @param       无
 * @retval      读取到的寄存器或内存的数据
 */
uint16_t LCD_DataRead(void)
{
    uint16_t temp = 0;
      
#if STM32_FMC_16
    temp = FMC_16_DataRead();
#endif
    
#if STM32_SPI
    temp = SPI_DataRead();
#endif
    
    return temp;
}
	  
/**
 * @brief       LCD模块接口初始化 
 * @param       无
 * @retval      无
 */
void lcd_port_init(void)
{	   
#if STM32_FMC_16
    fmc_init_16();
#endif
    
#if STM32_SPI
    spi_io_init();
#endif
}

/**
 * @brief       LT758写寄存器
 * @param       reg :寄存器编号/地址
 * @param       data:要写入寄存器的数据
 * @retval      无
 */
void write_758_reg(uint8_t reg, uint8_t data)
{
    LCD_CmdWrite(reg);
    LCD_DataWrite(data);
}

/**
 * @brief       LT758读寄存器
 * @param       reg :寄存器编号/地址
 * @retval      读取到的寄存器值
 */
uint8_t read_758_reg(uint8_t reg)
{
    LCD_CmdWrite(reg);
    return LCD_DataRead();
}

/**
 * @brief       LT758读状态寄存器
 * @param       无
 * @retval      读取到的寄存器值
 */
uint8_t read_758_statu_reg(void)
{
    return LCD_StatusRead();
}

/**
 * @brief       打开LCD模块电源(LCD_5V) 
 * @param       无
 * @retval      无
 */
void lcd_power_on(void)
{	  
    LCD_PWREN_GPIO_CLK_ENABLE();        /* LCD_PWREN引脚时钟使能 */

    sys_gpio_set(LCD_PWREN_GPIO_PORT, LCD_PWREN_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* LCD_PWREN引脚模式设置(推挽输出) */
  
    LCD_PWREN(1);                       /* 开启LCD_5V */
    delay_ms(100);                      /* 等待电压稳定 */
}








