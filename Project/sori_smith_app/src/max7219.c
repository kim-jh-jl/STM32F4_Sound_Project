
/*
*********************************************************************************************************
* Module     : MAX7219.C
* Author     : Randy Rasa
* Description: MAX7219 LED Display Driver Routines
*
*  The Maxim MAX7219 is an LED display driver thant can control up to 64 individual LEDs, or
*  eight 7-segment LED digits, or any combination of individual LEDs and digits.  It frees the
*  host from the chore of constantly multiplexing the 8 rows and 8 columns.  In addition, it
*  takes care of brightness control (16 steps), and implements display test and display blank
*  (shutdown) features.
*
*  The host communicates with the MAX7219 using three signals: DATA, CLK, and LOAD.  This
*  modules bit-bangs them, but Motorola's SPI interface (or similar interface from other
*  manufacturers) may also be used to simplify and speed up the data transfer.
*                   ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___ ___
*    DATA _________|D15|D14|D13|D12|D11|D10|D09|D08|D07|D06|D05|D04|D03|D02|D01|D00|______
*         ________    __    __    __    __    __    __    __    __    __    __    ________
*    CLK          |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|  |__|
*                __________________________________________________________________    ___
*    LOAD ______|                                                                  |__|
*
*
*  Implementation Notes:
*
*  1. This module was written and tested using an Atmel AT89C2051 microcontroller, with the
*     MAX7219 connected to I/O pins P3.3 (LOAD), P3.4 (CLK), and P3.3 (DATA).
*
*  2. Macros are provided to simplify control of the DATA, CLK, and LOAD signals.  You may also use
*     memory-mapped output ports such as a 74HC374, but you'll need to replace the macros with
*     functions, and use a shadow register to store the state of all the output bits.
*
*  3. This module was tested with the evaluation version of Hi-Tech C-51, using the small memory model.
*     It should be portable to most other compilers, with minor modifications.
*
*  4. The MAX7219 is configured for "no decode" mode, rather than "code B" decoding.  This
*     allows the program to display more than the 0-9,H,E,L,P that code B provides.  However,
*     the "no decode" method requires that each character to be displayed have a corresponding
*     entry in a lookup table, to convert the ascii character to the proper 7-segment code.
*     MAX7219_LookupCode() provides this function, using the MAX7219_Font[] array.  If you need
*     to display more than the characters I have provided, simply add them to the table ...
*
*********************************************************************************************************
*/


/*
*********************************************************************************************************
* Include Header Files
*********************************************************************************************************
*/
#include "main.h"
#include "max7219.h"                                  // MAX7219 header file


/*
*********************************************************************************************************
* Constants
*********************************************************************************************************
*/

#define REG_DECODE        0x09                        // "decode mode" register
#define REG_INTENSITY     0x0a                        // "intensity" register
#define REG_SCAN_LIMIT    0x0b                        // "scan limit" register
#define REG_SHUTDOWN      0x0c                        // "shutdown" register
#define REG_DISPLAY_TEST  0x0f                        // "display test" register

#define INTENSITY_MIN     0x00                        // minimum display intensity
#define INTENSITY_MAX     0x0f                        // maximum display intensity

#define VER20

/*
*********************************************************************************************************
* Macros
*********************************************************************************************************
*/

#ifdef VER20
/*
#define DATA_0()      (GPIOB->BSRRH  = GPIO_Pin_15)
#define DATA_1()      (GPIOB->BSRRL   =  GPIO_Pin_15)

#define CLK_0()         (GPIOB->BSRRH  = GPIO_Pin_13)
#define CLK_1()         (GPIOB->BSRRL =  GPIO_Pin_13)

#define LOAD_0()      (GPIOB->BSRRH  = GPIO_Pin_12)
#define LOAD_1()      (GPIOB->BSRRL  =    GPIO_Pin_12)
*/

#define DATA_0()      (GPIOD->BSRRH  = GPIO_Pin_11)
#define DATA_1()      (GPIOD->BSRRL   =  GPIO_Pin_11)

#define CLK_0()         (GPIOD->BSRRH  = GPIO_Pin_9)
#define CLK_1()         (GPIOD->BSRRL =  GPIO_Pin_9)

#define LOAD_0()      (GPIOD->BSRRH  = GPIO_Pin_10)
#define LOAD_1()      (GPIOD->BSRRL  =    GPIO_Pin_10)
#else
#define DATA_0()      (GPIOB->BSRRH  = GPIO_Pin_15)
#define DATA_1()      (GPIOB->BSRRL   =  GPIO_Pin_15)

#define CLK_0()         (GPIOB->BSRRH  = GPIO_Pin_10)
#define CLK_1()         (GPIOB->BSRRL =  GPIO_Pin_10)

#define LOAD_0()      (GPIOB->BSRRH  = GPIO_Pin_14)
#define LOAD_1()      (GPIOB->BSRRL  =    GPIO_Pin_14)
#endif

/*
*********************************************************************************************************
* Private Data
*********************************************************************************************************
*/
// ... none ...


/*
*********************************************************************************************************
* Private Function Prototypes
*********************************************************************************************************
*/
unsigned char DigValues[] =	{0,0,0,0,0,0,0,0,0};
 
static void MAX7219_SendByte (unsigned char data);
static unsigned char MAX7219_LookupCode (char character);


// ...................................... Public Functions ..............................................


/*
*********************************************************************************************************
* MAX7219_Init()
*
* Description: Initialize MAX7219 module; must be called before any other MAX7219 functions.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
static void udelay(int ntime) {
		 volatile int i=0;
	   for (i=0;i<ntime*10;i++) {}
}

void MAX7219_Init (void)
{
	  int i=0;
	  int freq=0;
/*	
  DATA_DDR |= DATA_BIT;                               // configure "DATA" as output
  CLK_DDR  |= CLK_BIT;                                // configure "CLK"  as output
  LOAD_DDR |= LOAD_BIT;                               // configure "LOAD" as output
*/


	GPIO_InitTypeDef  GPIO_InitStructure;
	
  /* Enable the GPIO_LED Clock */
  //RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE); //VER20
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); 
#ifdef VER20
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 |GPIO_Pin_15; //VER20
	   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10 |GPIO_Pin_11;
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_14 |GPIO_Pin_15;
#endif
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  //GPIO_Init(GPIOB, &GPIO_InitStructure);   //VER20
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

#ifdef VER20
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);   //TOUCH INTERRUPT
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;   //BEEP
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);   //TOUCH INTERRUPT
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |  GPIO_Pin_2 |   GPIO_Pin_3 |  GPIO_Pin_4 |   GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |  GPIO_Pin_8 |   GPIO_Pin_9 |  GPIO_Pin_10 |   GPIO_Pin_11 |   GPIO_Pin_12 |   GPIO_Pin_13 |   GPIO_Pin_14 ;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
	
#else
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;   //BEEP
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;   //Touch Interrupt
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

#endif



	MAX7219_Write(REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
	MAX7219_Write(REG_SCAN_LIMIT, 7);                   // set up to scan all eight digits
  MAX7219_Write(REG_DECODE, 0x00);                   // set to "no decode" for all digits
	MAX7219_ShutdownStop();                                   // select normal operation (i.e. not shutdown)
  MAX7219_DisplayTestStop();                                // select normal operation (i.e. not test mode)
  MAX7219_Clear();                                                  // clear all digits	
	MAX7219_SetBrightness(INTENSITY_MAX);           // set to maximum intensity
 
	
	/*
	for (i=0;i<8;i++) {
			MAX7219_Freq(1,i,1);
		   Delay(50);
	}
	for (i=0;i<8;i++) {
			MAX7219_Freq(1,i,2);
		   Delay(50);
	}
	for (i=0;i<8;i++) {
			MAX7219_Freq(1,i,3);
		   Delay(50);
	}
	*/

	MAX7219_Display(6,0);
	MAX7219_Display(7,0);
	MAX7219_Freq(1,0,4);
	MAX7219_Freq(2,0,4);
	MAX7219_Freq(3,0,4);

}


/*
*********************************************************************************************************
* MAX7219_ShutdownStart()
*
* Description: Shut down the display.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_ShutdownStart (void)
{
    MAX7219_Write(REG_SHUTDOWN, 0);                     // put MAX7219 into "shutdown" mode
}


/*
*********************************************************************************************************
* MAX7219_ShutdownStop()
*
* Description: Take the display out of shutdown mode.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_ShutdownStop (void)
{
  MAX7219_Write(REG_SHUTDOWN, 1);                     // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStart()
*
* Description: Start a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayTestStart (void)
{
  MAX7219_Write(REG_DISPLAY_TEST, 1);                 // put MAX7219 into "display test" mode
}


/*
*********************************************************************************************************
* MAX7219_DisplayTestStop()
*
* Description: Stop a display test.
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_DisplayTestStop (void)
{
  MAX7219_Write(REG_DISPLAY_TEST, 0);                 // put MAX7219 into "normal" mode
}


/*
*********************************************************************************************************
* MAX7219_SetBrightness()
*
* Description: Set the LED display brightness
* Arguments  : brightness (0-15)
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_SetBrightness (char brightness)
{
  brightness &= 0x0f;                                 // mask off extra bits
  MAX7219_Write(REG_INTENSITY, brightness);           // set brightness
}


/*
*********************************************************************************************************
* MAX7219_Clear()
*
* Description: Clear the display (all digits blank)
* Arguments  : none
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Clear (void)
{
  char i;
  for (i=1; i <= 8; i++)
      MAX7219_Write(i, 0x00);                           // turn all segments off
}


/*
*********************************************************************************************************
* MAX7219_DisplayChar()
*
* Description: Display a character on the specified digit.
* Arguments  : digit = digit number (0-7)
*              character = character to display (0-9, A-Z)
* Returns    : none
* Anode common
*********************************************************************************************************
*/

#if 0
void MAX7219_Freq(char left_right, char freq,char color)
{
		 int x;
			// Array with information about which bit has to be set to display the correct number
			// numbers[0] -&gt; Displays 0
			// numbers[4] -&gt; Displays 4
			// ...                                     7e                0c               b6               9e                cc               da                  fa                0e                 fe                 de
			//char numbers[] = {0b011111110 ,0b00001100 ,0b10110110 ,0b10011110 ,0b11001100 ,0b11011010 ,0b11111010 ,0b00001110 ,0b11111110 ,0b11011110};
			char green[] = {0x1,0x4,0x10,0x40,0x1,0x4,0x10,0x40};
			char red[]     =  {0x2,0x8,0x20,0x80,0x2,0x8,0x20,0x80};
			char both[]    = {0x3,0xc,0x30,0xc0,0x3,0xc,0x30,0xc0};
			
			char bPosition   = 0x1;			
			char bPosition1 = 0x1;			
			char bPosition2 = 0x1;			
			
			if (left_right == 1) {   //left
						if (freq < 4) {
							   bPosition1   = bPosition << 4; 
							   bPosition2 = bPosition << 3; 
						} else {
							   bPosition1= bPosition << 3; 
							   bPosition2   = bPosition << 4; 
						}
				} else {
						if (freq < 4) {
								bPosition1 = bPosition << 2; 
							  bPosition2 = bPosition << 1; 
						} else {
							  bPosition1 = bPosition << 1; 
							  bPosition2 = bPosition << 2; 
						}				
			}	 
  
			// iterate through the numbers array
			for( x = 0; x < 8; x++){
				// check if bit is high at the current location of our selected number
				switch(color) {
					case 0:
									DigValues[x] = (0xff ^ bPosition1) & DigValues[x] ;
									DigValues[x] = (0xff ^ bPosition2) & DigValues[x] ;
									break;
					case 1:
									if ((green[freq]) & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition1;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition1) & DigValues[x] ;
									}
							    DigValues[x] = (0xff ^ bPosition2) & DigValues[x] ;									
									break;
					case 2:
									if (red[freq] & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition1;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition1) & DigValues[x] ;
									}						
							    DigValues[x] = (0xff ^ bPosition2) & DigValues[x] ;									
									break;
					case 3:
									if ((both[freq]) & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition1;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition1) & DigValues[x] ;
									}						
							    DigValues[x] = (0xff ^ bPosition2) & DigValues[x] ;
									break;
					case 4:
									// if bitX of numbers is high just binary OR bPosition to the current DigValues value
  								DigValues[x] |= bPosition1;
									DigValues[x] |= bPosition2;
									break;									
				}
				MAX7219_Write(x+1, DigValues[x]);   
			}	
}	
#else  //NEW _VER
void MAX7219_Freq(char left_right, char freq,char color)
{
		 int x;
			// Array with information about which bit has to be set to display the correct number
			// numbers[0] -&gt; Displays 0
			// numbers[4] -&gt; Displays 4
			// ...                                     7e                0c               b6               9e                cc               da                  fa                0e                 fe                 de
			//char numbers[] = {0b011111110 ,0b00001100 ,0b10110110 ,0b10011110 ,0b11001100 ,0b11011010 ,0b11111010 ,0b00001110 ,0b11111110 ,0b11011110};
			char green[] = {0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80};
			
			char bPosition   = 0x1;			
			
			if (left_right == 1) {   //left
					   bPosition   = bPosition << 4; 
			} else if (left_right == 2 ){
						bPosition = bPosition << 2; 
			}
  
			// iterate through the numbers array
			for( x = 0; x < 8; x++){
				// check if bit is high at the current location of our selected number
				switch(color) {
					case 0:
									DigValues[x] = (0xff ^ bPosition) & DigValues[x] ;
									break;
					case 1:
									if ((green[freq]) & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition) & DigValues[x] ;
									}
									break;
					case 2:
									if (green[freq] & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition) & DigValues[x] ;
									}						
									break;
					case 3:
									if ((green[freq]) & (1 << x) )
									{
												// if bitX of numbers is high just binary OR bPosition to the current DigValues value
												DigValues[x] |= bPosition;
									}else{
												// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
												DigValues[x] = (0xff ^ bPosition) & DigValues[x] ;
									}						
									break;
					case 4:
									// if bitX of numbers is high just binary OR bPosition to the current DigValues value
  								DigValues[x] |= bPosition;
									break;									
				}
				MAX7219_Write(x+1, DigValues[x]);   
			}	
}	
#endif
void MAX7219_Display(char dig, char number)
{
 int x;
	// Array with information about which bit has to be set to display the correct number
	// numbers[0] -&gt; Displays 0
	// numbers[4] -&gt; Displays 4
	// ...                                     7e                0c               b6               9e                cc               da                  fa                0e                 fe                 de
	//char numbers[] = {0b011111110 ,0b00001100 ,0b10110110 ,0b10011110 ,0b11001100 ,0b11011010 ,0b11111010 ,0b00001110 ,0b11111110 ,0b11011110};
	char numbers[] = {0x7e,0x0c ,0xb6 ,0x9e,0xcc,0xda,0xfa,0x0e,0xfe,0xde};
 
	// Dig positioning byte
	char bPosition = 0x1;
 
	// If [param]dig position is greater than 1
	if ( dig > 1){
		// shift bPosition left to the correct digit ([param]dig -1 )
		bPosition = bPosition << (dig-1);
	}
 
	// iterate through the numbers array
	for( x = 0;x < 7; x++){
		// check if bit is high at the current location of our selected number
		if ( numbers[number] & (1 << (x+1)) )
		{
					// if bitX of numbers is high just binary OR bPosition to the current DigValues value
					DigValues[x] |= bPosition;
		}else{
					// if bitX of numbers is low, invert bPosition and binary AND it with current DigValues value to set bit low
					DigValues[x] = (0xff ^ bPosition) & DigValues[x] ;
		}
		MAX7219_Write(x+1, DigValues[x]);   
	}
}

// ..................................... Private Functions ..............................................


/*
*********************************************************************************************************
* LED Segments:         a
*                     ----
*                   f|    |b
*                    |  g |
*                     ----
*                   e|    |c
*                    |    |
*                     ----  o dp
*                       d
*   Register bits:
*      bit:  7  6  5  4  3  2  1  0
*           dp  a  b  c  d  e  f  g
*********************************************************************************************************
*/
static const struct {
	char   ascii;
	char   segs;
} MAX7219_Font[] = {
  {' ', 0x00},
  {'0', 0x7e},
  {'1', 0x30},
  {'2', 0x6d},
  {'3', 0x79},
  {'4', 0x33},
  {'5', 0x5b},
  {'6', 0x5f},
  {'7', 0x70},
  {'8', 0x7f},
  {'9', 0x7b},
  {'A', 0x77},
  {'B', 0x1f},
  {'C', 0x4e},
  {'D', 0x3d},
  {'E', 0x4f},
  {'F', 0x47},
  {'\0', 0x00}
};

/*
*********************************************************************************************************
* MAX7219_LookupCode()
*
* Description: Convert an alphanumeric character to the corresponding 7-segment code.
* Arguments  : character to display
* Returns    : segment code
*********************************************************************************************************
*/
static unsigned char MAX7219_LookupCode (char character)
{
  char i;
  for (i = 0; MAX7219_Font[i].ascii; i++)             // scan font table for ascii code
    if (character == MAX7219_Font[i].ascii)
      return MAX7219_Font[i].segs;                    // return segments code
  return 0;                                           // code not found, return null (blank)
}


/*
*********************************************************************************************************
* MAX7219_Write()
*
* Description: Write to MAX7219
* Arguments  : reg_number = register to write to
*              dataout = data to write to MAX7219
* Returns    : none
*********************************************************************************************************
*/
void MAX7219_Write (unsigned char reg_number, unsigned char dataout)
{

#if 0
  LOAD_1();                                           // take LOAD high to begin
  LOAD_0();                                           // take LOAD low to latch in data	
	udelay(1000);			
  MAX7219_SendByte(reg_number);                       // write register number to MAX7219
  MAX7219_SendByte(dataout);                          // write data to MAX7219
	udelay(1000);		
	LOAD_1();                                           // take LOAD high to end

#else
  LOAD_1();                                           // take LOAD high to begin
	udelay(1);		
  MAX7219_SendByte(reg_number);                       // write register number to MAX7219
  MAX7219_SendByte(dataout);                          // write data to MAX7219
  LOAD_0();                                           // take LOAD low to latch in data
	udelay(1);		
  LOAD_1();                                           // take LOAD high to end
#endif
}


/*
*********************************************************************************************************
* MAX7219_SendByte()
*
* Description: Send one byte to the MAX7219
* Arguments  : dataout = data to send
* Returns    : none
*********************************************************************************************************
*/
static void MAX7219_SendByte (unsigned char dataout)
{
  char i;
  for (i=8; i>0; i--) {
    unsigned char mask = 1 << (i - 1);                // calculate bitmask
    CLK_0();                                          // bring CLK low
    if (dataout & mask)  {                            // output one data bit
       DATA_1();                                       //  "1"
    } else {                                              //  or
			 DATA_0();                                       //  "0"
		}
		udelay(1);		
    CLK_1();                                          // bring CLK high
		udelay(1);		
	}
}


