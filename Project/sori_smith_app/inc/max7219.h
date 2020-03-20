/*
*********************************************************************************************************
* Module     : MAX7219.H
* Author     : Randy Rasa
* Description: Header file for MAX7219.C (LED Display Driver Routines)
*********************************************************************************************************
*/


/*
*********************************************************************************************************
* Public Function Prototypes
*********************************************************************************************************
*/


#define	LDIGIT  7
#define  RDIGIT  6

void MAX7219_Init (void);
void MAX7219_ShutdownStart (void);
void MAX7219_ShutdownStop (void);
void MAX7219_DisplayTestStart (void);
void MAX7219_DisplayTestStop (void);
void MAX7219_SetBrightness (char brightness);
void MAX7219_Clear (void);
void MAX7219_Display(char digit, char character);
void MAX7219_Freq(char dig, char freq,char color);
void MAX7219_Write (unsigned char reg_number, unsigned char data);