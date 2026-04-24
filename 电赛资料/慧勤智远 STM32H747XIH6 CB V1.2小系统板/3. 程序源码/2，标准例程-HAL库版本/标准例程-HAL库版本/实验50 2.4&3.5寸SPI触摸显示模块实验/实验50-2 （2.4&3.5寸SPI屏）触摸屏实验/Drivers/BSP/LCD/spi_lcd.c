/**
 ****************************************************************************************************
 * @file        spi_lcd.c
 * @version     V1.0
 * @brief       SPI串口屏 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/LCD/spi_lcd.h"
#include "./BSP/LCD/lcd.h"
#include "./SYSTEM/delay/delay.h"


/**
 * @brief       向LCD驱动IC写命令 
 * @param       cmd: 要写入的命令
 * @retval      无
 */
void SPI_CmdWrite(uint8_t cmd)
{
    SPI_LCD_CS(0);
    SPI_LCD_DC(0);      /* 写命令 */
    spi4_read_write_byte(cmd);
    SPI_LCD_CS(1);
}  

/**
 * @brief       向LCD驱动IC写数据 
 * @param       data: 要写入的数据
 * @retval      无
 */
void SPI_DataWrite(uint8_t data)
{
    SPI_LCD_CS(0);
    SPI_LCD_DC(1);      /* 写数据 */
    spi4_read_write_byte(data);
    SPI_LCD_CS(1);
} 

/**
 * @brief       向LCD驱动IC写像素点的数据 
 * @param       无
 * @retval      无
 */
void SPI_DataWrite_Pixel(uint16_t data)
{ 
    SPI_DataWrite(data >> 8);
    SPI_DataWrite(data);  
}

/**
 * @brief       增加一个空时钟周期
 * @param       无
 * @retval      无
 */
void dummy_clock(void)
{
    GPIO_InitTypeDef gpio_init_struct;

    gpio_init_struct.Pin = SPI4_SCK_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;            /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                    /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;     /* 高速 */
    HAL_GPIO_Init(SPI4_SCK_GPIO_PORT, &gpio_init_struct);   /* 初始化SPI4_SCK引脚 */
  
    SPI_LCD_SCL(0);
    delay_us(1);
    SPI_LCD_SCL(1);          /* dummy clock cycle */
  
    gpio_init_struct.Pin = SPI4_SCK_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                /* 复用推挽输出 */
    gpio_init_struct.Alternate = GPIO_AF5_SPI4;             /* 复用为SPI4 */
    HAL_GPIO_Init(SPI4_SCK_GPIO_PORT, &gpio_init_struct);   /* 初始化SPI4_SCK引脚 */
}

/**
 * @brief       LCD读内存数据
 * @param       regno: 寄存器编号/地址
 * @retval      读取到的数据
 */
uint16_t lcd_read_ram(uint8_t regno)
{
    uint16_t color, colorl = 0, colorh = 0;
    uint16_t r = 0, g = 0, b = 0;

    SPI_LCD_CS(0);
    SPI_LCD_DC(0);                            /* 写命令 */
  
    spi4_read_write_byte(regno);              /* 写入要写的寄存器序号 */
  
    spi4_read_write_byte(0Xff);               /* dummy read */
    
    if (lcddev.id == 0X7796)
    {
        colorh = spi4_read_write_byte(0Xff);  /* 读高八位数据 */
        colorl = spi4_read_write_byte(0Xff);  /* 读低八位数据 */
        color = (colorh << 8) | colorl;
    }
    else if (lcddev.id == 0X7789)
    {
        r = spi4_read_write_byte(0Xff);       /* 读取R颜色分量值 */
        g = spi4_read_write_byte(0Xff);       /* 读取G颜色分量值 */
        b = spi4_read_write_byte(0Xff);       /* 读取B颜色分量值 */
        color = (((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
    }
    
    SPI_LCD_CS(1);                            /* 读完再拉高 */
   
    return  color;
}

/**
 * @brief       读取LCD驱动IC的ID
 * @param       regno: 寄存器编号/地址
 * @retval      读取到的ID
 */
uint16_t lcd_read_id(uint8_t regno)
{
    uint16_t id1 = 0, id2 = 0;

    SPI_LCD_CS(0);
    SPI_LCD_DC(0);                      /* 写命令 */
  
    spi4_read_write_byte(regno);        /* 写入要写的寄存器序号 */

    dummy_clock();                      /* dummy clock cycle */
    
    id1 = spi4_read_write_byte(0Xff);   /* 读到0X00/0X85 */    
    id1 = spi4_read_write_byte(0Xff);   /* 读取0X77/0X85 */
    id2 = spi4_read_write_byte(0Xff);   /* 读取0X96/0X52 */  
  
    SPI_LCD_CS(1);                      /* 读完再拉高 */

    return  (id1 << 8) | id2;
}

/**
 * @brief       SPI LCD接口初始化
 * @param       无
 * @retval      无
 */
void spi_lcd_init(void)
{    
    GPIO_InitTypeDef gpio_init_struct;

    /* IO 及 时钟配置 */
    spi_io_init();                     /* SPI接口初始化 */

    SPI_LCD_DC_GPIO_CLK_ENABLE();      /* SPI_LCD_DC引脚时钟使能 */
    SPI_LCD_BL_GPIO_CLK_ENABLE();      /* SPI_LCD_BL引脚时钟使能 */
    SPI_LCD_RST_GPIO_CLK_ENABLE();     /* SPI_LCD_RST引脚时钟使能 */

    gpio_init_struct.Pin = SPI_LCD_DC_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;                 /* 推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                         /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;          /* 高速 */
    HAL_GPIO_Init(SPI_LCD_DC_GPIO_PORT, &gpio_init_struct);      /* 初始化SPI_LCD_DC引脚 */ 

    gpio_init_struct.Pin = SPI_LCD_BL_GPIO_PIN;
    HAL_GPIO_Init(SPI_LCD_BL_GPIO_PORT, &gpio_init_struct);      /* 初始化SPI_LCD_BL引脚 */  
    
    gpio_init_struct.Pin = SPI_LCD_RST_GPIO_PIN;
    HAL_GPIO_Init(SPI_LCD_RST_GPIO_PORT, &gpio_init_struct);     /* 初始化SPI_LCD_RST引脚 */  
        
	  delay_ms(50);
     
    /* LCD复位 */
	  SPI_LCD_RST(1);
	  delay_ms(10);
	  SPI_LCD_RST(0);
	  delay_ms(50);
	  SPI_LCD_RST(1); 
		delay_ms(200); 
}


SPI_HandleTypeDef g_spi4_handle;        /* SPI句柄 */
DMA_HandleTypeDef g_dma_spi_handle;     /* DMA句柄 */

/**
 * @brief       SPI接口初始化
 * @param       无
 * @retval      无
 */
void spi_io_init(void)
{   
    GPIO_InitTypeDef gpio_init_struct;

    SPI4_SPI_CLK_ENABLE();                                     /* SPI4时钟使能 */
    SPI4_CS_GPIO_CLK_ENABLE();                                 /* SPI4_CS引脚时钟使能 */
    SPI4_SCK_GPIO_CLK_ENABLE();                                /* SPI4_SCK引脚时钟使能 */
    SPI4_MISO_GPIO_CLK_ENABLE();                               /* SPI4_MISO引脚时钟使能 */
    SPI4_MOSI_GPIO_CLK_ENABLE();                               /* SPI4_MOSI引脚时钟使能 */
   
    /* 配置SPI4/5的内核时钟源, 选择pll2_q_ck作为时钟源, 220Mhz */
    RCC->D2CCIP1R &= ~(7 << 16);    /* SPI45SEL[2:0]=0,清除原来的设置 */
    RCC->D2CCIP1R |= 1 << 16;       /* SPI45SEL[2:0]=1,选择pll2_q_ck作为SPI4/5的时钟源,在系统时钟初始化函数中设置为220Mhz */
    RCC->PLLCFGR |= 1 << 20;        /* DIVQ2EN = 1, 使能pll2_q_ck输出 */
    
    gpio_init_struct.Pin = SPI4_SCK_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_AF_PP;                   /* 复用推挽输出 */
    gpio_init_struct.Pull = GPIO_PULLUP;                       /* 上拉 */
    gpio_init_struct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;        /* 高速 */
    gpio_init_struct.Alternate = GPIO_AF5_SPI4;                /* 复用为SPI4 */
    HAL_GPIO_Init(SPI4_SCK_GPIO_PORT, &gpio_init_struct);      /* 初始化SPI4_SCK引脚 */
   
    gpio_init_struct.Pin = SPI4_MISO_GPIO_PIN;
    HAL_GPIO_Init(SPI4_MISO_GPIO_PORT, &gpio_init_struct);     /* 初始化SPI4_MISO引脚 */

    gpio_init_struct.Pin = SPI4_MOSI_GPIO_PIN;
    HAL_GPIO_Init(SPI4_MOSI_GPIO_PORT, &gpio_init_struct);     /* 初始化SPI4_MOSI引脚 */
    
    gpio_init_struct.Pin = SPI4_CS_GPIO_PIN;
    gpio_init_struct.Mode = GPIO_MODE_OUTPUT_PP;               /* 推挽输出 */
    HAL_GPIO_Init(SPI4_CS_GPIO_PORT, &gpio_init_struct);       /* 初始化SPI4_CS引脚 */
    
    g_spi4_handle.Instance = SPI4_SPI;                                      /* SPI4 */
    g_spi4_handle.Init.Mode = SPI_MODE_MASTER;                              /* SPI工作模式，设置为主模式 */
    g_spi4_handle.Init.Direction = SPI_DIRECTION_2LINES;                    /* SPI的通信模式:设置为全双工通信模式 */
    g_spi4_handle.Init.DataSize = SPI_DATASIZE_8BIT;                        /* 设置SPI的数据帧大小:SPI发送接收8位数据帧 */
    g_spi4_handle.Init.CLKPolarity = SPI_POLARITY_HIGH;                     /* 串行同步时钟的空闲状态为高电平 */
    g_spi4_handle.Init.CLKPhase = SPI_PHASE_2EDGE;                          /* 串行同步时钟的第二个跳变沿（上升或下降）开始采样数据 */
    g_spi4_handle.Init.NSS = SPI_NSS_SOFT;                                  /* NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号由SSI位决定 */
    g_spi4_handle.Init.NSSPMode=SPI_NSS_PULSE_DISABLE;                      /* NSS信号脉冲失能 */
    g_spi4_handle.Init.MasterKeepIOState=SPI_MASTER_KEEP_IO_STATE_ENABLE;   /* SPI主模式保持IO状态使能 */
    g_spi4_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;        /* 设置波特率预分频值:波特率预分频值设为32 */
    g_spi4_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                         /* 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始 */
    g_spi4_handle.Init.TIMode = SPI_TIMODE_DISABLE;                         /* 关闭TI模式 */
    g_spi4_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;         /* 关闭硬件CRC校验 */
    g_spi4_handle.Init.CRCPolynomial = 7;                                   /* 用于CRC计算的多项式 */
    HAL_SPI_Init(&g_spi4_handle);                                           /* 初始化SPI */
    
    __HAL_SPI_ENABLE(&g_spi4_handle);                                       /* 使能SPI4 */

    spi4_read_write_byte(0Xff);                                             /* 启动传输 */
}

/**
 * @brief       SPI4速度设置函数
 * @note        SPI4时钟选择来自pll2_q_ck, 为220Mhz
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

/**
 * @brief       SPI4模式设置
 * @param       mode    : 0, 8位普通模式; 1, 16位DMA模式;
 * @param       无
 * @retval      无
 */
void lcd_spi4_mode(uint8_t mode)
{
    if (mode == 0)   /* 8位普通模式,重新配置SPI */
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
        g_spi4_handle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;        /* 设置波特率预分频值:波特率预分频值设为32 */
        g_spi4_handle.Init.FirstBit = SPI_FIRSTBIT_MSB;                         /* 指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始 */
        g_spi4_handle.Init.TIMode = SPI_TIMODE_DISABLE;                         /* 关闭TI模式 */
        g_spi4_handle.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;         /* 关闭硬件CRC校验 */
        g_spi4_handle.Init.CRCPolynomial = 7;                                   /* 用于CRC计算的多项式 */
        HAL_SPI_Init(&g_spi4_handle);                                           /* 初始化SPI */
        
        __HAL_SPI_ENABLE(&g_spi4_handle);                                       /* 使能SPI4 */
    
        lcd_spi_speed_high();                                                   /* 设置SPI时钟频率 */
    }
    else     /* 16位DMA模式 */
    {      
        __HAL_SPI_DISABLE(&g_spi4_handle);             /* 关闭SPI */
        g_spi4_handle.Instance->CFG1 &= ~(0X1F << 0);  /* DSIZE[4:0]=0,清除原来的设置 */
        g_spi4_handle.Instance->CFG1 |= 15 << 0;       /* DSIZE[4:0]=15,设置SPI帧格式为16位 */
        g_spi4_handle.Instance->CFG1 |= 1 << 15;       /* TXDMAEN=1,使能发送DMA */
        __HAL_SPI_ENABLE(&g_spi4_handle);              /* 使能SPI */
    }
}

/**
 * @brief       SPI DMA配置
 * @param       meminc  : 存储器递增方式  0,存储器地址指针不增长; 1,存储器地址指针增长;
 * @retval      无
 */
void spi_dma_init(uint32_t meminc)
{
    __HAL_RCC_DMA2_CLK_ENABLE();                                          /* DMA2时钟使能 */

    g_dma_spi_handle.Instance = DMA2_Stream0;                             /* DMA2数据流0 */
    g_dma_spi_handle.Init.Request = DMA_REQUEST_SPI4_TX;                  /* SPI4 TX的DMA请求 */
    g_dma_spi_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;               /* 存储器到外设 */
    g_dma_spi_handle.Init.PeriphInc = DMA_PINC_DISABLE;                   /* 外设非增量模式 */
    g_dma_spi_handle.Init.MemInc = meminc;                                /* 设置存储器递增模式 */  
    g_dma_spi_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;  /* 外设数据长度:16位 */
    g_dma_spi_handle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;     /* 存储器数据长度:16位 */
    g_dma_spi_handle.Init.Mode = DMA_NORMAL;                              /* 普通模式 */
    g_dma_spi_handle.Init.Priority = DMA_PRIORITY_HIGH;                   /* 高优先级 */
    g_dma_spi_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;                /* 关闭FIFO模式 */
    g_dma_spi_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;        /* FIFO阈值设置 */
    g_dma_spi_handle.Init.MemBurst = DMA_MBURST_SINGLE;                   /* 存储器突发单次传输 */
    g_dma_spi_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;                /* 外设突发单次传输 */
  
    HAL_DMA_DeInit(&g_dma_spi_handle);                                    /* 先清除以前的设置 */
    HAL_DMA_Init(&g_dma_spi_handle);                                      /* 初始化DMA */  
}

/**
 * @brief       使用SPI到LCD的DMA传输方式在指定区域内填充单个颜色
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: 要填充的颜色
 * @retval      无
 */
void spi_dma_lcd_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t color)
{
		uint32_t lcdsize = (ex - sx + 1) * (ey - sy + 1);
		uint16_t addr[1];
  
    addr[0] = color;
    spi_dma_init(DMA_MINC_DISABLE);     /* SPI DMA配置 */
    HAL_DMA_Start(&g_dma_spi_handle, (uint32_t)addr, (uint32_t)&SPI4_SPI->TXDR, 0); /* 设置存储器和外设地址，暂时设置传输数据量为0 */
    lcd_set_window(sx, sy, (ex - sx + 1), (ey - sy + 1));  /* 设置窗口 */
    lcd_set_cursor(sx, sy);             /* 设置写入位置 */
    lcd_write_ram_prepare();            /* 开始写入GRAM */
  
    SPI_LCD_CS(0);
    SPI_LCD_DC(1);                      /* 写数据 */  
    lcd_spi4_mode(1);                   /* 设置为16位宽, 方便DMA传输 */
    SPI4_SPI->CR1 |= 1 << 9;            /* CSTART=1,启动传输 */

    while (lcdsize)
    {      
        __HAL_DMA_DISABLE(&g_dma_spi_handle);        /* 停止当前传输 */
  
        while (DMA2_Stream0->CR & 0X01);             /* 等待DMA2_Stream0可配置 */

        __HAL_DMA_CLEAR_FLAG(&g_dma_spi_handle, DMA_FLAG_TCIF0_4);        /* 清除上次的传输完成标志 */

        if (lcdsize > SPI_DMA_MAX_TRANS)
        {
            lcdsize -= SPI_DMA_MAX_TRANS;
            __HAL_DMA_SET_COUNTER(&g_dma_spi_handle, SPI_DMA_MAX_TRANS);  /* 设置DMA需要传输的数据量 */
        }
        else
        {
            __HAL_DMA_SET_COUNTER(&g_dma_spi_handle, lcdsize + 8);        /* 设置DMA需要传输的数据量 */
            lcdsize = 0;
        }
           
        __HAL_DMA_ENABLE(&g_dma_spi_handle);         /* 使能DMA，开启传输 */           

        while (__HAL_DMA_GET_FLAG(&g_dma_spi_handle, DMA_FLAG_TCIF0_4) == RESET);  /* 等待传输完成 */
    }
    
    __HAL_DMA_DISABLE(&g_dma_spi_handle);            /* 关闭DMA传输 */
    
    SPI_LCD_CS(1);  
    lcd_spi4_mode(0);   /* 恢复8位传输模式 */
}

/**
 * @brief       使用SPI到LCD的DMA传输方式在指定区域内填充指定颜色块
 * @param       (sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex - sx + 1) * (ey - sy + 1)
 * @param       color: 要填充的颜色数组首地址
 * @retval      无
 */
void spi_dma_lcd_color_fill(uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint16_t *color)
{
		uint32_t lcdsize = (ex - sx + 1) * (ey - sy + 1);
  
    spi_dma_init(DMA_MINC_ENABLE);      /* SPI DMA配置 */
    HAL_DMA_Start(&g_dma_spi_handle, (uint32_t)color, (uint32_t)&SPI4_SPI->TXDR, 0); /* 设置存储器和外设地址，暂时设置传输数据量为0 */
    lcd_set_window(sx, sy, (ex - sx + 1), (ey - sy + 1));  /* 设置窗口 */
    lcd_set_cursor(sx, sy);             /* 设置写入位置 */
    lcd_write_ram_prepare();            /* 开始写入GRAM */
  
    SPI_LCD_CS(0);
    SPI_LCD_DC(1);                      /* 写数据 */  
    lcd_spi4_mode(1);                   /* 设置为16位宽, 方便DMA传输 */
    SPI4_SPI->CR1 |= 1 << 9;            /* CSTART=1,启动传输 */

    while (lcdsize)
    {  
        __HAL_DMA_DISABLE(&g_dma_spi_handle);        /* 停止当前传输 */
  
        while (DMA2_Stream0->CR & 0X01);             /* 等待DMA2_Stream0可配置 */

        __HAL_DMA_CLEAR_FLAG(&g_dma_spi_handle, DMA_FLAG_TCIF0_4);        /* 清除上次的传输完成标志 */
   
        if (lcdsize > SPI_DMA_MAX_TRANS)
        {
            lcdsize -= SPI_DMA_MAX_TRANS;
            __HAL_DMA_SET_COUNTER(&g_dma_spi_handle, SPI_DMA_MAX_TRANS);  /* 设置DMA需要传输的数据量 */
        }
        else
        {
            __HAL_DMA_SET_COUNTER(&g_dma_spi_handle, lcdsize + 8);        /* 设置DMA需要传输的数据量 */
            lcdsize = 0;
        }
         
        __HAL_DMA_ENABLE(&g_dma_spi_handle);         /* 使能DMA，开启传输 */           

        while (__HAL_DMA_GET_FLAG(&g_dma_spi_handle, DMA_FLAG_TCIF0_4) == RESET);  /* 等待传输完成 */        
        
        if (lcdsize)
        {
            color += SPI_DMA_MAX_TRANS;
            DMA2_Stream0->M0AR = (uint32_t)color;    /* 改变DMA存储器0基地址 */
        }
    }
 
    __HAL_DMA_DISABLE(&g_dma_spi_handle);            /* 关闭DMA传输 */    
    
    SPI_LCD_CS(1);  
    lcd_spi4_mode(0);   /* 恢复8位传输模式 */
}








