/**
 ****************************************************************************************************
 * @file        dma.c
 * @version     V1.0
 * @brief       DMA 驱动代码
 ****************************************************************************************************
 * @attention   Waiken-Smart 慧勤智远
 *
 * 实验平台:    STM32H747XIH6小系统板
 *
 ****************************************************************************************************
 */

#include "./BSP/DMA/dma.h"
#include "./SYSTEM/delay/delay.h"


DMA_HandleTypeDef  g_usart_tx_dma_handle;         /* 串口TX DMA句柄 */
DMA_HandleTypeDef  g_usart_rx_dma_handle;         /* 串口RX DMA句柄 */

extern UART_HandleTypeDef g_uart1_handle;         /* UART句柄 */

/**
 * @brief       串口TX DMA初始化函数
 * @note        这里的传输形式是固定的, 这点要根据不同的情况来修改
 *              从存储器 -> 外设模式/8位数据宽度/存储器增量模式
 *
 * @param       dma_streamx : DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch          : DMA通道请求选择,范围:1~115(详见<<STM32H7xx参考手册>>),可设置的值参考:DMA_Request_selection  
 * @retval      无
 */
void dma_usart_tx_init(DMA_Stream_TypeDef *dma_streamx, uint32_t ch)
{ 
    if ((uint32_t)dma_streamx > (uint32_t)DMA2)                   /* 得到当前stream是属于DMA2还是DMA1 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                              /* DMA2时钟使能 */
    }
    else
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                              /* DMA1时钟使能 */
    }

    __HAL_LINKDMA(&g_uart1_handle, hdmatx, g_usart_tx_dma_handle);         /* 将DMA与USART1联系起来(发送DMA) */

    /* Tx DMA配置 */
    g_usart_tx_dma_handle.Instance = dma_streamx;                          /* 数据流选择 */
    g_usart_tx_dma_handle.Init.Request = ch;                               /* 输入DMA请求 */
    g_usart_tx_dma_handle.Init.Direction = DMA_MEMORY_TO_PERIPH;           /* 数据传输方向:存储器到外设 */
    g_usart_tx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;               /* 外设非增量模式 */
    g_usart_tx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;                   /* 存储器增量模式 */
    g_usart_tx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  /* 外设数据长度:8位 */
    g_usart_tx_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     /* 存储器数据长度:8位 */
    g_usart_tx_dma_handle.Init.Mode = DMA_NORMAL;                          /* 普通模式 */
    g_usart_tx_dma_handle.Init.Priority = DMA_PRIORITY_MEDIUM;             /* 中等优先级 */
    g_usart_tx_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;            /* 关闭FIFO模式 */
    g_usart_tx_dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;    /* FIFO阈值设置 */
    g_usart_tx_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;               /* 存储器突发单次传输 */
    g_usart_tx_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;            /* 外设突发单次传输 */

    HAL_DMA_DeInit(&g_usart_tx_dma_handle);   
    HAL_DMA_Init(&g_usart_tx_dma_handle);
}


uint8_t g_transfer_complete = 0;     /* DMA传输完成标志 */

/**
 * @brief       串口RX DMA初始化函数
 * @param       dma_streamx : DMA数据流,DMA1_Stream0~7/DMA2_Stream0~7
 * @param       ch          : DMA通道请求选择,范围:1~115(详见<<STM32H7xx参考手册>>),可设置的值参考:DMA_Request_selection  
 * @param       par         : 外设地址
 * @param       mar         : 存储器地址
 * @retval      无
 */
void dma_usart_rx_init(DMA_Stream_TypeDef *dma_streamx, uint32_t ch, uint32_t par, uint32_t mar)
{ 
    if ((uint32_t)dma_streamx > (uint32_t)DMA2)                   /* 得到当前stream是属于DMA2还是DMA1 */
    {
        __HAL_RCC_DMA2_CLK_ENABLE();                              /* DMA2时钟使能 */
    }
    else
    {
        __HAL_RCC_DMA1_CLK_ENABLE();                              /* DMA1时钟使能 */
    }

    //__HAL_LINKDMA(&g_uart1_handle, hdmarx, g_dma1_handle);      /* 将DMA与USART1联系起来(发送DMA) */

    /* Rx DMA配置 */
    g_usart_rx_dma_handle.Instance = dma_streamx;                          /* 数据流选择 */
    g_usart_rx_dma_handle.Init.Request = ch;                               /* 输入DMA请求 */
    g_usart_rx_dma_handle.Init.Direction = DMA_PERIPH_TO_MEMORY;           /* 数据传输方向:外设到存储器 */
    g_usart_rx_dma_handle.Init.PeriphInc = DMA_PINC_DISABLE;               /* 外设非增量模式 */
    g_usart_rx_dma_handle.Init.MemInc = DMA_MINC_ENABLE;                   /* 存储器增量模式 */
    g_usart_rx_dma_handle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;  /* 外设数据长度:8位 */
    g_usart_rx_dma_handle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;     /* 存储器数据长度:8位 */
    g_usart_rx_dma_handle.Init.Mode = DMA_NORMAL;                          /* 普通模式 */
    g_usart_rx_dma_handle.Init.Priority = DMA_PRIORITY_HIGH;               /* 高优先级 */
    g_usart_rx_dma_handle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;            /* 关闭FIFO模式 */
    g_usart_rx_dma_handle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;    /* FIFO阈值设置 */
    g_usart_rx_dma_handle.Init.MemBurst = DMA_MBURST_SINGLE;               /* 存储器突发单次传输 */
    g_usart_rx_dma_handle.Init.PeriphBurst = DMA_PBURST_SINGLE;            /* 外设突发单次传输 */

    HAL_DMA_DeInit(&g_usart_rx_dma_handle);   
    HAL_DMA_Init(&g_usart_rx_dma_handle);
 
    /* 根据使用的数据流来设置中断编号 */
    __HAL_DMA_ENABLE_IT(&g_usart_rx_dma_handle, DMA_IT_TC);                /* 使能传输完成中断 */
    HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 2, 0);                         /* 设置DMA中断优先级，抢占优先级2，子优先级0 */
    HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
     
    HAL_DMA_Start(&g_usart_rx_dma_handle, (uint32_t)par, (uint32_t)mar, USART_RX_DMA_TRANS);  /* 开启DMA传输 */
}

/**
 * @brief       DMA1_Stream1中断服务函数
 * @param       无
 * @retval      无
 */
void DMA1_Stream1_IRQHandler(void)
{
    if (__HAL_DMA_GET_FLAG(&g_usart_rx_dma_handle, DMA_FLAG_TCIF1_5) != RESET)  /* DMA传输完成 */
    {
        __HAL_DMA_CLEAR_FLAG(&g_usart_rx_dma_handle, DMA_FLAG_TCIF1_5);         /* 清除DMA传输完成中断标志位 */
        g_transfer_complete = 1;
        SCB_CleanInvalidateDCache();                                            /* 清除无效的D-Cache */
    } 
}







