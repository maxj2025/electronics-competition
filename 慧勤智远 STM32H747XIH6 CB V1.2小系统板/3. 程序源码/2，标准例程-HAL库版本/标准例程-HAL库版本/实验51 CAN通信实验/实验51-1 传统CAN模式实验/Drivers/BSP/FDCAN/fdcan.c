/**
 ****************************************************************************************************
 * @file        fdcan.c
 * @version     V1.0
 * @brief       FDCAN 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */
 
#include "./BSP/FDCAN/fdcan.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"


FDCAN_HandleTypeDef       g_fdcanx_handle;             /* FDCAN1句柄 */
FDCAN_TxHeaderTypeDef     g_fdcanx_txheade;            /* 发送消息 */
FDCAN_RxHeaderTypeDef     g_fdcanx_rxheade;            /* 接收消息 */


/**
 * @brief       FDCAN初始化
 * @param       presc   : 分频值，取值范围1~512;
 * @param       tsjw    : 重新同步跳跃时间单元.范围:1~128;
 * @param       ntsg1   : 时间段1的时间单元.取值范围2~256;
 * @param       ntsg2   : 时间段2的时间单元.取值范围2~128;
 * @param       mode    : FDCAN_MODE_NORMAL，普通模式; 
                          FDCAN_MODE_INTERNAL_LOOPBACK，内部回环模式;
                          FDCAN_MODE_EXTERNAL_LOOPBACK，外部回环模式;
                          FDCAN_MODE_RESTRICTED_OPERATION，限制操作模式
                          FDCAN_MODE_BUS_MONITORING，总线监控模式
 * @note        以上5个参数, 除了操作模式选择，其余的参数在函数内部会减1, 所以, 任何一个参数都不能等于0
 *              配置FDCAN的时钟源为PLL1Q, FDCAN输入时钟频率 fdcan_ker_ck = pll1_q_ck = 80Mhz
 *              波特率 = fdcan_ker_ck / ((ntseg1 + ntseg2 + 1) * presc);
 *              我们设置 fdcan_init(2, 8, 31, 8, mode), 则CAN波特率为:
 *              80M / ((31 + 8 + 1) * 2) = 1Mbps
 * @retval      0, 初始化成功; 其他, 初始化失败;
 */
uint8_t fdcan_init(uint16_t presc, uint8_t tsjw, uint16_t ntseg1, uint8_t ntseg2, uint32_t mode) 
{
    FDCAN_FilterTypeDef fdcan_filterconfig;

    HAL_FDCAN_DeInit(&g_fdcanx_handle);                              /* 先清除以前的设置 */
    g_fdcanx_handle.Instance = FDCAN1; 
    g_fdcanx_handle.Init.FrameFormat = FDCAN_FRAME_CLASSIC;          /* 传统模式 */
    g_fdcanx_handle.Init.Mode = mode;                                /* 操作模式设置  */
    g_fdcanx_handle.Init.AutoRetransmission = DISABLE;               /* 关闭自动重传！传统模式下一定要关闭！！！  */
    g_fdcanx_handle.Init.TransmitPause = DISABLE;                    /* 关闭传输暂停 */
    g_fdcanx_handle.Init.ProtocolException = DISABLE;                /* 关闭协议异常处理 */
  
    g_fdcanx_handle.Init.NominalPrescaler = presc;                   /* 设置比特率分频系数 */
    g_fdcanx_handle.Init.NominalSyncJumpWidth = tsjw;                /* 设置重新同步跳跃宽度 */
    g_fdcanx_handle.Init.NominalTimeSeg1 = ntseg1;                   /* 设置位时间段1 */
    g_fdcanx_handle.Init.NominalTimeSeg2 = ntseg2;                   /* 设置位时间段2 */
    
    g_fdcanx_handle.Init.MessageRAMOffset = 0;                       /* 消息RAM偏移 */
    g_fdcanx_handle.Init.StdFiltersNbr = 1;                          /* 标准消息ID过滤器数量 */
    g_fdcanx_handle.Init.ExtFiltersNbr = 0;                          /* 扩展消息ID过滤器数量 */
    g_fdcanx_handle.Init.RxFifo0ElmtsNbr = 1;                        /* 接收FIFO0元素数量 */
    g_fdcanx_handle.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;       /* 接收FIFO0元素数据字段大小：8字节 */
    g_fdcanx_handle.Init.RxBuffersNbr = 0;                           /* 专用接收缓冲区元素数量 */
    g_fdcanx_handle.Init.TxEventsNbr = 0;                            /* 发送事件FIFO元素数量 */
    g_fdcanx_handle.Init.TxBuffersNbr = 0;                           /* 专用发送缓冲区数量 */
    g_fdcanx_handle.Init.TxFifoQueueElmtsNbr = 1;                    /* 用于发送FIFO/队列的发送缓冲区数量 */
    g_fdcanx_handle.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;  /* 发送FIFO模式 */
    g_fdcanx_handle.Init.TxElmtSize = FDCAN_DATA_BYTES_8;            /* 发送缓冲区数据字段大小：8字节 */

    if (HAL_FDCAN_Init(&g_fdcanx_handle) != HAL_OK)                  /* 初始化FDCAN */
    {
        return 1;   
    }

     /* 过滤器配置 */
    fdcan_filterconfig.IdType = FDCAN_STANDARD_ID;                   /* 标准ID */
    fdcan_filterconfig.FilterIndex = 0;                              /* 滤波器索引 */
    fdcan_filterconfig.FilterType = FDCAN_FILTER_MASK;               /* 滤波器类型：传统位过滤 */
    fdcan_filterconfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;       /* 过滤配置：当过滤匹配以后存储在Rx FIFO0中 */
    fdcan_filterconfig.FilterID1 = 0x123;                            /* 过滤ID1：11位标准ID */
    fdcan_filterconfig.FilterID2 = 0x7FF;                            /* 过滤ID2：配置为传统位过滤，ID2是11位掩码 
                                                                      * 这里表示过滤接收和FilterID1完全一样的消息ID 
                                                                      */
    
    if (HAL_FDCAN_ConfigFilter(&g_fdcanx_handle, &fdcan_filterconfig) != HAL_OK) 
    {
        return 2;                                            
    }

    /* 配置全局过滤器，拒收所有不匹配的帧 */
    HAL_FDCAN_ConfigGlobalFilter(&g_fdcanx_handle, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE);

    /* 开启FDCAN */
    if (HAL_FDCAN_Start(&g_fdcanx_handle) != HAL_OK)
    {
        return 3;
    }  

    /* 使能接收FIFO 0新消息中断 */
    HAL_FDCAN_ActivateNotification(&g_fdcanx_handle, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);

    return 0;
}

/**
 * @brief       FDCAN底层驱动，引脚配置，时钟配置，中断配置
 * @note        此函数会被HAL_FDCAN_Init()调用
 * @param       hcan:FDCAN句柄
 * @retval      无;
 */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hcan)
{
    GPIO_InitTypeDef gpio_init_struct;
    RCC_PeriphCLKInitTypeDef fdcan_periphclk;
  
    if (FDCAN1 == hcan->Instance)
    {
        /* FDCAN时钟源配置为PLL1Q */
        fdcan_periphclk.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
        fdcan_periphclk.FdcanClockSelection = RCC_FDCANCLKSOURCE_PLL;
        HAL_RCCEx_PeriphCLKConfig(&fdcan_periphclk);

        __HAL_RCC_FDCAN_CLK_ENABLE();                         /* 使能FDCAN时钟 */
        FDCAN_RX_GPIO_CLK_ENABLE();                           /* FDCAN_RX引脚时钟使能 */
        FDCAN_TX_GPIO_CLK_ENABLE();                           /* FDCAN_TX引脚时钟使能 */
        
        gpio_init_struct.Pin = FDCAN_RX_GPIO_PIN;             /* FDCAN_RX引脚 */
        gpio_init_struct.Mode = GPIO_MODE_AF_PP;              /* 复用推挽输出 */
        gpio_init_struct.Pull = GPIO_PULLUP;                  /* 上拉 */
        gpio_init_struct.Speed = GPIO_SPEED_FREQ_HIGH;        /* 快速 */
        gpio_init_struct.Alternate = FDCAN_RX_GPIO_AF;        /* 复用为FDCAN1 */
        HAL_GPIO_Init(FDCAN_RX_GPIO_PORT, &gpio_init_struct); /* 初始化FDCAN_RX引脚 */

        gpio_init_struct.Pin = FDCAN_TX_GPIO_PIN;             /* FDCAN_TX引脚 */
        gpio_init_struct.Alternate = FDCAN_TX_GPIO_AF;        /* 复用为FDCAN1 */      
        HAL_GPIO_Init(FDCAN_TX_GPIO_PORT, &gpio_init_struct); /* 初始化FDCAN_TX引脚 */

#if FDCAN1_RX0_INT_ENABLE                                     /* 如果使能了FDCAN1接收RX0中断 */
        HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);                  /* 使能FDCAN1中断线0中断 */
        HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 1, 2);          /* 抢占优先级1，子优先级2 */
#endif
    }
}

/**
 * @brief       此函数会被HAL_FDCAN_DeInit调用
 * @param       hfdcan:fdcan句柄
 * @retval      无
 */
void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *hfdcan)
{
    __HAL_RCC_FDCAN_FORCE_RESET();       /* 复位FDCAN */
    __HAL_RCC_FDCAN_RELEASE_RESET();     /* 停止复位 */
    
#if FDCAN1_RX0_INT_ENABLE   
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
#endif
}

/**
 * @brief       FDCAN 发送一组数据
 * @note        发送格式固定为: 标准ID, 数据帧
 * @param       len     :数据长度，取值范围：FDCAN_DLC_BYTES_0 ~ FDCAN_DLC_BYTES_64
 * @param       msg     :数据指针，最大为64个字节
 * @retval      发送状态 0, 成功; 1, 失败;
 */
uint8_t fdcan_send_msg(uint8_t *msg, uint32_t len)
{
    g_fdcanx_txheade.Identifier = 0x123;                             /* 帧ID */
    g_fdcanx_txheade.IdType = FDCAN_STANDARD_ID;                     /* 11位标准ID */
    g_fdcanx_txheade.TxFrameType = FDCAN_DATA_FRAME;                 /* 发送数据帧 */
    g_fdcanx_txheade.DataLength = len;                               /* 数据长度 */
    g_fdcanx_txheade.ErrorStateIndicator = FDCAN_ESI_ACTIVE;         /* 传输节点显性错误 */
    g_fdcanx_txheade.BitRateSwitch = FDCAN_BRS_OFF;                  /* 关闭速率切换 */
    g_fdcanx_txheade.FDFormat = FDCAN_CLASSIC_CAN;                   /* 标准帧格式 */
    g_fdcanx_txheade.TxEventFifoControl = FDCAN_NO_TX_EVENTS;        /* 不存储发送事件 */
    g_fdcanx_txheade.MessageMarker = 0;                              /* 消息掩码 */ 

    /* 添加消息到发送FIFO并添加相应的发送请求 */
    if (HAL_FDCAN_AddMessageToTxFifoQ(&g_fdcanx_handle, &g_fdcanx_txheade, msg) != HAL_OK)  /* 发送消息 */
    {
        return 1;
    }

    return 0;
}

/**
 * @brief       FDCAN接收数据查询
 * @param       buf:数据缓存区
 * @retval      0, 没有接收到数据;
 *              其他, 接收到的数据长度;
 */
uint8_t fdcan_receive_msg(uint8_t *buf)
{
    /* 从RX FIFO0检索接收的消息 */
    if (HAL_FDCAN_GetRxMessage(&g_fdcanx_handle, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, buf) != HAL_OK)  /* 接收数据 */
    {
        return 0;
    }

    return g_fdcanx_rxheade.DataLength;
}

#if FDCAN1_RX0_INT_ENABLE           /* 使能FDCAN1 RX0中断 */

/**
 * @brief       FDCAN1中断线0中断服务函数
 * @param       无
 * @retval      无;
 */
void FDCAN1_IT0_IRQHandler(void)
{
    HAL_FDCAN_IRQHandler(&g_fdcanx_handle);
}

/**
 * @brief       接收FIFO0回调函数
 * @param       hfdcan:FDCAN句柄
 * @param       RxFifo0ITs:接收FIFO 0中断状态
 * @retval      无;
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    uint8_t i = 0;
    uint8_t rxdata[8];

    if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)     /* 接收FIFO0新消息中断 */ 
    {
        /* 提取FIFO0中接收到的数据 */
        HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &g_fdcanx_rxheade, rxdata);
        printf("id:%#x\r\n", g_fdcanx_rxheade.Identifier);
        printf("len:%d\r\n", g_fdcanx_rxheade.DataLength>>16);
        for (i = 0; i < 8; i++)
        {
            printf("rxdata[%d]:%d\r\n", i, rxdata[i]);
        }
        
        HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0);
    }
}

#endif






