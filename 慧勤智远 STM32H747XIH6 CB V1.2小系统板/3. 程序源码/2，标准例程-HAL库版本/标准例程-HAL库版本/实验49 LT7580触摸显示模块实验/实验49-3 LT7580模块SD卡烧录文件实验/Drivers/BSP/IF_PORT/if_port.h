/**
 ****************************************************************************************************
 * @file        if_port.h
 * @version     V1.0
 * @brief       LT7580模块 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */	

#ifndef _if_port_h
#define _if_port_h

#include "./SYSTEM/sys/sys.h"


/* 选择驱动方式 */
//#define STM32_FMC_8	  0		  /* 使用8位8080模式驱动显示模块 */
#define STM32_FMC_16	0		  /* 使用16位8080模式驱动显示模块 */
#define STM32_SPI		  1		  /* 使用4线SPI模式驱动显示模块 */


/******************************************************************************************/
/* LCD_RST 引脚 定义 */

#define LCD_RST_GPIO_PORT               GPIOH
#define LCD_RST_GPIO_PIN                GPIO_PIN_5
#define LCD_RST_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOH_CLK_ENABLE(); }while(0)   /* PH口时钟使能 */

/* LCD PWREN 引脚 定义 
 * LCD_PWREN引脚用于IO控制LCD_5V的开启关闭, 开启LCD_5V需要将LCD_PWREN引脚输出1.
 */

#define LCD_PWREN_GPIO_PORT             GPIOI
#define LCD_PWREN_GPIO_PIN              GPIO_PIN_11
#define LCD_PWREN_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOI_CLK_ENABLE(); }while(0)   /* PI口时钟使能 */

/* LCD复位引脚 */
#define LCD_RST(x)      do{ x ? \
                            HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_GPIO_PIN, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(LCD_RST_GPIO_PORT, LCD_RST_GPIO_PIN, GPIO_PIN_RESET); \
                        }while(0)

/* LCD电源控制引脚 */
#define LCD_PWREN(x)    do{ x ? \
                            HAL_GPIO_WritePin(LCD_PWREN_GPIO_PORT, LCD_PWREN_GPIO_PIN, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(LCD_PWREN_GPIO_PORT, LCD_PWREN_GPIO_PIN, GPIO_PIN_RESET); \
                        }while(0)

/******************************************************************************************/
/* 选择FMC总线驱动 */
#if STM32_FMC_16

/* LCD RST/BL/WR/RD/CS/RS 引脚 定义 
 * LCD_D0~D15,由于引脚太多,就不在这里定义了,直接在fmc_init_16里面修改.所以在移植的时候,除了改
 * 这6个IO口, 还得改fmc_init_16里面的D0~D15所在的IO口.
 */

/* LCD复位引脚为开发板LCD接口共用 所以不在这里定义 LCD_RST引脚 */
//#define LCD_RST_GPIO_PORT               GPIOA
//#define LCD_RST_GPIO_PIN                GPIO_PIN_8
//#define LCD_RST_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOA_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */

#define LCD_BL_GPIO_PORT                GPIOB
#define LCD_BL_GPIO_PIN                 GPIO_PIN_1
#define LCD_BL_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOB_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */
                        
#define LCD_WR_GPIO_PORT                GPIOD
#define LCD_WR_GPIO_PIN                 GPIO_PIN_5
#define LCD_WR_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */

#define LCD_RD_GPIO_PORT                GPIOD
#define LCD_RD_GPIO_PIN                 GPIO_PIN_4
#define LCD_RD_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */

/* LCD_CS(需要根据LCD_FMC_NEX设置正确的IO口) 和 LCD_RS(需要根据LCD_FMC_AX设置正确的IO口) 引脚 定义 */
#define LCD_CS_GPIO_PORT                GPIOD
#define LCD_CS_GPIO_PIN                 GPIO_PIN_7
#define LCD_CS_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOD_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */

#define LCD_RS_GPIO_PORT                GPIOE
#define LCD_RS_GPIO_PIN                 GPIO_PIN_3
#define LCD_RS_GPIO_CLK_ENABLE()        do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* 所在IO口时钟使能 */

/* FMC相关参数 定义 
 * 注意: 我们默认是通过FMC块1来连接LCD, 块1有4个片选: FMC_NE1~4
 *
 * 修改LCD_FMC_NEX, 对应的LCD_CS_GPIO相关设置也得改
 * 修改LCD_FMC_AX , 对应的LCD_RS_GPIO相关设置也得改
 */
#define LCD_FMC_NEX         1               /* 使用FMC_NE1接LCD_CS,取值范围只能是: 1 ~ 4 */
#define LCD_FMC_AX          19              /* 使用FMC_A18接LCD_RS,取值范围是: 0 ~ 25 */


/* LCD_BASE的详细计算方法:
 * 我们一般使用FMC的块1(BANK1)来驱动TFTLCD液晶屏, 块1地址范围总大小为256MB,均分成4块:
 * 存储块1(FMC_NE1)地址范围: 0X6000 0000 ~ 0X63FF FFFF
 * 存储块2(FMC_NE2)地址范围: 0X6400 0000 ~ 0X67FF FFFF
 * 存储块3(FMC_NE3)地址范围: 0X6800 0000 ~ 0X6BFF FFFF
 * 存储块4(FMC_NE4)地址范围: 0X6C00 0000 ~ 0X6FFF FFFF
 *
 * 我们需要根据硬件连接方式选择合适的片选(连接LCD_CS)和地址线(连接LCD_RS)
 * 开发板使用FMC_NE1连接LCD_CS, FMC_A19连接LCD_RS ,16位数据线,计算方法如下:
 * 首先FMC_NE1的基地址为: 0X6000 0000;       NEx的基址为(x=1/2/3/4): 0X6000 0000 + (0X400 0000 * (x - 1))
 * FMC_A19对应地址值: 2^19 * 2 = 0X100000;   FMC_Ay对应的地址为(y = 0 ~ 25): 2^y * 2
 *
 * LCD_BASE0,对应LCD_RS = 0(LCD寄存器); LCD_BASE1,对应LCD_RS = 1(LCD数据)
 * 则 LCD_BASE1的地址为:  0X6000 0000 + 2^19 * 2 = 0X6010 0000
 *    LCD_BASE0的地址可以为 A19 = 0(即LCD_RS = 0)的其他地址.
 * LCD_BASE0和LCD_BASE1均为16位数据宽度
 * 因此 LCD_BASE0的地址可以为LCD_BASE0 = LCD_BASE1 - 2 = 0X6010 0000 - 2
 *
 * 更加通用的计算公式为((片选脚FMC_NEx)x=1/2/3/4, (RS接地址线FMC_Ay)y=0~25):
 *          LCD_BASE0 = (0X6000 0000 + (0X400 0000 * (x - 1))) | (2^y * 2 - 2)
 *          等效于(使用移位操作)
 *          LCD_BASE0 = (0X6000 0000 + (0X400 0000 * (x - 1))) | ((1 << y) * 2 - 2)
 */
#define LCD_BASE0        (uint32_t)((0X60000000 + (0X4000000 * (LCD_FMC_NEX - 1))) | (((1 << LCD_FMC_AX) * 2) - 2))
#define LCD_BASE1        (uint32_t)((0X60000000 + (0X4000000 * (LCD_FMC_NEX - 1))) | ((1 << LCD_FMC_AX) * 2))

/* LCD背光控制 */
#define LCD_BL(x)       do{ x ? \
                            HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(LCD_BL_GPIO_PORT, LCD_BL_GPIO_PIN, GPIO_PIN_RESET); \
                        }while(0)

#endif

/******************************************************************************************/
/* 选择SPI驱动 */
#if STM32_SPI	

/* SPI4 引脚 定义 */
#define SPI4_CS_GPIO_PORT               GPIOE
#define SPI4_CS_GPIO_PIN                GPIO_PIN_4
#define SPI4_CS_GPIO_CLK_ENABLE()       do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_SCK_GPIO_PORT              GPIOE
#define SPI4_SCK_GPIO_PIN               GPIO_PIN_2
#define SPI4_SCK_GPIO_CLK_ENABLE()      do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_MISO_GPIO_PORT             GPIOE
#define SPI4_MISO_GPIO_PIN              GPIO_PIN_5
#define SPI4_MISO_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

#define SPI4_MOSI_GPIO_PORT             GPIOE
#define SPI4_MOSI_GPIO_PIN              GPIO_PIN_6
#define SPI4_MOSI_GPIO_CLK_ENABLE()     do{ __HAL_RCC_GPIOE_CLK_ENABLE(); }while(0)   /* PE口时钟使能 */

/* SPI4相关定义 */
#define SPI4_SPI                        SPI4
#define SPI4_SPI_CLK_ENABLE()           do{ __HAL_RCC_SPI4_CLK_ENABLE(); }while(0)    /* SPI4时钟使能 */

extern SPI_HandleTypeDef g_spi4_handle;                                               /* SPI句柄 */

/* SPI片选信号 */
#define SPI_CS(x)       do{ x ? \
                            HAL_GPIO_WritePin(SPI4_CS_GPIO_PORT, SPI4_CS_GPIO_PIN, GPIO_PIN_SET) : \
                            HAL_GPIO_WritePin(SPI4_CS_GPIO_PORT, SPI4_CS_GPIO_PIN, GPIO_PIN_RESET); \
                        }while(0)

/* SPI总线速度设置 */
#define SPI_SPEED_2         0
#define SPI_SPEED_4         1
#define SPI_SPEED_8         2
#define SPI_SPEED_16        3
#define SPI_SPEED_32        4
#define SPI_SPEED_64        5
#define SPI_SPEED_128       6
#define SPI_SPEED_256       7
                         
#endif
                        
/******************************************************************************************/
/* 函数声明 */

void lcd_port_init(void);                 /* LCD模块接口初始化 */
void LCD_CmdWrite(uint8_t cmd);           /* 向LT7580写命令 */
void LCD_DataWrite(uint8_t data);         /* 向LT7580写数据 */
void LCD_DataWrite_Pixel(uint16_t data);  /* 向LT7580写像素点的数据   */
uint8_t LCD_StatusRead(void);             /* 读状态寄存器 */
uint16_t LCD_DataRead(void);              /* 读指令寄存器数据 */
                         
/* FMC总线驱动相关函数 */
void fmc_init_16(void);
void FMC_16_CmdWrite(uint8_t cmd);
void FMC_16_DataWrite(uint8_t data);
void FMC_16_DataWrite_Pixel(uint16_t data);
uint8_t FMC_16_StatusRead(void);
uint16_t FMC_16_DataRead(void);

/* SPI驱动相关函数 */                        
void spi_io_init(void);
void spi4_set_speed(uint8_t speed);
uint8_t spi4_read_write_byte(uint8_t txdata);
void SPI_CmdWrite(uint8_t cmd);
void SPI_DataWrite(uint8_t data);
void SPI_DataWrite_Pixel(uint16_t data);
uint8_t SPI_StatusRead(void);
uint16_t SPI_DataRead(void);
	 
void Delay_us(uint32_t time);  /* 延时函数us级(自定义) */
void Delay_ms(uint32_t time);  /* 延时函数ms级(自定义) */

void write_758_reg(uint8_t reg, uint8_t data);  /* LT758写寄存器 */
uint8_t read_758_reg(uint8_t reg);              /* LT758读寄存器 */
uint8_t read_758_statu_reg(void);               /* LT758读状态寄存器 */

void lcd_power_on(void);                        /* 打开LCD模块电源 */

#endif




