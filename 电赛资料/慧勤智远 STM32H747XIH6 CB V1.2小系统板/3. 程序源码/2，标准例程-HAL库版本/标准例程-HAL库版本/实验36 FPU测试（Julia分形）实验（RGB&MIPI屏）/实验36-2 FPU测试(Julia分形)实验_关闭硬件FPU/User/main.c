/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       FPU测试(Julia分形) 实验
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/MPU/mpu.h"
#include "./BSP/SDRAM/sdram.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/LCD/ltdc.h"
#include "./BSP/TIMER/timer.h"


/* 36_2,本版本为关闭硬件FPU版本. */

/* FPU模式提示 */
#if __FPU_USED==1
#define SCORE_FPU_MODE                  "FPU On"
#else
#define SCORE_FPU_MODE                  "FPU Off"
#endif

#define     ITERATION           128         /* 迭代次数 */
#define     REAL_CONSTANT       0.285f      /* 实部常量 */
#define     IMG_CONSTANT        0.01f       /* 虚部常量 */

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
/* 颜色表 */
uint32_t g_color_map[ITERATION];
#else
/* 颜色表 */
uint16_t g_color_map[ITERATION];
#endif

/* 缩放因子列表 */
const uint16_t zoom_ratio[] =
{
    120, 110, 100, 150, 200, 275, 350, 450,
    600, 800, 1000, 1200, 1500, 2000, 1500,
    1200, 1000, 800, 600, 450, 350, 275, 200,
    150, 100, 110,
};

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888 || LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888

/**
 * @brief       初始化颜色表(RGB888、ARGB8888颜色像素格式)
 * @param       clut     : 颜色表指针
 * @retval      无
 */
void julia_clut_init(uint32_t *clut)
{
    uint32_t i = 0x00;
    uint32_t  red = 0, green = 0, blue = 0;

    for (i = 0; i < ITERATION; i++) /* 产生颜色表 */
    {
        /* 产生RGB颜色值 */
        red = (i * 8 * 256 / ITERATION) % 256;
        green = (i * 6 * 256 / ITERATION) % 256;
        blue = (i * 4 * 256 / ITERATION) % 256;
        
        red = red << 16;
        green = green << 8;
        clut[i] = red | green | blue | 0XFF000000;
    }
}

#else

/**
 * @brief       初始化颜色表(RGB565颜色像素格式)
 * @param       clut     : 颜色表指针
 * @retval      无
 */
void julia_clut_init(uint16_t *clut)
{
    uint32_t i = 0x00;
    uint16_t  red = 0, green = 0, blue = 0;

    for (i = 0; i < ITERATION; i++) /* 产生颜色表 */
    {
        /* 产生RGB颜色值 */
        red = (i * 8 * 256 / ITERATION) % 256;
        green = (i * 6 * 256 / ITERATION) % 256;
        blue = (i * 4 * 256 / ITERATION) % 256;
        
        /* 将RGB888,转换为RGB565 */
        red = red >> 3;
        red = red << 11;
        green = green >> 2;
        green = green << 5;
        blue = blue >> 3;
        clut[i] = red + green + blue;
    }
}

#endif

#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_ARGB8888
/* LCD 缓存 */
uint32_t g_lcdbuf[1280];
#elif LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888
/* LCD 缓存 */
uint8_t g_lcdbuf[1280 * 3];
#else
/* LCD 缓存 */
uint16_t g_lcdbuf[1280];
#endif

/**
 * @brief       产生Julia分形图形
 * @param       size_x   : 屏幕x方向的尺寸
 * @param       size_y   : 屏幕y方向的尺寸
 * @param       offset_x : 屏幕x方向的偏移
 * @param       offset_y : 屏幕y方向的偏移
 * @param       zoom     : 缩放因子
 * @retval      无
 */
void julia_generate_fpu(uint16_t size_x, uint16_t size_y, uint16_t offset_x, uint16_t offset_y, uint16_t zoom)
{
    uint8_t i;
    uint16_t x, y;
    float tmp1, tmp2;
    float num_real, num_img;
    float radius;

    for (y = 0; y < size_y; y++)
    {
        for (x = 0; x < size_x; x++)
        {
            num_real = y - offset_y;
            num_real = num_real / zoom;
            num_img = x - offset_x;
            num_img = num_img / zoom;
            i = 0;
            radius = 0;

            while ((i < ITERATION - 1) && (radius < 4))
            {
                tmp1 = num_real * num_real;
                tmp2 = num_img * num_img;
                num_img = 2 * num_real * num_img + IMG_CONSTANT;
                num_real = tmp1 - tmp2 + REAL_CONSTANT;
                radius = tmp1 + tmp2;
                i++;
            }

            if (lcdltdc.pwidth != 0)
            {            
#if LTDC_PIXFORMAT == LTDC_PIXFORMAT_RGB888           
            
                g_lcdbuf[x * 3 + 2] = (g_color_map[i] & 0XFF0000) >> 16;      /* 保存RGB888红色值到lcdbuf */
                g_lcdbuf[x * 3 + 1] = (g_color_map[i] & 0XFF00) >> 8;         /* 保存RGB888绿色值到lcdbuf */
                g_lcdbuf[x * 3] = (g_color_map[i] & 0XFF);                    /* 保存RGB888蓝色值到lcdbuf */
            
#else
            
                g_lcdbuf[x] = g_color_map[i];                                 /* 保存颜色值到lcdbuf */
                     
#endif
            }
        }
        
        if (lcdltdc.pwidth != 0)
        {
            ltdc_color_fill(0, y, lcddev.width - 1, y, (uint16_t *)g_lcdbuf); /* DMA2D填充 */
        }
    }
}

uint8_t g_timeout;  /* 计时全局变量 */

int main(void)
{  
    uint8_t key;
    uint8_t i = 0;
    uint8_t autorun = 0;
    float time;
    char buf[50];
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
    timx_int_init(65535, 24000 - 1);        /* 10Khz计数频率,最大计时6.5秒溢出 */
  
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "FPU TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:+", RED);           
    lcd_show_string(30, 130, 200, 16, 16, "WK_UP:AUTO/MANUL", RED);     /* 显示提示信息 */

    delay_ms(1000);
    julia_clut_init(g_color_map);           /* 初始化颜色表 */
  
    while (1)
    {        
        key = key_scan(0);

        switch (key)
        {
            case KEY0_PRES:
                i++;

                if (i > sizeof(zoom_ratio) / 2 - 1)
                {
                    i = 0; /* 限制范围 */
                }

                break;

//            case WKUP_PRES:
//                if (i)
//                {
//                    i--;
//                }
//                else
//                {
//                    i = sizeof(zoom_ratio) / 2 - 1;
//                }

//                break;

            case WKUP_PRES:
                autorun = !autorun; /* 自动/手动 */
                break;
            
            default:break;
        }

        if (autorun == 1)   /* 自动时,自动设置缩放因子 */
        {
            i++;
            LED1(0);        /* 点亮LED1(蓝灯)，自动设置 */
          
            if (i > sizeof(zoom_ratio) / 2 - 1)
            {
                i = 0;      /* 限制范围 */
            }
        }
        else 
        {
            LED1(1);        /* 熄灭LED1(蓝灯)，手动设置 */
        }

        if (lcdltdc.pwidth == 0)                                /* 非RGB/MIPI屏 */
        {        
            lcd_set_window(0, 0, lcddev.width, lcddev.height);  /* 设置窗口 */
            lcd_write_ram_prepare();
        }
        
        TIMX_INT->CNT = 0;  /* 重设定时器TIMX的计数器值 */
        g_timeout = 0;
        
        julia_generate_fpu(lcddev.width, lcddev.height, lcddev.width / 2, lcddev.height / 2, zoom_ratio[i]);
        
        time = TIMX_INT->CNT + (uint32_t)g_timeout * 65536;
        
        sprintf(buf, "%s: zoom:%d  runtime:%0.1fms\r\n", SCORE_FPU_MODE, zoom_ratio[i], time / 10);
        lcd_show_string(5, lcddev.height - 5 - 12, lcddev.width - 5, 12, 12, buf, RED); /* 显示当前运行情况 */
        printf("%s", buf);                                                              /* 输出到串口 */
        LED0_TOGGLE();                                                                  /* LED0(绿灯)闪烁 */
    }
}




