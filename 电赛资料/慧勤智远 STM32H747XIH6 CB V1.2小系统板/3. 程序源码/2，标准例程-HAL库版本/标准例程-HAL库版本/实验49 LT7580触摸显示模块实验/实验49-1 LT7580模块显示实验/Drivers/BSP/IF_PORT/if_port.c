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

SRAM_HandleTypeDef g_sram_handle;       /* SRAM句柄(用于控制LCD) */

/**
 * @brief       SRAM底层驱动，时钟使能，引脚分配
 * @note        此函数会被HAL_SRAM_Init()调用,初始化读写总线引脚
 * @param       hsram:SRAM句柄
 * @retval      无
 */
void HAL_SRAM_MspInit(SRAM_HandleTypeDef *hsram)
{
    GPIO_InitTypeDef gpio_init_struct;

    __HAL_RCC_FMC_CLK_ENABLE();                         /* 使能FMC时钟 */
    __HAL_RCC_GPIOD_CLK_ENABLE();                       /* 使能GPIOD时钟 */
    __HAL_RCC_GPIOE_CLK_ENABLE();                       /* 使能GPIOE时钟 */

    /* 初始化PD0,1,8,9,10,14,15 */
    gpio_init_struct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10| \
                           GPIO_PIN_14 | GPIO_PIN_15;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;            /* 复用推挽 */
    gpio_init_struct.Pull = GPIO_PULLUP;                /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;      /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF12_FMC;         /* 复用为FMC */

    HAL_GPIO_Init(GPIOD, &gpio_init_struct);            /* 初始化IO口 */

    /* 初始化PE7,8,9,10,11,12,13,14,15 */
    gpio_init_struct.Pin = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 \
                           | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &gpio_init_struct);            /* 初始化IO口 */
}

/**
 * @brief       初始化FMC
 * @param       无
 * @retval      无
 */
void fmc_init_16(void)
{ 	
    GPIO_InitTypeDef gpio_init_struct;
    FMC_NORSRAM_TimingTypeDef fmc_read_handle;
    FMC_NORSRAM_TimingTypeDef fmc_write_handle;
  
    LCD_CS_GPIO_CLK_ENABLE();   /* LCD_CS脚时钟使能 */
    LCD_WR_GPIO_CLK_ENABLE();   /* LCD_WR脚时钟使能 */
    LCD_RD_GPIO_CLK_ENABLE();   /* LCD_RD脚时钟使能 */
    LCD_RS_GPIO_CLK_ENABLE();   /* LCD_RS脚时钟使能 */

    gpio_init_struct.Pin = LCD_CS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_MEDIUM;        /* 中速 */
    gpio_init_struct.Alternate = GPIO_AF12_FMC;             /* 复用为FMC */
    HAL_GPIO_Init(LCD_CS_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_CS引脚 */

    gpio_init_struct.Pin = LCD_WR_GPIO_PIN;
    HAL_GPIO_Init(LCD_WR_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_WR引脚 */

    gpio_init_struct.Pin = LCD_RD_GPIO_PIN;
    HAL_GPIO_Init(LCD_RD_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_RD引脚 */

    gpio_init_struct.Pin = LCD_RS_GPIO_PIN;
    HAL_GPIO_Init(LCD_RS_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_RS引脚 */

    g_sram_handle.Instance = FMC_NORSRAM_DEVICE;
    g_sram_handle.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
    
    g_sram_handle.Init.NSBank = FMC_NORSRAM_BANK1;                        /* 使用NE1 */
    g_sram_handle.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;     /* 地址/数据线不复用 */
    g_sram_handle.Init.MemoryType=FMC_MEMORY_TYPE_SRAM;                   /* SRAM */
    g_sram_handle.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_16;    /* 16位数据宽度 */
    g_sram_handle.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;   /* 是否使能突发访问,仅对同步突发存储器有效,此处未用到 */
    g_sram_handle.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW; /* 等待信号的极性,仅在突发模式访问下有用 */
    g_sram_handle.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;      /* 存储器是在等待周期之前的一个时钟周期还是等待周期期间使能NWAIT */
    g_sram_handle.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;       /* 存储器写使能 */
    g_sram_handle.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;              /* 等待使能位,此处未用到 */
    g_sram_handle.Init.ExtendedMode = FMC_EXTENDED_MODE_ENABLE;           /* 读写使用不同的时序 */
    g_sram_handle.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;  /* 是否使能同步传输模式下的等待信号,此处未用到 */
    g_sram_handle.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;              /* 禁止突发写 */
    g_sram_handle.Init.ContinuousClock=FMC_CONTINUOUS_CLOCK_SYNC_ASYNC;
    
    /* FMC读时序控制寄存器 */
    fmc_read_handle.AddressSetupTime = 0x0F;          /* 地址建立时间(ADDSET)为15个fmc_ker_ck 1/220M = 4.5ns * 15 = 67.5ns */
    fmc_read_handle.AddressHoldTime = 0x00;
    fmc_read_handle.DataSetupTime = 0x4E;             /* 数据保存时间(DATAST)为78个fmc_ker_ck = 4.5 * 78 = 351ns */
                                                      /* 因为液晶驱动IC的读数据的时候，速度不能太快 */
    fmc_read_handle.AccessMode = FMC_ACCESS_MODE_A;   /* 模式A */
    /* FMC写时序控制寄存器 */
    fmc_write_handle.AddressSetupTime = 0x0F;         /* 地址建立时间(ADDSET)为15个fmc_ker_ck = 4.5ns * 15 = 67.5ns */
    fmc_write_handle.AddressHoldTime = 0x00;
    fmc_write_handle.DataSetupTime = 0x0F;            /* 数据保存时间(DATAST)为15个fmc_ker_ck = 4.5ns * 15 = 67.5ns */
                                                      /* 某些液晶驱动IC的写信号脉宽，初始化的时候需要设置大一些 */
    fmc_write_handle.AccessMode = FMC_ACCESS_MODE_A;  /* 模式A */
    HAL_SRAM_Init(&g_sram_handle, &fmc_read_handle, &fmc_write_handle);
    delay_ms(50);
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

SPI_HandleTypeDef g_spi4_handle;        /* SPI句柄 */

/**
 * @brief       SPI接口初始化
 * @param       无
 * @retval      无
 */
void spi_io_init(void)
{    
    g_spi4_handle.Instance = SPI4_SPI;                                      /* SPI4 */
    g_spi4_handle.Init.Mode = SPI_MODE_MASTER;                              /* SPI工作模式，设置为主模式 */
    g_spi4_handle.Init.Direction = SPI_DIRECTION_2LINES;                    /* SPI的通信模式:设置为全双工通信模式 */
    g_spi4_handle.Init.DataSize = SPI_DATASIZE_8BIT;                        /* 设置SPI的数据帧大小:SPI发送接收8位数据帧 */
    g_spi4_handle.Init.CLKPolarity = SPI_POLARITY_HIGH;                     /* 串行同步时钟的空闲状态为高电平 */
    g_spi4_handle.Init.CLKPhase = SPI_PHASE_2EDGE;                          /* 串行同步时钟的第二个跳变沿（上升或下降）开始采样数据 */
    g_spi4_handle.Init.NSS = SPI_NSS_SOFT;                                  /* NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号由SSI位决定 */
    g_spi4_handle.Init.NSSPMode=SPI_NSS_PULSE_DISABLE;                      /* NSS信号脉冲失能 */
    g_spi4_handle.Init.MasterKeepIOState=SPI_MASTER_KEEP_IO_STATE_ENABLE;   /* SPI主模式保持IO状态使能 */
    g_spi4_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;         /* 设置波特率预分频值:波特率预分频值设为4 */
    g_spi4_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                         /* 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始 */
    g_spi4_handle.Init.TIMode = SPI_TIMODE_DISABLE;                         /* 关闭TI模式 */
    g_spi4_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;         /* 关闭硬件CRC校验 */
    g_spi4_handle.Init.CRCPolynomial = 7;                                   /* 用于CRC计算的多项式 */
    HAL_SPI_Init(&g_spi4_handle);                                           /* 初始化SPI */
    
    __HAL_SPI_ENABLE(&g_spi4_handle);                                       /* 使能SPI4 */

    spi4_read_write_byte(0Xff);                                             /* 启动传输 */
}

/**
 * @brief       SPI4底层驱动，时钟使能，引脚配置
 * @note        此函数会被HAL_SPI_Init()调用
 * @param       hspi:SPI句柄
 * @retval      无
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef gpio_init_struct;
    RCC_PeriphCLKInitTypeDef spi4_clk_init;

    SPI4_SPI_CLK_ENABLE();                                          /* SPI4时钟使能 */
    SPI4_CS_GPIO_CLK_ENABLE();                                      /* SPI4_CS引脚时钟使能 */
    SPI4_SCK_GPIO_CLK_ENABLE();                                     /* SPI4_SCK引脚时钟使能 */
    SPI4_MISO_GPIO_CLK_ENABLE();                                    /* SPI4_MISO引脚时钟使能 */
    SPI4_MOSI_GPIO_CLK_ENABLE();                                    /* SPI4_MOSI引脚时钟使能 */

    spi4_clk_init.PeriphClockSelection = RCC_PERIPHCLK_SPI4;        /* 设置SPI4时钟源 */
    spi4_clk_init.Spi45ClockSelection = RCC_SPI4CLKSOURCE_D2PCLK1;  /* SPI4时钟源选择rcc_pclk2时钟 */
    HAL_RCCEx_PeriphCLKConfig(&spi4_clk_init);
    
    gpio_init_struct.Pin = SPI4_SCK_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                        /* 复用推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                            /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;             /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF5_SPI4;                     /* 复用为SPI4 */
    HAL_GPIO_Init(SPI4_SCK_GPIO_PORT, &gpio_init_struct);           /* 初始化SPI4_SCK引脚 */
   
    gpio_init_struct.Pin = SPI4_MISO_GPIO_PIN;
    HAL_GPIO_Init(SPI4_MISO_GPIO_PORT, &gpio_init_struct);          /* 初始化SPI4_MISO引脚 */

    gpio_init_struct.Pin = SPI4_MOSI_GPIO_PIN;
    HAL_GPIO_Init(SPI4_MOSI_GPIO_PORT, &gpio_init_struct);          /* 初始化SPI4_MOSI引脚 */
    
    gpio_init_struct.Pin = SPI4_CS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;                    /* 推挽输出 */
    HAL_GPIO_Init(SPI4_CS_GPIO_PORT, &gpio_init_struct);            /* 初始化SPI4_CS引脚 */
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
    speed &= 0X07;                                         /* 限制范围 */
    __HAL_SPI_DISABLE(&g_spi4_handle);                     /* 关闭SPI */
    g_spi4_handle.Instance->CFG1 &= ~(7 << 28);            /* MBR[2:0]=0,清除原来的分频设置 */
    g_spi4_handle.Instance->CFG1 |= (uint32_t)speed << 28; /* MBR[2:0]=speed,设置SPI时钟分频系数 */
    __HAL_SPI_ENABLE(&g_spi4_handle);                      /* 使能SPI */
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
  
//    uint8_t rxdata;

//    HAL_SPI_TransmitReceive(&g_spi4_handle, &txdata, &rxdata, 1, 1000);

//    return rxdata;            /* 返回收到的数据 */
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
    GPIO_InitTypeDef gpio_init_struct;

    LCD_PWREN_GPIO_CLK_ENABLE();        /* LCD_PWREN引脚时钟使能 */

    gpio_init_struct.Pin = LCD_PWREN_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;               /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                       /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;        /* 高速 */
    HAL_GPIO_Init(LCD_PWREN_GPIO_PORT, &gpio_init_struct);     /* 初始化LCD_PWREN引脚 */
   
    LCD_PWREN(1);                       /* 开启LCD_5V */
    delay_ms(100);                      /* 等待电压稳定 */
}







