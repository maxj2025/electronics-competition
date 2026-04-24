void EK790007_Initial_Code(void)

{
	//************* Reset LCD Driver ****************//
	LCD_nRESET = 1;
	delayms(5); // Delay 5ms // This delay time is necessary
	LCD_nRESET = 0;
	delayms(10); // Delay 10ms // This delay time is necessary
	LCD_nRESET = 1;
	delayms(120); // Delay 120 ms
	//************* Start Initial Sequence **********//
	
	EK790007_CMD(0x80);
	EK790007_INDEX(0x8B);
	
	EK790007_CMD(0x81);
	EK790007_INDEX(0x78);

	EK790007_CMD(0x82);
	EK790007_INDEX(0x84);

	EK790007_CMD(0x83);
	EK790007_INDEX(0x88);

	EK790007_CMD(0x84);
	EK790007_INDEX(0xA8);	

	EK790007_CMD(0x85);
	EK790007_INDEX(0xE3);
	
	EK790007_CMD(0x86);
	EK790007_INDEX(0x88);
	
	EK790007_CMD(0xB1);
	EK790007_INDEX(0x04);
	
	//EK790007_CMD(0xB2);
	//EK790007_INDEX(0x10);//2 lane MIPI 	
}