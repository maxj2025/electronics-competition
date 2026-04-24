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
    sys_gpio_set(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);  /* SCK引脚模式设置(推挽输出) */
  
    SPI_LCD_SCL(0);
    delay_us(1);
    SPI_LCD_SCL(1);          /* dummy clock cycle */

    sys_gpio_set(SPI4_SCK_GPIO_PORT, SPI4_SCK_GPIO_PIN,
                 SYS_GPIO_MODE_AF, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SCK引脚模式设置(复用推挽输出) */  
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
    /* IO 及 时钟配置 */
    spi_io_init();                     /* SPI接口初始化 */

    SPI_LCD_DC_GPIO_CLK_ENABLE();      /* SPI_LCD_DC引脚时钟使能 */
    SPI_LCD_BL_GPIO_CLK_ENABLE();      /* SPI_LCD_BL引脚时钟使能 */
    SPI_LCD_RST_GPIO_CLK_ENABLE();     /* SPI_LCD_RST引脚时钟使能 */
  
    sys_gpio_set(SPI_LCD_DC_GPIO_PORT, SPI_LCD_DC_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SPI_LCD_DC引脚模式设置(推挽输出) */

    sys_gpio_set(SPI_LCD_BL_GPIO_PORT, SPI_LCD_BL_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SPI_LCD_BL引脚模式设置(推挽输出) */

    sys_gpio_set(SPI_LCD_RST_GPIO_PORT, SPI_LCD_RST_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_HIGH, SYS_GPIO_PUPD_PU);   /* SPI_LCD_RST引脚模式设置(推挽输出) */
                 
	  delay_ms(50);
     
    /* LCD复位 */
	  SPI_LCD_RST(1);
	  delay_ms(10);
	  SPI_LCD_RST(0);
	  delay_ms(50);
	  SPI_LCD_RST(1); 
		delay_ms(200); 
}

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

    /* 配置SPI4/5的内核时钟源, 选择pll2_q_ck作为时钟源, 220Mhz */
    RCC->D2CCIP1R &= ~(7 << 16);    /* SPI45SEL[2:0]=0,清除原来的设置 */
    RCC->D2CCIP1R |= 1 << 16;       /* SPI45SEL[2:0]=1,选择pll2_q_ck作为SPI4/5的时钟源,在系统时钟初始化函数中设置为220Mhz */
 
    /* 这里只针对SPI4_SPI初始化 */
    RCC->APB2RSTR |= 1 << 13;       /* 复位SPI4_SPI */
    RCC->APB2RSTR &= ~(1 << 13);    /* 停止复位SPI4_SPI */

    SPI4_SPI->CR1 |= 1 << 12;       /* SSI=1,设置内部SS信号为高电平 */
    SPI4_SPI->CFG1 = 4 << 28;       /* MBR[2:0]=4,设置spi_ker_ck为32分频. */
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
 * @note        SPI4时钟选择来自pll2_q_ck, 为220Mhz
 *              SPI速度 = spi_ker_ck / 2^(speed + 1)
 * @param       speed: SPI时钟分频系数
 * @retval      无
 */
void spi4_set_speed(uint8_t speed)
{
    speed &= 0X07;                           /* 限制范围 */
    SPI4_SPI->CR1 &= ~(1 << 0);              /* SPE=0,SPI设备失能 */
    SPI4_SPI->CFG1 &= ~(7 << 28);            /* MBR[2:0]=0,清除原来的分频设置 */
    SPI4_SPI->CFG1 |= (uint32_t)speed << 28; /* MBR[2:0]=speed,设置SPI时钟分频系数 */
    SPI4_SPI->CR1 |= 1 << 0;                 /* SPE=1,SPI设备使能 */
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

/**
 * @brief       SPI4模式设置
 * @param       mode    : 0, 8位普通模式; 1, 16位DMA模式;
 * @param       无
 * @retval      无
 */
void lcd_spi4_mode(uint8_t mode)
{
    uint32_t tempreg = 0;

    if (mode == 0)   /* 8位普通模式,重新配置SPI */
    {
        RCC->APB2RSTR |= 1 << 13;       /* 复位SPI4_SPI */
        RCC->APB2RSTR &= ~(1 << 13);    /* 停止复位SPI4_SPI */

        SPI4_SPI->CR1 |= 1 << 12;       /* SSI=1,设置内部SS信号为高电平 */
        SPI4_SPI->CFG1 = 4 << 28;       /* MBR[2:0]=4,设置spi_ker_ck为32分频. */
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
    
        lcd_spi_speed_high();           /* 设置SPI时钟频率 */
    }
    else     /* 16位DMA模式 */
    {      
        SPI4_SPI->CR1 &= ~(1 << 0);     /* SPE=0,SPI设备失能 */
        SPI4_SPI->CFG1 &= ~(0X1F << 0); /* DSIZE[4:0]=0,清除原来的设置 */
        SPI4_SPI->CFG1 |= 15 << 0;      /* DSIZE[4:0]=15,设置SPI帧格式为16位 */
        SPI4_SPI->CFG1 |= 1 << 15;      /* TXDMAEN=1,使能发送DMA */
        SPI4_SPI->CR1 |= 1 << 0;        /* SPE=1,SPI设备使能 */
    }
}

/**
 * @brief       SPI DMA配置
 * @param       mar     : 存储器0地址
 * @param       meminc  : 存储器递增方式  0,存储器地址指针不增长; 1,存储器地址指针增长;
 * @retval      无
 */
void spi_dma_init(uint32_t mar, uint8_t meminc)
{
    uint32_t tempreg = 0;
  
    RCC->AHB1ENR |= 1 << 1;             /* DMA2时钟使能 */
    RCC->D3AMR |= 1 << 0;               /* DMAMUX时钟使能 */

    while (DMA2_Stream0->CR & 0X01);    /* 等待DMA2_Stream0可配置 */

    DMAMUX1_Channel8->CCR = 84;         /* DMA2_stream0的通道选择: 84,即SPI4_TX对应的通道 */

    DMA2->LIFCR |= 0X3D << 6 * 0;       /* 清空通道0上所有中断标志 */
    DMA2_Stream0->FCR = 0X0000021;      /* 设置为默认值 */

    DMA2_Stream0->PAR = (uint32_t)&SPI4_SPI->TXDR; /* 外设地址为:SPI4_SPI->TXDR */
    DMA2_Stream0->M0AR = mar;           /* 存储器0地址为: mar */
    DMA2_Stream0->NDTR = 0;             /* 暂时设置传输数据量为0 */
    tempreg |= 1 << 6;                  /* 存储器到外设模式 */
    tempreg |= 0 << 8;                  /* 非循环模式(即使用普通模式) */
    tempreg |= 0 << 9;                  /* 外设非增量模式 */
    tempreg |= meminc << 10;            /* 设置存储器递增模式 */
    tempreg |= 1 << 11;                 /* 外设数据长度:16位 */
    tempreg |= 1 << 13;                 /* 存储器数据长度:16位 */
    tempreg |= 2 << 16;                 /* 高优先级 */
    tempreg |= 0 << 21;                 /* 外设突发单次传输 */
    tempreg |= 0 << 23;                 /* 存储器突发单次传输 */
    DMA2_Stream0->CR = tempreg;         /* 设置CR寄存器 */
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
    spi_dma_init((uint32_t)addr, 0);    /* SPI DMA配置 */
    lcd_set_window(sx, sy, (ex - sx + 1), (ey - sy + 1));  /* 设置窗口 */
    lcd_set_cursor(sx, sy);             /* 设置写入位置 */
    lcd_write_ram_prepare();            /* 开始写入GRAM */
  
    SPI_LCD_CS(0);
    SPI_LCD_DC(1);                      /* 写数据 */  
    lcd_spi4_mode(1);                   /* 设置为16位宽, 方便DMA传输 */
    SPI4_SPI->CR1 |= 1 << 9;            /* CSTART=1,启动传输 */

    while (lcdsize)
    {      
        DMA2_Stream0->CR &= ~(1 << 0);               /* 停止当前传输 */
  
        while (DMA2_Stream0->CR & 0X01);             /* 等待DMA2_Stream0可配置 */

        DMA2->LIFCR |= 1 << 5;                       /* 清除上次的传输完成标记 */

        if (lcdsize > SPI_DMA_MAX_TRANS)
        {
            lcdsize -= SPI_DMA_MAX_TRANS;
            DMA2_Stream0->NDTR = SPI_DMA_MAX_TRANS;  /* 设置DMA需要传输的数据量 */
        }
        else
        {
            DMA2_Stream0->NDTR = lcdsize + 8;        /* 设置DMA需要传输的数据量 */
            lcdsize = 0;
        }
                
        DMA2_Stream0->CR |= 1 << 0;                  /* 开启DMA传输 */

        while ((DMA2->LISR & (1 << 5)) == 0);        /* 等待传输完成 */
    }
    
    DMA2_Stream0->CR &= ~(1 << 0);                   /* 关闭DMA传输 */
    
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
  
    spi_dma_init((uint32_t)color, 1);   /* SPI DMA配置 */
    lcd_set_window(sx, sy, (ex - sx + 1), (ey - sy + 1));  /* 设置窗口 */
    lcd_set_cursor(sx, sy);             /* 设置写入位置 */
    lcd_write_ram_prepare();            /* 开始写入GRAM */
  
    SPI_LCD_CS(0);
    SPI_LCD_DC(1);                      /* 写数据 */  
    lcd_spi4_mode(1);                   /* 设置为16位宽, 方便DMA传输 */
    SPI4_SPI->CR1 |= 1 << 9;            /* CSTART=1,启动传输 */

    while (lcdsize)
    {      
        DMA2_Stream0->CR &= ~(1 << 0);               /* 停止当前传输 */
  
        while (DMA2_Stream0->CR & 0X01);             /* 等待DMA2_Stream0可配置 */

        DMA2->LIFCR |= 1 << 5;                       /* 清除上次的传输完成标记 */

        if (lcdsize > SPI_DMA_MAX_TRANS)
        {
            lcdsize -= SPI_DMA_MAX_TRANS;
            DMA2_Stream0->NDTR = SPI_DMA_MAX_TRANS;  /* 设置DMA需要传输的数据量 */
        }
        else
        {
            DMA2_Stream0->NDTR = lcdsize + 8;        /* 设置DMA需要传输的数据量 */
            lcdsize = 0;
        }
                
        DMA2_Stream0->CR |= 1 << 0;                  /* 开启DMA传输 */

        while ((DMA2->LISR & (1 << 5)) == 0);        /* 等待传输完成 */
        
        if (lcdsize)
        {
            color += SPI_DMA_MAX_TRANS;
            DMA2_Stream0->M0AR = (uint32_t)color;    /* 改变DMA存储器0基地址 */
        }
    }
    
    DMA2_Stream0->CR &= ~(1 << 0);                   /* 关闭DMA传输 */
    
    SPI_LCD_CS(1);  
    lcd_spi4_mode(0);   /* 恢复8位传输模式 */
}







