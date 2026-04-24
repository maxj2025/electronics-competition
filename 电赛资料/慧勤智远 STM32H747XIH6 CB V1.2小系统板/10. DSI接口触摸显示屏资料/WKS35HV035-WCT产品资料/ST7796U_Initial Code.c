void ST7796U_Initial_Code(void)
{
	LCD_RESET = 1;
	delayms(1); 
	LCD_RESET = 0;
	delayms(10); 
	LCD_RESET = 1;
	delayms(200); 
	
	ST7796U_Command(0x11);     
	delayms(200);               

	ST7796U_Command(0xF0);     
	ST7796U_Data(0xC3);   

	ST7796U_Command(0xF0);     
	ST7796U_Data(0x96);   
	
	ST7796U_Command(0x36);   
	ST7796U_Data(0x48);   

	ST7796U_Command(0x3A);     
	ST7796U_Data(0x55); 

	ST7796U_Command(0xB4);     
	ST7796U_Data(0x01);   

	ST7796U_Command(0xB7);     
	ST7796U_Data(0xC6);   

	ST7796U_Command(0xB9);     
	ST7796U_Data(0x02);   
	ST7796U_Data(0xE0);   

	ST7796U_Command(0xC0);     
	ST7796U_Data(0x80);   
	ST7796U_Data(0x16);   

	ST7796U_Command(0xC1);     
	ST7796U_Data(0x19);   

	ST7796U_Command(0xC2);     
	ST7796U_Data(0xA7);   

	ST7796U_Command(0xC5);     
	ST7796U_Data(0x16);   

	ST7796U_Command(0xE8);     
	ST7796U_Data(0x40);   
	ST7796U_Data(0x8A);   
	ST7796U_Data(0x00);   
	ST7796U_Data(0x00);   
	ST7796U_Data(0x29);   
	ST7796U_Data(0x19);   
	ST7796U_Data(0xA5);   
	ST7796U_Data(0x33);   

	ST7796U_Command(0xE0);     
	ST7796U_Data(0xF0);   
	ST7796U_Data(0x07);   
	ST7796U_Data(0x0D);   
	ST7796U_Data(0x04);   
	ST7796U_Data(0x05);   
	ST7796U_Data(0x14);   
	ST7796U_Data(0x36);   
	ST7796U_Data(0x54);   
	ST7796U_Data(0x4C);   
	ST7796U_Data(0x38);   
	ST7796U_Data(0x13);   
	ST7796U_Data(0x14);   
	ST7796U_Data(0x2E);   
	ST7796U_Data(0x34);   

	ST7796U_Command(0xE1);     
	ST7796U_Data(0xF0);   
	ST7796U_Data(0x10);   
	ST7796U_Data(0x14);   
	ST7796U_Data(0x0E);   
	ST7796U_Data(0x0C);   
	ST7796U_Data(0x08);   
	ST7796U_Data(0x35);   
	ST7796U_Data(0x44);   
	ST7796U_Data(0x4C);   
	ST7796U_Data(0x26);   
	ST7796U_Data(0x10);   
	ST7796U_Data(0x12);   
	ST7796U_Data(0x2C);   
	ST7796U_Data(0x32); 

	ST7796U_Command(0xF0);     
	ST7796U_Data(0x3C);   

	ST7796U_Command(0xF0);     
	ST7796U_Data(0x69);   

	delayms(200);                

	ST7796U_Command(0x21);     

	ST7796U_Command(0x29);  
	
	ST7796U_Command(0x2C);
}






