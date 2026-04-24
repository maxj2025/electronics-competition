/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.2
 * @brief       综合测试 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 * V1.1
 * 修改了一些MIPI屏的参数配置
 *
 * V1.2
 * 修改了一些MIPI屏的参数配置
 * 添加对5.5寸1080P MIPI屏的兼容
 *
 ****************************************************************************************************
 */
 
#include "os.h"
#include "spb.h"
#include "common.h"
#include "ebook.h"
#include "picviewer.h"
#include "audioplay.h"
#include "videoplay.h"
#include "calendar.h"
#include "settings.h" 
#include "nesplay.h"
#include "notepad.h"
#include "exeplay.h"
#include "paint.h"
#include "camera.h"
//#include "recorder.h"  
#include "usbplay.h" 
//#include "netplay.h"
//#include "wirelessplay.h"
#include "calculator.h"
#include "appplay.h"
#include "ledplay.h"
#include "keyplay.h"
#include "vmeterplay.h"
#include "usb_app.h"

 
#if !(__ARMCC_VERSION >= 6010050)       /* 不是AC6编译器，即使用AC5编译器时 */
#define __ALIGNED_8     __align(8)      /* AC5使用这个 */
#else                                   /* 使用AC6编译器时 */
#define __ALIGNED_8     __ALIGNED(8)    /* AC6使用这个 */
#endif

extern volatile uint8_t system_task_return; /* 任务强制返回标志 */

/******************************************************************************************/
/* UCOSII任务设置 */

/* START 任务 配置 */
#define START_TASK_PRIO                 10                  /* 开始任务的优先级设置为最低 */
#define START_STK_SIZE                  128                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK START_TASK_STK[START_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void start_task(void *pdata);                               /* 任务函数 */


/* 串口 任务 配置 */
#define USART_TASK_PRIO                 7                   /* 任务优先级 */
#define USART_STK_SIZE                  128                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK USART_TASK_STK[USART_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void usart_task(void *pdata);                               /* 任务函数 */


/* 主 任务 配置 */
#define MAIN_TASK_PRIO                  6                   /* 任务优先级 */
#define MAIN_STK_SIZE                   1200                /* 堆栈大小 */

__ALIGNED_8 static OS_STK MAIN_TASK_STK[MAIN_STK_SIZE];     /* 任务堆栈,8字节对齐 */
void main_task(void *pdata);                                /* 任务函数 */


/* 监视 任务 配置 */
#define WATCH_TASK_PRIO                 3                   /* 任务优先级 */
#define WATCH_STK_SIZE                  256                 /* 堆栈大小 */

__ALIGNED_8 static OS_STK WATCH_TASK_STK[WATCH_STK_SIZE];   /* 任务堆栈,8字节对齐 */
void watch_task(void *pdata);                               /* 任务函数 */

/******************************************************************************************/

/**
 * @brief       外部内存测试(最大支持32M字节内存测试)
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,成功; 1,失败.
 */
uint8_t system_exsram_test(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint32_t i = 0;
    uint16_t j = 0;
    uint32_t temp = 0;
    uint32_t sval = 0;  /* 在地址0读到的数据 */
    uint32_t *tempbuf;
  
    tempbuf = mymalloc(SRAMIN, 1024 * 4);
    lcd_show_string(x, y, lcddev.width, y + fsize, fsize, "Ex Memory Test:    0KB", WHITE);

    /* 每隔32K字节,写入一个数据,总共写入1024个数据,刚好是32M字节 */
    for(i = 0; i < 32 * 1024 * 1024; i += 32 * 1024)
    {
        tempbuf[temp] = *(volatile uint32_t *)(BANK6_SDRAM_ADDR + i); /* 保存原来的数据 */
        delay_us(1);
        *(volatile uint32_t*)(BANK6_SDRAM_ADDR + i) = temp; 
        temp++;
    }

    /* 依次读出之前写入的数据,进行校验 */
    for(i = 0; i < 32 * 1024 * 1024; i += 32 * 1024)
    {
        temp = *(volatile uint32_t*)(BANK6_SDRAM_ADDR + i);
        delay_us(1);
        *(volatile uint32_t*)(BANK6_SDRAM_ADDR + i) = tempbuf[j++]; 

        if (i == 0)
        {
            sval = temp;
        }
        else if (temp <= sval)
        {
            break;                      /* 后面读出的数据一定要比第一次读到的数据大 */
        }
      
        delay_us(10);                   /* 延时一下再显示 */                   
        lcd_show_xnum(x + 15 * (fsize / 2), y, (temp - sval + 1) * 32, 5, fsize, 0, WHITE); /* 显示内存容量 */
    }

    myfree(SRAMIN, tempbuf);            /* 释放内存 */
    
    if (i >= 32 * 1024 * 1024)
    {
//        lcd_show_xnum(x + 15 * (fsize / 2), y, i / 1024, 5, fsize, 0, WHITE);   /* 显示内存值 */
        return 0;   /* 内存正常,成功 */
    }

    return 1;       /* 失败 */
}

/**
 * @brief       显示错误信息,停止运行,持续提示错误信息
 * @param       x, y : 坐标
 * @param       err  : 错误信息
 * @param       fsize: 字体大小
 * @retval      无
 */
void system_error_show(uint16_t x, uint16_t y, uint8_t *err, uint8_t fsize)
{
    while (1)
    {
        lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, (char *)err, RED);
        delay_ms(400);
        lcd_fill(x, y, lcddev.width - 1, y + fsize, BLACK);
        delay_ms(100);
        LED0_TOGGLE();
    }
}

/**
 * @brief       显示错误信息, 显示以后(2秒), 继续运行
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @param       str  : 字符串
 * @retval      无
 */
void system_error_show_pass(uint16_t x, uint16_t y, uint8_t fsize, uint8_t *str)
{
    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, (char *)str, RED);
    delay_ms(2000);
}

/**
 * @brief       擦除整个SPI FLASH(即所有资源都删除),以快速更新系统.
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,没有擦除; 1,擦除了;
 */
uint8_t system_files_erase(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint8_t key;
    uint8_t t = 0;

    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, "Erase all system files?", RED);

    while (1)
    {
        t++;

        if (t == 20)
        {
            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "WK_UP:NO / KEY0:YES", RED);
        }

        if (t == 40)
        {
            gui_fill_rectangle(x, y + fsize, lcddev.width, fsize, BLACK); /* 清除显示 */
            t = 0;
            LED0_TOGGLE();
        }

        key = key_scan(0);

        if (key == WKUP_PRES)   /* 不擦除，用户取消了 */
        {
            gui_fill_rectangle(x, y, lcddev.width, fsize * 2, BLACK); /* 清除显示 */
            //g_point_color = WHITE;
            LED0(1);
            return 0;
        }

        if (key == KEY0_PRES)   /* 要擦除，要重新来过 */
        {
            LED0(1);
            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "Erasing SPI FLASH...", RED);

            norflash_erase_chip();  /* 全片擦除，效率高 */

            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "Erasing SPI FLASH OK      ", RED);
            delay_ms(600);
            return 1;
        }

        delay_ms(10);
    }
}

/**
 * @brief       字库更新确认提示.
 * @param       x, y : 坐标
 * @param       fsize: 字体大小
 * @retval      0,不需要更新; 1,确认要更新;
 */
uint8_t system_font_update_confirm(uint16_t x, uint16_t y, uint8_t fsize)
{
    uint8_t key;
    uint8_t t = 0;
    uint8_t res = 0;
  
    lcd_show_string(x, y, lcddev.width, lcddev.height, fsize, "Update font?", RED);

    while (1)
    {
        t++;

        if (t == 20)
        {
            lcd_show_string(x, y + fsize, lcddev.width, lcddev.height, fsize, "WK_UP:NO / KEY0:YES", RED);
        }
        
        if (t == 40)
        {
            gui_fill_rectangle(x, y + fsize, lcddev.width, fsize, BLACK); /* 清除显示 */
            t = 0;
            LED0_TOGGLE();
        }

        key = key_scan(0);

        if (key == WKUP_PRES)
        {
            break;      /* 不更新 */
        }

        if (key == KEY0_PRES)
        {
            res = 1;    /* 要更新 */
            break;
        }

        delay_ms(10);
    }

    LED0(1);
    gui_fill_rectangle(x, y, lcddev.width, fsize * 2, BLACK); /* 清除显示 */
    
    return res;
}

//uint8_t g_tpad_failed_flag = 1;     /* TPAD失效标记，使用WK_UP作为退出按键 */

/**
 * @brief       系统初始化
 * @param       无
 * @retval      无
 */
void system_init(void)
{  
    uint16_t okoffset = 162;
    uint16_t xpos = 0,ypos = 0;
    uint16_t i = 0,j = 0;
    uint16_t temp = 0;
    uint8_t res;
    uint32_t dtsize,dfsize;
    uint8_t *stastr = 0;
    uint8_t *version = 0; 
    uint8_t verbuf[12];
    uint8_t fsize;
    uint8_t icowidth;
    uint16_t icowidth1;
    uint16_t icowidth2;
    uint16_t icowidth3;
    
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    exeplay_app_check();                    /* 检测是否需要运行APP代码 */
    usart_init(120, 115200);                /* 初始化USART */
    usmart_init(240);	                      /* 初始化USMART */        
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */    
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    bl24cxx_init();                         /* 初始化24CXX */
    norflash_init();                        /* 初始化NORFLASH */
    adc3_init();                            /* 初始化ADC3(使能内部温度传感器) */
    
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */

    tp_dev.init();                          /* 触摸屏初始化 */
    gui_init();
    piclib_init();                          /* piclib初始化 */
    slcd_dma_init();                        /* SLCD DMA初始化 */
    usbapp_init();
    exfuns_init();                          /* FATFS 申请内存 */

    version = mymalloc(SRAMIN, 31);         /* 申请31个字节内存 */
 
REINIT: /* 重新初始化 */

    lcd_clear(BLACK);                       /* 黑屏 */
    g_point_color = WHITE;
    g_back_color = BLACK;
    j = 0;

    /* 显示版权信息 */
    ypos = 2;

    if (lcddev.width <= 272)
    {
        fsize = 12;
        icowidth = 24;
        icowidth1 = 64;
			  icowidth2 = 124;
			  icowidth3 = icowidth1 + icowidth2;
			  xpos = (lcddev.width - icowidth3) / 2;
        okoffset = 190;                
        app_show_mono_icos(xpos, ypos, icowidth1, icowidth, (uint8_t *)APP_WKS_ICO6424, RED, BLACK);	
        app_show_mono_icos(xpos + icowidth1, ypos, icowidth2, icowidth, (uint8_t *)APP_WKS_ICO12424, WHITE, BLACK);
    }
    else if (lcddev.width == 320)
    {
        fsize = 16;
        icowidth = 32;
        icowidth1 = 88;
			  icowidth2 = 164;
			  icowidth3 = icowidth1 + icowidth2;
			  xpos = (lcddev.width - icowidth3) / 2;
        okoffset = 250;
        app_show_mono_icos(xpos, ypos, icowidth1, icowidth, (uint8_t *)APP_WKS_ICO8832, RED, BLACK);	
        app_show_mono_icos(xpos + icowidth1, ypos, icowidth2, icowidth, (uint8_t *)APP_WKS_ICO16432, WHITE, BLACK);		
    }
    else if (lcddev.width == 800)
    {
        fsize = 24;
        icowidth = 32;
        icowidth1 = 88;
			  icowidth2 = 164;
			  icowidth3 = icowidth1 + icowidth2;
			  xpos = (lcddev.width - icowidth3) / 2;
        okoffset = 370;
        app_show_mono_icos(xpos, ypos, icowidth1, icowidth, (uint8_t *)APP_WKS_ICO8832, RED, BLACK);	
        app_show_mono_icos(xpos + icowidth1, ypos, icowidth2, icowidth, (uint8_t *)APP_WKS_ICO16432, WHITE, BLACK);		
    }
    else 
    {
        fsize = 24;
        icowidth = 48;
        icowidth1 = 128;
			  icowidth2 = 248;
			  icowidth3 = icowidth1 + icowidth2;
			  xpos = (lcddev.width - icowidth3) / 2;
        okoffset = 370;
        app_show_mono_icos(xpos, ypos, icowidth1, icowidth, (uint8_t *)APP_WKS_ICO12848, RED, BLACK);	
        app_show_mono_icos(xpos + icowidth1, ypos, icowidth2, icowidth, (uint8_t *)APP_WKS_ICO24848, WHITE, BLACK);	
    }		

  	ypos = 5 + icowidth + 5;	
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "WKS STM32H747 Core Board", g_point_color);
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "Copyright (C) 2022-2032", g_point_color);
    app_get_version(verbuf, HARDWARE_VERSION, 2);
    strcpy((char *)version, "HARDWARE:");
    strcat((char *)version, (const char *)verbuf);
    strcat((char *)version, ", SOFTWARE:");
    app_get_version(verbuf, SOFTWARE_VERSION, 3);
    strcat((char *)version, (const char *)verbuf);
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)version, g_point_color);
    sprintf((char *)verbuf, "LCD ID:%04X", lcddev.id);  /* LCD ID打印到verbuf里面 */
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)verbuf, g_point_color);  /* 显示LCD ID */

    /* 开始硬件检测初始化 */
    LED0(0);
    LED1(0);    /* 同时点亮2个LED */
    
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "CPU:STM32H747XIH6 480Mhz", g_point_color);
    lcd_show_string(5, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "FLASH:2048KB  SRAM:1060KB", g_point_color);

    /* 外部SDRAM检测 */
    if (system_exsram_test(5, ypos + fsize * j, fsize)) 
    {
        system_error_show(5, ypos + fsize * j++, "EX Memory Error!", fsize);
    }
    
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);
    my_mem_init(SRAMEX);     /* SDRAM检测后,会搞乱内存分配,得重新初始化内存管理 */
    
    LED0(1);
    LED1(1);                 /* 同时关闭2个LED */
    
    /* SD NAND检测 */
    if (sdnand_init())       /* 检测不到SD NAND */
    {
        system_error_show(5, ypos + fsize * j++, "SD NAND Error!!", fsize);
    }
    else
    {
        temp = g_sdnand_info.CardCapacity >> 20;  /* SD NAND容量 */
    }
    
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SD NAND:    MB", g_point_color);
    lcd_show_xnum(5 + 8 * (fsize / 2), ypos + fsize * j, temp, 4, fsize, 0, g_point_color);  /* 显示SD NAND大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);    
    
    /* SPI FLASH检测 */
    if (norflash_read_id() != W25Q256)  /* 读取QSPI FLASH ID */
    {
        system_error_show(5, ypos + fsize * j++, "SPI Flash Error!!", fsize);
    }
    else temp = 32 * 1024;  /* 32M字节大小 */

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash:     KB", g_point_color);
    lcd_show_xnum(5 + 10 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color); /* 显示spi flash大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* 检测是否需要擦除SPI FLASH? */
    res = key_scan(1);

    if (res == WKUP_PRES)   /* 启动的时候，按下WKUP按键，则擦除SPI FLASH字库和文件系统区域 */
    {
        res = system_files_erase(5, ypos + fsize * j, fsize);

        if (res)goto REINIT;
    }

    /* RTC检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "RTC Check...", g_point_color);

    if (rtc_init())system_error_show_pass(5 + okoffset, ypos + fsize * j++, fsize, "ERROR"); /* RTC检测 */
    else
    {
        calendar_get_time(&calendar);/* 得到当前时间 */
        calendar_get_date(&calendar);/* 得到当前日期 */
        lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);
    }

	  /* 将摄像头接口的PWDN(PG7)引脚拉高，摄像头POWER DOWN，否则有时候SD卡初始化不通过 */
    RCC->AHB4ENR |= 1 << 6;     /* 使能PG口时钟 */
  
    sys_gpio_set(GPIOG, SYS_GPIO_PIN7,
                 SYS_GPIO_MODE_OUT, SYS_GPIO_OTYPE_PP, SYS_GPIO_SPEED_MID, SYS_GPIO_PUPD_PU);   /* PG7引脚模式设置,推挽输出 */
  
    sys_gpio_pin_set(GPIOG, SYS_GPIO_PIN7, 1);  /* PG7输出1 */
    
    /* 检查SPI FLASH的文件系统 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "FATFS Check...", g_point_color);   /* FATFS检测 */
    f_mount(fs[0], "0:", 1);                    /* 挂载SD卡 */
    f_mount(fs[1], "1:", 1);                    /* 挂载SPI FLASH */
    f_mount(fs[2], "2:", 1);                    /* 挂载SD NAND */
    delay_ms(100);
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);
    
    if (lcddev.width == 800 && lcddev.height == 480)  
    {  
        j--;
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
    }
    
    /* SD卡检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SD Card:      MB", g_point_color); /* FATFS检测 */
    temp = 0;

    do
    {
        temp++;
        res = exfuns_get_free("0:", &dtsize, &dfsize);  /* 得到SD卡剩余容量和总容量 */
        delay_ms(200);
    } 
    while (res && temp < 5); /* 连续检测5次 */

    if (res == 0)   /* 得到容量正常 */
    {
        gui_phy.memdevflag |= 1 << 0;   /* 设置SD卡在位 */
        temp = dtsize >> 10; /* 单位转换为MB */
        stastr = "OK";
    }
    else
    {
        temp = 0;   /* 出错了,单位为0 */
        stastr = "ERROR";
    }

    lcd_show_xnum(5 + 8 * (fsize / 2), ypos + fsize * j, temp, 6, fsize, 0, g_point_color);	/* 显示SD卡容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)stastr, g_point_color); /* SD卡状态 */
       
    /* 25Q256检测,如果不存在文件系统,则先创建 */
    temp = 0;

    do
    {
        temp++;
        res = exfuns_get_free("1:", &dtsize, &dfsize); /* 得到FLASH剩余容量和总容量 */
        delay_ms(200);
    } 
    while (res && temp < 20); /* 连续检测20次 */

    if (res == 0X0D)          /* 文件系统不存在 */
    {   
        i++;
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash Disk Formatting...", g_point_color); /* 格式化FLASH */
        res = f_mkfs("1:", 0, 0, FF_MAX_SS);     /* 格式化SPI FLASH,1:,盘符;0,自动选择文件系统类型 */

        if (res == 0)
        {
            f_setlabel((const TCHAR *)"1:WKS");             /* 设置SPI Flash磁盘的名字为：WKS */
            lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 标志格式化成功 */
            res = exfuns_get_free("1:", &dtsize, &dfsize);  /* 重新获取容量 */
        }
    }

    if (res == 0)   /* 得到FLASH卡剩余容量和总容量 */
    {
        gui_phy.memdevflag |= 1 << 1;   /* 设置SPI FLASH在位 */

        if ((lcddev.width == 800 && lcddev.height == 480) && i)  
        {  
            j--;
            lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * j + fsize, g_back_color);
        }

        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SPI Flash Disk:     KB", g_point_color); /* FATFS检测 */     
    
        temp = dtsize;
    }
    else system_error_show(5, ypos + fsize * (j + 1), "Flash Fat Error!", fsize);   /* flash 文件系统错误 */

    lcd_show_xnum(5 + 15 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color);   /* 显示FLASH容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);	/* FLASH状态 */
 
    /* SD NAND检测 */
    temp = 0;
    
    do
    {
        temp++;
        res = exfuns_get_free("2:", &dtsize, &dfsize); /* 得到SD NAND剩余容量和总容量 */
        delay_ms(200);
    }
    while(res && temp < 20);                           /* 连续检测20次 */
    
    if (res == 0X0D)                                   /* 文件系统不存在 */
    {
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SD NAND Disk Formatting...", g_point_color); /* 格式化SD NAND */
        res = f_mkfs("2:", 0, 0, FF_MAX_SS);           /* 格式化SD NAND,2:,盘符;0,自动选择文件系统类型 */
        
        if (res == 0)
        {
            f_setlabel((const TCHAR *)"2:SD NAND");         /* 设置SD NAND磁盘的名字为：SD NAND */
            lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 标志格式化成功 */
            res = exfuns_get_free("2:", &dtsize, &dfsize);  /* 重新获取容量 */
        }
    }
    
    if (res == 0)     /* 得到SD NAND剩余容量和总容量 */
    {
        gui_phy.memdevflag |= 1 << 2;   /* 设置SD NAND在位 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SD NAND Disk:    MB", g_point_color); /* FATFS检测 */
        temp = dtsize >> 10;            /* 单位转换为MB */
    }
    else system_error_show(5, ypos + fsize * (j + 1), "SD NAND Fat Error!", fsize);   /* SD NAND文件系统错误 */
  
    lcd_show_xnum(5 + 13 * (fsize / 2), ypos + fsize * j, temp, 4, fsize, 0, g_point_color);   /* 显示SD NAND容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);	/* SD NAND状态 */
    
    /* U盘检测 */
    usbapp_mode_set(0); /* 设置为U盘模式 */
    temp = 0;

    while (usbx.hdevclass == 0 && temp < 1600)   /* 等待U盘被检测,最多等待8秒 */
    {
        usbapp_pulling();
        
        if ((usbx.bDeviceState & (1 << 6)) == 0 && temp > 300)break; /* 1.5秒钟之内,没有检测到设备插入,则直接跳过,不再等待 */

        delay_ms(5);
        temp++;
    }

    if (usbx.hdevclass == 1)         /* 检测到了U盘 */
    {   
        fs[3]->pdrv = 3;
        f_mount(fs[3], "3:", 1);     /* 挂载U盘 */
        res = exfuns_get_free((uint8_t *)"3:", &dtsize, &dfsize);   /* 得到U盘剩余容量和总容量 */
    }
    else res = 0XFF;

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "U Disk:     MB", g_point_color);  /* U盘容量大小 */

    if (res == 0)   /* 得到容量正常 */
    {
        gui_phy.memdevflag |= 1 << 3;   /* 设置U盘在位 */
        temp = dtsize >> 10; /* 单位转换为MB */
        stastr = "OK";
    }
    else
    {
        temp = 0; /* 出错了,单位为0 */
        stastr = "ERROR";
    }

    lcd_show_xnum(5 + 7 * (fsize / 2), ypos + fsize * j, temp, 5, fsize, 0, g_point_color); /* 显示U盘容量大小 */
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, (char *)stastr, g_point_color);  /* U盘状态 */

    /* 24C16检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "24C16 Check...", g_point_color);

    if (bl24cxx_check())system_error_show(5, ypos + fsize * (j + 1), "24C16 Error!", fsize);    /* 24C16检测 */
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);
    
    /* 字库检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Font Check...", g_point_color);
    res = key_scan(1); /* 检测按键 */

    if (res == WKUP_PRES)   /* WK_UP按下，更新？确认 */
    {
        res = system_font_update_confirm(5, ypos + fsize * (j + 1), fsize);
    }
    else res = 0;

    if (fonts_init() || (res == 1))   /* 检测字体,如果字体不存在/强制更新,则更新字库 */
    {
        res = 0; /* 按键无效 */

        if (fonts_update_font(5, ypos + fsize * j, fsize, "0:", g_point_color) != 0)        /* 从SD卡更新 */
        {
            tim8_int_init(100 - 1, 24000 - 1); /* 启动TIM8 轮询USB,10ms中断一次 */  
            
            if (fonts_update_font(5, ypos + fsize * j, fsize, "3:", g_point_color) != 0)    /* 从U盘更新 */
            {
                system_error_show(5, ypos + fsize * (j + 1), "Font Error!", fsize);         /* 字体错误 */
            }
            
            TIM8->CR1 &= ~(1 << 0);            /* 关闭定时器8 */
        }

        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK);     /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Font Check...", g_point_color);
    }

    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color); /* 字库检测OK */

    /* 系统文件检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Files Check...", g_point_color);

    while (app_system_file_check("1"))       /* 系统文件检测 */
    {
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, (fsize / 2) * 8, fsize, fsize, "Updating", g_point_color); /* 显示updating */
        app_boot_cpdmsg_set(5, ypos + fsize * j, fsize);    /* 设置到坐标 */
        temp = 0;
        tim8_int_init(100 - 1, 24000 - 1);   /* 启动TIM8 轮询USB,10ms中断一次 */  

        if (app_system_file_check("0"))      /* 检查SD卡系统文件完整性 */
        {
            if (app_system_file_check("3"))
            {
                res = 9;                     /* 标记为不可用的盘 */
            }
            else
            {
                res = 3;                     /* 标记为U盘 */
            }
        }
        else
        {
            res = 0;                         /* 标记为SD卡 */
        }

        if (res == 0 || res == 3)            /* 完整了才更新 */
        {
            sprintf((char *)verbuf, "%d:", res);

            if (app_system_update(app_boot_cpdmsg, verbuf))   /* 更新? */
            {
                system_error_show(5, ypos + fsize * (j + 1), "SYSTEM File Error!", fsize);
            }
        }

        TIM8->CR1 &= ~(1 << 0);              /* 关闭定时器8 */        
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
        lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Files Check...", g_point_color);

        if (app_system_file_check("1"))      /* 更新了一次，再检测，如果还有不全，说明SD卡文件就不全！ */
        {
            system_error_show(5, ypos + fsize * (j + 1), "SYSTEM File Lost!", fsize);
        }
        else break;
    }

    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    /* 触摸屏检测 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "Touch Check...", g_point_color);
    res = key_scan(1);  /* 检测按键 */

    g_back_color = WHITE;
    
    if (tp_init() || (res == KEY0_PRES && (tp_dev.touchtype & 0X80) == 0))   /* 有更新/按下了KEY0且不是电容屏,执行校准 */
    {
        if (res == 1)
        {
            tp_adjust();
        }

        res = 0;        /* 按键无效 */
        goto REINIT;    /* 重新开始初始化 */
    }
    
    g_back_color = BLACK;
    
    lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);   /* 触摸屏检测OK */

    if (lcddev.width == 800 && lcddev.height == 480)  
    {  
        j--;
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
    }
    
    /* 系统参数加载 */
    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Parameter Load...", g_point_color);

    if (app_system_parameter_init())system_error_show(5, ypos + fsize * (j + 1), "Parameter Load Error!", fsize); /* 参数加载 */
    else lcd_show_string(5 + okoffset, ypos + fsize * j++, lcddev.width, lcddev.height, fsize, "OK", g_point_color);

    if (lcddev.width == 800 && lcddev.height == 480)  
    {  
        j--;
        lcd_fill(5, ypos + fsize * j, lcddev.width - 1, ypos + fsize * (j + 1), BLACK); /* 填充底色 */
    }

    lcd_show_string(5, ypos + fsize * j, lcddev.width, lcddev.height, fsize, "SYSTEM Starting...", g_point_color);

    myfree(SRAMIN, version);
    delay_ms(1500);
}

int main(void)
{
    system_init();                                              /* 系统初始化 */

    OSInit();                                                   /* UCOS初始化 */
    OSTaskCreateExt((void(*)(void *) )start_task,               /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&START_TASK_STK[START_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )START_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )START_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&START_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )START_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */
    OSStart();                                                  /* 开始任务 */
}

//extern OS_EVENT *audiombox; /* 音频播放任务邮箱 */

/**
 * @brief       开始任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr = 0;
    CPU_INT32U cnts;
    pdata = pdata;
    
    OSStatInit();                           /* 初始化统计任务.这里会延时1秒钟左右 */
    
    /* 根据配置的节拍频率配置SysTick */
    cnts = (CPU_INT32U)((480 * 1000000) / OS_TICKS_PER_SEC);
    OS_CPU_SysTickInit(cnts);
    
    app_srand(OSTime);
//    audiombox = OSMboxCreate((void *) 0);   /* 创建邮箱 */
    
    OS_ENTER_CRITICAL();                                        /* 进入临界区(无法被中断打断) */
  
    OSTaskCreateExt((void(*)(void *) )main_task,                /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&MAIN_TASK_STK[MAIN_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )MAIN_TASK_PRIO,            /* 任务优先级 */
                    (INT16U         )MAIN_TASK_PRIO,            /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&MAIN_TASK_STK[0],        /* 任务堆栈栈底 */
                    (INT32U         )MAIN_STK_SIZE,             /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskCreateExt((void(*)(void *) )usart_task,              	/* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&USART_TASK_STK[USART_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )USART_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )USART_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&USART_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )USART_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskCreateExt((void(*)(void *) )watch_task,               /* 任务函数 */
                    (void *          )0,                        /* 传递给任务函数的参数 */
                    (OS_STK *        )&WATCH_TASK_STK[WATCH_STK_SIZE - 1], /* 任务堆栈栈顶 */
                    (INT8U          )WATCH_TASK_PRIO,           /* 任务优先级 */
                    (INT16U         )WATCH_TASK_PRIO,           /* 任务ID，这里设置为和优先级一样 */
                    (OS_STK *        )&WATCH_TASK_STK[0],       /* 任务堆栈栈底 */
                    (INT32U         )WATCH_STK_SIZE,            /* 任务堆栈大小 */
                    (void *          )0,                        /* 用户补充的存储区 */
                    (INT16U         )OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR | OS_TASK_OPT_SAVE_FP); /* 任务选项,为了保险起见，所有任务都保存浮点寄存器的值 */

    OSTaskSuspend(START_TASK_PRIO);                             /* 挂起开始任务 */
    OS_EXIT_CRITICAL();                                         /* 退出临界区(可以被中断打断) */
}

/**
 * @brief       主任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void main_task(void *pdata)
{
    uint8_t selx;
    uint16_t tcnt = 0;

    spb_init(0);        /* 初始化SPB界面 */
    spb_load_mui();     /* 加载SPB主界面 */

    slcd_frame_show(0); /* 显示界面 */

    while (1)
    {
        selx = spb_move_chk();
        system_task_return = 0;                 /* 清退出标志 */

        switch (selx)                           /* 发生了双击事件 */
        {
            case 0:
                ebook_play();
                break;                          /* 电子图书 */
            case 1:
                picviewer_play();
                break;                          /* 数码相框 */
//            case 2:
//                audio_play();
//                break;                        /* 音乐播放 */
            case 3:
                video_play();
                break;                          /* 视频播放 */
            case 4:
                calendar_play();
                break;                          /* 时钟 */
            case 5:
                sysset_play();
                break;                          /* 系统设置 */
            case 6:
                nes_play();
                break;                          /* NES游戏机 */
            case 7:
                notepad_play();
                break;                          /* 记事本 */
            case 8:
                exe_play();
                break;                          /* 运行器 */
            case 9:
                paint_play();
                break;                          /* 手写画笔 */
            case 10:
                camera_play();
                break;                          /* 摄像头 */
            case 11:
                //recorder_play();    
                break;                          /* 录音 */
            case 12:
                usb_play();
                break;                          /* USB连接 */
            case 13:
                //net_play();
                if (lcddev.width <= 480)
                {
                    app_play();     /* APP */
                }            
                break;                          /* 网络测试 */
            case 14:
                //wireless_play();
                if (lcddev.width <= 480) 
                {
                    vmeter_play();  /* 电压表 */
                }              
                break;                          /* 无线传书 */
            case 15:
                calc_play();
                break;                          /* 计算器 */
            case 16:
                app_play();
                break;                          /* APP */
            case 17:
                vmeter_play();
                break;                          /* 电压表 */
        }

        if (selx != 0XFF)spb_load_mui();        /* 显示主界面 */

        delay_ms(1000 / OS_TICKS_PER_SEC);      /* 延时一个时钟节拍 */
        tcnt++;

        if (tcnt == 500)           /* OS_TICKS_PER_SEC个节拍为1秒钟 */
        {
            tcnt = 0;

            if (lcddev.width > 480)                              
            {
                spb_stabar_msg_show(0);         /* 更新状态栏信息 */
            }
        }
    }
}

volatile uint8_t memshow_flag = 1;              /* 默认打印mem使用率 */

/**
 * @brief       串口 任务,执行最不需要时效性的代码
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void usart_task(void *pdata)
{
    uint16_t alarmtimse = 0;
    float psin, psex, pstcm;
    pdata = pdata;

    while (1)
    {
        delay_ms(1000);

        if (alarm.ringsta & 1 << 7)   /* 执行闹钟扫描函数 */
        {
            calendar_alarm_ring(alarm.ringsta & 0x3); /* 闹铃 */
            alarmtimse++;

            if (alarmtimse > 300)     /* 超过300次了,5分钟以上 */
            {
                alarm.ringsta &= ~(1 << 7); /* 关闭闹铃 */
            }
        }
        else if (alarmtimse)
        {
            alarmtimse = 0;
        }

//        if (gsmdev.mode == 3)             /* 蜂鸣器,来电提醒 */
//        {
//            phone_ring(); 
//        }
        
        if (systemset.lcdbklight == 0)      /* 自动背光控制 */
        {
            app_lcd_auto_bklight(); 
        }
        
        if (memshow_flag == 1)
        {
            psin = my_mem_perused(SRAMIN);
            psex = my_mem_perused(SRAMEX);
            pstcm = my_mem_perused(SRAMDTCM);
            printf("in:%3.1f,ex:%3.1f,dtcm:%3.1f\r\n", psin / 10, psex / 10, pstcm / 10);  /* 打印内存占用率 */
        }
    }
}

volatile uint8_t system_task_return;            /* 任务强制返回标志. */
volatile uint8_t ledplay_led0_sta = 0;          /* ledplay任务,LED0(绿灯)的控制状态 */

/**
 * @brief       监视 任务
 * @param       pdata : 传入参数(未用到)
 * @retval      无
 */
void watch_task(void *pdata)
{
    OS_CPU_SR cpu_sr = 0;
    uint8_t t = 0;
    uint8_t key;
    pdata = pdata;
    uint8_t res;
  
    while (1)
    {
        if (alarm.ringsta & 1 << 7)   /* 闹钟在执行 */
        {
            calendar_alarm_msg((lcddev.width - 200) / 2, (lcddev.height - 160) / 2); /* 闹钟处理 */
        }

        if (g_gif_decoding)           /* gif正在解码中 */
        {
            key = pic_tp_scan();

            if (key == 1 || key == 3)g_gif_decoding = 0; /* 停止GIF解码 */
        }

        if (ledplay_led0_sta == 0)    /* 仅当ledplay_led0_sta等于0的时候,正常熄灭LED0(绿灯) */
        { 
            if (t == 9)LED0(1);       /* LED0(绿灯)亮100ms左右 */

            if (t == 249)
            {
                LED0(0);              /* 2.5秒钟左右亮一次 */
                t = 0;
            }
        }

        t++;
        
        key = key_scan(0);					  /* 按键扫描 */
      
        if (key == WKUP_PRES)					/* WKUP按键按下了 */
        {
            system_task_return = 1;
          
            if (g_gif_decoding)g_gif_decoding = 0;  /* 不再播放gif */
        }      

        if ((t % 60) == 0)   /* 600ms左右检测1次 */
        {
            /* SD卡在位检测 */
            if ((DCMI->CR & 0X01) == 0) /* 摄像头不工作的时候 */
            {
                OS_ENTER_CRITICAL();    /* 进入临界区(无法被中断打断) */
                res = sdmmc_get_status();    /* 查询SD卡状态 */
                OS_EXIT_CRITICAL();     /* 退出临界区(可以被中断打断) */

                if (res == 0XFF)
                {
                    gui_phy.memdevflag &= ~(1 << 0); /* 标记SD卡不在位 */
                    OS_ENTER_CRITICAL();/* 进入临界区(无法被中断打断) */
                    sd_init();          /* 重新检测SD卡 */
                    OS_EXIT_CRITICAL(); /* 退出临界区(可以被中断打断) */
                }
                else if ((gui_phy.memdevflag & (1 << 0)) == 0)  /* SD不在位? */
                {
                    f_mount(fs[0], "0:", 1);        /* 重新挂载SD卡 */
                    gui_phy.memdevflag |= 1 << 0;   /* 标记SD卡在位了 */
                }
            }
            
            /* U盘在位检测 */
            if (usbx.hdevclass == 1)
            {
                if ((gui_phy.memdevflag & (1 << 3)) == 0)   /* U盘不在位 */
                {
                    fs[3]->pdrv = 3;
                    f_mount(fs[3], "3:", 1);        /* 重新挂载U盘 */
                    gui_phy.memdevflag |= 1 << 3;   /* 标记U盘在位 */
                }
            }
            else
            {
                gui_phy.memdevflag &= ~(1 << 3);    /* U盘被拔出了 */
            }

              /* gsm检测 */
//            gsm_status_check();       /* gsm检测! */
        }

//        gsm_cmsgin_check();           /* 来电/短信 监测 */

        if (usbx.mode == USBH_MSC_MODE) /* U盘模式,才处理 */
        {
            while ((usbx.bDeviceState & 0XC0) == 0X40)   /* USB设备插入了,但是还没连接成功,猛查询 */
            {
                usbapp_pulling();       /* 轮询处理USB事务 */
                delay_ms(1);            /* 不能像HID那么猛...,U盘比较慢 */
            }

            usbapp_pulling();           /* 处理USB事务 */
        }
        
        delay_ms(10);
    }
}

/**
 * @brief       硬件错误处理
 * @param       无
 * @retval      无
 */
void HardFault_Handler(void)
{
    uint32_t i;
    uint8_t t = 0;
    uint32_t temp;
    temp = SCB->CFSR;               /* fault状态寄存器(@0XE000ED28)包括:MMSR,BFSR,UFSR */
    printf("CFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->HFSR;               /* 硬件fault状态寄存器 */
    printf("HFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->DFSR;               /* 调试fault状态寄存器 */
    printf("DFSR:%8X\r\n", temp);   /* 显示错误值 */
    temp = SCB->AFSR;               /* 辅助fault状态寄存器 */
    printf("AFSR:%8X\r\n", temp);   /* 显示错误值 */

    while (t < 5)
    {
        t++;
        LED1_TOGGLE();              /* 闪烁LED1(蓝灯) */

        for (i = 0; i < 0X1FFFFF; i++);
    }
}







