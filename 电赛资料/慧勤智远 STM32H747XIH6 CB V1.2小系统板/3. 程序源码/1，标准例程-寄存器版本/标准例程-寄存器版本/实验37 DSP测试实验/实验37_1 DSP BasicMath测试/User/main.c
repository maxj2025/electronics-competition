/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       DSP BasicMath 实验
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
#include "./BSP/TIMER/timer.h"
#include "./CMSIS/DSP/Include/arm_math.h"


#define DELTA   0.0001f             /* 误差值 */

/**
 * @brief       sin cos 测试
 * @param       angle : 起始角度
 * @param       times : 运算次数
 * @param       mode  : 是否使用DSP库
 *   @arg       0 , 不使用DSP库;
 *   @arg       1 , 使用DSP库;
 *
 * @retval      0,    运算成功;
 *              0XFF, 运算失败;
 */
uint8_t sin_cos_test(float angle, uint32_t times, uint8_t mode)
{
    float sinx, cosx;
    float result;
    uint32_t i = 0;

    if (mode == 0)
    {
        for (i = 0; i < times; i++)
        {
            cosx = cosf(angle);                 /* 不使用DSP优化的sin，cos函数 */
            sinx = sinf(angle);
            result = sinx * sinx + cosx * cosx; /* 计算结果应该等于1 */
            result = fabsf(result - 1.0f);      /* 对比与1的差值 */

            if (result > DELTA)
            {
                return 0XFF;                    /* 判断失败 */
            }

            angle += 0.001f;                    /* 角度自增 */
        }
    }
    else
    {
        for (i = 0; i < times; i++)
        {
            cosx = arm_cos_f32(angle);          /* 使用DSP优化的sin，cos函数 */
            sinx = arm_sin_f32(angle);
            result = sinx * sinx + cosx * cosx; /* 计算结果应该等于1 */
            result = fabsf(result - 1.0f);      /* 对比与1的差值 */

            if (result > DELTA)
            {
                return 0XFF;                    /* 判断失败 */
            }

            angle += 0.001f;                    /* 角度自增 */
        }
    }

    return 0;                                   /* 任务完成 */
}

uint8_t g_timeout;  /* 计时全局变量 */

int main(void)
{  
    float time;
    char buf[50];
    uint8_t res;
  
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(120, 115200);                /* 初始化USART */
    led_init();							                /* 初始化LED */   
    mpu_memory_protection();                /* 保护相关存储区域 */  
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    timx_int_init(65535, 24000 - 1);        /* 10Khz计数频率,最大计时6.5秒溢出 */

    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "DSP BasicMath TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 120, 200, 16, 16, "No DSP runtime:", RED); 
    lcd_show_string(30, 150, 200, 16, 16, "Use DSP runtime:", RED);  /* 显示提示信息 */
  
    while (1)
    {
        /* 不使用DSP优化 */
        TIMX_INT->CNT = 0;     /* 重设定时器TIMX的计数器值 */
        g_timeout = 0;
        res = sin_cos_test(PI / 6, 200000, 0);
        time = TIMX_INT->CNT + (uint32_t)g_timeout * 65536;
        sprintf(buf, "%0.1fms\r\n", time / 10);

        if (res == 0)
        {
            lcd_show_string(30 + 16 * 8, 120, 100, 16, 16, buf, BLUE);       /* 显示运行时间 */
        }
        else
        {
            lcd_show_string(30 + 16 * 8, 120, 100, 16, 16, "error！", BLUE); /* 显示当前运行情况 */
        }

        /* 使用DSP优化 */
        TIMX_INT->CNT = 0;     /* 重设定时器TIMX的计数器值 */
        g_timeout = 0;
        res = sin_cos_test(PI / 6, 200000, 1);
        time = TIMX_INT->CNT + (uint32_t)g_timeout * 65536;
        sprintf(buf, "%0.1fms\r\n", time / 10);

        if (res == 0)
        {
            lcd_show_string(30 + 16 * 8, 150, 100, 16, 16, buf, BLUE);       /* 显示运行时间 */
        }
        else
        {
            lcd_show_string(30 + 16 * 8, 150, 100, 16, 16, "error！", BLUE); /* 显示错误 */
        }

        LED0_TOGGLE();         /* LED0(绿灯)闪烁 */
    }
}




