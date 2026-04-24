void HX8399_Initial_Code(void)

{
	//************* Reset LCD Driver ****************//
	LCD_nRESET = 1;
	delayms(5); // Delay 5ms // This delay time is necessary
	LCD_nRESET = 0;
	delayms(10); // Delay 10ms // This delay time is necessary
	LCD_nRESET = 1;
	delayms(120); // Delay 120 ms
	//************* Start Initial Sequence **********//

	HX8399_CMD(0xB9);
	HX8399_INDEX(0xFF);
	HX8399_INDEX(0x83);
	HX8399_INDEX(0x99);


	HX8399_CMD(0x11); //Sleep-out
	Delayms(120);

	HX8399_CMD(0x29); //Dispaly on
	Delayms(120);

}



void HX8399_EnterDeepSleep_Code(void)
{
	
	HX8399_CMD(0x10); //Sleep-In
	Delayms(50);
	HX8399_CMD(0xB9);
	HX8399_INDEX(0xFF);
	HX8399_INDEX(0x83);
	HX8399_INDEX(0x99);
	HX8399_CMD(0xB1);
	HX8399_INDEX(0x01);
	Delayms(120);	
}


void HX8399_ExitDeepSleep_Code(void)
{
	HX8399_CMD(0xB9);
	HX8399_INDEX(0xFF);
	HX8399_INDEX(0x83);
	HX8399_INDEX(0x99);
	HX8399_CMD(0xB1);
	HX8399_INDEX(0x00);
	Delayms(120);
}


