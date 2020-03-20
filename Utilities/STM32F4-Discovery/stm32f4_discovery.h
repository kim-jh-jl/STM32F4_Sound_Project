/**
  ******************************************************************************
  * @file    stm32f4_discovery.h
  * @author  MCD Application Team
  * @version V1.1.0
  * @date    28-October-2011
  * @brief   This file contains definitions for STM32F4-Discovery Kit's Leds and 
  *          push-button hardware resources.
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
  ******************************************************************************  
  */ 
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F4_DISCOVERY_H
#define __STM32F4_DISCOVERY_H

#define VER20

#ifdef __cplusplus
 extern "C" {
#endif
                                              
/* Includes ------------------------------------------------------------------*/
 #include "stm32f4xx.h"
   
/** @addtogroup Utilities
  * @{
  */
  
/** @addtogroup STM32F4_DISCOVERY
  * @{
  */
      
/** @addtogroup STM32F4_DISCOVERY_LOW_LEVEL
  * @{
  */ 

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Exported_Types
  * @{
  */
typedef enum 
{
  LED_30MIN = 0,
  LED_60MIN = 1,
  LED_PLAY  = 2
} Led_TypeDef;

typedef enum 
{  
  BUTTON_USER = 0,
} Button_TypeDef;

typedef enum 
{  
  BUTTON_MODE_GPIO = 0,
  BUTTON_MODE_EXTI = 1
} ButtonMode_TypeDef;     
/**
  * @}
  */ 

/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Exported_Constants
  * @{
  */ 

/** @addtogroup STM32F4_DISCOVERY_LOW_LEVEL_LED
  * @{
  */
//VER20 으로 인한 수정
#define LEDn                             3
#define LED_30MIN_PIN                         GPIO_Pin_14
#define LED_30MIN_GPIO_PORT                   GPIOD
#define LED_30MIN_GPIO_CLK                    RCC_AHB1Periph_GPIOD  

#define LED_60MIN_PIN                         GPIO_Pin_15
#define LED_60MIN_GPIO_PORT                 GPIOD
#define LED_60MIN_GPIO_CLK                    RCC_AHB1Periph_GPIOD  
  
  
#define LED_PLAY_PIN                             GPIO_Pin_8
#define LED_PLAY_GPIO_PORT                   GPIOD
#define LED_PLAY_GPIO_CLK                    RCC_AHB1Periph_GPIOD  
  
/**
  * @}
  */ 
  
/** @addtogroup STM32F4_DISCOVERY_LOW_LEVEL_BUTTON
  * @{
  */  
#define BUTTONn                          1  

/**
 * @brief Wakeup push-button
 */
#define USER_BUTTON_PIN               								GPIO_Pin_0
#define USER_BUTTON_GPIO_PORT          				GPIOA
#define USER_BUTTON_GPIO_CLK           					RCC_AHB1Periph_GPIOA
#define USER_BUTTON_EXTI_LINE          						EXTI_Line0
#define USER_BUTTON_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define USER_BUTTON_EXTI_PIN_SOURCE    		EXTI_PinSource0
#define USER_BUTTON_EXTI_IRQn          					EXTI0_IRQn 


#define USER_L_KEY_PIN               								GPIO_Pin_0
#define USER_L_KEY_GPIO_PORT          				GPIOA
#define USER_L_KEY_GPIO_CLK           					RCC_AHB1Periph_GPIOA
#define USER_L_KEY_EXTI_LINE          						EXTI_Line0
#define USER_L_KEY_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define USER_L_KEY_EXTI_PIN_SOURCE    		EXTI_PinSource0
#define USER_L_KEY_EXTI_IRQn          					EXTI0_IRQn 

#define USER_R_KEY_PIN               								GPIO_Pin_0
#define USER_R_KEY_GPIO_PORT          				GPIOA
#define USER_R_KEY_GPIO_CLK           					RCC_AHB1Periph_GPIOA
#define USER_R_KEY_EXTI_LINE          						EXTI_Line0
#define USER_R_KEY_EXTI_PORT_SOURCE   EXTI_PortSourceGPIOA
#define USER_R_KEY_EXTI_PIN_SOURCE    		EXTI_PinSource0
#define USER_R_KEY_EXTI_IRQn          					EXTI0_IRQn 
/**
  * @}
  */ 
  
/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Exported_Macros
  * @{
  */  
/**
  * @}
  */ 


/** @defgroup STM32F4_DISCOVERY_LOW_LEVEL_Exported_Functions
  * @{
  */
void STM_EVAL_LEDInit(Led_TypeDef Led);
void STM_EVAL_LEDOn(Led_TypeDef Led);
void STM_EVAL_LEDOff(Led_TypeDef Led);
void STM_EVAL_LEDToggle(Led_TypeDef Led);
void STM_EVAL_PBInit(Button_TypeDef Button, ButtonMode_TypeDef Button_Mode);
uint32_t STM_EVAL_PBGetState(Button_TypeDef Button);
/**
  * @}
  */

/** @addtogroup STM3210B_EVAL_SD_SPI
  * @{
  */
/**
  * @brief  SD SPI Interface pins
  */
#define LCD_SPI                           					SPI1
#define LCD_SPI_CLK                       			RCC_APB2Periph_SPI1
#define LCD_SPI_SCK_PIN                   	GPIO_Pin_5                
#define LCD_SPI_SCK_GPIO_PORT      	GPIOA                       
#define LCD_SPI_SCK_GPIO_CLK           RCC_APB1Periph_GPIOA
#define LCD_SPI_MISO_PIN                    GPIO_Pin_6                 
#define LCD_SPI_MISO_GPIO_PORT      GPIOA                       
#define LCD_SPI_MISO_GPIO_CLK         RCC_APB1Periph_GPIOA
#define LCD_SPI_MOSI_PIN                    GPIO_Pin_7                
#define LCD_SPI_MOSI_GPIO_PORT       GPIOA                      
#define LCD_SPI_MOSI_GPIO_CLK          RCC_APB1Periph_GPIOA
	
	
	
#define LCD_CS_LOW()        GPIO_WriteBit(LCD_CS_GPIO_PORT, LCD_CS_PIN, Bit_RESET)
#define LCD_CS_HIGH()       GPIO_WriteBit(LCD_CS_GPIO_PORT, LCD_CS_PIN, Bit_SET)
#define LCD_CS2_LOW()      GPIO_WriteBit(LCD_CS2_GPIO_PORT, LCD_CS2_PIN, Bit_RESET)
#define LCD_CS2_HIGH()     GPIO_WriteBit(LCD_CS2_GPIO_PORT, LCD_CS2_PIN, Bit_SET)


#define LCD_LEFT_ON()         GPIO_WriteBit(LCD_CS_GPIO_PORT, LCD_CS_PIN, Bit_RESET)
#define LCD_LEFT_OFF()        GPIO_WriteBit(LCD_CS_GPIO_PORT, LCD_CS_PIN, Bit_SET)
#define LCD_RIGHT_ON()       GPIO_WriteBit(LCD_CS2_GPIO_PORT, LCD_CS2_PIN, Bit_RESET)
#define LCD_RIGHT_OFF()      GPIO_WriteBit(LCD_CS2_GPIO_PORT, LCD_CS2_PIN, Bit_SET)


#ifdef VER20
#define LCD_D_C_PIN                              GPIO_Pin_8     
#define LCD_D_C_GPIO_PORT                GPIOA                      	
#define LCD_D_C_GPIO_CLK                   RCC_APB1Periph_GPIOA

#define LCD_CS_PIN                              GPIO_Pin_1
#define LCD_CS_GPIO_PORT                GPIOC                      	
#define LCD_CS_GPIO_CLK                   RCC_APB1Periph_GPIOC

#define LCD_CS2_PIN                              GPIO_Pin_2
#define LCD_CS2_GPIO_PORT                GPIOC                      	
#define LCD_CS2_GPIO_CLK                   RCC_APB1Periph_GPIOC

#define LCD_RESET_PIN                          GPIO_Pin_5    
#define LCD_RESET_GPIO_PORT             GPIOB                      	
#define LCD_RESET_GPIO_CLK                RCC_APB1Periph_GPIOB

#define LCD_12V_PIN                          GPIO_Pin_15
#define LCD_12V_GPIO_PORT             GPIOE                       	
#define LCD_12V_GPIO_CLK                RCC_APB1Periph_GPIOE
#else
#define LCD_D_C_PIN                              GPIO_Pin_2     
#define LCD_D_C_GPIO_PORT                GPIOE                       	
#define LCD_D_C_GPIO_CLK                   RCC_APB1Periph_GPIOE

#define LCD_CS_PIN                              GPIO_Pin_1
#define LCD_CS_GPIO_PORT                GPIOA                       	
#define LCD_CS_GPIO_CLK                   RCC_APB1Periph_GPIOA

#define LCD_CS2_PIN                              GPIO_Pin_15     
#define LCD_CS2_GPIO_PORT                GPIOA                       	
#define LCD_CS2_GPIO_CLK                   RCC_APB1Periph_GPIOA

#define LCD_RESET_PIN                          GPIO_Pin_5    
#define LCD_RESET_GPIO_PORT             GPIOE                       	
#define LCD_RESET_GPIO_CLK                RCC_APB1Periph_GPIOE

#define LCD_12V_PIN                          GPIO_Pin_14
#define LCD_12V_GPIO_PORT             GPIOE                       	
#define LCD_12V_GPIO_CLK                RCC_APB1Periph_GPIOE
#endif

#ifdef VER20
#define TOUCH_INT_PIN                          GPIO_Pin_0
#define TOUCH_INT_GPIO_PORT             GPIOA                       	
#define TOUCH_INT_GPIO_CLK                RCC_APB1Periph_GPIOA
#else
#define TOUCH_INT_PIN                          GPIO_Pin_5
#define TOUCH_INT_GPIO_PORT             GPIOC                       	
#define TOUCH_INT_GPIO_CLK                RCC_APB1Periph_GPIOC
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4_DISCOVERY_H */
/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

 

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
