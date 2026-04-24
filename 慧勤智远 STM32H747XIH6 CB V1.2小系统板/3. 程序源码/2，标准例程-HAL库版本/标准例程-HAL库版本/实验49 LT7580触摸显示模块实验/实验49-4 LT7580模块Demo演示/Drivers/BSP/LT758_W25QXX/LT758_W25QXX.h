#ifndef __LT758_W25QXX_H
#define __LT758_W25QXX_H
#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/delay/delay.h"
#include "./SYSTEM/usart/usart.h"
#include "./BSP/LT758_Lib/LT758_Lib.h"
#include "./BSP/LT758/LT758.h"


#if STM32_SPI
#define SS_RESET SPI_CS(0)
#define SS_SET   SPI_CS(1)
#endif

#define W25X_WriteEnable	    	0x06       
#define W25X_WriteDisable		    0x04 
#define W25X_ReadStatusReg	   	0x05 
#define W25X_ReadStatus2Reg		  0x35 
#define W25X_WriteStatus2Reg    0x31 
#define W25X_WriteStatusReg		  0x01 
#define W25X_ReadData		      	0x03 
#define W25X_FastReadData		    0x0B 
#define W25X_FastReadDual		    0x3B 
#define W25X_PageProgram		    0x02
#define W25X_BlockErase32KB		  0x52
#define W25X_BlockErase64KB		  0xD8 
#define W25X_SectorErase		    0x20 
#define W25X_ChipErase			    0xC7 
#define W25X_PowerDown			    0xB9 
#define W25X_ReleasePowerDown  	0xAB 
#define W25X_DeviceID			      0xAB 
#define W25X_ManufactDeviceID 	0x90 
#define W25X_JedecDeviceID		  0x9F

uint16_t W25QXX_ReadID(void);
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite);
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead);
void W25QXX_Erase_Chip(void);
void W25QXX_SectorErase(uint32_t addr);
void W25QXX_BlockErase32KB(uint32_t addr);
void W25QXX_BlockErase64KB(uint32_t addr);
void W25QXX_Enter_4Byte_AddressMode(void);
uint8_t W25QXX_ReadSR(void);   //¶ÁÈ¡W25QXXµÄ×´Ì¬¼Ä´æÆ÷


/*************************** winbond NandFlash **************************/
uint8_t W25NXX_ReadSR(uint8_t reg);  //¶ÁÈ¡W25N01µÄ×´Ì¬¼Ä´æÆ÷

/*¶ÁÈ¡Æ÷¼þID*/
uint32_t W25NXX_ReadID(void);
//-----------------------------------------

/*Ð´Ê¹ÄÜºÍÊ§ÄÜ*/
void W25NXX_Write_Enable(void);   			//W25QXXÐ´Ê¹ÄÜ
void W25NXX_Write_Disable(void);   			//W25QXXÐ´½ûÖ¹
//-----------------------------------------

/*¶ÁÐ´×´Ì¬¼Ä´æÆ÷*/
void W25NXX_Write_SR(uint8_t reg, uint8_t val);			   //¶ÁÈ¡W25QXXµÄ×´Ì¬¼Ä´æÆ÷
//-----------------------------------------

/*²Á³ý*/
void W25NXX_EraseFlash(unsigned short block_num);
void W25NXX_Erase_Block(uint16_t block);
//-----------------------------------------

void W25NXX_WritePageAddr_Data(uint8_t* pBuffer,uint16_t ColumnAddr,uint32_t PageAddr,uint16_t NumByteToWrite);
void W25NXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void W25NXX_ReadPageAddr_Data(uint8_t* pBuffer,uint16_t ColumnAddr,uint32_t PageAddr,uint16_t NumByteToRead);
void W25NXX_Read_NoCheck(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);

/*************************** winbond NandFlash **************************/


/*************************** Microchip SST26VF032B Flash **************************/
uint8_t SST26VF_ReadSR(void);
void SST26VF_Write_Enable(void);
void SST26VF_Write_Disable(void);
void SST26VF_Wait_Busy(void);
uint32_t SST26VF_ReadID(void);
void SST26VF_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);
void SST26VF_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void SST26VF_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
void SST26VF_Global_Block_Protection_Unlock(void);
void SST26VF_Global_Block_Protection_Lock(void);
void SST26VF_Erase_Chip(void);
void SST26VF_Erase_Sector4K(uint32_t Dst_Addr);
void SST26VF_EraseBlock64KB(uint32_t addr);
#endif
