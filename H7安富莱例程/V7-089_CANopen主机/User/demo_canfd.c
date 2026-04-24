/*
*********************************************************************************************************
*
*	模块名称 : CAN FD应用。
*	文件名称 : demo_canfd.c
*	版    本 : V1.0
*	说    明 : CAN FD应用
*
*	修改记录 :
*		版本号   日期         作者        说明
*		V1.0    2023-05-11   Eric2013    正式发布
*
*	Copyright (C), 2021-2030, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/
#include "bsp.h"
#include "can_network.h"
#include "CO_app_STM32.h"
#include "OD.h"
#include "301/CO_SDOclient.h"


/*
*********************************************************************************************************
*	                                            变量和宏定义
*********************************************************************************************************
*/
#define CANopenSlaveID1   0x18
#define CANopenSlaveID2   0x19

TIM_HandleTypeDef htim17;
CANopenNodeSTM32 canOpenNodeSTM32;
extern FDCAN_HandleTypeDef hfdcan2;
extern CO_t* CO;

uint8_t g_ucTempBuf[20];
uint32_t g_uiReadSize;

uint32_t obj_ledvarID1;
uint32_t obj_ledvarID2;

/*
*********************************************************************************************************
*	                                            函数
*********************************************************************************************************
*/
static void MX_TIM17_Init(void);
uint8_t CANopen_WriteClock(uint16_t _year, uint8_t _mon, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint32_t _ms, uint32_t _Interval_ms);
CO_SDO_abortCode_t read_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                            uint16_t index, uint8_t subIndex,
                            uint8_t *buf, size_t bufSize, size_t *readSize);
CO_SDO_abortCode_t write_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                             uint16_t index, uint8_t subIndex,
                             uint8_t *data, size_t dataSize);					

/*
*********************************************************************************************************
*	函 数 名: DemoCANFD
*	功能说明: CAN测试
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
void DemoCANFD(void)
{
	uint8_t ucKeyCode;		/* 按键代码 */
	uint8_t tx_buf[64];

	bsp_StartAutoTimer(0, 50);	/* 启动1个500ms的自动重装的定时器 */
	
	/* 配置一个1ms周期的定时器中断用于刷CAN */
	MX_TIM17_Init();
	
	/* 初始化CANopen */
	canOpenNodeSTM32.CANHandle = &hfdcan2;              /* 使用CANFD2接口 */
	canOpenNodeSTM32.HWInitFunction = bsp_InitCan2;     /* 初始化CANFD2 */
	canOpenNodeSTM32.timerHandle = &htim17;             /* 使用的定时器句柄 */
	canOpenNodeSTM32.desiredNodeID = 26;                /* Node-ID */
	canOpenNodeSTM32.baudrate = 500;                   /* 波特率，单位KHz */
	canopen_app_init(&canOpenNodeSTM32);
	
	OD_RAM.x2004_obj_ledvarID1 = 0;
	OD_RAM.x2005_obj_ledvarID2 = 0;	
	CANopen_WriteClock(2023,10,5,10,10,10,0,500);       /* 设置年月日，时分秒，毫秒和时间戳上传周期 */
	
	while (1)
	{
		/* CAN处理 */
		canopen_app_process(); 
		
		/* 判断定时器超时时间 */
		if (bsp_CheckTimer(0))	
		{            
			/* 每隔500ms 进来一次 */  
			bsp_LedToggle(2);
		}
		
		/* 按键滤波和检测由后台systick中断服务程序实现，我们只需要调用bsp_GetKey读取键值即可。 */
		ucKeyCode = bsp_GetKey();	/* 读取键值, 无键按下时返回 KEY_NONE = 0 */
		if (ucKeyCode != KEY_NONE)
		{
			switch (ucKeyCode)
			{
				case KEY_DOWN_K1:			/* K1键按下，复位节点0x18和节点0x19 */
					CO_NMT_sendCommand(CO->NMT, CO_NMT_RESET_COMMUNICATION, CANopenSlaveID1);
					CO_NMT_sendCommand(CO->NMT, CO_NMT_RESET_COMMUNICATION, CANopenSlaveID2);
					break;

				case KEY_DOWN_K2:			/* K2键按下, SDO设置节点0x18和0x19的心跳停止 */
					g_ucTempBuf[0] = 0x00;
					g_ucTempBuf[1] = 0x00;
					write_SDO(CO->SDOclient, CANopenSlaveID1, 0x1017, 0x00, g_ucTempBuf, 2);
					write_SDO(CO->SDOclient, CANopenSlaveID2, 0x1017, 0x00, g_ucTempBuf, 2);
					break;

				case KEY_DOWN_K3:			/* K3键按下, SDO设置节点0x18和0x19的心跳为1秒 */
					g_ucTempBuf[0] = 0xE8;
					g_ucTempBuf[1] = 0x03;
					write_SDO(CO->SDOclient, CANopenSlaveID1, 0x1017, 0x00, g_ucTempBuf, 2);
					write_SDO(CO->SDOclient, CANopenSlaveID2, 0x1017, 0x00, g_ucTempBuf, 2);
					break;
				
				case JOY_DOWN_OK:          /* 摇杆OK键, 更新节点RPDO */
					OD_RAM.x2004_obj_ledvarID1++;
					OD_RAM.x2005_obj_ledvarID2++;
					CO_TPDOsendRequest(&CO->TPDO[0]);
				
					HAL_Delay(10);
					read_SDO(CO->SDOclient, CANopenSlaveID1, 0x2000, 0x00, g_ucTempBuf, sizeof(g_ucTempBuf), &g_uiReadSize);
					printf("CANopenSlaveID1 = %02d, %02d, %02d, %02d", g_ucTempBuf[0], g_ucTempBuf[1], g_ucTempBuf[2], g_ucTempBuf[3]);
					read_SDO(CO->SDOclient, CANopenSlaveID2, 0x2000, 0x00, g_ucTempBuf, sizeof(g_ucTempBuf), &g_uiReadSize);
					printf("CANopenSlaveID2 = %02d, %02d, %02d, %02d", g_ucTempBuf[0], g_ucTempBuf[1], g_ucTempBuf[2], g_ucTempBuf[3]);
					break;

				default:
					/* 其它的键值不处理 */
					break;
			}
		}
	}
}

/*
*********************************************************************************************************
*	函 数 名: MX_TIM17_Init
*	功能说明: 提供1ms的时间基准
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/
static void MX_TIM17_Init(void)
{
	htim17.Instance = TIM17;
	htim17.Init.Prescaler = 200-1;
	htim17.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim17.Init.Period = 1000;
	htim17.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim17.Init.RepetitionCounter = 0;
	htim17.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	if (HAL_TIM_Base_Init(&htim17) != HAL_OK)
	{
        Error_Handler(__FILE__, __LINE__);
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance==TIM17)
	{
		__HAL_RCC_TIM17_CLK_ENABLE();

		HAL_NVIC_SetPriority(TIM17_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM17_IRQn);
	}
}

void TIM17_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htim17);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) 
{
	if (htim == canopenNodeSTM32->timerHandle) 
	{
		canopen_app_interrupt();
	}
}

/* 平年的每月天数表 */
const uint8_t mon_table[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

/*
*********************************************************************************************************
*	函 数 名: Is_Leap_Year
*	功能说明: 判断是否为闰年
*	形    参：无
*	返 回 值: 1,是.0,不是
*********************************************************************************************************
*/
static uint8_t Is_Leap_Year(uint16_t _year)
{                     
	if (_year % 4 == 0) /* 必须能被4整除 */
	{ 
		if (_year % 100 == 0) 
		{ 
			if (_year % 400 == 0)
			{
				return 1;	/* 如果以00结尾,还要能被400整除 */
			}
			else 
			{
				return 0;   
			}

		}
		else 
		{
			return 1;   
		}
	}
	else 
	{
		return 0; 
	}
}  

/*
*********************************************************************************************************
*        函 数 名: CANopen_WriteClock
*        功能说明: 设置Canopen时间戳
*        形    参：无
*        返 回 值: 1表示成功 0表示错误
*********************************************************************************************************
*/
uint8_t CANopen_WriteClock(uint16_t _year, uint8_t _mon, uint8_t _day, uint8_t _hour, uint8_t _min, uint8_t _sec, uint32_t _ms, uint32_t _Interval_ms)
{
        uint16_t t;
        uint32_t seccount=0;
		uint32_t days;

        if (_year < 2000 || _year > 2099)
        {
                return 0;        /* _year范围1970-2099，此处设置范围为2000-2099 */   
        } 
   
        for (t = 1984; t < _year; t++)         /* 把所有年份的秒钟相加 */
        {
                if (Is_Leap_Year(t))                /* 判断是否为闰年 */
                {
                        seccount += 31622400;        /* 闰年的秒钟数 */
                }
                else
                {
                        seccount += 31536000;         /* 平年的秒钟数 */
                }
        }

        _mon -= 1;

        for (t = 0; t < _mon; t++)         /* 把前面月份的秒钟数相加 */
        {
                seccount += (uint32_t)mon_table[t] * 86400;        /* 月份秒钟数相加 */

                if (Is_Leap_Year(_year) && t == 1)
                {
                        seccount += 86400;        /* 闰年2月份增加一天的秒钟数 */
                }                        
        }

        seccount += (uint32_t)(_day - 1) * 86400;        /* 把前面日期的秒钟数相加 */

        seccount += (uint32_t)_hour * 3600;                /* 小时秒钟数 */

        seccount += (uint32_t)_min * 60;        /* 分钟秒钟数 */

        seccount += _sec;        /* 最后的秒钟加上去 */
            
		_ms = _ms + (seccount%(60*60*24))*1000;
		days = seccount/(60*60*24);
		
		CO_TIME_set(CO->TIME,  _ms,  days,  _Interval_ms);
		
        return 1;      
}

/*
*********************************************************************************************************
*        函 数 名: read_SDO
*        功能说明: SDO读取
*        形    参：-
*        返 回 值: -
*********************************************************************************************************
*/
CO_SDO_abortCode_t read_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                            uint16_t index, uint8_t subIndex,
                            uint8_t *buf, size_t bufSize, size_t *readSize)
{
    CO_SDO_return_t SDO_ret;
 
    // setup client (this can be skipped, if remote device don't change)
    SDO_ret = CO_SDOclient_setup(SDO_C,
                                 CO_CAN_ID_SDO_CLI + nodeId,
                                 CO_CAN_ID_SDO_SRV + nodeId,
                                 nodeId);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return CO_SDO_AB_GENERAL;
    }
 
    // initiate upload
    SDO_ret = CO_SDOclientUploadInitiate(SDO_C, index, subIndex, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return CO_SDO_AB_GENERAL;
    }
 
    // upload data
    do {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;
 
        SDO_ret = CO_SDOclientUpload(SDO_C,
                                     timeDifference_us,
                                     false,
                                     &abortCode,
                                     NULL, NULL, NULL);
        if (SDO_ret < 0) {
            return abortCode;
        }
 
        bsp_DelayUS(timeDifference_us);
    } while(SDO_ret > 0);
 
    // copy data to the user buffer (for long data function must be called
    // several times inside the loop)
    *readSize = CO_SDOclientUploadBufRead(SDO_C, buf, bufSize);
 
    return CO_SDO_AB_NONE;
}

/*
*********************************************************************************************************
*        函 数 名: write_SDO
*        功能说明: SDO写入
*        形    参：-
*        返 回 值: -
*********************************************************************************************************
*/
CO_SDO_abortCode_t write_SDO(CO_SDOclient_t *SDO_C, uint8_t nodeId,
                             uint16_t index, uint8_t subIndex,
                             uint8_t *data, size_t dataSize)
{
    CO_SDO_return_t SDO_ret;
    bool_t bufferPartial = false;
 
    // setup client (this can be skipped, if remote device is the same)
    SDO_ret = CO_SDOclient_setup(SDO_C,
                                 CO_CAN_ID_SDO_CLI + nodeId,
                                 CO_CAN_ID_SDO_SRV + nodeId,
                                 nodeId);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return -1;
    }
 
    // initiate download
    SDO_ret = CO_SDOclientDownloadInitiate(SDO_C, index, subIndex,
                                           dataSize, 1000, false);
    if (SDO_ret != CO_SDO_RT_ok_communicationEnd) {
        return -1;
    }
 
    // fill data
    size_t nWritten = CO_SDOclientDownloadBufWrite(SDO_C, data, dataSize);
    if (nWritten < dataSize) {
        bufferPartial = true;
        // If SDO Fifo buffer is too small, data can be refilled in the loop.
    }
 
    //download data
    do {
        uint32_t timeDifference_us = 10000;
        CO_SDO_abortCode_t abortCode = CO_SDO_AB_NONE;
 
        SDO_ret = CO_SDOclientDownload(SDO_C,
                                       timeDifference_us,
                                       false,
                                       bufferPartial,
                                       &abortCode,
                                       NULL, NULL);
        if (SDO_ret < 0) {
            return abortCode;
        }
 
        bsp_DelayUS(timeDifference_us);
    } while(SDO_ret > 0);
 
    return CO_SDO_AB_NONE;
}

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
