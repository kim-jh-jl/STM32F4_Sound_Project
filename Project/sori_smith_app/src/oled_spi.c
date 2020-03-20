//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//
//  UG-2864KSWMG01 (2864-AGE) Reference Code
//
//    Dot Matrix: 128*64
//    Driver IC : SSD1309 (Solomon Systech)
//    Interface : I2C
//    Revision  :
//    Date      : 2012/10/08
//    Author    :
//    Editor    : Humphrey Lin
//

//
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

#include "main.h"
#include "lcd_fonts.h"
#include "lcd.h"
#include "defines.h"
#include "tm_stm32f4_spi.h"


#define VER20
#ifdef VER20
#define LCD_C_D_PORT		GPIO_E
#else
#endif

#ifdef FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#define delay_ms(x)		vTaskDelay( x );  
#else
#define delay_ms(x)		delay( x );  
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (10000 * nCount); index != 0; index--)
  {
  }
}
#endif





uint8_t xcharacter, ycharacter;			// x character(0-3), y character(0-15)
uint8_t cursor_flag, cursorx, cursory;	// x and y cursor position(0-3, 0-15)
uint8_t Korean_buffer[32];				// 32 byte Korean font buffer


#define I2C_SLAVE_OLED     0x78 // 

#define LCD_DEMO

#define Timed(x) Timeout = 0xFFFF; while (x) { if (Timeout-- == 0) goto errReturn;}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Patterns
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

u8 Write_Data(uint8_t Data){
	vu32 Timeout = 0;
	
  Timed(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
	GPIO_SetBits(LCD_D_C_GPIO_PORT,LCD_D_C_PIN);
#if 1
		TM_SPI_Send(SPI1, Data);
#else
	 SPI_I2S_SendData(SPI1, Data);//puts config register into continious measurement mode.
#endif
	
	Timed(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
	 return SUCCESS;
	
	errReturn:
 // Any cleanup here
  return ERROR;		

}

u8 Write_Command (u8 command)
{
		vu32 Timeout = 0;
		Timed(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_BSY));
		GPIO_ResetBits(LCD_D_C_GPIO_PORT,LCD_D_C_PIN);
#if 1
		TM_SPI_Send(SPI1, command);
#else
	  SPI_I2S_SendData(SPI1, command);//puts config register into continious measurement mode.
#endif
	  Timed(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));
	  GPIO_SetBits(LCD_D_C_GPIO_PORT,LCD_D_C_PIN);  
	return SUCCESS;
	
	errReturn:
 // Any cleanup here
  return ERROR;		
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Start_Column(unsigned char d)
{
	Write_Command(0x00+d%16);		// Set Lower Column Start Address for Page Addressing Mode
						//   Default => 0x00
	Write_Command(0x10+d/16);		// Set Higher Column Start Address for Page Addressing Mode
						//   Default => 0x10
}


void Set_Addressing_Mode(unsigned char d)
{
	Write_Command(0x20);			// Set Memory Addressing Mode
	Write_Command(d);			//   Default => 0x02
						//     0x00 => Horizontal Addressing Mode
						//     0x01 => Vertical Addressing Mode
						//     0x02 => Page Addressing Mode
}


void Set_Column_Address(unsigned char a, unsigned char b)
{
	Write_Command(0x21);			// Set Column Address
	Write_Command(a);			//   Default => 0x00 (Column Start Address)
	Write_Command(b);			//   Default => 0x7F (Column End Address)
}


void Set_Page_Address(unsigned char a, unsigned char b)
{
	Write_Command(0x22);			// Set Page Address
	Write_Command(a);			//   Default => 0x00 (Page Start Address)
	Write_Command(b);			//   Default => 0x07 (Page End Address)
}


void Set_Start_Line(unsigned char d)
{
	Write_Command(0x40|d);			// Set Display Start Line
						//   Default => 0x40 (0x00)
}


void Set_Contrast_Control(unsigned char d)
{
	Write_Command(0x81);			// Set Contrast Control for Bank 0
	Write_Command(d);			//   Default => 0x7F
}


void Set_Segment_Remap(unsigned char d)
{
	Write_Command(d);			// Set Segment Re-Map
						//   Default => 0xA0
						//     0xA0 => Column Address 0 Mapped to SEG0
						//     0xA1 => Column Address 0 Mapped to SEG127
}


void Set_Entire_Display(unsigned char d)
{
	Write_Command(d);			// Set Entire Display On / Off
						//   Default => 0xA4
						//     0xA4 => Normal Display
						//     0xA5 => Entire Display On
}


void Set_Inverse_Display(unsigned char d)
{
	Write_Command(d);			// Set Inverse Display On/Off
						//   Default => 0xA6
						//     0xA6 => Normal Display
						//     0xA7 => Inverse Display On
}


void Set_Multiplex_Ratio(unsigned char d)
{
	Write_Command(0xA8);			// Set Multiplex Ratio
	Write_Command(d);			//   Default => 0x3F (1/64 Duty)
}


void Set_Display_On_Off(unsigned char d)	
{
	Write_Command(d);			// Set Display On/Off
						//   Default => 0xAE
						//     0xAE => Display Off
						//     0xAF => Display On
}


void Set_Start_Page(unsigned char d)
{
	Write_Command(0xB0|d);			// Set Page Start Address for Page Addressing Mode
						//   Default => 0xB0 (0x00)
}


void Set_Common_Remap(unsigned char d)
{
	Write_Command(d);			// Set COM Output Scan Direction
						//   Default => 0xC0
						//     0xC0 => Scan from COM0 to 63
						//     0xC8 => Scan from COM63 to 0
}


void Set_Display_Offset(unsigned char d)
{
	Write_Command(0xD3);			// Set Display Offset
	Write_Command(d);			//   Default => 0x00
}


void Set_Display_Clock(unsigned char d)
{
	Write_Command(0xD5);			// Set Display Clock Divide Ratio / Oscillator Frequency
	Write_Command(d);			//   Default => 0x70
						//     D[3:0] => Display Clock Divider
						//     D[7:4] => Oscillator Frequency
}


void Set_Low_Power(unsigned char d)
{
	Write_Command(0xD8);			// Set Low Power Display Mode
	Write_Command(d);			//   Default => 0x04 (Normal Power Mode)
}


void Set_Precharge_Period(unsigned char d)
{
	Write_Command(0xD9);			// Set Pre-Charge Period
	Write_Command(d);			//   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
						//     D[3:0] => Phase 1 Period in 1~15 Display Clocks
						//     D[7:4] => Phase 2 Period in 1~15 Display Clocks
}


void Set_Common_Config(unsigned char d)
{
	Write_Command(0xDA);			// Set COM Pins Hardware Configuration
	Write_Command(d);			//   Default => 0x12
						//     Alternative COM Pin Configuration
						//     Disable COM Left/Right Re-Map
}


void Set_VCOMH(unsigned char d)
{
	Write_Command(0xDB);			// Set VCOMH Deselect Level
	Write_Command(d);			//   Default => 0x34 (0.78*VCC)
}


void Set_NOP()
{
	Write_Command(0xE3);			// Command for No Operation
}


void Set_Command_Lock(unsigned char d)
{
	Write_Command(0xFD);			// Set Command Lock
	Write_Command(d);			//   Default => 0x12
						//     0x12 => Driver IC interface is unlocked from entering command.
						//     0x16 => All Commands are locked except 0xFD.
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Global Variables
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#define XLevelL		0x00
#define XLevelH		0x10
#define XLevel		((XLevelH&0x0F)*16+XLevelL)
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xCF


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM(unsigned char Data)
{
			unsigned char i,j;

				for(i=0;i<8;i++)
				{
					Set_Start_Page(i);
					Set_Start_Column(0x00);

					for(j=0;j<128;j++)
					{
							Write_Data(Data);
					}
			}
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Partial or Full Screen)
//
//    a: Start Page
//    b: End Page
//    c: Start Column
//    d: Total Columns
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_Block(unsigned char Data, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char i,j;
	for(i=a;i<(b+1);i++)
	{
		Set_Start_Page(i);
		Set_Start_Column(c);

		for(j=0;j<d;j++)
		{
				Write_Data(Data);
		}


	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Checkboard (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Checkerboard()
{
unsigned char i,j;

			for(i=0;i<8;i++)
			{
				Set_Start_Page(i);
				Set_Start_Column(0x00);


				for(j=0;j<64;j++)
				{

					Write_Data(0x55);
					Write_Data(0xAA);
				}
			}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Frame (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Frame()
{
unsigned char i,j;

	Set_Start_Page(0x00);
	Set_Start_Column(XLevel);

	for(i=0;i<Max_Column;i++)
	{
		Write_Data(0x01);
	}

	Set_Start_Page(0x07);
	Set_Start_Column(XLevel);

	for(i=0;i<Max_Column;i++)
	{
		Write_Data(0x80);
	}

	for(i=0;i<8;i++)
	{
		Set_Start_Page(i);
		for(j=0;j<Max_Column;j+=(Max_Column-1))
		{
			Set_Start_Column(XLevel+j);
			Write_Data(0xFF);
		}
	}
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Character (5x7)
//
//    a: Database
//    b: Ascii
//    c: Start Page
//    d: Start Column
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#ifdef FONT_5
void Show_Font57(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char *Src_Pointer;
unsigned char i;

	Src_Pointer=(unsigned char *)&E_font_5[b][0];

	Set_Start_Page(c);
	Set_Start_Column(d);

	for(i=0;i<5;i++)
	{
		Write_Data(*Src_Pointer);
		Src_Pointer++;
	}
	I2CWrite_Data_O(0x00);
}
#endif

void Show_Font16(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char *Src_Pointer;
unsigned char i;

	Src_Pointer=(unsigned char *)&E_font_16[b][0];

	Set_Start_Page(c);
	Set_Start_Column(d);

	for(i=0;i<8;i++)
	{
		Write_Data(*Src_Pointer);
		Src_Pointer++;
	}
	Set_Start_Page(c+1);
	Set_Start_Column(d);

	for(i=0;i<8;i++)
	{
		Write_Data(*Src_Pointer);
		Src_Pointer++;
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show String
//
//    a: Database
//    b: Start Page
//    c: Start Column
//    * Must write "0" in the end...
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_String(unsigned char a, unsigned char *Data_Pointer, unsigned char b, unsigned char c)
{
unsigned char *Src_Pointer;

#if 0
	Src_Pointer=Data_Pointer;
	Show_Font57(1,0,b,c);			// No-Break Space
	while(1)
	{
		Show_Font57(a,*Src_Pointer,b,c);
		Src_Pointer++;
		c+=6;
		if(*Src_Pointer == 0) break;
	}
#else
	Src_Pointer=Data_Pointer;
	Show_Font16(1,0,b,c);			// No-Break Space
	while(1)
	{
		Show_Font16(a,*Src_Pointer,b,c);
		Src_Pointer++;
		c+=8;
		if(*Src_Pointer == 0) break;
	}
	#endif
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Pattern (Partial or Full Screen)
//
//    a: Start Page
//    b: End Page
//    c: Start Column
//    d: Total Columns
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Show_Pattern(unsigned char *Data_Pointer, unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
unsigned char *Src_Pointer;
unsigned char i,j;

	Src_Pointer=Data_Pointer;
	for(i=a;i<(b+1);i++)
	{
		Set_Start_Page(i);
		Set_Start_Column(c);
		
		for(j=0;j<d;j++)
		{
				Write_Data(*Src_Pointer);
				Src_Pointer++;
		}
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Vertical / Fade Scrolling (Full Screen)
//
//    a: Scrolling Direction
//       "0x00" (Upward)
//       "0x01" (Downward)
//    b: Set Numbers of Row Scroll per Step
//    c: Set Time Interval between Each Scroll Step
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Vertical_Scroll(unsigned char a, unsigned char b, unsigned char c)
{
unsigned int i,j;

	switch(a)
	{
		case 0:
			for(i=0;i<Max_Row;i+=b)
			{
				Set_Start_Line(i);
				for(j=0;j<c;j++)
				{
						delay_ms(1);
				}
			}
			break;
		case 1:
			for(i=0;i<Max_Row;i+=b)
			{
				Set_Start_Line(Max_Row-i);
				for(j=0;j<c;j++)
				{
						delay_ms(1);
				}
			}
			break;
	}
	Set_Start_Line(0x00);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade In (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_In()
{
unsigned int i;	

	Set_Display_On_Off(0xAF);
	for(i=0;i<(Brightness+1);i++)
	{
		Set_Contrast_Control(i);
	  delay_ms( 20 );
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Fade Out (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fade_Out()
{
unsigned int i;	

	for(i=(Brightness+1);i>0;i--)
	{
		Set_Contrast_Control(i-1);
		delay_ms(5);
	}
	Set_Display_On_Off(0xAE);
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Sleep Mode
//
//    "0x00" Enter Sleep Mode
//    "0x01" Exit Sleep Mode
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Sleep(unsigned char a)
{
	switch(a)
	{
		case 0:
			Set_Display_On_Off(0xAE);
			Set_Entire_Display(0xA5);
			break;
		case 1:
			Set_Entire_Display(0xA4);
			Set_Display_On_Off(0xAF);
			break;
	}
}


//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Connection Test
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Test()
{
	delay_ms( 10 );

	Set_Entire_Display(0xA5);		// Enable Entire Display On (0xA4/0xA5)

	while(1)
	{
		Set_Display_On_Off(0xAF);	// Display On (0xAE/0xAF)
		delay_ms(2);
		Set_Display_On_Off(0xAE);	// Display Off (0xAE/0xAF)
		delay_ms(2);
	}
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Initialization
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void OLED_Init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;


#ifdef VER20
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  /* Configure _LED_PORT pin5, pin6 */
  GPIO_InitStructure.GPIO_Pin =    GPIO_Pin_1 | GPIO_Pin_2 ;  //OLED_CS#,OLED_CS2#
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);	
#else
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

  /* Configure _LED_PORT pin5, pin6 */
  GPIO_InitStructure.GPIO_Pin =    GPIO_Pin_1 | GPIO_Pin_15 ;  //OLED_CS#,OLED_CS2#
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);		
#endif


#ifdef VER20
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  GPIO_InitStructure.GPIO_Pin =    GPIO_Pin_15 ;   //12V_ON
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);	

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStructure.GPIO_Pin =    GPIO_Pin_8 ;   //C_D#,
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);	

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Pin =    GPIO_Pin_5 ;   //OLED_RESET
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);	


	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET);     
	
 	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_RESET);    //OLED_RESET
  Delay(10 );	
	GPIO_WriteBit(GPIOB, GPIO_Pin_5, Bit_SET);    //OLED_RESET
	
  GPIO_WriteBit(GPIOE, GPIO_Pin_15, Bit_SET);   //12.5V ON
  Delay(10 );
	
#else
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_2  | GPIO_Pin_3 | GPIO_Pin_5 | GPIO_Pin_14  | GPIO_Pin_15 ;
  GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	GPIO_WriteBit(GPIOE, GPIO_Pin_2, Bit_SET);     
 	GPIO_WriteBit(GPIOE, GPIO_Pin_5, Bit_RESET);    //OLED_RESET
  Delay(10 );	
	GPIO_WriteBit(GPIOE, GPIO_Pin_5, Bit_SET);    //OLED_RESET
	GPIO_WriteBit(GPIOE, GPIO_Pin_14, Bit_SET);   //12.5V ON
  Delay(10 );
#endif


GPIO_WriteBit(LCD_CS_GPIO_PORT,LCD_CS_PIN, Bit_RESET);      
GPIO_WriteBit(LCD_CS2_GPIO_PORT,LCD_CS2_PIN, Bit_RESET);     	


	Set_Command_Lock(0x12);			// Unlock Driver IC (0x12/0x16)
	Set_Display_On_Off(0xAE);		// Display Off (0xAE/0xAF)
	Set_Display_Clock(0xA0);		// Set Clock as 116 Frames/Sec
	Set_Multiplex_Ratio(0x3F);		// 1/64 Duty (0x0F~0x3F)
	Set_Display_Offset(0x00);		// Shift Mapping RAM Counter (0x00~0x3F)
	Set_Start_Line(0x00);			// Set Mapping RAM Display Start Line (0x00~0x3F)
	Set_Low_Power(0x05);			// Set Low Power Display Mode (0x04/0x05)
	Set_Addressing_Mode(0x02);		// Set Page Addressing Mode (0x00/0x01/0x02)
	Set_Segment_Remap(0xA1);		// Set SEG/Column Mapping (0xA0/0xA1)
	Set_Common_Remap(0xC8);			// Set COM/Row Scan Direction (0xC0/0xC8)
	Set_Common_Config(0x12);		// Set Alternative Configuration (0x02/0x12)
	Set_Contrast_Control(Brightness);	// Set SEG Output Current
	Set_Precharge_Period(0x22);		// Set Pre-Charge as 2 Clocks & Discharge as 2 Clocks
	Set_VCOMH(0x34);										// Set VCOM Deselect Level
	Set_Entire_Display(0xA4);				// Disable Entire Display On (0xA4/0xA5)
	//Set_Inverse_Display(0xA6);		  // Disable Inverse Display On (0xA6/0xA7)

	Fill_RAM(0x00);				// Clear Screen
	Set_Display_On_Off(0xAF);		// Display On (0xAE/0xAF)
	//Set_Display_On_Off(0xAE);		// Display On (0xAE/0xAF)

GPIO_WriteBit(LCD_CS_GPIO_PORT,LCD_CS_PIN, Bit_SET);      
GPIO_WriteBit(LCD_CS2_GPIO_PORT,LCD_CS2_PIN, Bit_SET);     	


}


#ifdef LCD_DEMO
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Main Program
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void oled_test()
{
char  *Name="testing";
char  *tstring0="장기현대표";
char  *tstring1="늦어져서  미안";
char  *tstring2="해요 조금만  ";
char  *tstring3="인내해 주세요";

	
						// WiseChip
char  *Tel="010-5176-3847 ";
						// +886-37-587168
unsigned char i;


	OLED_Init();
	//Test();
	while(1)
	{
	 // Show Pattern - UniV OLED
	 // Show_Pattern((unsigned char *)&Logo_Test[0],0x02,0x05,XLevel+0x28,0x30);
		delay_ms( 2000);

		#if 0
	// Fade In/Out (Full Screen)
		Fade_Out();
		Fade_In();
		Fade_Out();
		Fade_In();
		delay_ms(2);

	   // Scrolling (Full Screen)
		Vertical_Scroll(0x00,0x01,0x60);
		// Upward
				delay_ms(2);
		Vertical_Scroll(0x01,0x01,0x60);
						// Downward
			delay_ms(2);

	// All Pixels On (Test Pattern)
		Fill_RAM(0xFF);
		delay_ms(2);

	// Checkerboard (Test Pattern)
		Checkerboard();
		delay_ms(100);
#endif
		Fill_RAM(0x00);			// Clear Screen
	// Frame (Test Pattern)
		//Frame();
		delay_ms(100);

   GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);      
	 GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);      
 
	  GLCD_string_16(0,0,(uint8_t *)tstring0,1);
	  GLCD_string_16(1,0,(uint8_t *)tstring1,1);
	
    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_RESET);      
  	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_RESET);      


    GLCD_string_16(2,0,(uint8_t *)tstring2,1);
	  GLCD_string_16(3,0,(uint8_t *)tstring3,1);

    GPIO_WriteBit(GPIOA, GPIO_Pin_1, Bit_SET);      
  	GPIO_WriteBit(GPIOA, GPIO_Pin_15, Bit_SET);      


    delay_ms(10);
	//Fill_RAM(0x00);			// Clear Screen
		break;
	}

}
#endif

void GLCD_LightOn(void)
{
 
}

void GLCD_LightOff(void)
{
 
}

void GLCD_DisplayOn(void)
{
     Set_Display_On_Off(0xAF);
	   Set_Contrast_Control(Brightness);
}


void GLCD_DisplayOff(void)
{
     Set_Display_On_Off(0xAE);	
}

void GLCD_Clear( void )					// clear GLCD screen
{
			Fill_RAM(0x00);			// Clear Screen
}

void GLCD_Full( void )					// clear GLCD screen
{
			Fill_RAM(0xFF);			// Clear Screen
}

void GLCD_Logo(void)	 {		// set English character position on GLCD
uint8_t  width=   logo[0];
uint8_t  rows= logo[1]/8;
uint8_t  row_start,row_end;
uint8_t	col_start,col_end;

			row_start=(8-rows)/2;
      row_end = row_start+rows-1;
	
			col_start=(128-width)/2;
		  col_end	=col_start+width;
	
      Show_Pattern((unsigned char *)&logo[2],row_start,row_end,col_start,col_end);
}	


void GLCD_xy(uint8_t x, uint8_t y)			// set English character position on GLCD
{
	xcharacter = x;					// x = 0 - 3 
	ycharacter = y;					// y = 0 - 15
}

void GLCD_xy_row_8(uint8_t row, uint8_t x, uint8_t y)	// set character upper/lower row on GLCD
{
	GLCD_xy(x, y);					// x = 0 - 7, y = 0 - 15

	Set_Start_Page(x);
	Set_Start_Column(y);

}

void GLCD_xy_row_16(uint8_t row, uint8_t x, uint8_t y)	// set character upper/lower row on GLCD
{
	GLCD_xy(x, y);					// x = 0 - 3, y = 0 - 15

	Set_Start_Page(x*2+row);
	Set_Start_Column(y);
}

void GLCD_icon_10(uint8_t x, uint8_t y, uint8_t * pIcon, uint8_t batang)	// display a 8x16 Dot English(ASCII) on GLCD
{
	  uint8_t i;

  	GLCD_xy_row_16(0, x, y);			// display upper row
  	for( i = 0; i < 10; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
	 I2C_GenerateSTOP(I2C1, ENABLE); 
  
	 GLCD_xy_row_16(1, x, y);				// display lower row
  	for( i = 10; i <20 ;i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
		}

}


void GLCD_icon_16(uint8_t x, uint8_t y, uint8_t * pIcon, uint8_t batang)	// display a 8x16 Dot English(ASCII) on GLCD
{
	  uint8_t i;

  	GLCD_xy_row_16(0, x, y);			// display upper row
  	for( i = 0; i < 8; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
  	for( i = 16; i < 24; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
	 I2C_GenerateSTOP(I2C1, ENABLE); 
  
	 GLCD_xy_row_16(1, x, y);				// display lower row
  	for( i = 8; i <16 ;i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
		}
  	for( i = 24; i < 32; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
 
}

void GLCD_icon_20(uint8_t x, uint8_t y, uint8_t * pIcon, uint8_t batang)	// display a 8x16 Dot English(ASCII) on GLCD
{
	  uint8_t i;

	 	GLCD_xy_row_16(0, x, y);			// display upper row
  	for( i = 0; i < 20; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
  
	 GLCD_xy_row_16(1, x, y);				// display lower row
  	for( i = 20; i <40 ;i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
		}

  	GLCD_xy_row_16(0, x+1, y);			// display upper row
  	for( i = 40; i < 60; i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
	 }		
  
	 GLCD_xy_row_16(1, x+1, y);				// display lower row
  	for( i = 60; i <80 ;i++ ) {
				if( batang ) { Write_Data(pIcon[i]); }
				else {  Write_Data(~pIcon[i]); }
		}


}

void GLCD_English_8(uint8_t Ecode, uint8_t batang)	// display a 8x16 Dot English(ASCII) on GLCD
{
	uint8_t x, y, i;

  	x = xcharacter;
  	y = ycharacter;

  	GLCD_xy_row_8(0, x, y);			// display upper row

		for(i=0;i<8;i++)
	  {
				if( batang ) { Write_Data(E_font_8[Ecode][i]); }
				else {  Write_Data(~E_font_8[Ecode][i]); }
		}
    ycharacter += 8;
}

void GLCD_English_16(uint8_t Ecode, uint8_t batang)	// display a 8x16 Dot English(ASCII) on GLCD
{
	  uint8_t x, y, i;

	
  	x = xcharacter;
  	y = ycharacter;

  	GLCD_xy_row_16(0, x, y);			// display upper row
		for( i = 0; i <= 7; i++ ) {
					if( batang ) { Write_Data(E_font_16[Ecode][i]); }
					else {  Write_Data(~E_font_16[Ecode][i]); }
		}
  
	 GLCD_xy_row_16(1, x, y);				// display lower row
  	for( i = 8; i <= 15; i++ ) {
		if( ( cursor_flag == 1 ) && ( xcharacter == cursorx ) && ( ycharacter == cursory ) ) {
				if( batang ) { Write_Data(E_font_16[Ecode][i]| 0X80); }
				else {  Write_Data(~(E_font_16[Ecode][i]|0X80)); }
		}  else {
				if( batang ) { Write_Data(E_font_16[Ecode][i]); }
				else {  Write_Data(~E_font_16[Ecode][i]); }
		}
	}
	ycharacter += 8;
}


#ifdef KOREAN_FONT
void GLCD_Korean_16(uint32_t Kcode, uint8_t batang)	// display a 16x16 Dot Korean on GLCD
{
	uint8_t x, y, i;
	uint8_t cho_5bit, joong_5bit, jong_5bit;
	uint8_t cho_bul, joong_bul, jong_bul, jong_flag;
	uint32_t character;

	cho_5bit = table_cho[( Kcode >> 10 ) & 0x001F];		// get cho, joong, jong 5 bit
	joong_5bit = table_joong[( Kcode >> 5 ) & 0x001F];
	jong_5bit = table_jong[Kcode & 0x001F];

	if( jong_5bit == 0 ) {								// don't have jongsung
		jong_flag = 0;
      	cho_bul = bul_cho1[joong_5bit];
      	if( ( cho_5bit == 1 ) || ( cho_5bit == 16 ) ) { joong_bul = 0; }
      	else { joong_bul = 1; }
	}
	else {												// have jongsung
    	jong_flag = 1;
      	cho_bul = bul_cho2[joong_5bit];
      	if( ( cho_5bit == 1 ) || ( cho_5bit == 16 ) ) { joong_bul = 2; }
      	else { joong_bul = 3; }
      	jong_bul = bul_jong[joong_5bit];
	}

  	character = cho_bul * 20 + cho_5bit;				// copy chosung font

  	for( i = 0; i <= 31; i++ ) { Korean_buffer[i] = K_font[character][i]; }

  	character = 8 * 20 + joong_bul * 22 + joong_5bit;	// OR joongsung font
  	
	for(i = 0; i <= 31; i++) { Korean_buffer[i] |= K_font[character][i]; }

  	if( jong_flag == 1 ) {								// if jongsung, OR jongsung font
    	character = 8 * 20 + 4 * 22 +jong_bul * 28 + jong_5bit;
      	for(i = 0; i <= 31; i++) { Korean_buffer[i] |= K_font[character][i]; }
    }

  	x = xcharacter;
  	y = ycharacter;

  	GLCD_xy_row_16(0, x, y);								// display upper left row
  	for( i = 0; i <= 7; i++ ) {
		if( batang ) { Write_Data(Korean_buffer[i]); }
		else { Write_Data( ~(Korean_buffer[i])); }
	}

  	GLCD_xy_row_16(1, x, y);								// display lower left row

  	for( i = 16; i <= 23; i++ ) {
		if( ( cursor_flag == 1 ) && ( xcharacter == cursorx ) && ( ycharacter == cursory ) ) {
			if( batang ) { Write_Data( Korean_buffer[i] | 0x80); }
			else { Write_Data( ~(Korean_buffer[i] | 0x80)); }
		}  else {
			if( batang ) { Write_Data(Korean_buffer[i]); }
			else { Write_Data(~(Korean_buffer[i])); }
		}
    }

	ycharacter += 8;

  	y = ycharacter;
  	GLCD_xy_row_16(0, x, y);								// display upper right row

  	for( i = 8; i <= 15; i++ ) {
			if( batang ) { Write_Data(Korean_buffer[i]); }
			else { Write_Data(~(Korean_buffer[i])); }
	}

  	GLCD_xy_row_16(1, x, y);								// display lower right row

	for( i = 24; i <= 31; i++ ) {
  		if( ( cursor_flag == 1 ) && ( xcharacter == cursorx ) && ( ycharacter == ( cursory + 1 ) ) ) {
			if( batang ) { Write_Data( Korean_buffer[i] | 0x80); }
			else { Write_Data( ~(Korean_buffer[i] | 0x80)); }
		}
    	else {
			if( batang ) { Write_Data(Korean_buffer[i]); }
			else { Write_Data(~(Korean_buffer[i])); }
		}
	}
	ycharacter += 8;
}

uint32_t kssm_convert( uint32_t kssm )
{
	uint8_t high,low;
	uint32_t index;
    uint32_t return_data;

	high = (kssm>>8)&0xff;
	low  =  kssm    &0xff;

	if( ( kssm >= 0xb0a1 ) && ( kssm <= 0xc8fe ) ) {
		index  = (high - 0xb0) * 94 + low - 0xa1;
		return_data =  KSTable[index];
		return return_data;
	}
	return 0;
}
#endif

void GLCD_string_8(uint8_t x, uint8_t y, uint8_t *string, uint8_t batang)	// display a string on GLCD
{
	uint8_t character1;
	uint32_t character2;

	GLCD_xy(x, y);										// x = 0 - 7, y = 0 - 15

	while( *string != '\0' ) {
		character1 = *string;
		string++;
		if( ( ( character1 >> 7 ) & 0x01 ) == 0 ) {
			GLCD_English_8( character1, batang );					// display English character
		}
		else {
#ifdef KOREA_FONT
			character2 = *string;
			character2 = 256 * character1 + ( character2 & 0xFF );
			string++;
			character2 = kssm_convert(character2);
			GLCD_Korean_16(character2, batang );					// display Korean character
#endif
		}
	}
}

void GLCD_string_16(uint8_t x, uint8_t y, uint8_t *string, uint8_t batang)	// display a string on GLCD
{
	uint8_t character1;
	uint32_t character2;

	GLCD_xy(x, y);										// x = 0 - 3, y = 0 - 15

	while( *string != '\0' ) {
		character1 = *string;
		string++;
		if( ( ( character1 >> 7 ) & 0x01 ) == 0 ) {
			GLCD_English_16( character1, batang );					// display English character
		}
		else {
#ifdef KOREAN_FONT
			character2 = *string;
			character2 = 256 * character1 + ( character2 & 0xFF );
			string++;
			character2 = kssm_convert(character2);
			GLCD_Korean_16( character2, batang );					// display Korean character
#endif			
		}
	}
}


void LCD_DisplayStringLine(uint8_t Line, uint8_t *ptr)
{
#if 1
	if (Line > 7) Line=Line-8;
	GLCD_string_8(Line,0,(uint8_t *)ptr,1);
#else
	 printf("%s\n",ptr);
#endif
}



