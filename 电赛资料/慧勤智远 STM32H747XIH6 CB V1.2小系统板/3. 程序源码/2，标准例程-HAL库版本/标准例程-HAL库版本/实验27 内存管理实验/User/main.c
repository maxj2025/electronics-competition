/**
 ****************************************************************************************************
 * @file        main.c
 * @version     V1.0
 * @brief       内存管理 实验
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
#include "./USMART/usmart.h"
#include "./BSP/KEY/key.h"
#include "./MALLOC/malloc.h"


int main(void)
{ 
    uint8_t t = 0;
    uint8_t key;
    uint8_t *p_sramin = NULL;
    uint8_t *p_sramex = NULL;
    uint8_t *p_sram12 = NULL;
    uint8_t *p_sram4 = NULL;
    uint8_t *p_sramdtcm = NULL;
    uint8_t *p_sramitcm = NULL;
    uint32_t tp_sramin = 0;
    uint32_t tp_sramex = 0;
    uint32_t tp_sram12 = 0;
    uint32_t tp_sram4 = 0;
    uint32_t tp_sramdtcm = 0;
    uint32_t tp_sramitcm = 0;
    uint8_t paddr[32];                      
    uint16_t memused;
  
    sys_cache_enable();                     /* 使能L1-Cache */
    HAL_Init();                             /* 初始化HAL库 */
    sys_stm32_clock_init(192, 5, 2, 4);     /* 设置时钟, 480Mhz */
    delay_init(480);                        /* 延时初始化 */
    usart_init(115200);                     /* 初始化USART */  
    usmart_init(240);	                      /* 初始化USMART */    
    led_init();                             /* 初始化LED */
    mpu_memory_protection();                /* 保护相关存储区域 */
    sdram_init();                           /* 初始化SDRAM */
    lcd_init();                             /* 初始化LCD */
    key_init();                             /* 初始化按键 */
  
    my_mem_init(SRAMIN);                    /* 初始化内部内存池(AXI) */
    my_mem_init(SRAMEX);                    /* 初始化外部内存池(SDRAM) */
    my_mem_init(SRAM12);                    /* 初始化SRAM12内存池(SRAM1+SRAM2) */
    my_mem_init(SRAM4);                     /* 初始化SRAM4内存池(SRAM4) */
    my_mem_init(SRAMDTCM);                  /* 初始化DTCM内存池(DTCM) */
    my_mem_init(SRAMITCM);                  /* 初始化ITCM内存池(ITCM) */
    
    lcd_show_string(30, 50, 200, 16, 16, "STM32H747", RED);
    lcd_show_string(30, 70, 200, 16, 16, "MALLOC TEST", RED);
    lcd_show_string(30, 90, 200, 16, 16, "WKS SMART", RED);
    lcd_show_string(30, 110, 200, 16, 16, "KEY0:Malloc & WR & Show", RED);
    lcd_show_string(30, 130, 200, 16, 16, "WK_UP:Free", RED);
    
    lcd_show_string(30, 162, 200, 16, 16, "SRAMIN   USED:", BLUE);
    lcd_show_string(30, 178, 200, 16, 16, "SRAMEX   USED:", BLUE);
    lcd_show_string(30, 194, 200, 16, 16, "SRAM12   USED:", BLUE);
    lcd_show_string(30, 210, 200, 16, 16, "SRAM4    USED:", BLUE);
    lcd_show_string(30, 226, 200, 16, 16, "SRAMDTCM USED:", BLUE);    
    lcd_show_string(30, 242, 200, 16, 16, "SRAMITCM USED:", BLUE); 
    
    while (1)
    {        
        key = key_scan(0);        /* 不支持连按 */
      
        if (key == KEY0_PRES)     /* KEY0按下 */
        {
            /* 申请2K字节内存 */
            p_sramin = mymalloc(SRAMIN, 2048);
            p_sramex = mymalloc(SRAMEX, 2048);
            p_sram12 = mymalloc(SRAM12, 2048);
            p_sram4 = mymalloc(SRAM4, 2048);
            p_sramdtcm = mymalloc(SRAMDTCM, 2048);
            p_sramitcm = mymalloc(SRAMITCM, 2048);
                       
            if ((p_sramin != NULL) && (p_sramex != NULL) && (p_sram12 != NULL) && (p_sram4 != NULL) && (p_sramdtcm != NULL) && (p_sramitcm != NULL))   /* 判断内存申请是否成功 */
            {
                /* 向申请到的内存写入一些内容并显示 */
                sprintf((char *)p_sramin, "SRAMIN: Malloc Test%03d", t + SRAMIN);
                lcd_show_string(30, 354, 239, 16, 16, (char *)p_sramin, BLUE);
                sprintf((char *)p_sramex, "SRAMEX: Malloc Test%03d", t + SRAMEX);
                lcd_show_string(30, 370, 239, 16, 16, (char *)p_sramex, BLUE);              
                sprintf((char *)p_sram12, "SRAM12: Malloc Test%03d", t + SRAM12);
                lcd_show_string(30, 386, 239, 16, 16, (char *)p_sram12, BLUE);
                sprintf((char *)p_sram4, "SRAM4: Malloc Test%03d", t + SRAM4);
                lcd_show_string(30, 402, 239, 16, 16, (char *)p_sram4, BLUE);              
                sprintf((char *)p_sramdtcm, "SRAMDTCM: Malloc Test%03d", t + SRAMDTCM);
                lcd_show_string(30, 418, 239, 16, 16, (char *)p_sramdtcm, BLUE);              
                sprintf((char *)p_sramitcm, "SRAMITCM: Malloc Test%03d", t + SRAMITCM);
                lcd_show_string(30, 434, 239, 16, 16, (char *)p_sramitcm, BLUE);
            }
            else
            {
                myfree(SRAMIN, p_sramin);
                myfree(SRAMEX, p_sramex);              
                myfree(SRAM12, p_sram12);   
                myfree(SRAM4, p_sram4);   
                myfree(SRAMDTCM, p_sramdtcm);
                myfree(SRAMITCM, p_sramitcm);   /* 释放内存 */
                p_sramin = NULL;
                p_sramex = NULL;              
                p_sram12 = NULL; 
                p_sram4 = NULL;                
                p_sramdtcm = NULL;
                p_sramitcm = NULL;              /* 指向空地址 */
            }
        }
        else if (key == WKUP_PRES)              /* WK_UP按下 */
        {            
                myfree(SRAMIN, p_sramin);
                myfree(SRAMEX, p_sramex);          
                myfree(SRAM12, p_sram12);   
                myfree(SRAM4, p_sram4);   
                myfree(SRAMDTCM, p_sramdtcm);
                myfree(SRAMITCM, p_sramitcm);   /* 释放内存 */
                p_sramin = NULL;
                p_sramex = NULL;                        
                p_sram12 = NULL; 
                p_sram4 = NULL;                
                p_sramdtcm = NULL;
                p_sramitcm = NULL;              /* 指向空地址 */
        }
        
        /* 显示申请到内存的首地址 */
        if ((tp_sramin != (uint32_t)p_sramin) || (tp_sramex != (uint32_t)p_sramex) || (tp_sram12 != (uint32_t)p_sram12) || (tp_sram4 != (uint32_t)p_sram4) || (tp_sramdtcm != (uint32_t)p_sramdtcm) || (tp_sramitcm != (uint32_t)p_sramitcm))
        {
            tp_sramin = (uint32_t)p_sramin;
            tp_sramex = (uint32_t)p_sramex;          
            tp_sram12 = (uint32_t)p_sram12;
            tp_sram4 = (uint32_t)p_sram4;          
            tp_sramdtcm = (uint32_t)p_sramdtcm;
            tp_sramitcm = (uint32_t)p_sramitcm;
            
            sprintf((char *)paddr, "SRAMIN: Addr: 0x%08X", (uint32_t)p_sramin);
            lcd_show_string(30, 258, 239, 16, 16, (char *)paddr, BLUE);
            sprintf((char *)paddr, "SRAMEX: Addr: 0x%08X", (uint32_t)p_sramex);
            lcd_show_string(30, 274, 239, 16, 16, (char *)paddr, BLUE);          
            sprintf((char *)paddr, "SRAM12: Addr: 0x%08X", (uint32_t)p_sram12);
            lcd_show_string(30, 290, 239, 16, 16, (char *)paddr, BLUE);
            sprintf((char *)paddr, "SRAM4: Addr: 0x%08X", (uint32_t)p_sram4);
            lcd_show_string(30, 306, 239, 16, 16, (char *)paddr, BLUE);
            sprintf((char *)paddr, "SRAMDTCM: Addr: 0x%08X", (uint32_t)p_sramdtcm);
            lcd_show_string(30, 322, 239, 16, 16, (char *)paddr, BLUE);
            sprintf((char *)paddr, "SRAMITCM: Addr: 0x%08X", (uint32_t)p_sramitcm);
            lcd_show_string(30, 338, 239, 16, 16, (char *)paddr, BLUE);
        }
        else if ((p_sramin == NULL) || (p_sramex == NULL) || (p_sram12 == NULL) || (p_sram4 == NULL) || (p_sramdtcm == NULL) || (p_sramitcm == NULL))   
        {
            lcd_fill(30, 258, 239, 479, WHITE);     /* 清除显示 */
        }
        
        if (++t == 20)
        {
            t = 0;
             
            memused = my_mem_perused(SRAMIN);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 162, 200, 16, 16, (char *)paddr, BLUE);   /* 显示内部SRAM使用率 */                        

            memused = my_mem_perused(SRAMEX);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 178, 200, 16, 16, (char *)paddr, BLUE);   /* 显示外部内存使用率 */      
          
            memused = my_mem_perused(SRAM12);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 194, 200, 16, 16, (char *)paddr, BLUE);   /* 显示内部SRAM12使用率 */

            memused = my_mem_perused(SRAM4);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 210, 200, 16, 16, (char *)paddr, BLUE);   /* 显示内部SRAM4使用率 */
          
            memused = my_mem_perused(SRAMDTCM);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 226, 200, 16, 16, (char *)paddr, BLUE);   /* 显示内部DTCM使用率 */

            memused = my_mem_perused(SRAMITCM);
            sprintf((char *)paddr, "%d.%01d%%", memused / 10, memused % 10);
            lcd_show_string(30 + 112, 242, 200, 16, 16, (char *)paddr, BLUE);   /* 显示内部ITCM使用率 */
          
            LED0_TOGGLE();     /* LED0(绿灯)闪烁 */
        }
        
        delay_ms(10);
    }
}




