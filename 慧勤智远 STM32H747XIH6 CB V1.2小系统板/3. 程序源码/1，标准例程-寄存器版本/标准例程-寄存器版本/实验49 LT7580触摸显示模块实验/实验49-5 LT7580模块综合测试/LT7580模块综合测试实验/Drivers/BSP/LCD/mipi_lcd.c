/**
 ****************************************************************************************************
 * @file        mipi_lcd.c
 * @version     V1.0
 * @brief       mipilcd 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/LCD/mipi_lcd.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/LCD/ltdc.h"
#include "./SYSTEM/delay/delay.h"


/* mipi_lcd_ex.c存放各个MIPI LCD驱动IC的初始化部分代码,以简化mipi_lcd.c,该.c文件
 * 不直接加入到工程里面,只有mipi_lcd.c会用到,所以通过include的形式添加.(不要在
 * 其他文件再包含该.c文件!!否则会报错!)	
 */
#include "./BSP/LCD/mipi_lcd_ex.c"


DSI_HandleTypeDef   g_dsi_handle;                 /* DSI句柄 */

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
extern uint32_t ltdc_lcd_framebuf[1280][800];     /* 定义最大屏分辨率时,LTDC所需的帧缓存数组大小 */
#else
extern uint16_t ltdc_lcd_framebuf[1280][800];     /* 定义最大屏分辨率时,LTDC所需的帧缓存数组大小 */
//extern uint16_t ltdc_lcd_framebuf1[1280][800];  /* 使用LTDC层2时使用（默认使用LTDC层1） */
#endif

extern uint32_t *g_ltdc_framebuf[2];              /* LTDC LCD帧缓存数组指针,必须指向对应大小的内存区域 */


/**
  * @brief  DSI写DCS/Generic指令
  * @param  channelnbr : 虚拟通道ID 
  * @param  cmd        : 要写入的命令
  * @param  pdata      : 数据存储区 
  * @param  size       : 要写入的字节数 
  * @retval 0, 成功; 1, 失败;
  */
uint8_t dsi_io_write(uint16_t channelnbr, uint16_t cmd, uint8_t *pdata, uint16_t size)
{
    uint8_t ret = 0;

    if (size == 0)          /* 写DCS短数据包，不带参数 */
    {
        if (HAL_DSI_ShortWrite(&g_dsi_handle, channelnbr, DSI_DCS_SHORT_PKT_WRITE_P0, cmd, (uint32_t)pdata[0]) != HAL_OK)
        {
            ret = 1;
        }
    }
    else if (size == 1)     /* 写DCS短数据包，带一个参数 */
    {
        if (HAL_DSI_ShortWrite(&g_dsi_handle, channelnbr, DSI_DCS_SHORT_PKT_WRITE_P1, cmd, (uint32_t)pdata[0]) != HAL_OK)
        {
            ret = 1;
        }
    }
    else                    /* 写DCS长数据包 */
    {
        if (HAL_DSI_LongWrite(&g_dsi_handle, channelnbr, DSI_DCS_LONG_PKT_WRITE, size, cmd, pdata) != HAL_OK)
        {
            ret = 1;
        }
    }

    return ret;
}

/**
  * @brief  DSI读DCS/Generic指令
  * @param  channelnbr : 虚拟通道ID 
  * @param  cmd        : 要读取的命令
  * @param  pdata      : 数据存储区 
  * @param  size       : 要读取的字节数 
  * @retval 0, 成功; 1, 失败;
  */
uint8_t dsi_io_read(uint16_t channelnbr, uint16_t cmd, uint8_t *pdata, uint16_t size)
{
    uint8_t ret = 0;     

    /* 读DCS短数据包 */
    if (HAL_DSI_Read(&g_dsi_handle, channelnbr, pdata, size, DSI_DCS_SHORT_PKT_READ, cmd, pdata) != HAL_OK)
    {
        ret = 1;
    }

    return ret;
}

/**
  * @brief  向MIPI LCD驱动IC写命令
  * @param  channelnbr : 虚拟通道ID 
  * @param  cmd        : 要写入的命令
  * @param  pdata      : 数据存储区 
  * @param  size       : 要写入的字节数 
  * @retval 0, 成功; 1, 失败;
  */
uint8_t mipilcd_write_cmd(uint16_t channelnbr, uint16_t cmd, const uint8_t *pdata, uint16_t length)
{
    return dsi_io_write(channelnbr, cmd, (uint8_t *)pdata, length);
}

/**
 * @brief       初始化DSI HOST
 * @param       无
 * @retval      0, 成功; 1, 失败.
 */
uint8_t dsi_host_init(void)
{
    DSI_PLLInitTypeDef pll_init;
    DSI_VidCfgTypeDef vidcfg; 
    uint32_t dsi_phy_ck;            /* D-PHY时钟频率 */
    uint32_t pixel_clk;             /* LTDC像素时钟频率 */
    uint32_t total_line;            /* 水平总宽度 */
    
    RCC->APB3ENR |= 1 << 4;         /* 使能DSI时钟 */   
  
    RCC->APB3RSTR |= 1 << 4;        /* 复位DSI */
    RCC->APB3RSTR &= ~(1 << 4);     /* 结束复位 */
  
    g_dsi_handle.Instance = DSI;                                                  /* DSI */
    g_dsi_handle.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE; /* 自动时钟通道控制 */
    g_dsi_handle.Init.TXEscapeCkdiv = 4;                                          /* TX escape clock分频因子为4 */
  
#if   MIPILCD_35_320480  
    g_dsi_handle.Init.NumberOfLanes = DSI_ONE_DATA_LANE;                          /* 1路数据线通道 */
#else
    g_dsi_handle.Init.NumberOfLanes = DSI_TWO_DATA_LANES;                         /* 2路数据线通道 */
#endif
  
    /**
     * 配置D-PHY时钟频率，D-PHY时钟用作blclk_ck(DSI byte lane clock)时钟源,
     * D-PHY时钟频率为F_PHY_Mhz = (NDIV * HSE_Mhz) / (IDF * ODF) 
     * 设置NDIV = 28, IDF = 1, ODF = 1, 则dsi_phy_ck时钟频率为:
     * F_PHY = 28 * 25 / (1 * 1) = 700Mhz
     */
    pll_init.PLLNDIV = 28;                                 /* DSI PLL循环分频系数, 取值范围: 10~125 */
    pll_init.PLLIDF = DSI_PLL_IN_DIV1;                     /* DSI PLL输入分频系数 */
    pll_init.PLLODF = DSI_PLL_OUT_DIV1;                    /* DSI PLL输出分频系数 */
    
    if (HAL_DSI_Init(&g_dsi_handle, &pll_init) != HAL_OK)  /* 初始化DSI配置 */
    {
        return 1;
    }
 
    /* Video模式配置 */
    total_line = LCD_DEFAULT_WIDTH + MIPI_HSYNC + MIPI_HBP + MIPI_HFP;
    dsi_phy_ck = 700000000;
    
    if (MIPILCD_35_320480)
    {
        pixel_clk = 25000000;
    }
    else if (MIPILCD_70_1024600)
    {
        pixel_clk = 50000000;    
    }
    else
    {
        pixel_clk = 60000000;    
    }
    
    vidcfg.VirtualChannelID = 0;                                   /* 虚拟通道ID */
    vidcfg.ColorCoding = DSI_RGB565;                               /* 设置DPI颜色格式 */
    vidcfg.LooselyPacked = DSI_LOOSELY_PACKED_DISABLE;             /* 不使能Loosely packet(仅在18位颜色格式时使用) */
    vidcfg.Mode = DSI_VID_MODE_BURST;                              /* 设置Video模式的传输方式为突发模式 */
    vidcfg.PacketSize = LCD_DEFAULT_WIDTH;                         /* 设置Video数据包大小 */
    vidcfg.NumberOfChunks = 0;                                     /* 设置在行周期要传输的块数 */
    vidcfg.NullPacketSize = 0xFFFU;                                /* 设置空包大小 */
    vidcfg.HSPolarity = DSI_HSYNC_ACTIVE_LOW;                      /* 水平同步极性:低电平有效 */
    vidcfg.VSPolarity = DSI_VSYNC_ACTIVE_LOW;                      /* 垂直同步极性:低电平有效 */
    vidcfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;               /* 数据使能极性:高电平有效 */
    vidcfg.HorizontalSyncActive = (uint16_t)(MIPI_HSYNC * ((double)dsi_phy_ck / 8 / pixel_clk));  /* 水平同步宽度(在blclk_ck时钟周期下计数) */
    vidcfg.HorizontalBackPorch = (uint16_t)(MIPI_HBP * ((double)dsi_phy_ck / 8 / pixel_clk));     /* 水平后沿宽度(在blclk_ck时钟周期下计数) */
    vidcfg.HorizontalLine = (uint16_t)(total_line * ((double)dsi_phy_ck / 8 / pixel_clk));        /* 水平总宽度(在blclk_ck时钟周期下计数) */
    vidcfg.VerticalSyncActive = MIPI_VSYNC;                        /* 垂直同步高度 */
    vidcfg.VerticalBackPorch = MIPI_VBP;                           /* 垂直后沿高度 */
    vidcfg.VerticalFrontPorch = MIPI_VFP;                          /* 垂直前沿高度 */
    vidcfg.VerticalActive = LCD_DEFAULT_HEIGHT;                    /* 垂直有效高度 */
    vidcfg.LPCommandEnable = DSI_LP_COMMAND_ENABLE;                /* 使能在低功耗模式下的指令传输 */
    vidcfg.LPLargestPacketSize = 4;                                /* 设置低功耗模式下在VSA,VBP和VFP期间传输最大包大小(字节数) */
    vidcfg.LPVACTLargestPacketSize = 4;                            /* 设置低功耗模式下在VACT期间传输最大包大小(字节数) */

    vidcfg.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;       /* 使能在HFP期间内返回低功耗模式 */
    vidcfg.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;       /* 使能在HBP期间内返回低功耗模式 */
    vidcfg.LPVerticalActiveEnable       = DSI_LP_VACT_ENABLE;      /* 使能在VACT期间内返回低功耗模式 */
    vidcfg.LPVerticalFrontPorchEnable   = DSI_LP_VFP_ENABLE;       /* 使能在VFP期间内返回低功耗模式 */
    vidcfg.LPVerticalBackPorchEnable    = DSI_LP_VBP_ENABLE;       /* 使能在VBP期间内返回低功耗模式 */
    vidcfg.LPVerticalSyncActiveEnable   = DSI_LP_VSYNC_ENABLE;     /* 使能在VSYNC期间内返回低功耗模式 */ 
    vidcfg.FrameBTAAcknowledgeEnable    = DSI_FBTAA_DISABLE;       /* 禁能在一帧结束时请求应答确认 */
    
    if (HAL_DSI_ConfigVideoMode(&g_dsi_handle, &vidcfg) != HAL_OK) /* 配置Video模式 */
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief       初始化LTDC
 * @param       无
 * @retval      无
 */
void dsi_ltdc_init(void)
{
    uint32_t tempreg = 0;

    RCC->APB3ENR |= 1 << 3;             /* 使能LTDC时钟 */
  
    /* LTDC配置 */
    tempreg = 0 << 28;                  /* 像素时钟极性:低电平有效 */
    tempreg |= 0 << 29;                 /* 数据使能极性:低电平有效 */
    tempreg |= 0 << 30;                 /* 垂直同步极性:低电平有效 */
    tempreg |= 0 << 31;                 /* 水平同步极性:低电平有效 */
    LTDC->GCR = tempreg;                /* 设置全局控制寄存器 */

    tempreg = (MIPI_VSYNC - 1) << 0;    /* 垂直同步高度 */
    tempreg |= (MIPI_HSYNC - 1) << 16;  /* 水平同步宽度 */
    LTDC->SSCR = tempreg;               /* 设置同步大小配置寄存器 */
  
    tempreg = (MIPI_VSYNC + MIPI_VBP - 1) << 0;     /* 累加垂直后沿高度 */
    tempreg |= (MIPI_HSYNC + MIPI_HBP - 1) << 16;   /* 累加水平后沿宽度 */
    LTDC->BPCR = tempreg;                           /* 设置后沿配置寄存器 */
 
    tempreg = (MIPI_VSYNC + MIPI_VBP + LCD_DEFAULT_HEIGHT - 1) << 0;    /* 累加有效高度 */
    tempreg |= (MIPI_HSYNC + MIPI_HBP + LCD_DEFAULT_WIDTH - 1) << 16;   /* 累加有效宽度 */
    LTDC->AWCR = tempreg;                                               /* 设置有效宽度配置寄存器 */

    tempreg = (MIPI_VSYNC + MIPI_VBP + LCD_DEFAULT_HEIGHT + MIPI_VFP - 1) << 0;   /* 总高度 */
    tempreg |= (MIPI_HSYNC + MIPI_HBP + LCD_DEFAULT_WIDTH + MIPI_HFP- 1) << 16;   /* 总宽度 */
    LTDC->TWCR = tempreg;                                                         /* 设置总宽度配置寄存器 */

    LTDC->BCCR = LTDC_BACKLAYERCOLOR;   /* 设置背景层颜色寄存器(RGB888格式) */
    ltdc_switch(1);                     /* 开启LTDC */
    
    g_ltdc_framebuf[0] = (uint32_t *)&ltdc_lcd_framebuf;
    /* 层配置 */
    ltdc_layer_parameter_config(0, (uint32_t)g_ltdc_framebuf[0], LTDC_PIXFORMAT, 255, 0, 6, 7, 0X000000);   /* 层参数配置 */
    ltdc_layer_window_config(0, 0, 0, LCD_DEFAULT_WIDTH, LCD_DEFAULT_HEIGHT);                               /* 层窗口配置 */
    
    /* LTDC参数设置 */
    lcdltdc.pwidth = LCD_DEFAULT_WIDTH;   /* 面板宽度,单位:像素 */    
    lcdltdc.pheight = LCD_DEFAULT_HEIGHT; /* 面板高度,单位:像素 */ 
    lcddev.width = lcdltdc.pwidth;        /* 设置lcddev的宽度参数 */
    lcddev.height = lcdltdc.pheight;      /* 设置lcddev的高度参数 */
    lcdltdc.pixformat = LTDC_PIXFORMAT;   /* 颜色像素格式 */
    
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888
    lcdltdc.pixsize = 4;    /* 每个像素占4个字节 */
#elif LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
    lcdltdc.pixsize = 3;    /* 每个像素占3个字节 */
#else
    lcdltdc.pixsize = 2;    /* 每个像素占2个字节 */
#endif

    ltdc_display_dir(1);                  /* 默认橫屏，在lcd_init函数里面设置 */
    ltdc_select_layer(0);                 /* 选择第1层 */
}

/**
 * @brief       初始化DSI接口
 * @param       无
 * @retval      0, 成功; 1, 失败.
 */
uint8_t dsi_init(void)
{
    uint8_t mipi_id[3];
    uint8_t reg_data[4] = {0};
    
    DSI_BL_GPIO_CLK_ENABLE();         /* DSI_BL引脚时钟使能 */
    DSI_RST_GPIO_CLK_ENABLE();        /* DSI_RST引脚时钟使能 */  
  
    sys_gpio_set(DSI_BL_GPIO_PORT, DSI_BL_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* DSI_BL引脚模式设置,推挽输出 */
  
    sys_gpio_set(DSI_RST_GPIO_PORT, DSI_RST_GPIO_PIN,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* DSI_RST引脚模式设置,推挽输出 */
  
    /* MIPI LCD复位 */
    DSI_RST(1);
    delay_ms(20);
    DSI_RST(0);
    delay_ms(50);
    DSI_RST(1); 
    delay_ms(200);
  
    DSI_BL(0);                  /* 关闭背光 */
 
    dsi_host_init();            /* 初始化DSI HOST */
  
#if   MIPILCD_35_320480  
    ltdc_clk_set(200, 25, 8);   /* 设置像素时钟 25Mhz */
#elif MIPILCD_70_1024600
    ltdc_clk_set(300, 25, 6);   /* 设置像素时钟 50Mhz */
#else
    ltdc_clk_set(300, 25, 5);   /* 设置像素时钟 60Mhz */
#endif

    dsi_ltdc_init();            /* 初始化DSI的LTDC */    
    delay_ms(50);
    
    /* 在LTDC初始化后开启DSI模块(为了避免同步问题，DSI应在使能LTDC后再开启) */
    HAL_DSI_Start(&g_dsi_handle);
    
    /* 为读取操作开启DSI的BTA请求 */
    HAL_DSI_ConfigFlowControl(&g_dsi_handle, DSI_FLOW_CONTROL_BTA);        
    delay_ms(10);
 
    if (MIPILCD_70_1024600)
    {
        lcddev.id = 0X9007;
    }
    else
    {
        /* 检查是否有MIPI屏接入 */ 
        dsi_io_read(0, 0xD3, mipi_id, 3); /* 尝试ST7796 ID的读取 */
        lcddev.id = (mipi_id[1] << 8) | mipi_id[2];
        
        if (lcddev.id != 0X7796)    /* 不是ST7796,尝试看看是不是ST7703 */
        {
            reg_data[0] = 0x98;
            reg_data[1] = 0x81;
            reg_data[2] = 0x00;     /* Page 0 */
            mipilcd_write_cmd(0, 0xFF, (uint8_t *)reg_data, 3);
            mipilcd_write_cmd(0, 0x00, (uint8_t *)reg_data, 0); /* 先写一个空指令 */  
           
            dsi_io_read(0, 0x04, mipi_id, 3); /* 尝试ST7703 ID的读取 */
           
            if (mipi_id[0] == 0x38 && mipi_id[1] == 0x21 && mipi_id[2] == 0x1F) 
            {
                lcddev.id = 0X7703;
            }
            else                              /* 不是ST7703,尝试看看是不是ILI9881 */
            {
                reg_data[0] = 0x98;
                reg_data[1] = 0x81;
                reg_data[2] = 0x06;
                mipilcd_write_cmd(0, 0xFF, (uint8_t *)reg_data, 3);
                dsi_io_read(0, 0xF0, &mipi_id[0], 1);
                dsi_io_read(0, 0xF1, &mipi_id[1], 1);
                dsi_io_read(0, 0xF2, &mipi_id[2], 1);
                lcddev.id = (mipi_id[0] << 8) | mipi_id[1];
              
                if (lcddev.id != 0X9881)      /* 不是ILI9881D,尝试是不是ILI9881C */
                {
                    reg_data[0] = 0x98;
                    reg_data[1] = 0x81;
                    reg_data[2] = 0x01;        
                    mipilcd_write_cmd(0, 0xFF, (uint8_t *)reg_data, 3);
                    dsi_io_read(0, 0x00, &mipi_id[0], 1);
                    dsi_io_read(0, 0x01, &mipi_id[1], 1);
                    dsi_io_read(0, 0x02, &mipi_id[2], 1);
                    lcddev.id = (mipi_id[0] << 8) | mipi_id[1];
                }
            }
        }
    }    
    
    if (lcddev.id == 0X7796 || lcddev.id == 0X7703 || lcddev.id == 0X9881 || lcddev.id == 0X9007)
    {
#if   MIPILCD_35_320480
        mipi_lcd_ex_st7796_cmdinit();
#elif MIPILCD_70_1024600
        mipi_lcd_ex_ek79007_cmdinit();
#elif MIPILCD_50_7201280
        mipi_lcd_ex_ili9881d_cmdinit();
#elif MIPILCD_55_7201280
        mipi_lcd_ex_st7703_cmdinit();
#elif MIPILCD_80_8001280
        mipi_lcd_ex_ili9881c80_cmdinit();
#elif MIPILCD_10_8001280
        mipi_lcd_ex_ili9881c10_cmdinit();
#endif
        return 0;    
    }
        
    return 1;
}

/**
 * @brief       HAL库内部函数用到的延时
 * @param       Delay : 要延时的毫秒数
 * @retval      无
 */
void HAL_Delay(uint32_t Delay)
{
    delay_ms(Delay);
}








