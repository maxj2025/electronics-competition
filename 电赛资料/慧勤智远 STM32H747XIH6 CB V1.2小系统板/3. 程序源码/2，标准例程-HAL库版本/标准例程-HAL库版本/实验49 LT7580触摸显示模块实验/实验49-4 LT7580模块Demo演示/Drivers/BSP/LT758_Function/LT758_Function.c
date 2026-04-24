
#include "./BSP/LT758_Function/LT758_Function.h"

uint16_t X_Pos;
uint16_t Y_Pos;

#define FirstRow_X1 52
#define FirstRow_Y1 108
#define FirstRow_X2 466
#define FirstRow_Y2 151

#define SecondRow_X1 555
#define SecondRow_Y1 108
#define SecondRow_X2 969
#define SecondRow_Y2 151

#define USE_SPI_FLASH_CS0 0

uint8_t Point = 0;
uint8_t enter_flag = 0;
uint8_t enter_Point = 0;

const extern unsigned char gImage_pen_il[256];
const extern unsigned char gImage_arrow_il[256];
const extern unsigned char gImage_busy_im[256];
const extern unsigned char gImage_no_im[256];

uint16_t X_Output_Max = LCD_XSIZE_TFT;
uint16_t Y_Output_Max = LCD_YSIZE_TFT;
//uint8_t gt911flag = 0;
//uint8_t ft5216flag = 0;
uint8_t pcx_iic_read[20];
uint8_t touch_state = 0, press = 0;
uint8_t ctp_active_index = 0;
unsigned long H_JPG_Addr[] = {0x00000010,0x0004ab98,0x00095720,0x000fa964,0x0013a19c,0x0015ab28,0x00160004,0x0016559c,0x0016a970,0x0016ff44,0x0017532c,0x0017a7e0,0x0017b308,0x0017c0a0,0x0017ce90,0x0017e180,0x0017f510,0x001805ec,0x00181a88,0x00182a40,0x00183cb8,0x00185004,0x001860d0,0x00187428,0x00188638,0x00189d5c,0x0018af98,0x0018c534,0x0018db30,0x0018edf0,0x0018f340,0x00191bdc,0x001963bc,0x0019b3a0,0x0019f650,0x001a347c,0x001a71d4,0x001ab1c8,0x001af7c8,0x001b389c,0x001b6c54,0x001b9850,0x001bcb00,0x001bf15c,0x001c41cc,0x001c8604,0x001cbf14,0x001cea6c,0x001d38f0,0x001de944,0x001f4030,0x00219ec4,0x00236c5c,0x002558ec,0x002727a0,0x00291bc8,0x002a959c,0x002d2350,0x0031534c,0x0034b60c,0x00382cec,0x003e7948,0x0045476c,0x004a498c,0x004e228c,0x0053b474,0x0053e2b8,0x00541178,0x00543ea4,0x00546cb0,0x00549aec,0x0054c984,0x0054f798,0x0055267c,0x005554f4,0x005582dc,0x0055b0f8,0x0055df50,0x00560e0c};
  
unsigned long GIF_JPG_Addr[] = {0x00000010,0x00005fc8,0x0000bde4,0x00011d18,0x00017e68,0x0001e4b0,0x00024ca0,0x0002b5e4,0x00031e8c,0x00038888,0x0003efd0,0x000455c4,0x0004b92c,0x00051c90,0x00057da0,0x0005df44,0x00064334,0x0006a9c0,0x00070f6c,0x000774a4,0x0007d8f0,0x00083dcc,0x0008a134,0x00090414,0x0009675c,0x0009cd20,0x000a323c,0x000a9938,0x000b0050,0x000b67d4,0x000bcc8c,0x000c31b4,0x000c96dc,0x000cfce8,0x000d6198,0x000dc6c4,0x000e2ab4,0x000e8f90,0x000ef27c,0x000f56fc,0x000fbb2c,0x001020ec,0x00108528,0x0010e994,0x00114d48,0x0011b188,0x001213ec,0x001276a8,0x0012d8b4,0x00133c00,0x00139dc0,0x00140024,0x00146200,0x0014c4f8,0x00152678,0x0015889c,0x0015ea88,0x00164d50,0x0016aee0,0x0017107c,0x001771d0,0x0017d430,0x0018357c,0x001896dc,0x0018f8bc,0x00195bc8,0x0019bdac,0x001a1ffc,0x001a8178,0x001ae394,0x001b4604,0x001baa9c,0x001c10c0,0x001c7bd0,0x001ce784,0x001d54c0,0x001dc1ec,0x001e2f98,0x001e9bbc,0x001f0860,0x001f7124,0x001fd878,0x00203bac,0x00209e20,0x00210054,0x0021649c,0x0021c794,0x00222a90,0x00228c7c,0x0022f028,0x00235224,0x0023b510,0x002417b8,0x00247bac,0x0024de64,0x0025416c,0x0025a3cc,0x002607c4,0x00266a98,0x0026ce1c,0x00273150,0x0027967c,0x0027fa54,0x00285ec4,0x0028c310,0x002928b8,0x00298d54,0x0029f26c,0x002a5724,0x002abd44,0x002b21d4,0x002b86e0,0x002beb54,0x002c5168,0x002cb618,0x002d1afc,0x002d7f44,0x002de51c,0x002e49c4,0x002eaefc};
  
unsigned long KJ_JPG_Addr[] =	{0x00000010,0x0003cec4,0x0007a120,0x000b74d3,0x000f4fd1,0x00132755,0x0017056c,0x001ae6d4,0x001ed624,0x0022ca6d,0x0026bdd0,0x002abc43,0x002eb3af,0x0032ac80,0x0036b13a,0x003ab3bf,0x003ebac8,0x0042ceb3,0x0046e340,0x004af8bf,0x004f3c65,0x00537dfa,0x0057c476,0x005c0cb7,0x00605b2e,0x0064a709,0x0068f610,0x006d5086,0x0071a982,0x0075fc47,0x007a553e,0x007ea7d7,0x0082ff97,0x00875fa4,0x008bbe7a,0x00901d33,0x00947938,0x0098d17d,0x009d24c9,0x00a1735d,0x00a5c20b,0x00aa115f,0x00ae6305,0x00b2ac37,0x00b6f738,0x00bb42c1,0x00bf8c06,0x00c3cefb,0x00c80ac6,0x00cc3ec6,0x00d06742,0x00d49412,0x00d8c314,0x00dcef78};

uint8_t First_press = 0;
uint16_t First_pressX, First_pressY;

uint16_t tpflag = 0;
uint16_t tpCOUNT = 0;
uint16_t tptime = 0;
uint16_t two_click;

#define slide_lenght 50

typedef struct //B4
{
	uint8_t dir; //0:x  1:y
	uint8_t first;
	uint8_t mode; //0:正常模式  1:循环模式
	uint8_t num;
	uint8_t numx;
	uint8_t numy;
	uint8_t sum;
	uint16_t pic[10]; //存储用于滑动的图片编号
	int16_t x;
	int16_t y;
	int16_t dx;
	int16_t dy;
	uint16_t lastX;
	uint16_t lastY;
	uint16_t count;
	int16_t buf[5];
	uint8_t pos;
} Gesture_Info;

int f9_i, f9_j;
int KK; //用于保存df的数据
int dfx, dfy;

Gesture_Info gGesXY;
uint8_t gFirstTouch = 0;
//uint8_t gOpenGesture = 0;
uint8_t penflag = 0;
int16_t Slide_xw, Slide_yw;
uint8_t Y_slide_flag = 0;
uint8_t X_slide_flag = 0;

#define JPG_SIZE 0x005890E8

#define Font_24_Size 0x00092D00
#define Font_32_Size 0x00105000
#define Font_24_Addr 0x0087A508  //modify

#define Font_32_Addr Font_24_Addr+Font_24_Size
#define Font_72_Addr Font_32_Addr+Font_32_Size

const unsigned char gImage_pen_il[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X96,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X91,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XA4,0X15,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,
0XA9,0X01,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X46,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X6A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XA4,0X05,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X01,0X46,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X6A,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA4,0X05,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X01,0X46,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X40,0X51,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X14,0X69,0XAA,0XAA,
0XAA,0XAA,0XAA,0XA4,0X01,0X14,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X44,0X1A,0XAA,
0XAA,0XAA,0XAA,0XAA,0X40,0X11,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X04,0X41,0XAA,
0XAA,0XAA,0XAA,0XAA,0XA4,0X01,0X10,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X44,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X11,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X04,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X01,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XA9,0X00,0X1A,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

const unsigned char gImage_arrow_il[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X46,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X41,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X6A,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X00,0X1A,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X06,0XAA,0XAA,0XAA,0XAA,
0X40,0X00,0X00,0X01,0XAA,0XAA,0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XAA,
0X40,0X00,0X15,0X55,0X5A,0XAA,0XAA,0XAA,0X40,0X10,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,
0X40,0X64,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0X41,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,
0X46,0XA9,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,0X5A,0XA9,0X01,0XAA,0XAA,0XAA,0XAA,0XAA,
0X6A,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X40,0X6A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0X90,0X1A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X90,0X1A,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XA4,0X06,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA9,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

const unsigned char gImage_busy_im[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,
0XAA,0XAA,0X54,0X00,0X00,0X05,0X6A,0XAA,0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,
0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X44,0X44,0X45,0XAA,0XAA,0XAA,0XAA,0X94,0X11,0X11,0X05,0XAA,0XAA,
0XAA,0XAA,0X95,0X04,0X44,0X15,0XAA,0XAA,0XAA,0XAA,0XA5,0X41,0X10,0X56,0XAA,0XAA,
0XAA,0XAA,0XA9,0X50,0X41,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0X54,0X05,0X6A,0XAA,0XAA,
0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X94,0X45,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X94,0X05,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0X50,0X01,0X6A,0XAA,0XAA,0XAA,0XAA,0XA9,0X40,0X40,0X5A,0XAA,0XAA,
0XAA,0XAA,0XA5,0X00,0X10,0X16,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X04,0X44,0X05,0XAA,0XAA,0XAA,0XAA,0X94,0X11,0X11,0X05,0XAA,0XAA,
0XAA,0XAA,0X94,0X44,0X44,0X45,0XAA,0XAA,0XAA,0XAA,0X95,0X11,0X11,0X15,0XAA,0XAA,
0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,0XAA,0XAA,0X54,0X00,0X00,0X05,0X6A,0XAA,
0XAA,0XAA,0X55,0X55,0X55,0X55,0X6A,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

const unsigned char gImage_no_im[256] = { /* 0X00,0X02,0X20,0X00,0X20,0X00, */
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0X55,0XAA,0XAA,0XAA,
0XAA,0XAA,0XA9,0X40,0X00,0X5A,0XAA,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XA9,0X00,0X15,0X55,0X00,0X1A,0XAA,
0XAA,0XA4,0X00,0X6A,0XAA,0X50,0X06,0XAA,0XAA,0XA4,0X00,0X6A,0XAA,0XA4,0X06,0XAA,
0XAA,0X90,0X00,0X1A,0XAA,0XA9,0X01,0XAA,0XAA,0X90,0X10,0X06,0XAA,0XA9,0X01,0XAA,
0XAA,0X40,0X64,0X01,0XAA,0XAA,0X40,0X6A,0XAA,0X40,0X69,0X00,0X6A,0XAA,0X40,0X6A,
0XAA,0X40,0X6A,0X40,0X1A,0XAA,0X40,0X6A,0XAA,0X40,0X6A,0X90,0X06,0XAA,0X40,0X6A,
0XAA,0X40,0X6A,0XA4,0X01,0XAA,0X40,0X6A,0XAA,0X40,0X6A,0XA9,0X00,0X6A,0X40,0X6A,
0XAA,0X40,0X6A,0XAA,0X40,0X1A,0X40,0X6A,0XAA,0X90,0X1A,0XAA,0X90,0X05,0X01,0XAA,
0XAA,0X90,0X1A,0XAA,0XA4,0X00,0X01,0XAA,0XAA,0XA4,0X06,0XAA,0XA9,0X00,0X06,0XAA,
0XAA,0XA4,0X01,0X6A,0XAA,0X40,0X06,0XAA,0XAA,0XA9,0X00,0X15,0X55,0X00,0X1A,0XAA,
0XAA,0XAA,0X40,0X00,0X00,0X00,0X6A,0XAA,0XAA,0XAA,0X94,0X00,0X00,0X05,0XAA,0XAA,
0XAA,0XAA,0XA9,0X40,0X00,0X5A,0XAA,0XAA,0XAA,0XAA,0XAA,0X95,0X55,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,0XAA,
};

void Cs_Select(void)
{
	Enable_SFlash_SPI();

	Select_SFI_1();//选择SPI CS1    Select SPI CS1 for double NorFlash board
	Select_nSS_drive_on_xnsfcs1();//选择CS1为758的片选引脚        
	
	csc = 1; 
	clk = 6;

	Flash_ID = W25QXX_ReadID();
	
	if(Flash_ID>=0xEF13 && Flash_ID<=0xEF18)
	{
		Flash_Type = 0;//norflash
		if(Flash_ID == 0xEF18)
		{
			w25q256 = 1;//256norflash
			LT758_W25QXX_Enter_4Byte_AddressMode();//4bytes地址寻址
			Select_SFI_32bit_Address();
		}
	}
	else
	{ 
		Flash_ID = W25NXX_ReadID();
		if((Flash_ID >= 0xEFAA21 && Flash_ID <= 0xEFAA23)  || Flash_ID == 0x0B1100 || (Flash_ID == 0x2C242C) || (Flash_ID == 0x0BE30B) || (Flash_ID == 0xc891c8)) 
		{
			write_758_reg(0xb7,read_758_reg(0xb7) | 0x40);
			Flash_Type = 1;//nandflash
		}	
	}

	printf("Flash_cs1_ID:%x\r\n",Flash_ID);

	Select_SFI_0();//选择SPI CS0    Select SPI CS1 for double NorFlash board
	Select_nSS_drive_on_xnsfcs0();//选择CS0为758的片选引脚       
	
	csc = 0;
	clk = 6;

	Flash_ID = W25QXX_ReadID();
	
	if(Flash_ID>=0xEF13 && Flash_ID<=0xEF18)
	{
		Flash_Type = 0;//norflash
		if(Flash_ID == 0xEF18)
		{
			w25q256 = 1;//256norflash
			LT758_W25QXX_Enter_4Byte_AddressMode();//4bytes地址寻址
			Select_SFI_32bit_Address();
		}
	}
	else
	{ 
		Flash_ID = W25NXX_ReadID();
		if((Flash_ID >= 0xEFAA21 && Flash_ID <= 0xEFAA23)  || Flash_ID == 0x0B1100 || (Flash_ID == 0x2C242C) || (Flash_ID == 0x0BE30B) || (Flash_ID == 0xc891c8)) 
		{
			write_758_reg(0xb7,read_758_reg(0xb7) | 0x40);
			Flash_Type = 1;//nandflash
		}	
	}

	printf("Flash_cs0_ID:%x\r\n",Flash_ID);
}


void CT_DELAY_US(int val)
{
	delay_us(val * 3);
}

/*IO口模拟时序的方法，i2cdrv里面的是配置寄存器的方法*/
/*****************************I2C启动函数***************************/
void I2C_start(void) // SCL保持高电平的时候，SDA由高变低
{
}

/*****************************CTI2C停止函数**************************/
void I2C_stop(void) // SCL保持高电平的时候，SDA由低变高的话就是结束信号
{
}

/********************CTI2C写1byte函数,S为需要写的内容*****************/
/* 主机发送一个起始信号S→发送从机7位地址和1位方向，方向位表示写→主机释放SDA线方便从机给回应→
有从机匹配到地址，拉低SDA线作为ACK→主机重新获得SDAk开始传输8位数据→主机释放SDA线方便从机给回应→
从机收到数据拉低SDA线作为ACK告诉主机数据接收成功→主机发出停止信号。*/

uint8_t I2C_write_byte(const uint8_t s) // 通信过程写地址或写数据需要用到这个函数
{
  return 0;
}

///****************CTI2C读1byte函数，返回值为所读数据*****************/
/*读过程： 主机发送一个起始信号S→发送从机7位地址和1位方向，方向位表示读→
主机释放SDA线方便从机给回应→有从机匹配到地址，拉低SDA线作为ACK→
从机继续占用SDA线，用SDA传输8位数据给主机→从机释放SDA线(拉高)方便主机给回应→
主机接收到数据→主机获得SDA线控制并拉低SDA线作为ACK告诉从机数据接收成功→主机发出停止信号。*/

uint8_t I2C_read_byte(uint8_t ack) // CTI2C读1byte函数，并返回读到的数据
{
  uint8_t ch_data;
	return ch_data;
}

void TP_read_XY(void) // I2C read 流程：START+SLAVE ADDRESS+REGISTER ADDRESS+RESTART+SLAVE ADDRESS +DATA+STOP,坐标的数据通过iic读取
{
	uint16_t x, y;
	uint8_t i = 0;
	I2C_start();
	I2C_write_byte(0x28);
	I2C_write_byte(0x81);
	I2C_write_byte(0x4E);
	I2C_stop();

	I2C_start(); // 再次启动CTI2C总线
	I2C_write_byte(0x29);
	for (i = 0; i < 8; i++)
	{
		pcx_iic_read[i] = I2C_read_byte(0);
	}
	pcx_iic_read[i] = I2C_read_byte(1);
	I2C_stop();

	if (pcx_iic_read[0] & 0x80) // buffer status 为1时才是数据有效
	{
		x = pcx_iic_read[3];
		x = x << 8 | pcx_iic_read[2];

		y = pcx_iic_read[5];
		y = y << 8 | pcx_iic_read[4];

		if (LCD_XSIZE_TFT != X_Output_Max)
		{
			x = x * LCD_XSIZE_TFT / X_Output_Max;
			y = y * LCD_YSIZE_TFT / Y_Output_Max;
		}
		X_Pos = x;
		Y_Pos = y;
		touch_state = pcx_iic_read[0] & 0x0F;
		if (touch_state == 1)
		{
			if (First_press == 0)
			{
				First_press = 1;
				First_pressX = X_Pos;
				First_pressY = Y_Pos;
				tpCOUNT++;
				if(tpflag == 0)
				{
					tpflag=1;
				}
				if (tpCOUNT == 2&&tptime <= 200)
				{
					tpflag=0;
					tptime=0;
					tpCOUNT=0;
					two_click=1;
				}
				else if(tpCOUNT == 2&&tptime > 200)
				{
					tpflag=1;
					tptime=0;
					tpCOUNT=1;
				}
			}
		}
		else
		{
			First_press = 0;
		}
		// printf("sta=%d x=%d y=%d\r\n",touch_state,x,y);
		// printf("%d %d %d\r\n", touch_state, x, y);
		I2C_start();
		I2C_write_byte(0x28);
		I2C_write_byte(0x81);
		I2C_write_byte(0x4E);
		I2C_write_byte(0x00);
		I2C_stop();
	}
}

void TP_read_touch(void)
{
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
      Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
      
			if (First_press == 0)
			{
				First_press = 1;
				First_pressX = X_Pos;
				First_pressY = Y_Pos;
				tpCOUNT++;
				if (tpflag == 0)
				{
					tpflag=1;
				}
				if (tpCOUNT == 2&&tptime <= 200)
				{
					tpflag=0;
					tptime=0;
					tpCOUNT=0;
					two_click=1;
				}
				else if (tpCOUNT == 2&&tptime > 200)
				{
					tpflag=1;
					tptime=0;
					tpCOUNT=1;
				}        
      }        
    }
		else
		{
			First_press = 0;
		}
}

uint8_t Judge_Pos_touch(void)
{
  X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
  Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
  
	if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1) && (Y_Pos < FirstRow_Y2))
	{
		// printf("select 1\r\n");
		if (Point != 1)
		{
			Point = 1;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 108, H_JPG_Addr[30]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53) && (Y_Pos < FirstRow_Y2 + 53))
	{
		// printf("select 2\r\n");
		if (Point != 2)
		{
			Point = 2;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 162, H_JPG_Addr[31]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 2) && (Y_Pos < FirstRow_Y2 + 53 * 2))
	{
		// printf("select 3\r\n");
		if (Point != 3)
		{
			Point = 3;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 214, H_JPG_Addr[32]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 3) && (Y_Pos < FirstRow_Y2 + 53 * 3))
	{
		// printf("select 4\r\n");
		if (Point != 4)
		{
			Point = 4;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 268, H_JPG_Addr[33]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 4) && (Y_Pos < FirstRow_Y2 + 53 * 4))
	{
		// printf("select 5\r\n");
		if (Point != 5)
		{
			Point = 5;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 322, H_JPG_Addr[34]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 5) && (Y_Pos < FirstRow_Y2 + 53 * 5))
	{
		// printf("select 6\r\n");
		if (Point != 6)
		{
			Point = 6;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 374, H_JPG_Addr[35]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 6) && (Y_Pos < FirstRow_Y2 + 53 * 6))
	{
		// printf("select 7\r\n");
		if (Point != 7)
		{
			Point = 7;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 428, H_JPG_Addr[36]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 7) && (Y_Pos < FirstRow_Y2 + 53 * 7))
	{
		// printf("select 8\r\n");
		if (Point != 8)
		{
			Point = 8;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 481, H_JPG_Addr[37]);
		}
		return 1;
	}
	else if ((X_Pos > FirstRow_X1) && (X_Pos < 290) && (Y_Pos > FirstRow_Y1 + 53 * 8) && (Y_Pos < FirstRow_Y2 + 53 * 8))
	{
		// printf("select 9\r\n");
		if (Point != 9)
		{
			Point = 9;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 535, H_JPG_Addr[38]);
		}
		return 1;
	}
	else if ((X_Pos > 290) && (X_Pos < FirstRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 8) && (Y_Pos < FirstRow_Y2 + 53 * 8))
	{
		// printf("select 20\r\n");
		if (Point != 20)
		{
			Point = 20;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(FirstRow_X1, 535, H_JPG_Addr[38]);
		}
		return 1;
	}

	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1) && (Y_Pos < FirstRow_Y2))
	{
		// printf("select 10\r\n");
		if (Point != 10)
		{
			Point = 10;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 108, H_JPG_Addr[39]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53) && (Y_Pos < FirstRow_Y2 + 53))
	{
		// printf("select 11\r\n");
		if (Point != 11)
		{
			Point = 11;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 162, H_JPG_Addr[40]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 2) && (Y_Pos < FirstRow_Y2 + 53 * 2))
	{
		// printf("select 12\r\n");
		if (Point != 12)
		{
			Point = 12;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 214, H_JPG_Addr[41]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 3) && (Y_Pos < FirstRow_Y2 + 53 * 3))
	{
		// printf("select 13\r\n");
		if (Point != 13)
		{
			Point = 13;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 268, H_JPG_Addr[42]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 4) && (Y_Pos < FirstRow_Y2 + 53 * 4))
	{
		// printf("select 14\r\n");
		if (Point != 14)
		{
			Point = 14;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 322, H_JPG_Addr[43]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 5) && (Y_Pos < FirstRow_Y2 + 53 * 5))
	{
		// printf("select 15\r\n");
		if (Point != 15)
		{
			Point = 15;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 374, H_JPG_Addr[44]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 6) && (Y_Pos < FirstRow_Y2 + 53 * 6))
	{
		// printf("select 16\r\n");
		if (Point != 16)
		{
			Point = 16;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 428, H_JPG_Addr[45]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 7) && (Y_Pos < FirstRow_Y2 + 53 * 7))
	{
		// printf("select 17\r\n");
		if (Point != 17)
		{
			Point = 17;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 481, H_JPG_Addr[46]);
		}
		return 1;
	}
	else if ((X_Pos > SecondRow_X1) && (X_Pos < SecondRow_X2) && (Y_Pos > FirstRow_Y1 + 53 * 8) && (Y_Pos < FirstRow_Y2 + 53 * 8))
	{
		// printf("select 18\r\n");
		if (Point != 18)
		{
			Point = 18;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
			LT758_DMA_JPG(SecondRow_X1, 535, H_JPG_Addr[47]);
		}
		return 1;
	}
	else if ((X_Pos > 512) && (X_Pos < 1024) && (Y_Pos > 0) && (Y_Pos < 90))
	{
		// printf("select 19\r\n");
		if (Point != 19)
		{
			Point = 19;
		}
		return 1;
	}
	else if ((X_Pos > 0) && (X_Pos < 512) && (Y_Pos > 0) && (Y_Pos < 90))
	{
		// printf("select 21\r\n");
		if (Point != 21)
		{
			Point = 21;
		}
		return 1;
	}
	else
	{
		if (Point != 0)
		{
			Point = 0;
			LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
		}
		return 0;
	}
}

uint8_t Screen_delay_Count(uint16_t t)
{
	uint8_t touch = 0;
  if (t == 1) t = 2;
  
	while (t > 0)
	{
    tp_dev.scan(0);
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}
		if (press == 0 && touch_state == 1)
		{
			press = 1;
		}
		if (press == 1 && touch_state == 0)
		{
			press = 0;
			return 1;
		}
		t--;
		delay_ms(1);
	}
	return 0;
}

void refresh_main_icon(uint16_t x_pod)
{
	LT758_BTE_Memory_Copy(layer3_start_addr, LCD_XSIZE_TFT, 0, 455,
						layer3_start_addr, LCD_XSIZE_TFT, 0, 455,
						layer1_start_addr, LCD_XSIZE_TFT, 0, 455,
						0x0c, 1024, 145, 24);
	LT758_BTE_Memory_Copy(layer2_start_addr, LCD_XSIZE_TFT, x_pod, 455,
						layer2_start_addr, LCD_XSIZE_TFT, x_pod, 455,
						layer1_start_addr, LCD_XSIZE_TFT, x_pod, 455,
						0x0c, 130, 145, 24);
}

void main_menu(void)
{
	Canvas_Image_Start_address(layer1_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[0]);
	Canvas_Image_Start_address(layer2_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[1]);
	Canvas_Image_Start_address(layer3_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[0]);

	Canvas_Image_Start_address(layer1_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	while (1)
	{
		refresh_main_icon(65);
		if (Screen_delay_Count(500) == 1)
			break;
		refresh_main_icon(255);
		if (Screen_delay_Count(500) == 1)
			break;
		refresh_main_icon(445);
		if (Screen_delay_Count(500) == 1)
			break;
		refresh_main_icon(640);
		if (Screen_delay_Count(500) == 1)
			break;
		refresh_main_icon(830);
		if (Screen_delay_Count(500) == 1)
			break;
	}
	LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
}

void display_menu(void)
{
	Main_Image_Start_Address(layer1_start_addr);
	Main_Image_Width(LCD_XSIZE_TFT);
	Main_Window_Start_XY(0, 0);
	Canvas_Image_Start_address(layer1_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[2]);
}

void function_1(void)
{
	uint8_t touch = 0;

	LT758_DMA_JPG(0, 0, H_JPG_Addr[64]);
	while (1)//等待按键按下后退出
	{
    tp_dev.scan(0);
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}
   
		if (press == 0 && touch_state == 1)
		{
			press = 1;
		}
		if (press == 1 && touch_state == 0)
		{
			press = 0;
			break;
		}
	}
}

uint8_t function_2(void)
{
	uint32_t i, j, h, c, z;
	uint32_t resx1, resy1;
	while (1)
	{
		//--------------------- 圆 ------------------------
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		h = 0;
		do
		{
			i = rand() % 16777216;				   // 颜色
			resx1 = rand() % LCD_XSIZE_TFT;		   // 圆心x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 24); // 圆心y轴
			z = rand() % 200;					   // 半径
			j = rand() % 3;
			if (j)
				LT758_DrawCircle(resx1, resy1, z, i); // 无填充色圆
			else
				LT758_DrawCircle_Fill(resx1, resy1, z, i); // 填充色圆
			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			i = rand() % 16777216;				   // 颜色
			resx1 = rand() % LCD_XSIZE_TFT;		   // 圆心x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 24); // 圆心y轴
			z = rand() % 200;					   // 半径
			j = rand() % 3;
			if (j)
				LT758_DrawCircle(resx1, resy1, z, i); // 无填充色圆
			else
				LT758_DrawCircle_Fill(resx1, resy1, z, i); // 填充色圆
			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2500);

		//--------------------- 椭圆 ------------------------
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		h = 0;
		do
		{
			i = rand() % 16777216;				   // 颜色
			resx1 = rand() % LCD_XSIZE_TFT;		   // 圆心x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 24); // 圆心y轴
			z = rand() % 200;					   // 长半径
			c = rand() % 200;					   // 短半径
			j = rand() % 3;
			if (j)
				LT758_DrawEllipse(resx1, resy1, z, c, i); // 无填充色圆
			else
				LT758_DrawEllipse_Fill(resx1, resy1, z, c, i); // 填充色圆
			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			i = rand() % 16777216;				   // 颜色
			resx1 = rand() % LCD_XSIZE_TFT;		   // 圆心x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 24); // 圆心y轴
			z = rand() % 200;					   // 长半径
			c = rand() % 200;					   // 短半径
			j = rand() % 3;
			if (j)
				LT758_DrawEllipse(resx1, resy1, z, c, i); // 无填充色圆
			else
				LT758_DrawEllipse_Fill(resx1, resy1, z, c, i); // 填充色圆
			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2500);
	}
}

uint8_t function_3(void)
{
	uint32_t i, j, h, z, d, w;
	uint32_t resx1, resy1, resx2, resy2;
	while (1)
	{
		//----------------------- 线段 ------------------------
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		h = 0;
		do
		{
			resx1 = rand() % LCD_XSIZE_TFT;	  // 起点的x轴
			resy1 = rand() % (LCD_YSIZE_TFT); // 起点的y轴
			resx2 = rand() % LCD_XSIZE_TFT;	  // 终点的x轴
			resy2 = rand() % (LCD_YSIZE_TFT); // 终点的y轴
			i = rand() % 16777216;			  // 颜色
			w = rand() % 20;

			LT758_DrawLine_Width(resx1, resy1 + w, resx2, resy2 + w, i, w);

			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			resx1 = rand() % LCD_XSIZE_TFT;	  // 起点的x轴
			resy1 = rand() % (LCD_YSIZE_TFT); // 起点的y轴
			resx2 = rand() % LCD_XSIZE_TFT;	  // 终点的x轴
			resy2 = rand() % (LCD_YSIZE_TFT); // 终点的y轴
			i = rand() % 16777216;			  // 颜色
			w = rand() % 20;

			LT758_DrawLine_Width(resx1, resy1 + w, resx2, resy2 + w, i, w);

			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2500);

		//----------------------- 曲线 ------------------------

		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);

		h = 0;
		do
		{
			i = rand() % 16777216;			  // 颜色
			resx1 = rand() % LCD_XSIZE_TFT;	  // 区心的x轴
			resy1 = rand() % (LCD_YSIZE_TFT); // 区心的y轴
			z = rand() % 150;				  // 长短半径
			d = rand() % 4;					  // 方向
			j = rand() % 3;

			switch (d)
			{
			case 0:
				if (z > resx1)
					resx1 = 150;
				if (z > (600 - resy1))
					resy1 = 450;
				break;
			case 1:
				if (z > resx1)
					resx1 = 150;
				if (z > resy1)
					resy1 = 150;
				break;
			case 2:
				if (z > (1024 - resx1))
					resx1 = 875;
				if (z > resy1)
					resy1 = 150;
				break;
			case 3:
				if (z > (1024 - resx1))
					resx1 = 875;
				if (z > (600 - resy1))
					resy1 = 450;
				break;
			default:
				break;
			}

			if (j)
				LT758_SelectDrawCurve(resx1, resy1, z, z, i, d); // 硬件画曲线（无填充）
			else
				LT758_SelectDrawCurve_Fill(resx1, resy1, z, z, i, d); // 硬件画曲线（填充）
			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			i = rand() % 16777216;					// 颜色
			resx1 = rand() % (LCD_XSIZE_TFT - 150); // 区心的x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 150); // 区心的y轴
			z = rand() % 150;						// 长短半径
			d = rand() % 4;							// 方向
			j = rand() % 3;

			switch (d)
			{
			case 0:
				if (z > resx1)
					resx1 = 150;
				if (z > (600 - resy1))
					resy1 = 450;
				break;
			case 1:
				if (z > resx1)
					resx1 = 150;
				if (z > resy1)
					resy1 = 150;
				break;
			case 2:
				if (z > (1024 - resx1))
					resx1 = 875;
				if (z > resy1)
					resy1 = 150;
				break;
			case 3:
				if (z > (1024 - resx1))
					resx1 = 875;
				if (z > (600 - resy1))
					resy1 = 450;
				break;
			default:
				break;
			}

			if (j)
				LT758_SelectDrawCurve(resx1, resy1, z, z, i, d); // 硬件画曲线（无填充）
			else
				LT758_SelectDrawCurve_Fill(resx1, resy1, z, z, i, d); // 硬件画曲线（填充）
			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2500);
	}
}

unsigned short abs1(int val)
{
	if(val<0)	val = -val;
	return val;
}

uint8_t function_4(void)
{
	uint32_t i, j, h, z;
	uint32_t resx1, resy1, resx2, resy2;
	while (1)
	{
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		h = 0;
		do
		{
			resx1 = rand() % LCD_XSIZE_TFT;	  // 起点的x轴
			resy1 = rand() % (LCD_YSIZE_TFT); // 起点的y轴
			resx2 = rand() % LCD_XSIZE_TFT;	  // 终点的x轴
			resy2 = rand() % (LCD_YSIZE_TFT); // 终点的y轴
			i = rand() % 16777216;			  // 颜色
			j = rand() % 3;
			if (j)
				LT758_DrawSquare(resx1, resy1, resx2, resy2, i); // 无填充色矩形
			else
				LT758_DrawSquare_Fill(resx1, resy1, resx2, resy2, i); // 填充色矩形
			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			resx1 = rand() % LCD_XSIZE_TFT;	  // 起点的x轴
			resy1 = rand() % (LCD_YSIZE_TFT); // 起点的y轴
			resx2 = rand() % LCD_XSIZE_TFT;	  // 终点的x轴
			resy2 = rand() % (LCD_YSIZE_TFT); // 终点的y轴
			i = rand() % 16777216;			  // 颜色
			j = rand() % 3;
			if (j)
				LT758_DrawSquare(resx1, resy1, resx2, resy2, i); // 无填充色矩形
			else
				LT758_DrawSquare_Fill(resx1, resy1, resx2, resy2, i); // 填充色矩形
			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2000);

		// ---------------------------- 圆角矩形 -----------------------------
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		h = 0;
		do
		{
			resx1 = rand() % (LCD_XSIZE_TFT - 10); // 点1的x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 10); // 点1的y轴
			resx2 = rand() % (LCD_XSIZE_TFT - 10); // 点2的x轴
			resy2 = rand() % (LCD_YSIZE_TFT - 10); // 点2的y轴
			i = rand() % 16777216;				   // 颜色
			j = rand() % 3;
			z = rand() % 50; // 长半径

			if (abs1(resx2 - resx1) < (2 * z))
				z = 0;
			if (abs1(resy2 - resy1) < (2 * z))
				z = 0;

			if (j)
				LT758_DrawCircleSquare(resx1, resy1, resx2, resy2, z, z, i);
			else
				LT758_DrawCircleSquare_Fill(resx1, resy1, resx2, resy2, z, z, i);
			h++;
			if (Screen_delay_Count(100) == 1)
				return 1;
		} while (h < 20);

		h = 0;
		do
		{
			resx1 = rand() % (LCD_XSIZE_TFT - 10); // 点1的x轴
			resy1 = rand() % (LCD_YSIZE_TFT - 10); // 点1的y轴
			resx2 = rand() % (LCD_XSIZE_TFT - 10); // 点2的x轴
			resy2 = rand() % (LCD_YSIZE_TFT - 10); // 点2的y轴
			i = rand() % 16777216;				   // 颜色
			j = rand() % 3;
			z = rand() % 50; // 长半径

			if (abs1(resx2 - resx1) < (2 * z))
				z = 0;
			if (abs1(resy2 - resy1) < (2 * z))
				z = 0;

			if (j)
				LT758_DrawCircleSquare(resx1, resy1, resx2, resy2, z, z, i);
			else
				LT758_DrawCircleSquare_Fill(resx1, resy1, resx2, resy2, z, z, i);
			h++;
			if (Screen_delay_Count(1) == 1)
				return 1;
		} while (h < 2000);
	}
}

uint8_t function_5(void)
{	
	uint8_t temp = 55;

	while (1)
	{
		temp++;
		if(temp > 63)
		{
			temp = 56;
		}
		LT758_DMA_JPG(0, 0, H_JPG_Addr[temp]);
		if(Screen_delay_Count(30)) break;
	}
  
	return 0;
}

uint8_t NUM[20][3] = {{0x20,0x30,0x00}, {0x20,0x31,0x00}, {0x20,0x32,0x00}, {0x20,0x33,0x00}, {0x20,0x34,0x00}, {0x20,0x35,0x00}, {0x20,0x36,0x00}, {0x20,0x37,0x00}, {0x20,0x38,0x00}, {0x20,0x39,0x00}, {0x31,0x30,0x00}, {0x31,0x31,0x00}, {0x31,0x32,0x00}, {0x31,0x33,0x00}, {0x31,0x34,0x00}, {0x31,0x35,0x00}, {0x31,0x36,0x00}, {0x31,0x37,0x00}, {0x31,0x38,0x00}, {0x31,0x39,0x00}};

uint8_t function_6(void)
{
	uint8_t i1 = 0;
	uint8_t i2 = 1;
	uint8_t i3 = 2;
	uint8_t i4 = 3;
	uint8_t count = 0;

	Canvas_Image_Start_address(layer1_start_addr); // Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);			   // 画布图像宽度
	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT); // 设置工作窗口大小
	Main_Window_Start_XY(0, 0);						// 设置主窗口坐标
	LT758_Select_Outside_Font_Init(1, 0, Font_24_Addr, layer7_start_addr, Font_24_Size, 24, 1, 1, 0, 0);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[78]);

	Font_Background_select_Color();
	LT758_Print_Outside_Font_String(75, 210+(52*0), Black, 0xF3F3F5, "1");
	LT758_Print_Outside_Font_String(75, 210+(52*1), Black, 0xF3F3F5, "2");
	LT758_Print_Outside_Font_String(75, 210+(52*2), Black, 0xF3F3F5, "3");
	LT758_Print_Outside_Font_String(75, 210+(52*3), Black, 0xF3F3F5, "4");
	
	LT758_Print_Outside_Font_String(605, 148, White, 0xFF7300, "1");
	LT758_Print_Outside_Font_String(713, 148, White, 0xFF7300, "2");
	LT758_Print_Outside_Font_String(815, 148, White, 0xFF7300, "3");
	LT758_Print_Outside_Font_String(918, 148, White, 0xFF7300, "4");
	while (1)
	{
		LT758_Print_Outside_Font_String(150, 210+(52*0), Black, 0xF3F3F5, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(150, 210+(52*1), Black, 0xF3F3F5, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(150, 210+(52*2), Black, 0xF3F3F5, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(150, 210+(52*3), Black, 0xF3F3F5, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(222, 210+(52*0), Black, 0xF3F3F5, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(222, 210+(52*1), Black, 0xF3F3F5, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(222, 210+(52*2), Black, 0xF3F3F5, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(222, 210+(52*3), Black, 0xF3F3F5, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(305, 210+(52*0), Black, 0xF3F3F5, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(305, 210+(52*1), Black, 0xF3F3F5, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(305, 210+(52*2), Black, 0xF3F3F5, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(305, 210+(52*3), Black, 0xF3F3F5, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(380, 210+(52*0), Black, 0xF3F3F5, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(380, 210+(52*1), Black, 0xF3F3F5, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(380, 210+(52*2), Black, 0xF3F3F5, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(380, 210+(52*3), Black, 0xF3F3F5, &NUM[i4][0]);
		//=============================================================================
		LT758_Print_Outside_Font_String(600, 180+(70*0), Black, White, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(600, 180+(70*1), Black, White, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(600, 180+(70*2), Black, White, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(600, 180+(70*3), Black, White, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(710, 180+(70*0), Black, White, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(710, 180+(70*1), Black, White, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(710, 180+(70*2), Black, White, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(710, 180+(70*3), Black, White, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(810, 180+(70*0), Black, White, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(810, 180+(70*1), Black, White, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(810, 180+(70*2), Black, White, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(810, 180+(70*3), Black, White, &NUM[i4][0]);

		LT758_Print_Outside_Font_String(914, 180+(70*0), Black, White, &NUM[i1][0]);
		LT758_Print_Outside_Font_String(914, 180+(70*1), Black, White, &NUM[i2][0]);
		LT758_Print_Outside_Font_String(914, 180+(70*2), Black, White, &NUM[i3][0]);
		LT758_Print_Outside_Font_String(914, 180+(70*3), Black, White, &NUM[i4][0]);

		i1++;
		i2++;
		i3++;
		if (i1 > 19)
			i1 = 0;
		if (i2 > 19)
			i2 = 0;
		if (i3 > 19)
			i3 = 0;

		count++;
		if (count == 2)
		{
			count = 0;
			i4++;
			if (i4 > 19)
				i4 = 0;
		}

		if (Screen_delay_Count(490) == 1)
			return 1;
	}
}

uint8_t function_7(void)
{
	uint8_t i = 0;
	uint32_t j1 = 0, z1 = 0;
	uint32_t j2 = 0, z2 = 0;
	uint32_t h1 = 0, h2 = 0;

	uint32_t count = 0;

	Canvas_Image_Start_address(layer1_start_addr); // Layer 1;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);			   // 画布图像宽度
	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT); // 设置工作窗口大小

	LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, Blue2);

	while (1)
	{
		count = 1;
		while (count--)
		{
			for (i = 0; i < 11; i++)
			{
				j1 = rand() % 16777216; // 颜色
				z1 = rand() % 16777216; // 颜色
				j2 = rand() % 16777216; // 颜色
				z2 = rand() % 16777216; // 颜色
				h1 = rand() % 400;
				h2 = rand() % 400;

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 40 * (i + 1) - 19, 0, Blue2, 40, 600, 24);
				LT758_DrawCylinder(40 * (i + 1), 500, 18, 9, h1, j1, z1);

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 1000 - i * 40 - 24, 0, Blue2, 41, 600, 24);
				LT758_DrawQuadrangular(1000 - i * 40 - 24, 509 - h2, // 1
									   1000 - i * 40 - 10, 499 - h2, // 2
									   1014 - i * 40, 499 - h2,		 // 3
									   1000 - i * 40, 509 - h2,		 // 4
									   1000 - i * 40, 509,			 // 5
									   1014 - i * 40, 499,			 // 6
									   j2, z2);

				if (Screen_delay_Count(120) == 1)
					return 1;
			}

			for (i = 11; i > 0; i--)
			{
				j1 = rand() % 16777216; // 颜色
				z1 = rand() % 16777216; // 颜色
				j2 = rand() % 16777216; // 颜色
				z2 = rand() % 16777216; // 颜色
				h1 = rand() % 400;
				h2 = rand() % 400;

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 40 * (i + 1) - 19, 0, Blue2, 41, 600, 24);
				LT758_DrawCylinder(40 * (i + 1), 500, 18, 9, h1, j1, z1);

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 1000 - i * 40 - 24, 0, Blue2, 41, 600, 24);
				LT758_DrawQuadrangular(1000 - i * 40 - 24, 509 - h2, // 1
									   1000 - i * 40 - 10, 499 - h2, // 2
									   1014 - i * 40, 499 - h2,		 // 3
									   1000 - i * 40, 509 - h2,		 // 4
									   1000 - i * 40, 509,			 // 5
									   1014 - i * 40, 499,			 // 6
									   j2, z2);

				if (Screen_delay_Count(120) == 1)
					return 1;
			}
		}

		count = 10;
		while (count--)
		{
			for (i = 0; i < 11; i++)
			{
				j1 = rand() % 16777216; // 颜色
				z1 = rand() % 16777216; // 颜色
				j2 = rand() % 16777216; // 颜色
				z2 = rand() % 16777216; // 颜色
				h1 = rand() % 400;
				h2 = rand() % 400;

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 40 * (i + 1) - 19, 0, Blue2, 40, 600, 24);
				LT758_DrawCylinder(40 * (i + 1), 500, 18, 9, h1, j1, z1);

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 1000 - i * 40 - 24, 0, Blue2, 41, 600, 24);
				LT758_DrawQuadrangular(1000 - i * 40 - 24, 509 - h2, // 1
									   1000 - i * 40 - 10, 499 - h2, // 2
									   1014 - i * 40, 499 - h2,		 // 3
									   1000 - i * 40, 509 - h2,		 // 4
									   1000 - i * 40, 509,			 // 5
									   1014 - i * 40, 499,			 // 6
									   j2, z2);

				if (Screen_delay_Count(10) == 1)
					return 1;
			}

			for (i = 11; i > 0; i--)
			{
				j1 = rand() % 16777216; // 颜色
				z1 = rand() % 16777216; // 颜色
				j2 = rand() % 16777216; // 颜色
				z2 = rand() % 16777216; // 颜色
				h1 = rand() % 400;
				h2 = rand() % 400;

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 40 * (i + 1) - 19, 0, Blue2, 41, 600, 24);
				LT758_DrawCylinder(40 * (i + 1), 500, 18, 9, h1, j1, z1);

				LT758_BTE_Solid_Fill(layer1_start_addr, LCD_XSIZE_TFT, 1000 - i * 40 - 24, 0, Blue2, 41, 600, 24);
				LT758_DrawQuadrangular(1000 - i * 40 - 24, 509 - h2, // 1
									   1000 - i * 40 - 10, 499 - h2, // 2
									   1014 - i * 40, 499 - h2,		 // 3
									   1000 - i * 40, 509 - h2,		 // 4
									   1000 - i * 40, 509,			 // 5
									   1014 - i * 40, 499,			 // 6
									   j2, z2);

				if (Screen_delay_Count(10) == 1)
					return 1;
			}
		}
	}
}

uint8_t function_8(void)
{
	LT758_DMA_JPG(0, 0, H_JPG_Addr[57]);
	if (Screen_delay_Count(1000) == 1)
		return 1;
	LT758_DMA_JPG(0, 0, H_JPG_Addr[58]);
	if (Screen_delay_Count(1000) == 1)
		return 1;
	LT758_DMA_JPG(0, 0, H_JPG_Addr[59]);
	if (Screen_delay_Count(1000) == 1)
		return 1;
	LT758_DMA_JPG(0, 0, H_JPG_Addr[60]);
	if (Screen_delay_Count(1000) == 1)
		return 1;
	while (1)
	{
		LT758_DMA_JPG(0, 0, H_JPG_Addr[57]);
		if (Screen_delay_Count(10) == 1)
			return 1;
		LT758_DMA_JPG(0, 0, H_JPG_Addr[58]);
		if (Screen_delay_Count(10) == 1)
			return 1;
		LT758_DMA_JPG(0, 0, H_JPG_Addr[59]);
		if (Screen_delay_Count(10) == 1)
			return 1;
		LT758_DMA_JPG(0, 0, H_JPG_Addr[60]);
		if (Screen_delay_Count(10) == 1)
			return 1;
	}
}

void TpGesture(void)
{
	static uint8_t TouchCunt = 0;
	int16_t i = 0;
	uint16_t temp = 0;
	int16_t df = 0;
	uint8_t Slide_press = 0;
	if (touch_state)
	{
		Slide_press = 1;
    X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
    Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
    
		if (gFirstTouch == 0)
		{
			TouchCunt++;
			if (TouchCunt >= 8)
				gFirstTouch = 1;

			gGesXY.dx = X_Pos - gGesXY.lastX; //这里的last应该是上一次而不是最后
			gGesXY.buf[gGesXY.pos++] = gGesXY.dx;	 //将相邻两次读数x之差放在gGesXY.buf

			if (gGesXY.pos >= 5)
				gGesXY.pos = 0;

			gGesXY.lastX = X_Pos; //这里会获得last
			gGesXY.lastY = Y_Pos;
		}
		else
		{
			penflag = 1;
			//------移动X轴--------
			if (Slide_press)
			{
				Slide_press = 0;
				Slide_xw = X_Pos - First_pressX;
				Slide_yw = Y_Pos - First_pressY;
				if (Slide_xw >= slide_lenght || Slide_xw <= -slide_lenght || Slide_yw >= slide_lenght || Slide_yw <= -slide_lenght || touch_state)
				{
					if (abs(Slide_xw) * LCD_XSIZE_TFT > abs(Slide_yw) * LCD_YSIZE_TFT)
					{
						if (Y_slide_flag == 0)
						{
							X_slide_flag = 1;
							gGesXY.dx = X_Pos - gGesXY.lastX;
							gGesXY.buf[gGesXY.pos++] = gGesXY.dx;
							if (gGesXY.pos >= 5)
								gGesXY.pos = 0;
							for (i = 0; i < 5; i++)
							{
								df += gGesXY.buf[i];
								// DelayMS(1);
							}
							df /= 5;
							if (KK == 0)
								dfx = df;
							if ((df > 1) || (df < -1))
							{
								//左滑
								if (df < 0)
									gGesXY.x += (0 - df);
								if (gGesXY.x >= LCD_XSIZE_TFT * 4)
									gGesXY.x = LCD_XSIZE_TFT * 4;
								//右滑
								if (df >= 0)
									gGesXY.x -= df;
								if (gGesXY.x < 0)
									gGesXY.x = 0;
								gGesXY.lastX = X_Pos;
								gGesXY.lastY = Y_Pos;
								Main_Window_Start_XY(gGesXY.x, gGesXY.y);
								gGesXY.first = 1;
							}
						}
					}
				}
			}
		}
	}
	//------释放触摸屏--------
	else
	{
		Slide_press = 0;
		KK = 0;

		if (gFirstTouch == 1 && penflag == 1)
		{
			TouchCunt = 0;
			gFirstTouch = 0;
			gGesXY.first = 0;
			gGesXY.pos = 0;

			if (X_slide_flag == 1) //应该让它记录第一次滑动的数据作为比较
			{
				//手指释放后图片右滑
				if (Slide_xw >= 50 || (-50 < Slide_xw && Slide_xw <= 0)) //判断最后状态是向哪边，
				{
					for (i = 0; i < 5; i++)
					{
						if ((gGesXY.x > i * LCD_XSIZE_TFT) && (gGesXY.x < (i + 1) * LCD_XSIZE_TFT))
						{
							temp = i * LCD_XSIZE_TFT; // tem只能为0
							break;
						}
					}
					for (i = gGesXY.x; i >= temp; i--) //释放后右滑。这里的gGesXY.x就是右滑之后偏离的距离，释放后从这个偏移位置让窗口左移到0位置实现图片右移
					{
						Main_Window_Start_XY(i, gGesXY.y);
						delay_us(250);
					}
					gGesXY.x = temp; //之前的gGesXY.x 是标记的基础上加上偏移值。之后的gGesXY.x 是释放后标记的的x（0/800）
				}
				//手指释放后图片左滑
				if ((0 < Slide_xw && Slide_xw < 50) || Slide_xw <= -50)
				{
					for (i = 0; i < 5; i++) //这个for不随滑动改变
					{
						if ((gGesXY.x > i * LCD_XSIZE_TFT) && (gGesXY.x <= (i + 1) * LCD_XSIZE_TFT))
						{
							temp = (i + 1) * LCD_XSIZE_TFT; //这个tem只能是800
							break;
						}
					}
					for (i = gGesXY.x; i <= temp; i++)
					{
						Main_Window_Start_XY(i, gGesXY.y);
						delay_us(250);
					}
					gGesXY.x = temp; //所以tem只有0或800作为下一次移动Y轴时的固定值
				}
			}
		}
		penflag = 0;
		Y_slide_flag = 0;
		X_slide_flag = 0;
		for (i = 0; i < 5; i++)
		{
			gGesXY.buf[i] = 0;
		}
		if (gGesXY.x == 0)
		{
			gGesXY.x = LCD_XSIZE_TFT*3;
		}
		if (gGesXY.x == LCD_XSIZE_TFT*4)
		{
			gGesXY.x = LCD_XSIZE_TFT;
		}
	}
}

uint8_t function_9(void)
{
	uint8_t touch = 0;

	touch_state = 0;
	two_click = 0;
	KK = 0;

	Canvas_Image_Start_address(layer2_start_addr); // Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT * 5);		   //画布图像宽度

	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT * 5, LCD_YSIZE_TFT); //设置工作窗口大小

	LT758_DMA_JPG(LCD_XSIZE_TFT, 0, H_JPG_Addr[58]);

	Main_Image_Start_Address(layer2_start_addr); //设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT * 5);
	Main_Window_Start_XY(LCD_XSIZE_TFT, 0); //设置主窗口坐标

	LT758_DMA_JPG(LCD_XSIZE_TFT*2, 0, H_JPG_Addr[59]);
	LT758_DMA_JPG(LCD_XSIZE_TFT*3, 0, H_JPG_Addr[60]);

	LT758_DMA_JPG(0, 0, H_JPG_Addr[60]);
	LT758_DMA_JPG(LCD_XSIZE_TFT*4, 0, H_JPG_Addr[58]);

	for (f9_j = 0; f9_j < 3; f9_j++)
	{
		for (f9_i = 0; f9_i <= LCD_XSIZE_TFT; f9_i += 1)
		{
			Main_Window_Start_XY(LCD_XSIZE_TFT + f9_j * LCD_XSIZE_TFT + f9_i, 0);
			delay_us(500);
		}
	}
	delay_ms(150);
	for (f9_j = 0; f9_j < 3; f9_j++)
	{
		for (f9_i = 0; f9_i <= LCD_XSIZE_TFT; f9_i += 1)
		{
			Main_Window_Start_XY(LCD_XSIZE_TFT*4 - (f9_j * LCD_XSIZE_TFT + f9_i), 0);
			delay_us(500);
		}
	}

	Main_Window_Start_XY(LCD_XSIZE_TFT, 0);
	gGesXY.x = LCD_XSIZE_TFT;

	gGesXY.numx = 0;
	gGesXY.numy = 0;

	while (1)
	{
		TpGesture();
    tp_dev.scan(0);
    TP_read_touch();
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}   
    
		if ((two_click && touch_state==0) || key_scan(0) == KEY0_PRES)
		{
			two_click = 0;
			gGesXY.x = 0;
			gGesXY.y = 0;
			KK = 0;
			return 1;
		}
	}
}

void TpGesture_1(void)
{
	static uint8_t TouchCunt = 0;
	int16_t i = 0;
	uint16_t temp = 0;
	int16_t dy = 0;
	uint8_t Slide_press = 0;
	if (touch_state)
	{
		Slide_press = 1;
    X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
    Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
    
		if (gFirstTouch == 0)
		{
			TouchCunt++;
			if (TouchCunt >= 8)
				gFirstTouch = 1;

			gGesXY.dx = X_Pos - gGesXY.lastX; //这里的last应该是上一次而不是最后
			gGesXY.buf[gGesXY.pos++] = gGesXY.dx;	 //将相邻两次读数x之差放在gGesXY.buf

			if (gGesXY.pos >= 5)
				gGesXY.pos = 0;

			gGesXY.lastX = X_Pos; //这里会获得last
			gGesXY.lastY = Y_Pos;
		}
		else
		{
			penflag = 1;
			//------移动X轴--------
			if (Slide_press)
			{
				Slide_press = 0;
				Slide_xw = X_Pos - First_pressX;
				Slide_yw = Y_Pos - First_pressY;
				if (Slide_xw >= slide_lenght || Slide_xw <= -slide_lenght || Slide_yw >= slide_lenght || Slide_yw <= -slide_lenght || touch_state)
				{
					if (abs(Slide_xw) * LCD_XSIZE_TFT > abs(Slide_yw) * LCD_YSIZE_TFT)
					{
						if (Y_slide_flag == 0)
						{
						}
					}
					//------移动Y轴--------
					else
					{
						if (X_slide_flag == 0)
						{
							Y_slide_flag = 1;
							gGesXY.dy = Y_Pos - gGesXY.lastY;
							gGesXY.buf[gGesXY.pos++] = gGesXY.dy;
							if (gGesXY.pos >= 5)
								gGesXY.pos = 0;
							for (i = 0; i < 5; i++)
							{
								dy += gGesXY.buf[i];
								// DelayMS(1);
							}
							if (KK == 0)
								dfy = dy;
							dy /= 5;
							if ((dy > 1) || (dy < -1))
							{
								//上滑
								if (dy < 0)
									gGesXY.y += (0 - dy);
								if (gGesXY.y >= LCD_YSIZE_TFT * 4)
									gGesXY.y = LCD_YSIZE_TFT * 4;
								//下滑
								if (dy >= 0)
									gGesXY.y -= dy;
								if (gGesXY.y < 0)
									gGesXY.y = 0;
								gGesXY.lastX = X_Pos;
								gGesXY.lastY = Y_Pos;
								Main_Window_Start_XY(gGesXY.x, gGesXY.y);
								gGesXY.first = 1;
							}
						}
					}
				}
			}
		}
	}
	//------释放触摸屏--------
	else
	{
		Slide_press = 0;
		KK = 0;

		if (gFirstTouch == 1 && penflag == 1)
		{
			TouchCunt = 0;
			gFirstTouch = 0;
			gGesXY.first = 0;
			gGesXY.pos = 0;

			if (X_slide_flag == 0) //应该让它记录第一次滑动的数据作为比较
			{
				//手指释放后图片下滑===========================================
				if (Slide_yw >= 50 || (-50 < Slide_yw && Slide_yw <= 0))
				{
					// printf("下滑\r\n");
					for (i = 0; i < 5; i++)
					{
						if ((gGesXY.y >= i * LCD_YSIZE_TFT) && (gGesXY.y < (i + 1) * LCD_YSIZE_TFT))
						{
							temp = i * LCD_YSIZE_TFT;
							break;
						}
					}
					for (i = gGesXY.y; i >= temp; i--)
					{
						Main_Window_Start_XY(gGesXY.x, i);
						delay_us(250);
					}
					gGesXY.y = temp;
				}
				//手指释放后图片上滑========================================================
				if ((0 < Slide_yw && Slide_xw < 50) || Slide_yw <= -50)
				{
					for (i = 0; i < 5; i++)
					{
						if ((gGesXY.y > i * LCD_YSIZE_TFT) && (gGesXY.y <= (i + 1) * LCD_YSIZE_TFT))
						{
							temp = (i + 1) * LCD_YSIZE_TFT;
							break;
						}
					}
					for (i = gGesXY.y; i <= temp; i++)
					{
						Main_Window_Start_XY(gGesXY.x, i);
						delay_us(250);
					}
					gGesXY.y = temp;
				}
			}
		}
		penflag = 0;
		Y_slide_flag = 0;
		X_slide_flag = 0;
		for (i = 0; i < 5; i++)
		{
			gGesXY.buf[i] = 0;
		}
		if (gGesXY.y == 0)
		{
			gGesXY.y = LCD_YSIZE_TFT*3;
		}
		if (gGesXY.y == LCD_YSIZE_TFT*4)
		{
			gGesXY.y = LCD_YSIZE_TFT;
		}
	}
}

uint8_t function_20(void)
{
	uint8_t touch = 0;

	touch_state = 0;
	two_click = 0;
	KK = 0;

	Canvas_Image_Start_address(layer2_start_addr); // Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);			   //画布图像宽度

	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT * 5); //设置工作窗口大小

	LT758_DMA_JPG(0, LCD_YSIZE_TFT, H_JPG_Addr[61]);

	Main_Image_Start_Address(layer2_start_addr); //设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);
	Main_Window_Start_XY(0, LCD_YSIZE_TFT); //设置主窗口坐标

	LT758_DMA_JPG(0, LCD_YSIZE_TFT*2, H_JPG_Addr[62]);
	LT758_DMA_JPG(0, LCD_YSIZE_TFT*3, H_JPG_Addr[63]);
	LT758_DMA_JPG(0, LCD_YSIZE_TFT*4, H_JPG_Addr[61]);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[63]);

	for (f9_j = 0; f9_j < 3; f9_j++)
	{
		for (f9_i = 0; f9_i <= LCD_YSIZE_TFT; f9_i += 1)
		{
			Main_Window_Start_XY(0, LCD_YSIZE_TFT + f9_j * LCD_YSIZE_TFT + f9_i); //图片左移，窗口右移
			delay_us(500);
		}
	}
	delay_ms(150);
	for (f9_j = 0; f9_j < 3; f9_j++)
	{
		for (f9_i = 0; f9_i <= LCD_YSIZE_TFT; f9_i += 1)
		{
			Main_Window_Start_XY(0, LCD_YSIZE_TFT*4 - (f9_j * LCD_YSIZE_TFT + f9_i)); //图片左移，窗口右移
			delay_us(500);
		}
	}

	Main_Window_Start_XY(0, LCD_YSIZE_TFT);
	gGesXY.y = LCD_YSIZE_TFT;

	gGesXY.numx = 0;
	gGesXY.numy = 0;

	while (1)
	{
		TpGesture_1();
    tp_dev.scan(0);
    TP_read_touch();

    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}  
    
		if ((two_click && touch_state==0) || key_scan(0) == KEY0_PRES)    
		{
			two_click = 0;
			gGesXY.x = 0;
			gGesXY.y = 0;
			KK = 0;
			return 1;
		}
	}
}

uint8_t function_10(void)
{
	char c[2] = "0";
	uint16_t i = 0;
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t z = 0;

	Main_Image_Start_Address(layer2_start_addr); // 设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);
	Canvas_Image_Start_address(layer2_start_addr); // Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);			   // 画布图像宽度
	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT); // 设置工作窗口大小
	Main_Window_Start_XY(0, 0);						// 设置主窗口坐标

	while (1)
	{
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, Blue2);

		LT758_Select_Internal_Font_Init(16, 1, 1, 0, 0);
		LT758_Print_Internal_Font_String(10, 10, Black, Blue2, "Embedded 8x16 ASCII Character");

		LT758_Select_Internal_Font_Init(24, 1, 1, 0, 0);
		LT758_Print_Internal_Font_String(10, 30, Blue, Blue2, "Embedded 12x24 ASCII Character");

		LT758_Select_Internal_Font_Init(32, 1, 1, 0, 0);
		LT758_Print_Internal_Font_String(10, 55, Green, Blue2, "Embedded 16x32 ASCII Character");

		LT758_Print_Internal_Font_String(10, 95, Red, Blue2, "The Text Cursor");

		for (i = 0; i < 14; i++)
		{
			if (Screen_delay_Count(100) == 1)
				return 1;
			LT758_Text_cursor_Init(1, 15, 1 + i, 15 - i);
		}
		delay_ms(100);
		LT758_Text_cursor_Init(1, 15, 10, 2);

		c[0] = '0';
		for (i = 0; i < 10; i++)
		{
			if (Screen_delay_Count(5) == 1)
				return 1;
			LT758_Print_Internal_Font_String(10 + 16 * i, 135, Red, Blue2, &c[0]);
			c[0]++;
		}

		c[0] = 0;
		x = 0;
		y = 175;
		z = 0;
		for (i = 0; i < 127; i++)
		{
			if (Screen_delay_Count(5) == 1)
				return 1;

			LT758_Print_Internal_Font_String(x, y, Red, Blue2, &c[0]);
			c[0]++;

			z++;
			x = z * 16;
			if (x > (1023-16))
			{
				y = y + 40;
				x = 0;
				z = 0;
			}
			
			if (Screen_delay_Count(30) == 1)
				return 1;
		}

		i = 0;
		if (Screen_delay_Count(30) == 1)
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "T");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "h");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "e");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, " ");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "G");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "r");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "a");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "p");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "h");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "i");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "c");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, " ");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "C");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "u");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "r");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "s");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "o");
		if (Screen_delay_Count(30))
			return 1;
		LT758_Print_Internal_Font_String(20 + 16 * i++, 260, Red, Blue2, "r");
		if (Screen_delay_Count(30))
			return 1;

		LT758_Graphic_cursor_Init(1, 0xff, 0x00, 0, 0, (uint8_t *)gImage_pen_il);
		LT758_Graphic_cursor_Init(2, 0xff, 0x00, 0, 0, (uint8_t *)gImage_arrow_il);
		LT758_Graphic_cursor_Init(3, 0xff, 0x00, 0, 0, (uint8_t *)gImage_busy_im);
		LT758_Graphic_cursor_Init(4, 0xff, 0x00, 0, 0, (uint8_t *)gImage_no_im);

		LT758_Set_Graphic_cursor_Pos(1, 100, 300);
		if (Screen_delay_Count(490))
			return 1;
		LT758_Set_Graphic_cursor_Pos(2, 100, 300);
		if (Screen_delay_Count(490))
			return 1;
		LT758_Set_Graphic_cursor_Pos(3, 100, 300);
		if (Screen_delay_Count(490))
			return 1;
		LT758_Set_Graphic_cursor_Pos(4, 100, 300);
		if (Screen_delay_Count(490))
			return 1;
		LT758_Set_Graphic_cursor_Pos(1, 100, 300);

		for (i = 100; i < 924; i++)
		{
			LT758_Set_Graphic_cursor_Pos(1, i, 90);
			if (Screen_delay_Count(2))
				return 1;
		}

		for (i = 200; i < 800; i++)
		{
			LT758_Set_Graphic_cursor_Pos(2, i, i - 200);
			if (Screen_delay_Count(2))
				return 1;
		}

		for (i = 800; i > 100; i--)
		{
			LT758_Set_Graphic_cursor_Pos(3, i, 800 - i);
			if (Screen_delay_Count(2))
				return 1;
		}

		for (i = 924; i > 100; i--)
		{
			LT758_Set_Graphic_cursor_Pos(4, i, 400);
			if (Screen_delay_Count(2))
				return 1;
		}

		if (Screen_delay_Count(990))
			return 1;
		LT758_Disable_Text_Cursor();
		LT758_Disable_Graphic_Cursor();
	}
}

uint8_t function_11(void)
{
	Main_Image_Start_Address(layer2_start_addr); // 设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);
	Canvas_Image_Start_address(layer2_start_addr); // Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);			   // 画布图像宽度
	Active_Window_XY(0, 0);
	Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT); // 设置工作窗口大小
	Main_Window_Start_XY(0, 0);						// 设置主窗口坐标

	while (1)
	{
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		LT758_Select_Outside_Font_Init(1, 0, Font_24_Addr, layer7_start_addr, Font_24_Size, 24, 1, 1, 0, 0);
		if (Screen_delay_Count(500))
			return 1;
		LT758_Print_Outside_Font_String(445, 105, Red, White, (uint8_t *)"24X24楷体");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X2();
		Font_Height_X2();
		LT758_Print_Outside_Font_String(260, 165, Green, White, (uint8_t *)"24X24楷体长宽各扩1倍");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X3();
		Font_Height_X3();
		LT758_Print_Outside_Font_String(150, 250, Cyan, White, (uint8_t *)"24X24楷体长宽各扩2倍");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X4();
		Font_Height_X4();
		LT758_Print_Outside_Font_String(30, 360, Yellow, White, (uint8_t *)"24X24楷体长宽各扩3倍");
						
		LT758_Select_Outside_Font_Init(1, 0, Font_32_Addr, layer7_start_addr, Font_32_Size, 32, 1, 1, 0, 0);
		if (Screen_delay_Count(1000))
			return 1;

		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		LT758_Print_Outside_Font_String(430, 105, Red, White, (uint8_t *)"32X32隶书");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X2();
		Font_Height_X2();
		LT758_Print_Outside_Font_String(360, 165, Green, White, (uint8_t *)"32X32隶书");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X3();
		Font_Height_X3();
		LT758_Print_Outside_Font_String(285, 250, Blue, White, (uint8_t *)"32X32隶书");
		if (Screen_delay_Count(500))
			return 1;
		Font_Width_X4();
		Font_Height_X4();
		LT758_Print_Outside_Font_String(220, 360, Yellow, White, (uint8_t *)"32X32隶书");

		if (Screen_delay_Count(1000))
			return 1;
		Font_Width_X1();
		Font_Height_X1();
		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		LT758_Print_Outside_Font_String(460, 10, Red, White, (uint8_t *)"32X32隶书");
		LT758_Print_Outside_Font_String(64, 50, Blue, White, (uint8_t *)"LT758 可以让使用者创建字形或符号，可以创建半角（8*16、12*24、16*32 dots）或是全角（16*16、24*24、32*32 dots）的字形或符号，此功能支持 32768 半角字或 32768 全角字，半角字形编码范围是在 0000h ~ 7FFFh，而全角字形编码范围则是 8000h ~ FFFFh。当使用者输入字符码，则 LT758 将会将其索引至外部 SDRAM 字符空间，并且将字形或符号数据存到显示内存的区间。");
		LT758_Print_Outside_Font_String(64, 260, Red, White, (uint8_t *)"LT758的文字有以下几个功能：");
		LT758_Print_Outside_Font_String(0, 300, Green, White, (uint8_t *)"1、LT758 支持文字旋转功能，让显示字符可以逆时针旋转 90 度。");
		LT758_Print_Outside_Font_String(0, 340, Green, White, (uint8_t *)"2、LT758 提供字体线性放大的功能。");
		LT758_Print_Outside_Font_String(0, 380, Green, White, (uint8_t *)"3、LT758 提供字体透明功能。");
		LT758_Print_Outside_Font_String(0, 420, Green, White, (uint8_t *)"4、LT758 提供文字自动换行功能，在遇到工作视窗边缘时会自动换行。");
		LT758_Print_Outside_Font_String(0, 460, Green, White, (uint8_t *)"5、LT758提供文字对齐功能，让LCD在半形字、全形字交错的情况下可以显示的比较整齐。");

		if (Screen_delay_Count(3000))
			return 1;

		LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, White);
		//无背景
		LT758_Draw_Font(0,Font_72_Addr,layer2_start_addr,layer4_start_addr,LCD_XSIZE_TFT,72,72,0,Red,0,270,120,270+500,120+100,0,0,(uint8_t *)"72*72字体楷体",13);
		//带背景
		LT758_BTE_Solid_Fill(layer3_start_addr, LCD_XSIZE_TFT, 0, 0, Blue2, 480, 72, 24);
		LT758_Draw_Font(0,Font_72_Addr,layer3_start_addr,layer4_start_addr,LCD_XSIZE_TFT,72,72,0,Magenta,0,0,0,480,72,0,0,(uint8_t *)"72*72字体楷体",13);
		LT758_BTE_Memory_Copy(layer3_start_addr,LCD_XSIZE_TFT,0,0,layer3_start_addr,LCD_XSIZE_TFT,0,0,layer2_start_addr,LCD_XSIZE_TFT,270,240,0x0A,480,72,24);
		//带背景
		LT758_BTE_Solid_Fill(layer3_start_addr, LCD_XSIZE_TFT, 0, 0, Yellow, 480, 72, 24);
		LT758_Draw_Font(0,Font_72_Addr,layer3_start_addr,layer4_start_addr,LCD_XSIZE_TFT,72,72,0,Blue,0,0,0,480,72,0,0,(uint8_t *)"72*72字体楷体",13);
		LT758_BTE_Memory_Copy(layer3_start_addr,LCD_XSIZE_TFT,0,0,layer3_start_addr,LCD_XSIZE_TFT,0,0,layer2_start_addr,LCD_XSIZE_TFT,270,360,0x0A,480,72,24);
		//恢复窗口
		Canvas_Image_Start_address(layer2_start_addr);
		Canvas_image_width(LCD_XSIZE_TFT);
		Active_Window_XY(0, 0);
		Active_Window_WH(LCD_XSIZE_TFT, LCD_YSIZE_TFT);
		if (Screen_delay_Count(3000))
			return 1;
	}
}

uint8_t function_12(void)
{
	uint8_t flag = 0;
	uint16_t i = 0;
	uint32_t lay1 = layer2_start_addr;
	uint32_t lay2 = layer3_start_addr;
	uint32_t lay3 = layer4_start_addr;
	uint32_t lay4 = layer5_start_addr;
	uint32_t lay5 = layer6_start_addr;
	uint32_t lay6 = lay5 + 252 * 252 * 3;
	uint32_t lay7 = lay6 + 300 * 300 * 3;
	uint32_t lay8 = lay7 + 252 * 252 * 3;

	LT758_DMA_JPG(0, 0, H_JPG_Addr[60]);

	Canvas_Image_Start_address(lay1);
	LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, Red);

	Canvas_Image_Start_address(lay2);
	LT758_DrawSquare_Fill(0, 0, LCD_XSIZE_TFT, LCD_YSIZE_TFT, Blue);

	Canvas_Image_Start_address(lay3);

	Canvas_Image_Start_address(lay4);

	Canvas_image_width(252);
	Canvas_Image_Start_address(lay5);

	Canvas_image_width(300);
	Canvas_Image_Start_address(lay6);

	Canvas_image_width(252);
	Canvas_Image_Start_address(lay7);

	Canvas_image_width(300);
	Canvas_Image_Start_address(lay8);

#define tt 5

	while (1)
	{
		for (i = 0; i < 520; i = i + 4)
		{
			flag++;
			if (flag == 1)
			{
				LT758_BTE_Picture_Alpha_Blending(lay1, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, i, 175,
								   lay5, 252, 0, 0,
								   252, 252, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 1, lay5, 0, 0, 252, i, 175, 252, 252);
				LT758_BTE_Picture_Alpha_Blending(lay2, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 724 - i, 150,
								   lay6, 300, 0, 0,
								   300, 300, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 2, lay6, 0, 0, 300, 724 - i, 150, 300, 300);
			}
			else if (flag == 2)
			{
				flag = 0;
				LT758_BTE_Picture_Alpha_Blending(lay1, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, i, 175,
								   lay7, 252, 0, 0,
								   252, 252, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 1, lay7, 0, 0, 252, i, 175, 252, 252);
				LT758_BTE_Picture_Alpha_Blending(lay2, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 724 - i, 150,
								   lay8, 300, 0, 0,
								   300, 300, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 2, lay8, 0, 0, 300, 724 - i, 150, 300, 300);
			}

			if (Screen_delay_Count(1))
				return 1;
		}

		flag = 0;
		for (i = 0; i < 520; i = i + 4)
		{
			flag++;
			if (flag == 1)
			{
				LT758_BTE_Picture_Alpha_Blending(lay1, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 520 - i, 175,
								   lay5, 252, 0, 0,
								   252, 252, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 1, lay5, 0, 0, 252, 520 - i, 175, 252, 252);
				LT758_BTE_Picture_Alpha_Blending(lay2, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 724 - 500 + i, 150,
								   lay6, 300, 0, 0,
								   300, 300, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 2, lay6, 0, 0, 300, 724 - 500 + i, 150, 300, 300);
			}
			else if (flag == 2)
			{
				flag = 0;
				LT758_BTE_Picture_Alpha_Blending(lay1, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 520 - i, 175,
								   lay7, 252, 0, 0,
								   252, 252, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 1, lay7, 0, 0, 252, 520 - i, 175, 252, 252);

				LT758_BTE_Picture_Alpha_Blending(lay2, LCD_XSIZE_TFT, 0, 0,
								   layer1_start_addr, LCD_XSIZE_TFT, 724 - 500 + i, 150,
								   lay8, 300, 0, 0,
								   300, 300, 16, 24);
				if (Screen_delay_Count(tt))
					return 1;
				LT758_PIP_Init(1, 2, lay8, 0, 0, 300, 724 - 500 + i, 150, 300, 300);
			}

			if (Screen_delay_Count(1))
				return 1;
		}

		if (Screen_delay_Count(1000))
			return 1;

		Canvas_image_width(252);
		Canvas_Image_Start_address(lay5);
		LT758_DMA_JPG(0, 0, H_JPG_Addr[48]);

		Canvas_image_width(300);
		Canvas_Image_Start_address(lay6);
		LT758_DMA_JPG(0, 0, H_JPG_Addr[49]);


		LT758_PIP_Init(1, 1, lay5, 0, 0, 252, 0, 175, 252, 252);
		LT758_PIP_Init(1, 2, lay6, 0, 0, 300, 774, 150, 300, 300);

		for (i = 0; i < 550; i = i + 2)
		{
			LT758_Set_DisWindowPos(1, 1, i, 175);
			LT758_Set_DisWindowPos(1, 2, 734 - i, 150);
			if (Screen_delay_Count(10))
				return 1;
		}

		for (i = 0; i < 550; i = i + 2)
		{
			LT758_Set_DisWindowPos(1, 1, 550 - i, 175);
			LT758_Set_DisWindowPos(1, 2, 734 - 550 + i, 150);
			if (Screen_delay_Count(10))
				return 1;
		}

		if (Screen_delay_Count(1000))
			return 1;
	}
}

#define  BACKGROUND_H	980  // 背景的宽度
#define  BACKGROUND_W	520  // 背景的高度
#define  GRIDS_SIZE   75   // 网格大小
#define  Dot_Num      3    // 波形中点与点之间的点数

#define	 X1	((LCD_XSIZE_TFT-BACKGROUND_H)/2)
#define	 X2	(LCD_XSIZE_TFT-(LCD_XSIZE_TFT-BACKGROUND_H)/2)
#define  Y1 ((LCD_YSIZE_TFT-50-BACKGROUND_W)/2)
#define  Y2 (LCD_YSIZE_TFT-50-(LCD_YSIZE_TFT-60-BACKGROUND_W)/2)


#define TRIANGLE_BUFF_LEN 560
uint16_t gY_TriangleBuff[TRIANGLE_BUFF_LEN] = {0};
uint16_t gY_TriangleTemp[10] = {0};

#define SIN_BUFF_LEN 586  // 610
uint16_t gY_SinBuff_Sin[SIN_BUFF_LEN] = {0};
uint16_t gY_SinTemp[10] = {0};

int8_t gOffset = 1;    // 移动的点数

void Load_TriangleWaveData(void)
{
	uint8_t flag = 0;
	int16_t i = 0, count = 0;
	for(i = 0 ; i < TRIANGLE_BUFF_LEN-1 ; i++)
	{
		if(flag == 0)
		{
			count+=8;
			if(count >= 160)	count = 160,	flag = 1;
		}
		else
		{
			count-=8;
			if(count <= 0)	count = 0,	flag = 0;
		}
		gY_TriangleBuff[i] =  count + 80;
	}
}

void Load_SineWaveData(void)
{
	int16_t i = 0;
	uint8_t A = 120 , flag = 0;;
	
	for(i = 0 ; i < SIN_BUFF_LEN-1 ; i++)
	{
		if(flag == 0)
		{
			if(i%9==0)	A = A - 15;
			if(A <= 15)	flag = 1;
		}
		else
		{
			if(i%9==0)	A = A + 15;
			if(A >= 120)	flag = 0;
		}
		gY_SinBuff_Sin[i]=A*sin(((double)i*3.14159/180)*40)+400;
	}
}


void Move_WaveData(void)
{
	uint16_t i = 0;
	
	// 三角波形数据移动
	for(i = 0 ; i < gOffset ; i++)										gY_TriangleTemp[i] = gY_TriangleBuff[i];
	for(i = 0 ; i < TRIANGLE_BUFF_LEN-gOffset ; i++)	gY_TriangleBuff[i] = gY_TriangleBuff[i+gOffset];
	for(i = 0 ; i < gOffset ; i++)										gY_TriangleBuff[TRIANGLE_BUFF_LEN-gOffset+i-1] = gY_TriangleTemp[i];
	// 正弦波形数据移动
	for(i = 0 ; i < gOffset ; i++)							gY_SinTemp[i] = gY_SinBuff_Sin[i];
	for(i = 0 ; i < SIN_BUFF_LEN-gOffset ; i++)	gY_SinBuff_Sin[i] = gY_SinBuff_Sin[i+gOffset];
	for(i = 0 ; i < gOffset ; i++)							gY_SinBuff_Sin[SIN_BUFF_LEN-gOffset+i-1] = gY_SinTemp[i];
}

void LT758_DrawLine1
(
 uint16_t X_1        // X1坐标
,uint16_t Y_1        // Y1坐标
,uint16_t X_2        // X2坐标
,uint16_t Y_2        // Y2坐标
)
{
	Line_Start_XY(X_1,Y_1);
	Line_End_XY(X_2,Y_2);
	Start_Line();
	Check_2D_Busy();
}

void Drawing_TriangleWave(uint32_t color)
{
	uint16_t i = 0;
	
	Foreground_color_16M(color);
	for(i = 0 ; i < BACKGROUND_H/Dot_Num ; i++)
	{
		LT758_DrawLine1(X1+Dot_Num*i,gY_TriangleBuff[i],X1+Dot_Num*(i+1),gY_TriangleBuff[i+1]);
	}
}

void Drawing_SineWaveData(uint32_t color)
{
	uint16_t i = 0;
	
	Foreground_color_16M(color);
	for(i = 0 ; i < BACKGROUND_H/Dot_Num ; i++)
	{
		LT758_DrawLine1(X1+Dot_Num*i,gY_SinBuff_Sin[i],X1+Dot_Num*(i+1),gY_SinBuff_Sin[i+1]);
	}
}

uint8_t function_13(void)
{
	char buf[3] = {0};
	uint8_t flag = 0;
	uint8_t return_flag = 0;
	int16_t i = 0;
	//uint16_t temp = 0;
	uint8_t touch = 0;
  
	Load_TriangleWaveData();	// 装载三角波形数据
	Load_SineWaveData();			// 装载正弦波形数据

	
	sprintf(buf,"%2d",gOffset);
	LT758_Select_Outside_Font_Init(1, 0, Font_32_Addr, layer7_start_addr, Font_32_Size, 32, 1, 1, 0, 0);

	Active_Window_XY(0,0);
	Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);       //设置工作窗口大小
	Main_Window_Start_XY(0,0);				                   //设置主窗口坐标
	Canvas_image_width(LCD_XSIZE_TFT);				           //画布图像宽度
	Canvas_Image_Start_address(layer2_start_addr);	     //Layer 2;画布图像起始地址
	LT758_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Blue2);
	LT758_DrawSquare_Width(X1,Y1,X2,Y2,Green,Black,5);
	Foreground_color_16M(color16M_cyan);
	LT758_DrawSquare_Fill(615,550,810,592,Black);
	LT758_Print_Outside_Font_String(620,555,Red,Black,"移动点数：");
	LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
	LT758_Print_Outside_Font_String(100,555,Red,Black,"Button1：加1");
	LT758_Print_Outside_Font_String(350,555,Red,Black,"Button2：减1");

	Main_Image_Start_Address(layer2_start_addr);			   //设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);
	
	Canvas_Image_Start_address(layer3_start_addr);	     //Layer 3;画布图像起始地址
	LT758_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Blue2);
	LT758_DrawSquare_Width(X1,Y1,X2,Y2,Green,Black,5);	
	LT758_DrawSquare_Fill(615,550,810,592,Black);
	LT758_Print_Outside_Font_String(620,555,Red,Black,"移动点数：");
	LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
	LT758_Print_Outside_Font_String(100,555,Red,Black,"Button1：加1");
	LT758_Print_Outside_Font_String(350,555,Red,Black,"Button2：减1");

	Enable_Vsync_Interrupt();
	
	while(1)
	{
    tp_dev.scan(0);
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}    
     
		if(touch_state == 1 && press == 0)
		{
			press = 1;
      X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
      Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
      
			if ((X_Pos < X2) && (X_Pos > X1) && (Y_Pos >= Y1) && (Y_Pos <= Y2))
			{
				return_flag = 1;
			}
			if ((X_Pos < 300) && (Y_Pos >= Y2))
			{
				gOffset+=1;
				if(gOffset>= 10)	gOffset = 10;
				for(i = 0 ; i < 3 ; i++)	buf[i] = 0;
				sprintf(buf,"%2d",gOffset);
				Canvas_Image_Start_address(layer2_start_addr);	
				LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
				Canvas_Image_Start_address(layer3_start_addr);	
				LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
			}
			if ((X_Pos > 350) && (X_Pos < 600) && (Y_Pos >= Y2))
			{
				gOffset--;
				if(gOffset<=0)	gOffset = 0;
				for(i = 0 ; i < 3 ; i++)	buf[i] = 0;
				sprintf(buf,"%2d",gOffset);
				Canvas_Image_Start_address(layer2_start_addr);	
				LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
				Canvas_Image_Start_address(layer3_start_addr);	
				LT758_Print_Internal_Font_String(770,555,Red,Black,buf);
			}
		}
		if(touch_state == 0 && press == 1)
		{
			press = 0;
			if(return_flag)
			{
				Disable_Vsync_Interrupt();
				return 1;
			}
		}
		Move_WaveData();
		if(flag == 0)
		{
			Clear_Vsync_Interrupt_Flag();
			while((Read_Interrupt_status()&0x10)==0){};	
			Canvas_Image_Start_address(layer2_start_addr);	
			// 清数据
			LT758_DrawSquare_Width(X1,Y1,X2,Y2,Green,Black,5);
			// 画网格
			Foreground_color_16M(color16M_cyan);
			for(i = 0 ; i < BACKGROUND_W/GRIDS_SIZE ; i++)	LT758_DrawLine1(X1,Y1+GRIDS_SIZE*(i+1),X2,Y1+GRIDS_SIZE*(i+1));
			for(i = 0 ; i < BACKGROUND_H/GRIDS_SIZE ; i++)	LT758_DrawLine1(X1+GRIDS_SIZE*(i+1),Y1,X1+GRIDS_SIZE*(i+1),Y2);
			// 画波形
			Drawing_TriangleWave(Green);
			Drawing_SineWaveData(Yellow);
			
			Main_Image_Start_Address(layer2_start_addr);
				
			flag = 1;
		}

		else
		{
			Clear_Vsync_Interrupt_Flag();
			while((Read_Interrupt_status()&0x10)==0){};	
			Canvas_Image_Start_address(layer3_start_addr);	 	
			// 清数据
			LT758_DrawSquare_Width(X1,Y1,X2,Y2,Green,Black,5);
			// 画网格
			Foreground_color_16M(color16M_cyan);
			for(i = 0 ; i < BACKGROUND_W/GRIDS_SIZE ; i++)	LT758_DrawLine1(X1,Y1+GRIDS_SIZE*(i+1),X2,Y1+GRIDS_SIZE*(i+1));
			for(i = 0 ; i < BACKGROUND_H/GRIDS_SIZE ; i++)	LT758_DrawLine1(X1+GRIDS_SIZE*(i+1),Y1,X1+GRIDS_SIZE*(i+1),Y2);
			// 画波形
			Drawing_TriangleWave(Green);
			Drawing_SineWaveData(Yellow);
			
			Main_Image_Start_Address(layer3_start_addr);

			flag = 0;
		}
	}
}

uint8_t function_14(void)
{
	uint16_t i;

	Main_Window_Start_XY(0,0);

	Canvas_Image_Start_address(layer3_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	Active_Window_XY(0,0);
	Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);

	LT758_DMA_JPG(0, LCD_YSIZE_TFT*0, H_JPG_Addr[57]);
	LT758_DMA_JPG(0, LCD_YSIZE_TFT*1, H_JPG_Addr[58]);

	Canvas_Image_Start_address(layer2_start_addr);
	LT758_DrawSquare_Fill(0,0,LCD_XSIZE_TFT,LCD_YSIZE_TFT,Black);

	Main_Image_Start_Address(layer2_start_addr);
	Main_Image_Width(LCD_XSIZE_TFT);

	while(1)
	{
		for(i=0;i<32;i++)
		{
			LT758_BTE_Picture_Alpha_Blending(LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*2,1024,0,0,
								LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*3,1024,0,0,
								LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*1,1024,0,0,
								LCD_XSIZE_TFT,LCD_YSIZE_TFT,i, 24);

			if(Screen_delay_Count(100))	return 1;
		}
		if(Screen_delay_Count(2500))	return 1;

		for(i=32;i>0;i--)
		{
			LT758_BTE_Picture_Alpha_Blending(LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*2,1024,0,0,
								LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*3,1024,0,0,
								LCD_XSIZE_TFT*LCD_YSIZE_TFT*3*1,1024,0,0,
								LCD_XSIZE_TFT,LCD_YSIZE_TFT,i, 24);
			if(Screen_delay_Count(100))	return 1;
		}
		if(Screen_delay_Count(2500))	return 1;
	}
}


// #define FanW 228
// #define FanH 228
#define DLTIME 60

uint32_t color_16_to_24(uint16_t color_16)
{
	uint8_t rr,gg,bb;
	uint32_t color_24;
	rr = (color_16>>8)&0xF8;
	gg = (color_16>>3)&0xFC;
	bb = color_16&0x1F;
	bb = (bb<<3);
	color_24 = (rr<<16)+(gg<<8)+bb;
	return color_24;
}

void Show_Temperature(int val)
{
	int len,aLen;
	uint16_t rV,bV,cV;

	len = 559 - 367;
	aLen = (len*val)/100;

	LT758_DrawSquare_Fill(920,367,944,559 - aLen,color_16_to_24(0xDEDB));

	rV = val/3;
	if(rV>31) rV = 31;

	bV = 31- rV;

	cV = ((rV<<11)&0xF800)|((bV<<5)&0x07E0);

	LT758_DrawSquare_Fill(920,559 - aLen,944,559,color_16_to_24(cV));
}

void Show_Progress1(int val)
{
	int i;

	for(i=0;i<val;i++)
	{
		LT758_DrawSquare_Fill(44+i*29 ,90,64+i*29,129,color_16_to_24(0xFCCD));
	}

	for(i=val;i<10;i++)
	{
		LT758_DrawSquare_Fill(44+i*29 ,90,64+i*29,129,color_16_to_24(0x5ACB));
	}
}

void Show_Progress2(int val)
{
	int i;

	for(i=0;i<val;i++)
	{
		LT758_DrawSquare_Fill(44+i*29,211,64+i*29,250,color_16_to_24(0xFFE0));
	}

	for(i=val;i<10;i++)
	{
		LT758_DrawSquare_Fill(44+i*29,211,64+i*29,250,color_16_to_24(0x5ACB));
	}
}

void Button_ON_OFF(int val)
{
	if(val)
	{
		// 开关亮
		LT758_DrawCircle_Fill(112,463,40,color_16_to_24(0xF800));  // 硬件画圆
		LT758_BTE_Memory_Copy(layer4_start_addr,88,0,0,layer4_start_addr,88,0,0,layer2_start_addr,LCD_XSIZE_TFT,68,525,0x0c,88,62,24);
	}
	else
	{
		// 开关暗
		LT758_DrawCircle_Fill(112,463,40,color_16_to_24(0x3800));  // 硬件画圆
		LT758_BTE_Memory_Copy(layer4_start_addr,88,0,62,layer4_start_addr,88,0,0,layer2_start_addr,LCD_XSIZE_TFT,68,525,0x0c,88,62,24);
	}
}

void Show_Text(int index)
{
	CGROM_Select_Internal_CGROM();
	Foreground_color_16M(color_16_to_24(0x001F));
	Background_color_16M(color_16_to_24(0xF79E));
	Font_Select_8x16_16x16();
	Goto_Text_XY(689+3,60+index*20);
	switch(index)
	{
		case 1: Show_String("AM07:50 Level=10"); break;
		case 2: Show_String("AM07:50 Level=20"); break;
		case 3: Show_String("AM07:50 Level=30"); break;
		case 4: Show_String("AM07:50 Level=40"); break;
		case 5: Show_String("AM07:50 Level=50"); break;
		case 6: Show_String("AM07:50 Level=60"); break;
		case 7: Show_String("AM07:50 Level=70"); break;
		case 8: Show_String("AM07:50 Level=80"); break;
		case 9: Show_String("AM07:50 Level=90"); break;
		default: break;
	}
}

void TEST_DoubleTriangle(uint16_t x,uint16_t y,uint16_t h,uint16_t h1,uint16_t l,uint16_t a,uint32_t color1,uint32_t color2)
{	
	float cos_x = 0;
	float sin_y = 0;
	
	uint16_t x1,y1,x2,y2,x3,y3,x4,y4;
	
	if(a<=90)     // 1
	{
		cos_x = cos(a*(3.1415926/180));
		sin_y = sin(a*(3.1415926/180));
		x1 = x - h*cos_x;
		y1 = y - h*sin_y;
		
		x2 = x - l*sin_y;
		y2 = y + l*cos_x;
		
		x3 = x + l*sin_y;
		y3 = y - l*cos_x;
		
		x4 = x + h1*cos_x;
		y4 = y + h1*sin_y;
	}
	
	else if((a>90)&&(a<180))  // 2
	{
		a = 180 - a;
		cos_x = cos(a*(3.1415926/180));
		sin_y = sin(a*(3.1415926/180));
		
		x1 = x + h*cos_x;
		y1 = y - h*sin_y;

		x2 = x - l*sin_y;
		y2 = y - l*cos_x;
		
		x3 = x + l*sin_y;
		y3 = y + l*cos_x;
		
		x4 = x - h1*cos_x;
		y4 = y + h1*sin_y;
	}
	 
	else if((a>=180)&&(a<270))  // 3
	{
		a = a - 180;
		cos_x = cos(a*(3.1415926/180));
		sin_y = sin(a*(3.1415926/180));
		
		x1 = x + h*cos_x;
		y1 = y + h*sin_y;

		x2 = x + l*sin_y;
		y2 = y - l*cos_x;
		
		x3 = x - l*sin_y;
		y3 = y + l*cos_x;
		
		x4 = x - h1*cos_x;
		y4 = y - h1*sin_y;
	}
	
	else if((a>=270)&&(a<360))  // 4
	{
		a = 360 - a;
		cos_x = cos(a*(3.1415926/180));
		sin_y = sin(a*(3.1415926/180));
		
		x1 = x - h*cos_x;
		y1 = y + h*sin_y;

		x2 = x + l*sin_y;
		y2 = y + l*cos_x;
		
		x3 = x - l*sin_y;
		y3 = y - l*cos_x;
		
		x4 = x + h1*cos_x;
		y4 = y - h1*sin_y;
	}
	
	LT758_DrawTriangle_Fill(x1,y1,x2,y2,x4,y4,color1);
	LT758_DrawTriangle_Fill(x1,y1,x3,y3,x4,y4,color2);
	
	LT758_DrawCircle_Fill(x,y,3,Red);
}

void Pointer_Show(uint16_t a)
{
	uint16_t a1 = 0;
	uint16_t a2 = 0;
	
	if(a < 13)
	{
		a1 = 320 + 3 * a;
	}
	else
	{
		a1 = (a - 13) * 3;
	}
	
	a2 = (a * 70) / 90 + 12;
	
	Canvas_Image_Start_address(layer6_start_addr);//
	Canvas_image_width(580);
	
	LT758_BTE_Memory_Copy(layer5_start_addr,LCD_XSIZE_TFT,240,310,
						  layer5_start_addr,LCD_XSIZE_TFT,240,310,
						  layer6_start_addr,580,0,0,
						  0x0c,580,250,24);

	TEST_DoubleTriangle(136,140,70,15,7,a1,color_16_to_24(0x001f),color_16_to_24(0x031f)); 
			
	TEST_DoubleTriangle(433,165,65,15,7,2*a2,color_16_to_24(0x001f),color_16_to_24(0x031f)); 
			
	LT758_BTE_Memory_Copy(layer6_start_addr,580,0,0,
						  layer6_start_addr,580,0,0,
						  layer2_start_addr,LCD_XSIZE_TFT,240,310,
						  0x0c,580,250,24);
	
	Canvas_Image_Start_address(layer2_start_addr);//
	Canvas_image_width(LCD_XSIZE_TFT);
}

void function_15(void)
{
	int  fanIndex = 0,proIndex1 = 0, proIndex2 = 0,tempIndex = 0;
	int  totalIndex = 1 , on_off_index = 1;
	int  textIndex = 0;

	Canvas_Image_Start_address(layer2_start_addr);	     //Layer 2;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);				           //画布图像宽度
	Active_Window_XY(0,0);
	Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);       //设置工作窗口大小
	Main_Window_Start_XY(0,0);				                   //设置主窗口坐标
	LT758_DMA_JPG(0, 0, H_JPG_Addr[4]);
	
	Main_Image_Start_Address(layer2_start_addr);			   //设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);

	//从Flash读取ON/OFF的数据
	Canvas_Image_Start_address(layer4_start_addr);//
	Canvas_image_width(88);//ON图片+OFF图片，共2张
	LT758_DMA_JPG(0, 60*0, H_JPG_Addr[12]);
	LT758_DMA_JPG(0, 60*1, H_JPG_Addr[11]);
	
	Canvas_Image_Start_address(layer5_start_addr);//
	Canvas_image_width(LCD_XSIZE_TFT);
	LT758_DMA_JPG(0, 0, H_JPG_Addr[4]);
	
	Canvas_Image_Start_address(layer2_start_addr);				//Layer 1
	Canvas_image_width(LCD_XSIZE_TFT);//
	Active_Window_XY(0,0);

	//Progress
	Show_Progress1(proIndex1);   //初始化进度条1
	Show_Progress2(proIndex2);   //初始化进度条2

	//Temperature
	Show_Temperature(tempIndex); //初始化温度条

	//POWER Button
	LT758_DrawCircle(112,463,50,color_16_to_24(0x8DBC)); // 硬件画圆

	Button_ON_OFF(on_off_index);

	LT758_Select_Internal_Font_Init(16,1,1,0,0);
	while(1)
	{
		LT758_DMA_JPG(430, 54, H_JPG_Addr[65+fanIndex]);
		
		fanIndex++;
		if(fanIndex>12) fanIndex = 0;
		totalIndex++;

		textIndex = totalIndex/10;	//文字显示
		if(textIndex==0)			      //清空文字显示区
		{
			LT758_DrawSquare_Fill(689,71,977,265,color_16_to_24(0xF79E));  // 硬件画矩形
		}
		else Show_Text(textIndex);//增加一行文字

		Show_Progress1(totalIndex/10);	//进度条1

		proIndex2++;
		if(proIndex2>18) proIndex2 = 0;

		Show_Progress2(proIndex2 /2);//进度条2

		//Temperature
		Show_Temperature(totalIndex);//温度显示
		
		Pointer_Show(totalIndex);

		if(totalIndex>=100)
		{
			 totalIndex = 1;
			 if(on_off_index==1) on_off_index = 0;
			 else on_off_index = 1;
			 Button_ON_OFF(on_off_index);//ON-OFF开关，指针，圆点
		}

		if(Screen_delay_Count(30))	break;//等待按键按下退出
	}
}

uint8_t function_16(void)
{
	uint8_t temp = 0,temp1 = 0;
	temp = 0;
	temp1 = 50;
	LT758_DMA_JPG(0, 0, H_JPG_Addr[56]);
	LT758_DMA_JPG(96, 100, H_JPG_Addr[temp1]);
	temp1 = 51;
	while (1)
	{
		LT758_DMA_JPG(512, 100, GIF_JPG_Addr[temp]+JPG_SIZE);//modify
		temp++;
		if(temp == 60)
		{
			LT758_DMA_JPG(96, 100, H_JPG_Addr[temp1]);
			temp1++;
			if(temp1>55)temp1 = 50;
		}
		if(temp > 119)
		{
			temp = 0;
			LT758_DMA_JPG(96, 100, H_JPG_Addr[temp1]);
			temp1++;
			if(temp1>55)temp1 = 50;
		}
		if(Screen_delay_Count(30))	break;
	}
	return 0;
}

uint8_t function_17(void)
{
	uint8_t lineWidth = 1;
	uint32_t color = 0;
	
	uint8_t first = 0;
	uint16_t x,y;
	char x_buf[4] = {0};
	char y_buf[4] = {0};
	uint8_t touch = 0;
	
	Main_Window_Start_XY(0,0);				                   //设置主窗口坐标
	Canvas_Image_Start_address(layer2_start_addr);	     //Layer 1;画布图像起始地址
	Canvas_image_width(LCD_XSIZE_TFT);				           //画布图像宽度
	Active_Window_XY(0,0);
	Active_Window_WH(LCD_XSIZE_TFT,LCD_YSIZE_TFT);	     //设置工作窗口大小
	
	LT758_DrawSquare_Fill(0,0,1024,600,Black);
	
	LT758_Select_Internal_Font_Init(24,1,1,0,0);

	LT758_Select_Outside_Font_Init(1, 0, Font_24_Addr, layer7_start_addr, Font_24_Size, 24, 1, 1, 0, 0);
	
	LT758_Print_Internal_Font_String(400,570,Red,Black,"X_Pos:");
	LT758_Print_Internal_Font_String(550,570,Red,Black,"Y_Pos:");
	LT758_DrawSquare_Fill(470,570,525,600,White);
	LT758_DrawSquare_Fill(620,570,675,600,White);
	LT758_Print_Internal_Font_String(475,570,Red,White,"0");
	LT758_Print_Internal_Font_String(625,570,Red,White,"0");
	
	//-----------------宽度-----------------
	LT758_DrawSquare_Fill(980,90,1024,140,White);
	LT758_DrawLine_Width(985,115,1020,115,Red,1);
	
	LT758_DrawSquare_Fill(980,180,1024,230,White);
	LT758_DrawLine_Width(985,204,1020,204,Red,3);
	
	LT758_DrawSquare_Fill(980,280,1024,330,White);
	LT758_DrawLine_Width(985,303,1020,303,Red,5);

	LT758_DrawSquare_Fill(980,380,1024,430,White);
	LT758_DrawLine_Width(985,402,1020,402,Red,7);
	
	LT758_DrawSquare_Fill(980,480,1024,530,White);
	LT758_DrawLine_Width(985,501,1020,501,Red,9);
	
	
	//-----------------颜色-----------------
	LT758_DrawSquare_Fill(50 ,0,100,45,Red);
	LT758_DrawSquare_Fill(150,0,200,45,Green);
	LT758_DrawSquare_Fill(250,0,300,45,Blue);
	LT758_DrawSquare_Fill(350,0,400,45,Magenta);
	LT758_DrawSquare_Fill(450,0,500,45,Grey-0x4321);
	LT758_DrawSquare_Fill(550,0,600,45,Blue2);
	LT758_DrawSquare_Fill(650,0,700,45,Yellow);
	
	LT758_DrawSquare_Fill(750,0,800,45,White);
	LT758_DrawSquare_Fill(755,5,795,40,Black);
	
	//-------------所选的宽度及颜色----------
	LT758_DrawSquare_Fill(0,275,45,325,White);
	LT758_DrawLine_Width(5,298,40,298,color,lineWidth);
	
	LT758_DrawSquare_Fill(50,50,975,565,White);
	LT758_Graphic_cursor_Init(1,0xff,0x00,0,0,(uint8_t*)gImage_pen_il);
	
	LT758_DrawSquare_Fill(860,0,925,45,0x808080);//modify
	LT758_Print_Outside_Font_String(870,10,Red,0x808080,"清除");//modify
	LT758_DrawSquare_Fill(950,0,1015,45,0x808080);//modify
	LT758_Print_Outside_Font_String(960,10,Red,0x808080,"返回");//modify
	
	Main_Image_Start_Address(layer2_start_addr);			   //设置主图像起始地址
	Main_Image_Width(LCD_XSIZE_TFT);
	
	while(1)
	{
    tp_dev.scan(0);
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}  
    
		if(touch_state == 1)
		{
			press = 1;
      X_Pos = tp_dev.x[0];           /* 记录当前坐标 */
      Y_Pos = tp_dev.y[0];           /* 记录当前坐标 */
      
			if(first == 0)
			{
				x = X_Pos;
				y = Y_Pos;
				first = 1;
			}
			LT758_Set_Graphic_cursor_Pos(1,X_Pos,Y_Pos);
			
			if((x>50)&&(x<960)&&(y>55)&&(y<555))
				LT758_DrawQuadrilateral_Fill(x,y+lineWidth,x+lineWidth,y,X_Pos+lineWidth,Y_Pos,X_Pos,Y_Pos+lineWidth,color);

			sprintf(x_buf, "%d", X_Pos);
			sprintf(y_buf, "%d", Y_Pos);
			LT758_DrawSquare_Fill(470,575,525,600,White);
			LT758_DrawSquare_Fill(620,575,675,600,White);
			LT758_Print_Internal_Font_String(475,570,Red,White,x_buf);
			LT758_Print_Internal_Font_String(625,570,Red,White,y_buf);
			
			x = X_Pos;
			y = Y_Pos;
			
			// 清除
			if((X_Pos<925)&&(X_Pos>860)&&(Y_Pos<=50))
			{
				LT758_DrawSquare_Fill(50,50,975,565,White);
			}
			
			// 线宽
			if((X_Pos>980)&&(X_Pos<=1024))
			{
				if ((Y_Pos>90)&&(Y_Pos<140))		lineWidth = 1;
				else if ((Y_Pos>180)&&(Y_Pos<230))	lineWidth = 3;
				else if ((Y_Pos>280)&&(Y_Pos<330))	lineWidth = 5;
				else if ((Y_Pos>380)&&(Y_Pos<430))	lineWidth = 7;
				else if ((Y_Pos>480)&&(Y_Pos<530))	lineWidth = 9;
				
				LT758_DrawSquare_Fill(0,275,45,325,White);
				LT758_DrawLine_Width(5,298,40,298,color,lineWidth);
			}
			
			// 颜色
			if(Y_Pos<=45)
			{
				if ((X_Pos>50)&&(X_Pos<100))		color = Red;
				else if ((X_Pos>150)&&(X_Pos<200))	color = Green;
				else if ((X_Pos>250)&&(X_Pos<300))	color = Blue;
				else if ((X_Pos>350)&&(X_Pos<400))	color = Magenta;
				else if ((X_Pos>450)&&(X_Pos<500))	color = Grey-0x4321;
				else if ((X_Pos>550)&&(X_Pos<600))	color = Blue2;
				else if ((X_Pos>560)&&(X_Pos<700))	color = Yellow;
				else if ((X_Pos>650)&&(X_Pos<800))	color = Black;
				
				LT758_DrawSquare_Fill(0,275,45,325,White);
				LT758_DrawLine_Width(5,298,40,298,color,lineWidth);
			}
		}
		if(touch_state == 0 && press == 1)
		{
			press = 0;
			first = 0;
			// 返回
			if((X_Pos<1024)&&(X_Pos>950)&&(Y_Pos<50))
			{
				LT758_Disable_Graphic_Cursor();
				break;
			}
		}
	}
	return 0;
}

uint8_t function_18(void)
{
  #if USE_SPI_FLASH_CS0
	uint8_t temp = 0;
	Select_SFI_0();
	Select_nSS_drive_on_xnsfcs0();
	while (1)
	{
		temp++;
		if(temp > 54)
		{
			temp = 0;
		}
		LT758_DMA_JPG(0, 0, KJ_JPG_Addr[temp]);
		if(Screen_delay_Count(10)) break;
	}
	Select_SFI_1();
	Select_nSS_drive_on_xnsfcs1();
  #endif
  
	return 0;
}

uint8_t function_19(void)
{
	main_menu();
	return 0;
}

uint8_t function_21(void)
{
  #if USE_SPI_FLASH_CS0
	uint16_t temp = 0;
	Canvas_Image_Start_address(layer1_start_addr);
	Canvas_image_width(LCD_XSIZE_TFT);
	Select_SFI_0();
	Select_nSS_drive_on_xnsfcs0();
	for(temp=0;temp<113;temp++)
	{
		LT758_DMA_JPG(0, 0, KJ_JPG_Addr[temp]);
		if(Screen_delay_Count(30)) break;
	}
	Select_SFI_1();
	Select_nSS_drive_on_xnsfcs1();
  #endif
  
	main_menu();
	return 0;
}

void function_main(void)
{
  uint8_t t = 0;
	uint16_t temp = 0;
	uint8_t touch = 0;

	if(Flash_Type == 0)//nor flash使能Quad
	{
		#if USE_SPI_FLASH_CS0
		LT758_SPI_Init(0,1);
		LT758_W25QXX_Quad_Enable();
		#endif
		LT758_SPI_Init(1,1);
		LT758_W25QXX_Quad_Enable();
	}
	
	//需要用到SPI_FLASH_CS0时，打开
	#if USE_SPI_FLASH_CS0
	Select_SFI_0();
	Select_nSS_drive_on_xnsfcs0();
	Canvas_Image_Start_address(layer1_start_addr);
	for(temp=0;temp<113;temp++)
	{
		LT758_DMA_JPG(0, 0, KJ_JPG_Addr[temp]);
		if(Screen_delay_Count(30)) break;
	}
	Select_SFI_1();
	Select_nSS_drive_on_xnsfcs1();
	#endif
  
	UNUSED(temp);
	main_menu();//首页图标滚动
  timx_int_init(10 - 1, 24000 - 1);  /* 初始化1ms定时器,设置10KHz的计数频率,100us*10=1ms */

	while (1)
	{
    t++;
    delay_ms(10);
    tp_dev.scan(0);
   
    if (t == 50)
    {
        t = 0;
        LED0_TOGGLE();
    }
    
    if (tp_dev.sta & TP_PRES_DOWN)   /* 有触摸按下 */ 
    {
      ctp_active_index = 1;
      touch = 1;
    }
    else if(touch == 1)              /* 之前有按下，并且松开了 */
    {
      touch_state = 0;
      touch = 0;
    }
    
		if (ctp_active_index)
		{
			ctp_active_index = 0;
      touch_state = tp_dev.sta & 0x01;
//			TP_read_XY();
		}
    
		if (touch_state == 1)
		{
			press = 1;
			Judge_Pos_touch();//扫描坐标判断当前按下哪个功能
		}
		if (press == 1 && touch_state == 0)
		{
			press = 0;
			if (Point > 0)//松手后根据Point的值判断进入哪个功能
			{
				enter_flag = 1;
				enter_Point = Point;
			}
			// printf("Point=%d\r\n", Point);
			Point = 0;
		}
		if (enter_flag)
		{
			enter_flag = 0;
			if (enter_Point == 1)
			{
				function_1();//进入功能1
			}
			else if (enter_Point == 2)
			{
				function_2();//进入功能2
			}
			else if (enter_Point == 3)
			{
				function_3();//进入功能3
			}
			else if (enter_Point == 4)
			{
				function_4();//进入功能4
			}
			else if (enter_Point == 5)
			{
				function_5();//进入功能5
			}
			else if (enter_Point == 6)
			{
				function_6();//进入功能6
			}
			else if (enter_Point == 7)
			{
				function_7();//进入功能7
			}
			else if (enter_Point == 8)
			{
				function_8();//进入功能8
			}
			else if (enter_Point == 9)
			{
				function_9();//进入功能9
			}
			else if (enter_Point == 20)
			{
				function_20();//进入功能20
			}
			else if (enter_Point == 10)
			{
				function_10();//进入功能10
				LT758_Disable_Text_Cursor();
				LT758_Disable_Graphic_Cursor();
			}
			else if (enter_Point == 11)
			{
				function_11();//进入功能11
			}
			else if (enter_Point == 12)
			{
				function_12();//进入功能12
				Disable_PIP1();//退出后关闭PIP功能
				Disable_PIP2();
			}
			else if (enter_Point == 13)
			{
				function_13();//进入功能13
			}
			else if (enter_Point == 14)
			{
				function_14();//进入功能14
			}
			else if (enter_Point == 15)
			{
				function_15();//进入功能15
			}
			else if (enter_Point == 16)
			{
				function_16();//进入功能16
			}
			else if (enter_Point == 17)
			{
				function_17();//进入功能17
			}
			else if (enter_Point == 18)
			{
				function_18();//进入功能18
			}
			else if (enter_Point == 19)
			{
				function_19();//进入功能19
			}
			else if (enter_Point == 21)
			{
				function_21();//进入功能21
			}
			enter_Point = 0;
			display_menu();//退出后显示功能选择界面
		}
	}
}
