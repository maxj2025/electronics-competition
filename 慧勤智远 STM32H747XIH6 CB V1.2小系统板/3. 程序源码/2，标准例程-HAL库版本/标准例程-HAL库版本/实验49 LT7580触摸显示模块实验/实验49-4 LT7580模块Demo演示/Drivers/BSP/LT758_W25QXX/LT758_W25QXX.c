#include "./BSP/LT758_W25QXX/LT758_W25QXX.h"

//------------------------------------------------------- 操作LT758外挂的W25QXX -------------------------------------------------------------
uint8_t W25QXX_ReadSR(void)   //读取W25QXX的状态寄存器
{ 
	if(Bypass_Mode == 0)
	{
		uint8_t byte=0;   
		nSS_Active();                            		//使能器件   
		SPI_Master_FIFO_Data_Put(W25X_ReadStatusReg); 	//发送读取状态寄存器命令    
		byte=SPI_Master_FIFO_Data_Put(0Xff);          	//读取一个字节  
		nSS_Inactive();                            		//取消片选     
		return byte;
	}
	else
	{
		uint8_t byte=0;
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_ReadStatusReg);
		byte=spi4_read_write_byte(0Xff);
		SS_SET;
		#endif
		return byte; 
	}
} 
	
//将WEL置位   
void W25QXX_Write_Enable(void)   //W25QXX写使能
{   
	if(Bypass_Mode == 0)
	{
		nSS_Active();                          			//使能器件   
		SPI_Master_FIFO_Data_Put(W25X_WriteEnable); 	//发送写使能  
		nSS_Inactive(); 														//取消片选   
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_WriteEnable);
		SS_SET;
		#endif
	}
} 
	
//将WEL清零  
void W25QXX_Write_Disable(void)   //W25QXX写禁止
{                            		
	if(Bypass_Mode == 0)
	{
		nSS_Active();                            		//使能器件   
		SPI_Master_FIFO_Data_Put(W25X_WriteDisable); 	//发送写禁止指令    
		nSS_Inactive(); 														//取消片选 
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_WriteDisable);
		SS_SET;
		#endif
	}
} 		

void W25QXX_Wait_Busy(void)   		//等待空闲
{   
	while((W25QXX_ReadSR()&0x01)==0x01) ;  		// 等待BUSY位清空
}

//读取芯片ID
//返回值如下:				   
//0XEF13,表示芯片型号为W25Q80  
//0XEF14,表示芯片型号为W25Q16    
//0XEF15,表示芯片型号为W25Q32  
//0XEF16,表示芯片型号为W25Q64 
//0XEF17,表示芯片型号为W25Q128
//0XEF18,表示芯片型号为W25Q256 	  
uint16_t W25QXX_ReadID(void)
{
	uint16_t Temp = 0;
	if(Bypass_Mode == 0)
	{
		nSS_Active();			
		SPI_Master_FIFO_Data_Put(0x90);//发送读取ID命令	    
		SPI_Master_FIFO_Data_Put(0x00); 	    
		SPI_Master_FIFO_Data_Put(0x00); 	    
		SPI_Master_FIFO_Data_Put(0x00); 	 			   
		Temp|=SPI_Master_FIFO_Data_Put(0xFF)<<8;  
		Temp|=SPI_Master_FIFO_Data_Put(0xFF);	 
		nSS_Inactive();				    
		return Temp;
	}
	else
	{
		#if STM32_SPI
		printf("by_pass\r\n");
		SS_RESET;
		spi4_read_write_byte(0x90);//发送读取ID命令	    
		spi4_read_write_byte(0x00); 	    
		spi4_read_write_byte(0x00); 	    
		spi4_read_write_byte(0x00); 	 			   
		Temp|=spi4_read_write_byte(0xFF)<<8;  
		Temp|=spi4_read_write_byte(0xFF);		
		SS_SET;
		#endif
		return Temp;														//取消片选  
	}
}   		

//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(最大32bit)
//NumByteToRead:要读取的字节数
void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint32_t NumByteToRead)   
{ 
	uint32_t i;
	
	if(Bypass_Mode == 0)
	{ 										    
		nSS_Active();                            			//使能器件   
		SPI_Master_FIFO_Data_Put(W25X_ReadData);         	//发送读取命令  
	
		if(w25q256)
			SPI_Master_FIFO_Data_Put((uint8_t)((ReadAddr)>>24));  	//256Flash发送32bit地址
		SPI_Master_FIFO_Data_Put((uint8_t)((ReadAddr)>>16));  	//发送24bit地址    
		SPI_Master_FIFO_Data_Put((uint8_t)((ReadAddr)>>8));   
		SPI_Master_FIFO_Data_Put((uint8_t)ReadAddr); 
		for(i=0;i<NumByteToRead;i++)
		{ 
			 pBuffer[i]=SPI_Master_FIFO_Data_Put(0XFF);   	//循环读数  
		}
		
		nSS_Inactive(); 
		
	}
	else
	{ 
		printf("by_bass\r\n");
		#if STM32_SPI
		SS_RESET;
    	spi4_read_write_byte(W25X_ReadData);         	//发送读取命令 

		if(w25q256)
			spi4_read_write_byte((uint8_t)((ReadAddr)>>24));  	//256Flash发送32bit地址
	
    	spi4_read_write_byte((uint8_t)((ReadAddr)>>16));  	//发送24bit地址    
    	spi4_read_write_byte((uint8_t)((ReadAddr)>>8));   
    	spi4_read_write_byte((uint8_t)ReadAddr);   
    	for(i=0;i<NumByteToRead;i++)
		{ 
        	pBuffer[i]=spi4_read_write_byte(0XFF);   	//循环读数  
    	}		
		SS_SET;
		#endif
	}		
} 

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void W25QXX_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
	uint16_t i;
	if(Bypass_Mode == 0)
	{ 										      
		W25QXX_Write_Enable();                  							//SET WEL 
		nSS_Active();                            							//使能器件   
		SPI_Master_FIFO_Data_Put(W25X_PageProgram);      					//发送写页命令 

		if(w25q256)
			SPI_Master_FIFO_Data_Put((uint8_t)((WriteAddr)>>24));  	//256Flash发送32bit地址
		
		SPI_Master_FIFO_Data_Put((uint8_t)((WriteAddr)>>16)); 					//发送24bit地址    
		SPI_Master_FIFO_Data_Put((uint8_t)((WriteAddr)>>8));   
		SPI_Master_FIFO_Data_Put((uint8_t)WriteAddr);
		
		#if STM32_SPI
		LCD_CmdWrite(0xB8);
		
		SS_RESET;
		spi4_read_write_byte(0x80);
		
		for(i=0;i<NumByteToWrite;i++)
		{
			spi4_read_write_byte(pBuffer[i]);
			delay_us(1);
		}
		SS_SET;
		
		#else
		for(i=0;i<NumByteToWrite;i++)
		{
			SPI_Master_FIFO_Data_Put(pBuffer[i]);
		}		
		#endif

		nSS_Inactive();                            							//取消片选 
		W25QXX_Wait_Busy();					   								//等待写入结束
	}
	else
	{  										    
		W25QXX_Write_Enable();                  							//SET WEL
		#if STM32_SPI
		SS_RESET;                          							//使能器件   
    	spi4_read_write_byte(W25X_PageProgram);      					//发送写页命令 

		if(w25q256)
		spi4_read_write_byte((uint8_t)((WriteAddr)>>24));  	//256Flash发送32bit地址
	
    	spi4_read_write_byte((uint8_t)((WriteAddr)>>16)); 					//发送24bit地址    
    	spi4_read_write_byte((uint8_t)((WriteAddr)>>8));   
    	spi4_read_write_byte((uint8_t)WriteAddr); 
    	for(i=0;i<NumByteToWrite;i++)spi4_read_write_byte(pBuffer[i]);	//循环写数  	
		SS_SET;
		#endif
		W25QXX_Wait_Busy();					   								//等待写入结束
	}	
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(最大32bit)
//NumByteToWrite:要写入的字节数
//CHECK OK
void W25QXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint32_t NumByteToWrite)   
{ 
	uint16_t pageremain;						
  
  pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
  if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
  while(1)
  {	   
    W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
    if(NumByteToWrite==pageremain)break;//写入结束了
    else //NumByteToWrite>pageremain
    {
      pBuffer+=pageremain;
      WriteAddr+=pageremain;	

      NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
      if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
      else pageremain=NumByteToWrite; 	  //不够256个字节了
    }
  }
} 

//擦除整个芯片		  
//等待时间超长...
void W25QXX_Erase_Chip(void)   
{	
	if(Bypass_Mode == 0)
	{ 										        
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   
		nSS_Active();                            	  //使能器件   
		SPI_Master_FIFO_Data_Put(W25X_ChipErase);   //发送片擦除命令  
		nSS_Inactive();                            	//取消片选     
		W25QXX_Wait_Busy();   				   		        //等待芯片擦除结束
	}
	else
	{  										     
		W25QXX_Write_Enable();                  //SET WEL 
		W25QXX_Wait_Busy();   

		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_ChipErase);		//发送片擦除命令  
		SS_SET;
		#endif
		W25QXX_Wait_Busy();   				   	//等待芯片擦除结束
	}
} 

void W25QXX_SectorErase(uint32_t addr)   
{ 	    
	if(Bypass_Mode == 0)
	{ 	
		addr *=4096;
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   
		nSS_Active();                            	   //使能器件   
		SPI_Master_FIFO_Data_Put(W25X_SectorErase);  //发送64KB块擦除命令 

	  if(w25q256)
			SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>16));  //发送24bit地址    
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>8));   
		SPI_Master_FIFO_Data_Put((uint8_t)addr);  		   //发送块地址
		nSS_Inactive();                              //取消片选     
		W25QXX_Wait_Busy();   				   	   //等待芯片擦除结束
	}
	else
	{  										     
		addr *=4096;
		W25QXX_Write_Enable();                  //SET WEL 	 
		W25QXX_Wait_Busy();   

		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_SectorErase);  	//发送扇区擦除指令 
		
		if(w25q256)
			spi4_read_write_byte((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		spi4_read_write_byte((uint8_t)((addr)>>16));  //发送24bit地址    
		spi4_read_write_byte((uint8_t)((addr)>>8));   
		spi4_read_write_byte((uint8_t)addr);
		SS_SET;
		#endif
		W25QXX_Wait_Busy();   				   	//等待擦除完成 	
	}
} 
void W25QXX_BlockErase64KB(uint32_t addr)   
{ 
	if(Bypass_Mode == 0)
	{ 										        
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   
		nSS_Active();                            	  //使能器件   
		SPI_Master_FIFO_Data_Put(W25X_BlockErase64KB);  //发送64KB块擦除命令 
		
	  if(w25q256)
			SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>16)); 	  //发送24bit地址    
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>8));   
		SPI_Master_FIFO_Data_Put((uint8_t)addr);  			  //发送块地址
		nSS_Inactive();                             //取消片选     
		W25QXX_Wait_Busy();   				   	  //等待芯片擦除结束
	}
	else
	{  										     
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   

		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_BlockErase64KB);  //发送64KB块擦除命令 
		
	  if(w25q256)
			spi4_read_write_byte((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		
		spi4_read_write_byte((uint8_t)((addr)>>16)); 	  //发送24bit地址    
		spi4_read_write_byte((uint8_t)((addr)>>8));   
		spi4_read_write_byte((uint8_t)addr);  			  //发送块地址
		SS_SET;
		#endif
		W25QXX_Wait_Busy();   				   	  //等待芯片擦除结束 	
	}
} 

void W25QXX_BlockErase32KB(uint32_t addr)   
{ 
	if(Bypass_Mode == 0)
	{ 										        
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   
		nSS_Active();                            	  //使能器件   
		SPI_Master_FIFO_Data_Put(W25X_BlockErase32KB);  //发送64KB块擦除命令 
		
	  if(w25q256)
			SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>16)); 	  //发送24bit地址    
		SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>8));   
		SPI_Master_FIFO_Data_Put((uint8_t)addr);  			  //发送块地址
		nSS_Inactive();                             //取消片选     
		W25QXX_Wait_Busy();   				   	  //等待芯片擦除结束
	}
	else
	{  										     
		W25QXX_Write_Enable();                 	 	  //SET WEL 
		W25QXX_Wait_Busy();   

		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(W25X_BlockErase32KB);  //发送64KB块擦除命令 
		
	  	if(w25q256)
			spi4_read_write_byte((uint8_t)((addr)>>24));  	//256Flash发送32bit地址
		
		spi4_read_write_byte((uint8_t)((addr)>>16)); 	  //发送24bit地址    
		spi4_read_write_byte((uint8_t)((addr)>>8));   
		spi4_read_write_byte((uint8_t)addr);  			  //发送块地址
		SS_SET;
		#endif
		W25QXX_Wait_Busy();   				   	  //等待芯片擦除结束 		
	}
} 

void W25QXX_Enter_4Byte_AddressMode(void)
{
	if(Bypass_Mode == 0)
	{ 										        
		nSS_Active();
		SPI_Master_FIFO_Data_Put(0xB7);
		nSS_Inactive();
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0xB7);
		SS_SET;
		#endif
	}
}

/*************************************************************** winbond NandFlash **********************************************************************/
//指令表
#define W25NXX_WriteEnable		0x06       
#define W25NXX_WriteDisable		0x04 
#define W25NXX_ReadStatusReg	0x05 
#define W25NXX_WriteStatusReg	0x01 
#define W25NXX_ReadData			0x03 
#define W25NXX_FastReadData		0x0B 
#define W25NXX_FastReadDual		0x3B 
#define W25NXX_PageProgram		0x02 
#define W25NXX_BlockErase		0xD8 
#define W25NXX_SectorErase		0x20 
#define W25NXX_ChipErase		0xC7 
#define W25NXX_PowerDown		0xB9 
#define W25NXX_ReleasePowerDown	0xAB 
#define W25NXX_DeviceID			0xAB 
#define W25NXX_ManufactDeviceID	0x90 
#define W25NXX_JedecDeviceID	0x9F


//读取W25QXX的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t W25NXX_ReadSR(uint8_t reg)   //读取W25N01的状态寄存器
{  
	uint8_t byte = 0;

	if(Bypass_Mode == 0)
	{
		nSS_Active();                                  	//使能器件 
		SPI_Master_FIFO_Data_Put(0x0F);   					//发送读取状态寄存器命令 
		SPI_Master_FIFO_Data_Put(reg);                     //寄存器地址 
		byte = SPI_Master_FIFO_Data_Put(0Xff);             //读取一个字节 
		nSS_Inactive();                                 	//取消片选  	
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x0F);   					//发送读取状态寄存器命令 
		spi4_read_write_byte(reg);                     //寄存器地址 
		byte = spi4_read_write_byte(0Xff);             //读取一个字节 
		SS_SET;
		#endif
	}
	
	return byte;  
} 

//将WEL置位   
void W25NXX_Write_Enable(void)   //W25QXX写使能
{
	if(Bypass_Mode == 0)
	{
		nSS_Active();                         			//使能器件   
		SPI_Master_FIFO_Data_Put(0x06); 	//发送写使能  
		nSS_Inactive();                          		//取消片选   
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x06); 	//发送写使能  
		SS_SET;
		#endif
	}
	
} 
	
//将WEL清零  
void W25NXX_Write_Disable(void)   //W25QXX写禁止
{  
	if(Bypass_Mode == 0)
	{
		nSS_Active();                          		//使能器件   
		SPI_Master_FIFO_Data_Put(0x04); 	//发送写禁止指令    
		nSS_Inactive();                            	//取消片选   
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x04); 	//发送写禁止指令  
		SS_SET;
		#endif
	}
} 	

// 3个状态寄存器
// 保护寄存器1：地址0xA0
// 配置寄存器2: 地址0xB0
// 状态寄存器3: 地址0xC0  (只读寄存器)
void W25NXX_Write_SR(uint8_t reg, uint8_t val)   
{ 
	W25NXX_Write_Enable();	
	if(Bypass_Mode == 0)
	{	
		nSS_Active(); 							//使能器件			 
		SPI_Master_FIFO_Data_Put(0x1F);          //发送写取状态寄存器命令    
		SPI_Master_FIFO_Data_Put(reg);                      //寄存器地址 
		SPI_Master_FIFO_Data_Put(val);               	      //写入一个字节      	      		
		nSS_Inactive();                                 //取消片选 
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x1F);          //发送写取状态寄存器命令    
		spi4_read_write_byte(reg);                      //寄存器地址 
		spi4_read_write_byte(val);               	      //写入一个字节    
		SS_SET;
		#endif
	}
	W25NXX_Write_Disable();
} 
	
void W25NXX_Wait_Busy(void)   		//等待空闲
{   
	while((W25NXX_ReadSR(0xc0)&0x01)==0x01);  		// 等待BUSY位清空
}  

//读取芯片ID
//返回值如下:				   
//0xefaa21,表示芯片型号为W25N01  
  

uint32_t W25NXX_ReadID(void)
{
	uint8_t Mfr_id = 0;
	uint16_t Device_id = 0;
	uint32_t id = 0;
	
	if(Bypass_Mode == 0)
	{
		nSS_Active();	
		SPI_Master_FIFO_Data_Put(0x9f);
		SPI_Master_FIFO_Data_Put(0x00);
		Mfr_id = SPI_Master_FIFO_Data_Put(0xff);
		Device_id = SPI_Master_FIFO_Data_Put(0xff);
		Device_id = Device_id<<8 | SPI_Master_FIFO_Data_Put(0xff);
		nSS_Inactive();
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x9f);
		spi4_read_write_byte(0x00);
		Mfr_id = spi4_read_write_byte(0xff);
		Device_id = spi4_read_write_byte(0xff);
		Device_id = Device_id<<8 | spi4_read_write_byte(0xff);
		SS_SET;
		#endif
	}
	id = Mfr_id;
	id = (id<<16)+Device_id;
	return id;
}  

/**********************擦除**********************/
//擦除128KB Block
void W25NXX_Erase_Block(uint16_t block)   
{               
	uint32_t addr = 0;	
	addr = 64 * block;  //输入的是Pag地址，一个块有64页(华邦1G/2G，芯天下1G/2G)。
	
	if(Flash_ID == 0x0BE30B)
	{
		addr = 128* block; //芯天下4G
	}
	
	
	W25NXX_Write_SR(0xA0, 0x00);          // 关flash保护功能
	
  W25NXX_Write_Enable();                //SET WEL 
  W25NXX_Wait_Busy();   
	if(Bypass_Mode == 0)
	{
		nSS_Active();  						    //使能器件
		SPI_Master_FIFO_Data_Put(0xD8);        	//发送片擦除命令  
		SPI_Master_FIFO_Data_Put((uint8_t)(addr>>16));
		SPI_Master_FIFO_Data_Put((uint8_t)(addr>>8)); 
		SPI_Master_FIFO_Data_Put((uint8_t)(addr));
		nSS_Inactive();                         //取消片选   
	}  	
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0xD8);        	//发送片擦除命令  
		spi4_read_write_byte((uint8_t)(addr>>16));
		spi4_read_write_byte((uint8_t)(addr>>8)); 
		spi4_read_write_byte((uint8_t)(addr));
		SS_SET;
		#endif
	}
	
	W25NXX_Wait_Busy();   				        //等待芯片擦除结束
	W25NXX_Write_SR(0xA0,0x7C);                 // 开flash保护功能
}
void W25NXX_EraseFlash(unsigned short block_num)           // 
{
	uint16_t block = 0;
	if(Flash_ID == 0x0BE30B)  //芯天下 4G 1 Block = 128 Page
	{
		block_num = 2048;
	}
	printf("Block_num:%d \r\n",block_num);
	for(block = 0 ;block < block_num ; block++)
	{
		W25NXX_Erase_Block(block);
	}
	printf("擦出结束\r\n");
}
//-----------------------------------------------------------------------


/**********************页模式**********************/

//单页写函数
//SPI在一页(0~65535)内写入不大于2048个字节的数据
//在指定地址开始写入最大2048字节的数据
//pBuffer:数据存储区
//ColumnAddr:开始写入的页内地址(0-2047)
//PageAddr:页地址(0-65535)
//NumByteToWrite:要写入的字节数(最大2048),该数不应该超过该页的剩余字节数!!!
void W25NXX_WritePageAddr_Data(uint8_t* pBuffer,uint16_t ColumnAddr,uint32_t PageAddr,uint16_t NumByteToWrite)   //不能写入超过2048 Byte（一页2048个Byte，后面的64 Byte非数据位）
{
 	uint16_t i;
//	每64页切换一次Plane
	if((PageAddr & 0x40) == 0x40)
	{
		ColumnAddr |=0x1000;
	}
 	
	W25NXX_Write_SR(0xA0, 0x00);               // 关flash保护功能
	
	//------写BUFF------
  W25NXX_Write_Enable();                  	//SET WEL 
	if(Bypass_Mode == 0)
	{
    nSS_Active();                               //使能器件   
    SPI_Master_FIFO_Data_Put(0x02);      	    //发送写BUFF命令   
    SPI_Master_FIFO_Data_Put(ColumnAddr>>8); 	//BUFF地址   
    SPI_Master_FIFO_Data_Put(ColumnAddr);
	
		for(i=0;i<NumByteToWrite;i++)SPI_Master_FIFO_Data_Put(pBuffer[i]);//循环写数 
		nSS_Inactive();                             //取消片选 
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x02);      	    //发送写BUFF命令   
    	spi4_read_write_byte(ColumnAddr>>8); 	//BUFF地址   
    	spi4_read_write_byte(ColumnAddr);
		for(i=0;i<NumByteToWrite;i++)spi4_read_write_byte(pBuffer[i]);//循环写数 
		SS_SET;
		#endif
	}
	W25NXX_Wait_Busy();					    	//等待写入结束
	
	//-------BUFF数据写入flash--------
	W25NXX_Write_Enable();
	if(Bypass_Mode == 0)
	{
		nSS_Active();                          	    //使能器件   
		SPI_Master_FIFO_Data_Put(0x10);      	    //发送写页命令   
		SPI_Master_FIFO_Data_Put(PageAddr>>16); 	//发送16it地址    
		SPI_Master_FIFO_Data_Put(PageAddr>>8); 		//发送16it地址    
		SPI_Master_FIFO_Data_Put(PageAddr);   
		nSS_Inactive();   
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x10);      	    //发送写页命令   
		spi4_read_write_byte(PageAddr>>16); 	//发送16it地址    
		spi4_read_write_byte(PageAddr>>8); 		//发送16it地址    
		spi4_read_write_byte(PageAddr);   
		SS_SET;
		#endif
	}
	W25NXX_Wait_Busy();					        //等待写入结束 
	
	W25NXX_Write_SR(0xA0, 0x7c);                // 开flash保护功能
	 
} 

//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界! 
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(32bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void W25NXX_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   //函数内部自动将32位地址转换为对应的页地址和页内地址
{             
	uint16_t WriteNum = 0;         //写入每一页的数据量
	uint16_t ColumnAddr;           //页内地址
	uint32_t PageAddr;             //页地址
	
//判断第一次写入时，对应的页内地址，页地址，写入的数据量。
	ColumnAddr = (WriteAddr%2048);
	PageAddr = WriteAddr/2048;
	
	if(NumByteToWrite<(2048-ColumnAddr)) WriteNum = NumByteToWrite;         
	else WriteNum = 2048-ColumnAddr;
	
	while(1)
	{
		W25NXX_WritePageAddr_Data(pBuffer,ColumnAddr,PageAddr,WriteNum);
		if(NumByteToWrite == WriteNum)break;    //如果已经写入的数据达到要求，则写入完毕，退出。因为NumByteToWrite是不断减少的，当减到最后一次的时候，要写入的数据必定等于剩余的数据。
		
		pBuffer += WriteNum;                    //数组头跟着移动
		ColumnAddr = 0;           				//下一次写入操作必定从页内0地址写起。		
		PageAddr++;								//下一次写入操作必定从新页开始写。
		NumByteToWrite -= WriteNum;				//总写入数据量根据已经写入的量进行减少。
		
		if(NumByteToWrite<2048)                //下一次不需要写满一页
		{
			 WriteNum =NumByteToWrite;
		}
		else WriteNum = 2048;                  //下一次要写满一页


	}
} 

void W25NXX_ReadPageAddr_Data(uint8_t* pBuffer,uint16_t ColumnAddr,uint32_t PageAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i; 
 
//	每64页切换一次Plane
	if((PageAddr & 0x40) == 0x40)
	{
		ColumnAddr |=0x1000;
	}
 
	
	W25NXX_Write_SR(0xB0,W25NXX_ReadSR(0xB0)|0x18);
	
  //------从flash读数据到BUFF-----	
	if(Bypass_Mode == 0)
	{
		nSS_Active();                            //使能器件   
		SPI_Master_FIFO_Data_Put(0x13);
		SPI_Master_FIFO_Data_Put(PageAddr>>16);             
    SPI_Master_FIFO_Data_Put(PageAddr>>8);     //发送16bit页地址，将对应页的数据提取到BUFF中     
    SPI_Master_FIFO_Data_Put(PageAddr);   
		nSS_Inactive(); 	
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x13);
		spi4_read_write_byte(PageAddr>>16);             
    	spi4_read_write_byte(PageAddr>>8);     //发送16bit页地址，将对应页的数据提取到BUFF中     
    	spi4_read_write_byte(PageAddr);    
		SS_SET;
		#endif
	}
	W25NXX_Wait_Busy();	

	//------再从BUFF读数据到单片机内-----
	if(Bypass_Mode == 0)
	{
		nSS_Active();   	
		SPI_Master_FIFO_Data_Put(0x03); 
		SPI_Master_FIFO_Data_Put(ColumnAddr>>8);  
		SPI_Master_FIFO_Data_Put(ColumnAddr); 	
		SPI_Master_FIFO_Data_Put(0x00);
		
		for(i=0;i<NumByteToRead;i++)
		{ 
			pBuffer[i]= SPI_Master_FIFO_Data_Put(0xFF);   	//循环读数  
		}

		 nSS_Inactive(); //---------------------	
	}
	else
	{
		#if STM32_SPI
		SS_RESET;
		spi4_read_write_byte(0x03); 
		spi4_read_write_byte(ColumnAddr>>8);  
		spi4_read_write_byte(ColumnAddr); 	
		spi4_read_write_byte(0x00);
		
		for(i=0;i<NumByteToRead;i++)
		{ 
			pBuffer[i]= spi4_read_write_byte(0xFF);   	//循环读数  
		}
		SS_SET;
		#endif
	}
	W25NXX_Wait_Busy();

}
//无检验读SPI FLASH 
//具有自动换页功能 
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(32bit)
//NumByteToRead:要读取的字节数(最大65535)
//CHECK OK
void W25NXX_Read_NoCheck(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   //函数内部自动将32位地址转换为对应的页地址和页内地址
{             
	uint16_t ReadNum = 0;         //读取每一页的数据量
	uint16_t ColumnAddr;          //业内地址
	uint32_t PageAddr;
	
//判断第一次读取时，对应的页内地址，页地址，写入的数据量。
	ColumnAddr = (ReadAddr%2048);
	PageAddr = ReadAddr/2048;
	
	if(NumByteToRead<(2048-ColumnAddr)) ReadNum = NumByteToRead;         
	else ReadNum = 2048-ColumnAddr;
	
	while(1)
	{
		W25NXX_ReadPageAddr_Data(pBuffer,ColumnAddr,PageAddr,ReadNum);
		if(NumByteToRead == ReadNum)break;    //如果已经写入的数据达到要求，则写入完毕，退出。因为NumByteToWrite是不断减少的，当减到最后一次的时候，要写入的数据必定等于剩余的数据。
		
		pBuffer += ReadNum;                     //数组头跟着移动
		ColumnAddr = 0;           				//下一次读取操作必定从页内0地址写起。		
		PageAddr++;								//下一次读取操作必定从新页开始写。
		NumByteToRead -= ReadNum;				//总读取数据量根据已经读取的量进行减少。
		
		if(NumByteToRead<2048)                //下一次不需要写满一页
		{
			 ReadNum =NumByteToRead;
		}
		else ReadNum = 2048;                  //下一次要写满一页


	}
} 
/*********************************************************************W25NXX*****************************************************************************************/



/*********************************************************************SST26VF032B************************************************************************************/

uint8_t SST26VF_ReadSR(void)   
{  
	uint8_t byte=0;   
	nSS_Active();                            		  
	SPI_Master_FIFO_Data_Put(0x05); 	   
	byte=SPI_Master_FIFO_Data_Put(0xFF);           
	nSS_Inactive();                              
	return byte;   
} 

void SST26VF_Write_Enable(void)   
{
	nSS_Active();                          		  
	SPI_Master_FIFO_Data_Put(0x06);  
	nSS_Inactive();                               	      
} 

void SST26VF_Write_Disable(void)   
{  
	nSS_Active();                            		   
	SPI_Master_FIFO_Data_Put(0x04);    
	nSS_Inactive();                               	      
} 	


void SST26VF_Wait_Busy(void)
{
	while((SST26VF_ReadSR()&0x01) == 0x01);
}

uint32_t SST26VF_ReadID(void)
{
	uint32_t Temp = 0;	  
	nSS_Active();				    
	SPI_Master_FIFO_Data_Put(0x9F); 
	Temp|=SPI_Master_FIFO_Data_Put(0xFF)<<16;	
	Temp|=SPI_Master_FIFO_Data_Put(0xFF)<<8;  
	Temp|=SPI_Master_FIFO_Data_Put(0xFF);	 
	nSS_Inactive();				    
	return Temp;
}

void SST26VF_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
	uint16_t i;   										    
	nSS_Active();                            			  
	SPI_Master_FIFO_Data_Put(0x0B);   //High Speed read

	SPI_Master_FIFO_Data_Put((uint8_t)((ReadAddr)>>16));  	   
	SPI_Master_FIFO_Data_Put((uint8_t)((ReadAddr)>>8));   
	SPI_Master_FIFO_Data_Put((uint8_t)ReadAddr);   
	SPI_Master_FIFO_Data_Put(0xFF);   //dummy
	for(i=0;i<NumByteToRead;i++)
	{ 
		pBuffer[i]=SPI_Master_FIFO_Data_Put(0XFF);    
	}
	nSS_Inactive();  				    	      
} 


void SST26VF_Write_Page(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
 	uint16_t i;  
	SST26VF_Write_Enable();                  				 
	nSS_Active();                            						  
	SPI_Master_FIFO_Data_Put(0x02);  


	SPI_Master_FIFO_Data_Put((uint8_t)((WriteAddr)>>16)); 				   
	SPI_Master_FIFO_Data_Put((uint8_t)((WriteAddr)>>8));   
	SPI_Master_FIFO_Data_Put((uint8_t)WriteAddr);  	      
	
	for(i=0;i<NumByteToWrite;i++)
	{
		SPI_Master_FIFO_Data_Put(pBuffer[i]);
	} 
	nSS_Inactive();                            				
	SST26VF_Wait_Busy();				   			
}

void SST26VF_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)   
{ 
	uint16_t pageremain;	   
	pageremain=256-WriteAddr%256; 		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;
	while(1)
	{	   
		SST26VF_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			 
			if(NumByteToWrite>256)pageremain=256;
			else pageremain=NumByteToWrite; 	  
		}
	}
} 

void SST26VF_Global_Block_Protection_Unlock(void)
{
	SST26VF_Write_Enable();                  				 
	nSS_Active();                            						  
	SPI_Master_FIFO_Data_Put(0x98);	
	nSS_Active(); 
	SST26VF_Write_Disable();
}

void SST26VF_Global_Block_Protection_Lock(void)
{
	SST26VF_Write_Enable();
	nSS_Active();
	
	SPI_Master_FIFO_Data_Put(0x42); 	

	SPI_Master_FIFO_Data_Put(0X55);    
	SPI_Master_FIFO_Data_Put(0X55);
	
	SPI_Master_FIFO_Data_Put(0XFF);    
	SPI_Master_FIFO_Data_Put(0XFF);
	SPI_Master_FIFO_Data_Put(0XFF);    
	SPI_Master_FIFO_Data_Put(0XFF);
	SPI_Master_FIFO_Data_Put(0XFF);    
	SPI_Master_FIFO_Data_Put(0XFF);
	SPI_Master_FIFO_Data_Put(0XFF);    
	SPI_Master_FIFO_Data_Put(0XFF);	
	nSS_Inactive();	
	
	SST26VF_Write_Disable();
}


void SST26VF_Erase_Chip(void)   
{                                   
	SST26VF_Write_Enable();                	 	 
	SST26VF_Wait_Busy();
	nSS_Active();                            	    
	SPI_Master_FIFO_Data_Put(0xC7);   
	nSS_Inactive();                               
	SST26VF_Wait_Busy();   				   		       
} 

void SST26VF_Erase_Sector4K(uint32_t Dst_Addr)   
{   
	Dst_Addr*=4096;
	SST26VF_Write_Enable();                   
	SST26VF_Wait_Busy();   
	nSS_Active();                            	  
	SPI_Master_FIFO_Data_Put(0x20);  
	
	SPI_Master_FIFO_Data_Put((uint8_t)((Dst_Addr)>>16));     
	SPI_Master_FIFO_Data_Put((uint8_t)((Dst_Addr)>>8));   
	SPI_Master_FIFO_Data_Put((uint8_t)Dst_Addr);  
	nSS_Inactive();                            	     	      
	SST26VF_Wait_Busy();   				   		
}  

void SST26VF_EraseBlock64KB(uint32_t addr)
{
	addr*=(64*1024);
	SST26VF_Write_Enable();                 	//SET WEL
	SST26VF_Wait_Busy();
	nSS_Active();                            	  	//使能器件
	SPI_Master_FIFO_Data_Put(0xD8);   			//发送片擦除命令
	
	SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>16)); 			//发送24bit地址
	SPI_Master_FIFO_Data_Put((uint8_t)((addr)>>8));
	SPI_Master_FIFO_Data_Put((uint8_t)addr);
	nSS_Inactive();                           	//取消片选
	SST26VF_Wait_Busy();   				   	//等待芯片擦除结束
}


/*********************************************************************SST26VF032B************************************************************************************/


