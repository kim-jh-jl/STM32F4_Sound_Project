/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/main.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  */ 

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "max7219.h"  
#include "lcd.h"
#include "defines.h"
#include "tm_stm32f4_spi.h"
#include "tm_stm32f4_i2c.h"
#include "stm32f4xx_flash.h"
#include "tm_stm32f4_usart.h"
#include "codec.h"
#include "stm32f4_discovery_audio_codec.h"

#define ADDRESS        0xD0 // 1101 000 0
#define NEW_VER

/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined MEDIA_USB_KEY
 USB_OTG_CORE_HANDLE          USB_OTG_Core;
 USBH_HOST                    USB_Host;
#endif

RCC_ClocksTypeDef RCC_Clocks;
__IO uint8_t RepeatState = 0;
__IO uint16_t CCR_Val = 16826;
extern __IO uint8_t LED_Toggle;
extern __IO uint8_t volume_right,volume_left ;
extern __IO uint8_t Timer_cnt0,Timer_cnt1;
extern uint8_t	IsButton_Left,IsButton_Right;
extern const unsigned char S_font_20[11][20];
extern const unsigned char S_font_40[11][80];
tsc_config_t	tsc_config;
u32 loption;
u8 lbeep=1;

uint8_t AUDIO_RIGHT=5,AUDIO_OLD_RIGHT=0;
uint8_t AUDIO_LEFT  =8,AUDIO_OLD_LEFT=0;
uint8_t  timer=0,oldtimer=0;
char lPlay=0;
char lSaveConf=0;
char lNeedSave=0;


/* Private function prototypes -----------------------------------------------*/
static void TIM_LED_Config(void);
static void display_status(uint8_t left_right);
static void gosleep(void);
static void wakeup(void);

static __IO uint32_t TimingDelay;

/* Private functions ---------------------------------------------------------*/
/* We need to implement own __FILE struct */
/* FILE struct is used from __FILE */
struct __FILE {
    int dummy;
};
 
/* You need this if you want use printf */
/* Struct FILE is implemented in stdio.h */
FILE __stdout;
 
int fputc(int ch, FILE *f) {
    /* Do your stuff here */
    /* Send your custom byte */
    /* Send byte to USART */
    TM_USART_Putc(USART1, ch);
    
    /* If everything is OK, you have to return character written */
    return ch;
    /* If character is not correct, you can return EOF (-1) to stop writing */
    //return -1;
}

static void gosleep(void) {
			MAX7219_Clear();    
			GPIO_WriteBit(GPIOE, GPIO_Pin_15, Bit_RESET);   //12.5V ON
			STM_EVAL_LEDOff(LED_30MIN);
			STM_EVAL_LEDOff(LED_60MIN);
			STM_EVAL_LEDOff(LED_PLAY);	
}

static void wakeup(void) {
  		GPIO_WriteBit(GPIOE, GPIO_Pin_15, Bit_SET);   //12.5V ON
			STM_EVAL_LEDOn(LED_30MIN);
			STM_EVAL_LEDOn(LED_60MIN);
			STM_EVAL_LEDOn(LED_PLAY);	
}

/**
  * @brief  Main program.
  * @param  None
  * @retval None
*/

static void udelay(int ntime) {
		 volatile int i=0;
	   for (i=0;i<ntime*10;i++) {}
}

void save_data_to_flash() {
  uint8_t i;
	uint32_t startAddress = 0x080E0000;//starting from 896KB, the beginning of last sector
  uint8_t flash_status = FLASH_COMPLETE;
	
  
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
  flash_status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  
  if (flash_status != FLASH_COMPLETE) {
    FLASH_Lock();
    return;
  }
  
  //program first run status bit
  flash_status = FLASH_ProgramWord((uint32_t)startAddress, 0x5A5A5A5A);
  flash_status = FLASH_ProgramWord((uint32_t)startAddress+0x4, tsc_config.audio_left);
  flash_status = FLASH_ProgramWord((uint32_t)startAddress+0x8, tsc_config.volume_left);
  flash_status = FLASH_ProgramWord((uint32_t)startAddress+0xc, tsc_config.audio_right);
  flash_status = FLASH_ProgramWord((uint32_t)startAddress+0x10, tsc_config.volume_right);
  flash_status = FLASH_ProgramWord((uint32_t)startAddress+0x14, tsc_config.left_right);	
	
	startAddress+=0x18;
	for (i=0;i<8;i++) {
		    FLASH_ProgramWord((uint32_t)startAddress+i*4, (uint32_t)tsc_config.left_v[i]);
	}
	startAddress+=0x20;
	for (i=0;i<8;i++) {
		    FLASH_ProgramWord((uint32_t)startAddress+i*4, (uint32_t)tsc_config.right_v[i]);
	}	
	FLASH_ProgramWord((uint32_t)startAddress+i*4, (uint32_t)loption);
	
    FLASH_Lock();	
}

void load_data_from_flash() {
	uint8_t i;  
	uint32_t startAddress = 0x080E0000;//starting from 896KB, the beginning of last sector
  //printf("load data\r\n");
	if (*(uint32_t *)startAddress == 0x5A5A5A5A) {
		//printf("saved data\r\n"); 
		tsc_config.audio_left=*(uint32_t *)(startAddress +4);
		tsc_config.volume_left=*(uint32_t *)(startAddress +8);
		tsc_config.audio_right=*(uint32_t *)(startAddress +0xc);
		tsc_config.volume_right=*(uint32_t *)(startAddress +0x10);
		tsc_config.left_right=*(uint32_t *)(startAddress +0x14);
		//printf("left_right=%d\r\n",tsc_config.left_right); 
		startAddress+=0x18;
		for (i=0;i<8;i++) {
			    tsc_config.left_v[i]=(uint8_t)*(uint32_t *)(startAddress +i*4);
			   	//printf("left_v %d = %d\r\n",i,tsc_config.left_v[i]);
		}
		startAddress+=0x20;
		for (i=0;i<8;i++) {
			    tsc_config.right_v[i]=(uint8_t)*(uint32_t *)(startAddress +i*4);	
			    //printf("right_v %d = %d\r\n",i,tsc_config.right_v[i]);
		}	
		loption=(uint8_t)*(uint32_t *)(startAddress +i*4);	
		lbeep=loption&0x1;
	} else {
		    //printf("reset data\r\n"); 
				tsc_config.audio_left=1;
				tsc_config.audio_right=1;
				tsc_config.volume_right=DEF_VOL;
				tsc_config.volume_left=DEF_VOL;
				tsc_config.left_right=1;
			for (i=0;i<8;i++) {
						tsc_config.left_v[i]=DEF_VOL;
			}
			for (i=0;i<8;i++) {
						tsc_config.right_v[i]=DEF_VOL;
						
			}			
			loption=0x1;
      save_data_to_flash() ;			
	}
}


#ifdef VER20
void beep(uint8_t nTime) {
	  uint16_t i,j;
	  if ( lbeep  ==1) {
				for (j=0;j<nTime;j++) {
						for (i=0;i<500;i++) {
								GPIOD->BSRRH  = GPIO_Pin_1; 
								udelay(420);		
								GPIOD->BSRRL   =  GPIO_Pin_1;
								udelay(420);		
						}
			}
	}
}
#else
void beep(uint8_t nTime) {
	  uint16_t i,j;
	  for (j=0;j<nTime;j++) {
				for (i=0;i<500;i++) {
						GPIOC->BSRRH  = GPIO_Pin_15; 
						udelay(420);		
						GPIOC->BSRRL   =  GPIO_Pin_15;
						udelay(420);		
				}
	}
}
#endif

static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = (10000 * nCount); index != 0; index--)
  {
  }
}



int main(void)
{
	uint8_t i=0,nKey,nTouch,nType;
	uint8_t str[20];
	uint8_t lPlaying=0;
  char lToggle=0;
	char lsleep=0;


	tsc_config.audio_left=1;
	tsc_config.audio_right=1;
	tsc_config.volume_right=20;;
	tsc_config.volume_left=20;;
	tsc_config.left_right=1;

	
  /* Initialize LEDS */
  STM_EVAL_LEDInit(LED_30MIN);
  STM_EVAL_LEDInit(LED_60MIN);
  STM_EVAL_LEDInit(LED_PLAY);
  
	STM_EVAL_LEDOn(LED_30MIN);
	STM_EVAL_LEDOn(LED_60MIN);
	STM_EVAL_LEDOff(LED_PLAY);
	
  /* Green Led On: start of application */


  
  /* SysTick end of count event each 10ms */
	
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 100);
	/* Set the Vector Table base address at 0x08008000 */
    
	
#if 0   //BOOT VECTOR
  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x20000);
#else
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x00000);
#endif
   TIM_LED_Config();
  /* Configure TIM4 Peripheral to manage LEDs lighting */
	//STM_EVAL_LEDOn(LED_PLAY);

#if 1
//WavePlayerInit(I2S_AudioFreq_44k);
 EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);
Codec_Init(I2S_AudioFreq_48k);
Audio_MAL_Init();
WavePlayBack(I2S_AudioFreq_48k);  		
// WaveplayerCtrlVolume(30);
	STM_EVAL_LEDOff(LED_30MIN);
	STM_EVAL_LEDOff(LED_60MIN);
	//Codec_Volume(0);
 while(1) {}
#endif
	
beep(1);
WavePlayerInit(I2S_AudioFreq_44k);

while (1) {
	    if (lsleep) {
				   if (GPIO_ReadInputDataBit(TOUCH_INT_GPIO_PORT,TOUCH_INT_PIN) ) {
								beep(1);
						    wakeup();
								nTouch=TM_I2C_ReadNoRegister(I2C2, 0xA0);
						    lsleep=0;
					 } else {						 
									continue;
					 }
			}
			if (GPIO_ReadInputDataBit(TOUCH_INT_GPIO_PORT,TOUCH_INT_PIN) ) {
				   beep(1);
					 nTouch=TM_I2C_ReadNoRegister(I2C2, 0xA0);
			     nType=nTouch >> 4;
							
			     if (nType == 1) {  //Right
 							    nKey=nTouch&0xf;
									switch(nKey) {
									case 0x1:
											tsc_config.right_v[tsc_config.audio_right-1]++;
											if (tsc_config.right_v[tsc_config.audio_right-1] > VOL_MAX) tsc_config.right_v[tsc_config.audio_right-1]=VOL_MAX;
									    lNeedSave=1;
											break;
									case 0x2:	
											tsc_config.right_v[tsc_config.audio_right-1]--;
											if (tsc_config.right_v[tsc_config.audio_right-1] < VOL_MIN) tsc_config.right_v[tsc_config.audio_right-1]=VOL_MIN;
									    lNeedSave=1;
											break;
									case 0x4:	
										   tsc_config.audio_right--;
											 if (tsc_config.audio_right == 0) tsc_config.audio_right=1;
											break;
									case 0x8:
											 tsc_config.audio_right++;
											 if (tsc_config.audio_right > 8) tsc_config.audio_right=8;
											break;
								}
							display_status(2);
							AUDIO_RIGHT= tsc_config.audio_right;
				 } else if (nType == 2) { //Center
						    nKey=nTouch&0xf;
								switch(nKey) {
											case 0x1:  //30min
												  if (lPlay == 0) {
															  lPlaying=0;
													}
													if   (lNeedSave == 1) {
																	beep(1);
																	WavePlayerPauseResume(0);
																	WavePlayerPauseResume2(0);
														
																	save_data_to_flash();
																	lNeedSave=0;
																	LCD_LEFT_ON();
																	LCD_RIGHT_ON();
																	GLCD_Clear();
																	GLCD_string_16(1,20,"<볼륨저장됨>",1);												
																	Delay(100);
														      GLCD_Clear();
																	display_status(1);
																	display_status(2);			
																	WavePlayerPauseResume(1);
																	WavePlayerPauseResume2(1);
														
													}																																						
													timer=30;
											    lPlay = 1;
											   Timer_cnt1=0;												
													break;
											case 0x2: //60min
												  if (lPlay == 0) {
															  lPlaying=0;
													}												
												 timer=60;
											   lPlay = 1;
											  Timer_cnt1=0;
												break;
											case 0x4:  //Play
													if (lPlay == 0) {
															lPlay = 1;
															if (timer == 0) {
																	timer=30;
																 Timer_cnt1=0;
															}
													} else {
															lPlay=0;
															WavePlayerPauseResume(0);
															WavePlayerPauseResume2(0);
															lPlaying=0;
													}
													break;	
								 }
						} else if (nType == 4) { //Left
 						 		nKey=nTouch&0xf;
								switch(nKey) {
									case 0x1:
											tsc_config.left_v[tsc_config.audio_left-1]++;
											if (tsc_config.left_v[tsc_config.audio_left-1] > VOL_MAX) tsc_config.left_v[tsc_config.audio_left-1]=VOL_MAX;
									    lNeedSave=1;
											break;
									case 0x2:	
											tsc_config.left_v[tsc_config.audio_left-1]--;
											if (tsc_config.left_v[tsc_config.audio_left-1] < VOL_MIN) tsc_config.left_v[tsc_config.audio_left-1]=VOL_MIN;
									    lNeedSave=1;
											break;
									case 0x4:	
										   tsc_config.audio_left--;
											 if (tsc_config.audio_left== 0) tsc_config.audio_left=1;
											break;
									case 0x8:
											 tsc_config.audio_left++;
											 if (tsc_config.audio_left > 8) tsc_config.audio_left=8;
											break;
								}

								display_status( 1);
  						  AUDIO_LEFT=  tsc_config.audio_left;
						}
					  MAX7219_Freq(1,( tsc_config.audio_left-1),1);
						MAX7219_Freq(2,( tsc_config.audio_right-1),1);
			}
			if (Timer_cnt0 > 8) {  //깜박 깜박하는거네
					Timer_cnt0 =0;
				  if (lToggle== 0) {
							lToggle =1;
						  MAX7219_Freq(1,(tsc_config.audio_left-1),1);
							MAX7219_Freq(2,( tsc_config.audio_right-1),1);
					} else {
						  lToggle =0;
						  if (lPlay== 1) {
											MAX7219_Freq(1,0,0);
											MAX7219_Freq(2,0,0);
							}
					}
			}
			
			if (timer != oldtimer) {
				 oldtimer=timer;
		     MAX7219_Display(LDIGIT,(uint8_t)timer/10);
	       MAX7219_Display(RDIGIT,timer%10);			
			}
			
			if (timer == 0) {
				    if (lPlay == 1) {
							 lPlay=0;
							 WavePlayerPauseResume(0);
							 WavePlayerPauseResume2(0);
							 gosleep();
							 lsleep=1;
							 //LED off
							 //
							 //BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
							 lPlaying = 0;
						}
			} else {
				   if (( lPlay == 1) && (lPlaying == 0)){
						  AUDIO_RIGHT=tsc_config.audio_right;
 	 	 				  AUDIO_LEFT=tsc_config.audio_left; 
						 
							lPlaying = 1;
						  AUDIO_OLD_RIGHT = 0;
							AUDIO_OLD_LEFT   = 0;						 
						  WavePlayerInit(I2S_AudioFreq_44k);
						  WavePlayBack(I2S_AudioFreq_44k);  
							AUDIO_OLD_RIGHT = AUDIO_RIGHT;
							AUDIO_OLD_LEFT   = AUDIO_LEFT;
						  WaveplayerCtrlVolume(volume_right);
						  WaveplayerCtrlVolume2(volume_left);
						  
					 }				
			}

			 if (volume_right != tsc_config.right_v[tsc_config.audio_right-1]) {
					volume_right = tsc_config.right_v[tsc_config.audio_right-1];
					WaveplayerCtrlVolume(volume_right);
			 }

			 if (volume_left != tsc_config.left_v[tsc_config.audio_left-1]) {
					volume_left = tsc_config.left_v[tsc_config.audio_left-1];
					WaveplayerCtrlVolume2(volume_left);
			 }					
			 
			
			if (   ( (AUDIO_RIGHT != AUDIO_OLD_RIGHT) || (AUDIO_LEFT != AUDIO_OLD_LEFT) )&& (lPlay == 1) ) {
				   //초기화가 되어 있으면 바꾸기만 해야 한다.
					 WavePlayBack(I2S_AudioFreq_44k);  				
				   AUDIO_OLD_RIGHT = AUDIO_RIGHT;
				   AUDIO_OLD_LEFT = AUDIO_LEFT;				
					 lPlaying=1;
			}
  }
}


static void display_status(uint8_t left_right) {
		uint8_t str[20];
	  uint8_t vol=0;
	  uint8_t freq=0;
	
						  if (left_right == 1) {
								    LCD_LEFT_ON();
								    LCD_RIGHT_OFF();
								    freq=tsc_config.audio_left;
									  vol=tsc_config.left_v[tsc_config.audio_left-1];
							} else {
								    LCD_LEFT_OFF();
								    LCD_RIGHT_ON();						
										freq=tsc_config.audio_right;								
										vol=tsc_config.right_v[tsc_config.audio_right-1];
							}
							switch(freq) {
							case 1:
								  GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
							    GLCD_icon_20(0,20,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[5][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
								case 2:
									GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[1][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 3:
								  GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[2][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 4:
								  GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[3][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 5:
								  GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[4][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 6:
								 GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[6][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 7:
								  GLCD_icon_20(0, 0,(uint8_t *)&S_font_40[10][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[8][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
							case 8:
									GLCD_icon_20(0,0,(uint8_t *)&S_font_40[1][0],1);
									GLCD_icon_20(0,20,(uint8_t *)&S_font_40[2][0],1);
									GLCD_icon_20(0,40,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,60,(uint8_t *)&S_font_40[0][0],1);
									GLCD_icon_20(0,80,(uint8_t *)&S_font_40[0][0],1);
									break;
						} 			
						GLCD_string_16(1,102,"Hz",1);
						if (lNeedSave == 1) {
						   GLCD_string_16(3,0,"*",1);												
						} else {
							   GLCD_string_16(3,0," ",1);												
						}
						GLCD_string_16(3,20,"볼륨:",1);												  
						GLCD_icon_20(2,60,(uint8_t *)&S_font_40[vol/10][0],1);
						GLCD_icon_20(2,80,(uint8_t *)&S_font_40[vol%10][0],1);
						
						LCD_CS_HIGH();
						LCD_CS2_HIGH();											

}

/**
  * @brief  Configures the TIM Peripheral for Led toggling.
  * @param  None
  * @retval None
  */
static void TIM_LED_Config(void)
{
  TIM_OCInitTypeDef  TIM_OCInitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  uint16_t prescalervalue = 0;
  
  /* TIM4 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Compute the prescaler value */
  prescalervalue = (uint16_t) ((SystemCoreClock ) / 550000) - 1;
  
  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = prescalervalue;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
  
  /* Enable TIM4 Preload register on ARR */
  TIM_ARRPreloadConfig(TIM4, ENABLE);
  
  /* TIM PWM1 Mode configuration: Channel */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = CCR_Val;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
  
  /* Output Compare PWM1 Mode configuration: Channel2 */
  TIM_OC1Init(TIM4, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM4, TIM_OCPreload_Disable);
    
  /* TIM Interrupts enable */
  TIM_ITConfig(TIM4, TIM_IT_CC1 , ENABLE);
  
  /* TIM4 enable counter */
  TIM_Cmd(TIM4, ENABLE);
  
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

  
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
