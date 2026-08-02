#include "qspi.h"

/**
 * @brief       OSPI发送命令
 * @param       cmd : 要发送的指令
 * @param       addr: 发送到的目的地址
 * @param       mode: 模式,详细位定义如下:
 *   @arg       mode[2:0]:  指令模式; 000,无指令;  001,单线传输指令; 010,双线传输指令; 011,四线传输指令; 100,八线传输指令.
 *   @arg       mode[5:3]:  地址模式; 000,无地址;  001,单线传输地址; 010,双线传输地址; 011,四线传输地址; 100,八线传输地址.
 *   @arg       mode[7:6]:  地址长度; 00,8位地址;   01,16位地址;      10,24位地址;      11,32位地址.
 *   @arg       mode[10:8]: 数据模式; 000,无数据;  001,单线传输数据; 010,双线传输数据; 011,四线传输数据; 100,八线传输数据.
 * @param       dmcycle: 空指令周期数
 * @retval      无
 */
void ospi_send_cmd(uint8_t cmd, uint32_t addr, uint16_t mode, uint8_t dmcycle)
{
  	OSPI_RegularCmdTypeDef sCommand;	                               /* OSPI常规命令结构体 */

    sCommand.OperationType = HAL_OSPI_OPTYPE_COMMON_CFG;             /* 通用配置(间接模式或自动轮询模式下使用) */
    sCommand.FlashId = HAL_OSPI_FLASH_ID_1;                          /* 选择FLASH1 */
    sCommand.Instruction = cmd;                                      /* 设置要发送的指令 */

    if(((mode >> 0) & 0x07) == 0)
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_NONE;            /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 1)
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE;          /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 2)
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_2_LINES;         /* 指令模式 */
    else if(((mode >> 0) & 0x07) == 3)
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_4_LINES;         /* 指令模式 */    
    else if(((mode >> 0) & 0x07) == 4)
    sCommand.InstructionMode = HAL_OSPI_INSTRUCTION_8_LINES;         /* 指令模式 */    
    
    sCommand.InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS;          /* 指令长度为8位 */
    sCommand.InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE;  /* 禁止指令阶段DTR模式 */
    sCommand.Address = addr;                                         /* 设置要发送的地址 */
    
    if(((mode >> 3) & 0x07) == 0)
    sCommand.AddressMode = HAL_OSPI_ADDRESS_NONE;                    /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 1)
    sCommand.AddressMode = HAL_OSPI_ADDRESS_1_LINE;                  /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 2)
    sCommand.AddressMode = HAL_OSPI_ADDRESS_2_LINES;                 /* 地址模式 */
    else if(((mode >> 3) & 0x07) == 3)
    sCommand.AddressMode = HAL_OSPI_ADDRESS_4_LINES;                 /* 地址模式 */    
    else if(((mode >> 3) & 0x07) == 4)
    sCommand.AddressMode = HAL_OSPI_ADDRESS_8_LINES;                 /* 地址模式 */  

    if(((mode >> 6) & 0x03) == 0)
    sCommand.AddressSize = HAL_OSPI_ADDRESS_8_BITS;                  /* 地址长度 */
    else if(((mode >> 6) & 0x03) == 1)
    sCommand.AddressSize = HAL_OSPI_ADDRESS_16_BITS;                 /* 地址长度 */
    else if(((mode >> 6) & 0x03) == 2)
    sCommand.AddressSize = HAL_OSPI_ADDRESS_24_BITS;                 /* 地址长度 */
    else if(((mode >> 6) & 0x03) == 3) 
    sCommand.AddressSize = HAL_OSPI_ADDRESS_32_BITS;                 /* 地址长度 */   
  
    sCommand.AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE;          /* 禁止地址阶段DTR模式 */    
    sCommand.AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE;     /* 无交替字节 */         

    if(((mode >> 8) & 0x07) == 0)
    sCommand.DataMode = HAL_OSPI_DATA_NONE;                          /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 1)
    sCommand.DataMode = HAL_OSPI_DATA_1_LINE;                        /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 2)
    sCommand.DataMode = HAL_OSPI_DATA_2_LINES;                       /* 数据模式 */
    else if(((mode >> 8) & 0x07) == 3)
    sCommand.DataMode = HAL_OSPI_DATA_4_LINES;                       /* 数据模式 */    
    else if(((mode >> 8) & 0x07) == 4)
    sCommand.DataMode = HAL_OSPI_DATA_8_LINES;                       /* 数据模式 */     
    
    sCommand.DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE;                /* 禁止数据阶段DTR模式 */
    sCommand.DummyCycles = dmcycle;                                  /* 设置空指令周期数 */
    sCommand.DQSMode = HAL_OSPI_DQS_DISABLE;                         /* 不使用DQS */
    sCommand.SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD;                /* 每次都发送指令 */   

    HAL_OSPI_Command(&hospi1, &sCommand, HAL_OSPI_TIMEOUT_DEFAULT_VALUE);  /* 设置OSPI命令配置参数 */
}

/**
 * @brief       OSPI接收指定长度的数据
 * @param       buf     : 接收数据缓冲区首地址
 * @param       datalen : 要传输的数据长度
 * @retval      0, 成功; 1, 失败.
 */
uint8_t ospi_receive(uint8_t *buf, uint32_t datalen)
{
    hospi1.Instance->DLR = datalen - 1;   /* 配置数据传输长度 */
  
    if (HAL_OSPI_Receive(&hospi1, buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK) 
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

/**
 * @brief       OSPI发送指定长度的数据
 * @param       buf     : 发送数据缓冲区首地址
 * @param       datalen : 要传输的数据长度
 * @retval      0, 成功; 1, 失败.
 */
uint8_t ospi_transmit(uint8_t *buf, uint32_t datalen)
{
    hospi1.Instance->DLR = datalen - 1;   /* 配置数据传输长度 */
  
    if (HAL_OSPI_Transmit(&hospi1, buf, HAL_OSPI_TIMEOUT_DEFAULT_VALUE) == HAL_OK)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}
