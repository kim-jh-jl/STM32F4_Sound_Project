/**
  ******************************************************************************
  * @file    Audio_playback_and_record/src/waveplayer.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    28-October-2011
  * @brief   I2S audio program
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tm_stm32f4_usart.h"


/** @addtogroup STM32F4-Discovery_Audio_Player_Recorder
* @{
*/

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#if defined MEDIA_IntFLASH
 /* This is an audio file stored in the Flash memory as a constant table of 16-bit data.
    The audio format should be WAV (raw / PCM) 16-bits, Stereo (sampling rate may be modified) */
extern uint8_t  AUDIO_RIGHT,AUDIO_OLD_RIGHT;
extern uint8_t  AUDIO_LEFT,AUDIO_OLD_LEFT;
extern uint16_t  AUDIO_SAMPLE[];

/* Audio file size and start address are defined here since the audio file is
    stored in Flash memory as a constant table of 16-bit data */
#define AUDIO_FILE_SZE          (131128)
//#define AUDIO_FILE_SZE          (40000)
#define AUIDO_START_ADDRESS     58 /* Offset relative to audio file header size */
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if defined MEDIA_USB_KEY
 extern __IO uint8_t Command_index;
 static uint32_t wavelen = 0;
 static char* WaveFileName ;
 static __IO uint32_t SpeechDataOffset = 0x00;
 __IO ErrorCode WaveFileStatus = Unvalid_RIFF_ID;
 UINT BytesRead;
 WAVE_FormatTypeDef WAVE_Format;
 uint16_t buffer1[_MAX_SS] ={0x00};
 uint16_t buffer2[_MAX_SS] ={0x00};
 uint8_t buffer_switch = 1;
 extern FATFS fatfs;
 extern FIL file;
 extern FIL fileR;
 extern DIR dir;
 extern FILINFO fno;
 extern uint16_t *CurrentPos,*CurrentPos2;
 extern USB_OTG_CORE_HANDLE USB_OTG_Core;
 extern uint8_t WaveRecStatus;
#endif

__IO uint32_t XferCplt = 0;
__IO uint8_t volume_right = 0,volume_left= 0, AudioPlayStart = 0;
__IO uint32_t WaveCounter;
uint8_t Buffer[6];
__IO uint32_t WaveCounter;
uint8_t Buffer[6];
__IO uint32_t WaveDataLength = 0;
extern __IO uint8_t Count;
extern __IO uint8_t RepeatState ;
extern __IO uint8_t LED_Toggle;
extern __IO uint8_t PauseResumeStatus ;
extern uint32_t AudioRemSize,Audio2RemSize;
static __IO uint32_t TimingDelay;

/* Private function prototypes -----------------------------------------------*/
static void Mems_Config(void);
static void EXTILine_Config(void);
#if defined MEDIA_USB_KEY
 static ErrorCode WavePlayer_WaveParsing(uint32_t *FileLen);
#endif

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Play wave from a mass storge
  * @param  AudioFreq: Audio Sampling Frequency
  * @retval None
*/

void WavePlayBack(uint32_t AudioFreq)
{
  /*
  Normal mode description:
  Start playing the audio file (using DMA stream) .
  Using this mode, the application can run other tasks in parallel since
  the DMA is handling the Audio Transfer instead of the CPU.
  The only task remaining for the CPU will be the management of the DMA
  Transfer Complete interrupt or the Half Transfer Complete interrupt in
  order to load again the buffer and to calculate the remaining data.
  Circular mode description:
  Start playing the file from a circular buffer, once the DMA is enabled it
  always run. User has to fill periodically the buffer with the audio data
  using Transfer complete and/or half transfer complete interrupts callbacks
  (EVAL_AUDIO_TransferComplete_CallBack() or EVAL_AUDIO_HalfTransfer_CallBack()...
  In this case the audio data file is smaller than the DMA max buffer
  size 65535 so there is no need to load buffer continuously or manage the
  transfer complete or Half transfer interrupts callbacks. */

  /* Start playing */
  AudioPlayStart = 1;
  RepeatState =0;

  /* Initialize wave player (Codec, DMA, I2C) */

 // WavePlayerInit(AudioFreq);

  /* Play on */
		EVAL_AUDIO_DeInit();
		AudioFlashPlay((uint16_t*)(AUDIO_SAMPLE + AUIDO_START_ADDRESS),AUDIO_FILE_SZE,AUIDO_START_ADDRESS);
}

/**
  * @brief  Pause or Resume a played wave
  * @param  state: if it is equal to 0 pause Playing else resume playing
  * @retval None
  */
void WavePlayerPauseResume(uint8_t state)
{
      //EVAL_AUDIO_PauseResume(state);    WM8731
}



/**
  * @brief  Configure the volune
  * @param  vol: volume value
  * @retval None
  */
uint8_t WaveplayerCtrlVolume(uint8_t vol)
{

  //EVAL_AUDIO_VolumeCtl(vol); WM8731
  return 0;
}



/**
  * @brief  Stop playing wave
  * @param  None
  * @retval None
  */
void WavePlayerStop(void)
{
    //EVAL_AUDIO_Stop(CODEC_PDWN_SW); WM8731
}

/**
* @brief  Initializes the wave player
* @param  AudioFreq: Audio sampling frequency
* @retval None
*/
int WavePlayerInit(uint32_t AudioFreq)
{

  /* Initialize I2S interface */
  EVAL_AUDIO_SetAudioInterface(AUDIO_INTERFACE_I2S);

  /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
  EVAL_AUDIO_Init(OUTPUT_DEVICE_AUTO, volume_right, AudioFreq );

  return 0;
}


/**
* @brief  Play wave file from internal Flash
* @param  None
* @retval None
*/
uint32_t AudioFlashPlay(uint16_t* pBuffer, uint32_t FullSize, uint32_t StartAdd)
{
  EVAL_AUDIO_Play((uint16_t*)pBuffer, (FullSize - StartAdd));  //AUDIO_DUAL
  return 0;

}


/*--------------------------------
Callbacks implementation:
the callbacks prototypes are defined in the stm324xg_eval_audio_codec.h file
and their implementation should be done in the user code if they are needed.
Below some examples of callback implementations.
--------------------------------------------------------*/
/**
* @brief  Calculates the remaining file size and new position of the pointer.
* @param  None
* @retval None
*/
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
  /* Calculate the remaining audio data in the file and the new size
  for the DMA transfer. If the Audio files size is less than the DMA max
  data transfer size, so there is no calculation to be done, just restart
  from the beginning of the file ... */
  /* Check if the end of file has been reached */

#ifdef AUDIO_MAL_MODE_NORMAL

#if defined MEDIA_IntFLASH

#if defined PLAY_REPEAT_OFF
  RepeatState = 1;
  EVAL_AUDIO_Stop(CODEC_PDWN_HW);
#else
  /* Replay from the beginning */
	STM_EVAL_LEDOn(LED_30MIN);
  AudioFlashPlay((uint16_t*)(AUDIO_SAMPLE + AUIDO_START_ADDRESS),AUDIO_FILE_SZE,AUIDO_START_ADDRESS);
#endif

#elif defined MEDIA_USB_KEY
  XferCplt = 1;
  if (WaveDataLength) WaveDataLength -= _MAX_SS;
  if (WaveDataLength < _MAX_SS) WaveDataLength = 0;

#endif

#else /* #ifdef AUDIO_MAL_MODE_CIRCULAR */


#endif /* AUDIO_MAL_MODE_CIRCULAR */
}

/**
* @brief  Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{
#ifdef AUDIO_MAL_MODE_CIRCULAR

#endif /* AUDIO_MAL_MODE_CIRCULAR */

  /* Generally this interrupt routine is used to load the buffer when
  a streaming scheme is used: When first Half buffer is already transferred load
  the new data to the first half of buffer while DMA is transferring data from
  the second half. And when Transfer complete occurs, load the second half of
  the buffer while the DMA is transferring from the first half ... */
  /*
  ...........
  */
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_Error_CallBack(void* pData)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}

  /* could also generate a system reset to recover from the error */
  /* .... */
}

/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
  return 0;
}


/*----------------------------------------------------------------------------*/

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in 10 ms.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
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

/**
* @}
*/


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
