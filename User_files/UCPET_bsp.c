/**
  ******************************************************************************
  * @file    stm3210e_eval.c
  * @author  MCD Application Team
  * @version V4.5.0
  * @date    07-March-2011
  * @brief   This file provides
  *            - set of firmware functions to manage Leds, push-button and COM ports
  *            - low level initialization functions for SD card (on SDIO), SPI serial
  *              flash (sDAC) and temperature sensor (LM75)
  *          available on STM3210E-EVAL evaluation board from STMicroelectronics.  
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

//#include "UCPET_bsp.h"


#include "stm32f2xx_spi.h"
#include "stm32f2xx_dma.h"
#include "stm32f2xx_tim.h"
#include "stm32f2xx_adc.h"
#include "stm32f2xx_rcc.h"


#include "PET_CMD.h"
#include <math.h>
#include "main.h"
#include "string.h"


#include "usbd_link_usr.h"
#include "usbd_link_desc.h"
#include "usbd_link_core.h"


#include "FreeRTOS.h"
#include "task.h"
#include "LinkUsbProcess.h"

#include "flash_if.h"
#include "AdcProcess.h"
#include "MeasureProcess.h"




#include "usbd_test_device_usr.h"
#include "usbd_test_device_desc.h"
#include "usbd_test_device_core.h"

#include "usbh_msc_usr.h"
#include "usbh_Enum_core.h" 

#include "eeprom.h"

  
/**
  * @}
  */
  
/** @defgroup STM3210E_EVAL_LOW_LEVEL_Exported_Macros
  * @{
  */ 


/** @defgroup USB_BSP_Private_Defines
  * @{
  */ 
#define USE_ACCURATE_TIME
#define TIM_MSEC_DELAY       0x01
#define TIM_USEC_DELAY       0x02
#define HOST_OVRCURR_PORT                  GPIOE
#define HOST_OVRCURR_LINE                  GPIO_Pin_1
#define HOST_OVRCURR_PORT_SOURCE           GPIO_PortSourceGPIOE
#define HOST_OVRCURR_PIN_SOURCE            GPIO_PinSource1
#define HOST_OVRCURR_PORT_RCC              RCC_APB2Periph_GPIOE
#define HOST_OVRCURR_EXTI_LINE             EXTI_Line1
#define HOST_OVRCURR_IRQn                  EXTI1_IRQn 

#ifdef USE_STM3210C_EVAL
#define HOST_POWERSW_PORT_RCC              RCC_APB2Periph_GPIOC
#define HOST_POWERSW_PORT                  GPIOC
#define HOST_POWERSW_VBUS                  GPIO_Pin_9
#else
#ifdef USE_USB_OTG_FS 
#define HOST_POWERSW_PORT_RCC              RCC_AHB1Periph_GPIOH
#define HOST_POWERSW_PORT                  GPIOH
#define HOST_POWERSW_VBUS                  GPIO_Pin_5
#endif
#endif


#define HOST_SOF_OUTPUT_RCC                RCC_APB2Periph_GPIOA
#define HOST_SOF_PORT                      GPIOA
#define HOST_SOF_SIGNAL                    GPIO_Pin_8



//DAC DEFS

#define DAC_CMD_WRITE_SELECTED		0x00000000 	 /*Write to Selected DAC Input Register*/
#define DAC_CMD_UPDATE_SELECTED		0x01000000 	 /*Update Selected DAC Registers*/
#define DAC_CMD_UPDATE_ALL  		0x02000000 	 /*Write to Selected DAC Input Register and Update All DAC Registers*/
#define DAC_CMD_UPDATE_RESPECTIVE  	0x03000000 	 /*Write to Selected DAC Input Register and Update Respective DAC Register*/
#define DAC_CMD_CLEARE_TO_ZERO		0x05000000 	 	/*Write to clear code register; clear to zero scale*/
#define DAC_CMD_CLEARE_TO_2V5	0x05000001 	 /*Write to clear code register; clear to midscale*/
#define DAC_CMD_CLEARE_TO_5V	0x05000002 	 /*Write to clear code register; clear to FULLSCALE*/
#define DAC_CMD_CLEARE			0x05000000 	 /*Write to clear code register; ignore CLR pin	  */
#define DAC_CMD_WRITE_TO_LDAC		0x06000000 	 /*Write to LDAC Register*/
#define DAC_CMD_SOFT_RESET  		0x07000000 	 /*Write to LDAC Register*/
#define DAC_CMD_PWR_OPERATION  		    0x04000000 	 /*Power-Down Commands*/
#define DAC_CMD_INTERNAL_REF_CMD_1  	0x08000000 	 /*Power down internal reference - static mode*/
#define DAC_CMD_INTERNAL_REF_CMD_2		0x08000001 	 /*Power up internal reference - static mode*/
#define DAC_CMD_INTERNAL_REF_CMD_3		0x09080000 	 /*Power up internal reference - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_4	    0x090A0000 	 /*Power up internal reference - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_5		0x090C0000 	 /*Power down internal reference all the time regardless of state of DACs - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_6		0x09000000 	 /*Switching internal reference mode from flexible mode to static mode*/
#define DAC_CH_A                		0x00000000 	 /*SELECT DAC Channel A*/
#define DAC_CH_B                		0x00100000 	 /*SELECT DAC Channel B*/
#define DAC_CH_C                		0x00200000 	 /*SELECT DAC Channel C*/
#define DAC_CH_D                		0x00300000 	 /*SELECT DAC Channel D*/
#define DAC_CH_E                		0x00400000 	 /*SELECT DAC Channel E*/
#define DAC_CH_F                		0x00500000 	 /*SELECT DAC Channel F*/
#define DAC_CH_G                		0x00600000 	 /*SELECT DAC Channel G*/
#define DAC_CH_H                		0x00700000 	 /*SELECT DAC Channel H*/
#define DAC_CH_ALL                		0x00F00000 	 /*Broadcast mode - Write to all DAC channels*/



/**
  * @brief  DAC SPI Interface pins
  */  
#define sDAC_SPI                           SPI1
#define sDAC_SPI_CLK                       RCC_APB2Periph_SPI1
#define sDAC_SPI_CLK_INIT                  RCC_APB2PeriphClockCmd


#define sDAC_SPI_SCK_PIN               GPIO_Pin_3                  /* PB.03 */
#define sDAC_SPI_SCK_GPIO_PORT         GPIOB                       /* GPIOB */
#define sDAC_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOB					
#define sDAC_SPI_SCK_SOURCE            GPIO_PinSource3
#define sDAC_SPI_SCK_AF                GPIO_AF_SPI1

#define sDAC_SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.07 */
#define sDAC_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define sDAC_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define sDAC_SPI_MOSI_SOURCE           GPIO_PinSource7
#define sDAC_SPI_MOSI_AF  			 GPIO_AF_SPI1

#define sDAC_CS_PIN                    GPIO_Pin_4                  /* PA.04 */
#define sDAC_CS_GPIO_PORT              GPIOA                       /* GPIOA */
#define sDAC_CS_GPIO_CLK               RCC_AHB1Periph_GPIOA

#define sDAC_CS_LOW()       GPIO_ResetBits(sDAC_CS_GPIO_PORT, sDAC_CS_PIN)
#define sDAC_CS_HIGH()      GPIO_SetBits(sDAC_CS_GPIO_PORT, sDAC_CS_PIN)  

/**
  * @}
  */
#define ADC3_DR_ADDRESS    ((uint32_t)0x4001224C)
#define ADC_Average_Num 16
#define ADC_Buf_Size 8


#define TIM3_CCR1_VAL 10


#define SYSTICK_CSR       (*((volatile unsigned long *)0xE000E010))


/* Private variables ---------------------------------------------------------*/
ADC_InitTypeDef ADC_InitStructure;
DMA_InitTypeDef DMA_InitStructure;


TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
TIM_OCInitTypeDef  TIM_OCInitStructure;



//量程本地变量

static uint8_t LoadCurrentRange = HI_RANGE;	  //LoadCurrentRange，当前帧的电流量程(HI,LO)
static uint8_t VbusCurrentRange = HI_RANGE;	  //VbusCurrentRange，当前帧的电流量程(HI,LO)
static uint8_t LoadCurrentRangeMode = HI_RANGE; //LoadCurrentRangeMode, 当前系统的电流量程模式（HI,LO,AUTO）
static uint8_t VbusCurrentRangeMode = HI_RANGE;	 //VbusCurrentRangeMode, 当前系统的电流量程模式（HI,LO,AUTO）
//static uint8_t EL_MODE;


/*USB 全局变量*/
USB_OTG_CORE_HANDLE    g_USB_link_dev  ;   // FOR LINK DEVICE
USB_OTG_CORE_HANDLE    USB_OTG_HS_Core  ;  // FOR TEST DEVICE AND HOST
USBH_HOST USB_Host; 
uint8_t USBD_at_FS;
uint8_t USBH_at_FS;


extern uint8_t HostDev_Manufacturer[64];
extern uint8_t HostDev_ProductString[64];
extern uint8_t HostDev_SerialNumString[64];

//const uint16_t CCR1_Val = 10;  //5us timebase 


//ADC 全局变量
__IO uint16_t ADC_ConvertedBuff0[ADC_Buf_Size];	//__IO = volatile,表示该变量的值觉的改变无法预知，
//__IO uint16_t ADC_ConvertedBuff1[ADC_Buf_Size];	 //告诉编译器不要去假设这个变量的值

__IO uint16_t ADC_ProcessBuff[ADC_Buf_Size];

//__IO uint16_t* pCurADC_ConvertedBuff = ADC_ConvertedBuff1; 	 //由于toggle程序先翻转，而第一次需要使用0，所以初始化成1




void UcpetBoardInit(void);
void USB_LINK_Init(void);
void Muti_CH_ADC_Init(void);
void GPIO_Configuration(void);
void USB_LINK_Init(void);
void ResetUsbLink(void);

void GPIO_Setting (uint8_t io_num,uint16_t para);
void DAC_Setting(uint8_t dac_name,int16_t dac_value);
void GPIO_Toggle(uint8_t io_num);

void AdcPtrToggle(void);
void GetRunTimeStats(void);
void T3_CCR1_Handle(void);

uint8_t GetLoadCurrentRange(void);
void SetLoadCurrentRange(uint8_t range)	;
uint8_t GetVbusCurrentRange(void)		;
void SetVbusCurrentRange(uint8_t range)	 ;
uint8_t GetCurrentRangeMode(uint8_t name);
void SetCurrentRangeMode(uint8_t name,uint8_t mode);

void USB_OTG_BSP_Init (USB_OTG_CORE_HANDLE *pdev);
void USB_OTG_BSP_uDelay (const uint32_t usec);
void USB_OTG_BSP_mDelay (const uint32_t msec);
void USB_OTG_BSP_EnableInterrupt (USB_OTG_CORE_HANDLE *pdev);
#ifdef USE_HOST_MODE
void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev);
void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev,uint8_t state);
#endif


/**************************************************************************************************/
/**
  * @brief  gpio init
  * @param  None
  * @retval None
  */
void GPIO_Configuration(void)
{
  
   /*初始化用作OUT的GPIO*/
  GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable the GPIOA Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	 //V3.0 
  /* Configure PA0-PA2,PA8,PA10,PA15 as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

      /* Enable the GPIOB Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    /* Configure PB4,PB6-PB9,PB14,PB15 as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_14|GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

        /* Enable the GPIOC Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    /* Configure PC6-12 as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

       /* Enable the GPIOD Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    /* Configure PD0-15 as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOD, &GPIO_InitStructure);

        /* Enable the GPIOE Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
   /* Configure PE015, as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

	      /* Enable the GPIOG Clock */
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
     /* Configure PG0-15 as pp output --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOG, &GPIO_InitStructure);

   /*初始化用作IN的GPIO PD0*/
    /* Configure  PA9 as FLOAT INPUT --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
     /* Configure  PE4 as FLOAT INPUT --------------*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOE, &GPIO_InitStructure);


   //初始化SPI用到的GPIO
  /*!< Enable the SPI clock */
  sDAC_SPI_CLK_INIT(sDAC_SPI_CLK, ENABLE);
  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(sDAC_SPI_SCK_GPIO_CLK |sDAC_SPI_MOSI_GPIO_CLK | sDAC_CS_GPIO_CLK, ENABLE);  
  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(sDAC_SPI_SCK_GPIO_PORT, sDAC_SPI_SCK_SOURCE, sDAC_SPI_SCK_AF);
  GPIO_PinAFConfig(sDAC_SPI_MOSI_GPIO_PORT, sDAC_SPI_MOSI_SOURCE, sDAC_SPI_MOSI_AF);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_DOWN;        
  /*!< SPI SCK pin configuration */
  GPIO_InitStructure.GPIO_Pin = sDAC_SPI_SCK_PIN;
  GPIO_Init(sDAC_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);
  /*!< SPI MOSI pin configuration */
  GPIO_InitStructure.GPIO_Pin =  sDAC_SPI_MOSI_PIN;
  GPIO_Init(sDAC_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);
  /*!< Configure sDAC Card CS pin in output pushpull mode ********************/
  GPIO_InitStructure.GPIO_Pin = sDAC_CS_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(sDAC_CS_GPIO_PORT, &GPIO_InitStructure);
  


}
																				   


/**
  * @brief  USB_OTG_BSP_Init
  *         Initilizes BSP configurations
  * @param  None
  * @retval None
  */

void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{


  GPIO_InitTypeDef GPIO_InitStructure;
 //#ifdef USE_USB_OTG_FS 
 
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);  
  
  /* Configure SOF VBUS ID DM DP Pins */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ; 
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;
  
    

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ; 
 //#endif 

// #ifdef USE_USB_OTG_HS 

  // ULPI
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                         RCC_AHB1Periph_GPIOC, ENABLE);    
  
  
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_OTG2_HS) ; // D0
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // CLK
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // D1
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource1, GPIO_AF_OTG2_HS) ; // D2
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // D7
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_OTG2_HS) ; // D3
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_OTG2_HS) ; // D4
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_OTG2_HS) ; // D5
  GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_OTG2_HS) ; // D6
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_OTG2_HS);  //PC3->OTG_HS_ULPI_NXT 
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_OTG2_HS);  //PC2->OTG_HS_ULPI_DIR
  GPIO_PinAFConfig(GPIOC,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // STP
  
  // CLK
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  // D0
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  ; 
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  
  
  // D1 D2 D3 D4 D5 D6 D7
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1  |
    GPIO_Pin_5 | GPIO_Pin_10 | 
      GPIO_Pin_11| GPIO_Pin_12 | 
        GPIO_Pin_13 ;
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);  
  
  
  // STP
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  ;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOC, &GPIO_InitStructure);  
  
  //NXT  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOH, &GPIO_InitStructure);  
  
  
  //NXT  & DIR
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(GPIOC, &GPIO_InitStructure);   
  
  
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS | 
                         RCC_AHB1Periph_OTG_HS_ULPI, ENABLE) ;    
   
   
 //#endif //USB_OTG_HS


  /* Intialize Timer for delay function */
  //USB_OTG_BSP_TimeInit();   
}
/**
  * @brief  USB_OTG_BSP_EnableInterrupt
  *         Configures USB Global interrupt
  * @param  None
  * @retval None
  */
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
  NVIC_InitTypeDef NVIC_InitStructure; 
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
#ifdef USE_USB_OTG_HS   
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 4;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
#endif

#ifdef USE_USB_OTG_FS   
  NVIC_InitStructure.NVIC_IRQChannel = OTG_FS_IRQn;  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
#endif
 
#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);  
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);   
#endif  
}

/**
  * @brief  BSP_Drive_VBUS
  *         Drives the Vbus signal through IO
  * @param  state : VBUS states
  * @retval None
  */

void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev, uint8_t state)
{
  /*
  On-chip 5 V VBUS generation is not supported. For this reason, a charge pump 
  or, if 5 V are available on the application board, a basic power switch, must 
  be added externally to drive the 5 V VBUS line. The external charge pump can 
  be driven by any GPIO output. When the application decides to power on VBUS 
  using the chosen GPIO, it must also set the port power bit in the host port 
  control and status register (PPWR bit in OTG_FS_HPRT).
  
  Bit 12 PPWR: Port power
  The application uses this field to control power to this port, and the core 
  clears this bit on an overcurrent condition.
  */
#ifndef USE_USB_OTG_HS   
  if (0 == state)
  { 
    /* DISABLE is needed on output of the Power Switch */
    GPIO_SetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
  }
  else
  {
    /*ENABLE the Power Switch by driving the Enable LOW */
    GPIO_ResetBits(HOST_POWERSW_PORT, HOST_POWERSW_VBUS);
  }
#endif  
}

/**
  * @brief  USB_OTG_BSP_ConfigVBUS
  *         Configures the IO for the Vbus and OverCurrent
  * @param  None
  * @retval None
  */

void  USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev)
{
 //no need control vbus here

  // prepareVBusForPD();
  // GPIO_Setting(RSW_DMDP_TEST,ON); //Apply vbus_gen to vbus_ab
  	
}



/**
  * @brief  USB_OTG_BSP_uDelay
  *         This function provides delay time in micro sec
  * @param  usec : Value of delay required in micro sec
  * @retval None
  */
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
  
//#ifdef USE_ACCURATE_TIME    
//  BSP_Delay(usec,TIM_USEC_DELAY); 
//#else
  __IO uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);
//#endif   
  
}


/**
  * @brief  USB_OTG_BSP_mDelay
  *          This function provides delay time in milli sec
  * @param  msec : Value of delay required in milli sec
  * @retval None
  */
void USB_OTG_BSP_mDelay (const uint32_t msec)
{ 
//#ifdef USE_ACCURATE_TIME  
 //   BSP_Delay(msec,TIM_MSEC_DELAY);   
//#else
    USB_OTG_BSP_uDelay(msec * 1000);   
//#endif    

}

//初始化FS USB LINK 
void USB_LINK_Init(void)
{
   /*初始化FS USB*/
   USBD_Init(&g_USB_link_dev,
            USB_OTG_FS_CORE_ID,
            &LINK_desc,             
            &USBD_FS_LINK_cb, 
            &LINK_USR_cb);

}

void USB_LINK_Suspend(void)
{
  USB_OTG_StopDevice(&g_USB_link_dev)  ;
  USB_LINK_Init();  
}



//设置开关
void GPIO_Setting (uint8_t io_num,uint16_t para)
{

 uint16_t io_pin = 0;
 GPIO_TypeDef* gpio_port = GPIOA;
 io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;
  else 
 	return;

 if(para == 0 ) //	
   {
	  GPIO_ResetBits(gpio_port, io_pin); 
   }
 else 
   {
	  GPIO_SetBits(gpio_port, io_pin);
   }

}

//读取开关状态  GPIO_ReadOutputDataBit
uint8_t GPIO_Reading (uint8_t io_num)
{

 uint16_t io_pin = 0;
 GPIO_TypeDef* gpio_port = GPIOA;
 io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;


  return  GPIO_ReadOutputDataBit(gpio_port,io_pin) ;
}



// LED_State_toggle

void GPIO_Toggle(uint8_t io_num)
{
  uint16_t io_pin = 0;
  GPIO_TypeDef* gpio_port = GPIOA;
  io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;
  else 
 	return;

   GPIO_ToggleBits(gpio_port, io_pin);
} 



/*********************************************/
/*	Muti_CH_ADC_Init					   			*/
/*  初始化8个ADC3通道，使用Scan模式，单次转换，软件触发模式*/
/*  使用DMA2_stream0 传输，使用debule buff模式*/

/*********************************************/ 
void Muti_CH_ADC_Init (void)
{
    ADC_InitTypeDef       ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
	 DMA_InitTypeDef       DMA_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;
//    NVIC_InitTypeDef NVIC_InitStructure;

   /* Enable  ADC3,DMA2 and GPIO clocks ****************************************/
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOF|RCC_AHB1Periph_DMA2, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
 
  DMA_InitStructure.DMA_Channel = DMA_Channel_2; 
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADC_ConvertedBuff0;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC3_DR_ADDRESS;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = ADC_Buf_Size;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;

  //DoubleBufferMode config: Two functions must be called before calling the DMA_Init() function:
  //DMA首次写入地址为ADC_ConvertedBuff1

  //DMA_DoubleBufferModeConfig(DMA2_Stream0,(uint32_t)&ADC_ConvertedBuff1,DMA_Memory_1);
  //DMA_DoubleBufferModeCmd(DMA2_Stream0,ENABLE);

  DMA_Init(DMA2_Stream0, &DMA_InitStructure);

  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);
   

  
   /*初始化GPIO*/


   /* ADC_LOAD_V = PF10 = ADC3_IN8*/
   /* ADC_LOAD_I = PF8 = ADC3_IN6*/
   /* ADC_VBUS_V = PF9 = ADC3_IN7*/
   /* ADC_VBUS_I = PF7 = ADC3_IN5*/
   /* ADC_DP = PF6 = ADC3_IN4*/
   /* ADC_DM = PF5 = ADC3_IN15*/
   /* ADC_ID (M2)= PF4 = ADC3_IN14*/
   /* ADC_VBUS_AB (M1) = PF3 = ADC3_IN9*/

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOF, &GPIO_InitStructure);



   /* ADC Common Init **********************************************************/
  ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_10Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);


  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;//软件触发
 // ADC_InitStructure.ADC_ExternalTrigConvEdge =ADC_ExternalTrigConvEdge_Falling ;// T3_CC1 触发
 // ADC_InitStructure.ADC_ExternalTrigConv =  ADC_ExternalTrigConv_T3_CC1 ;//	T3_CC1 触发
  ADC_InitStructure.ADC_NbrOfConversion = 8;
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_Init(ADC3, &ADC_InitStructure);


	   	  /* ADC3 regular channel Configuration */ 

   ADC_RegularChannelConfig(ADC3, ADC_Channel_6, 1, ADC_SampleTime_144Cycles);	  //ADC_LOAD_I
   ADC_RegularChannelConfig(ADC3, ADC_Channel_7, 2, ADC_SampleTime_144Cycles);	  //ADC_VBUS_V
   ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 3, ADC_SampleTime_144Cycles);	  //ADC_VBUS_I
   ADC_RegularChannelConfig(ADC3, ADC_Channel_4, 4, ADC_SampleTime_144Cycles);	  //ADC_DP
   ADC_RegularChannelConfig(ADC3, ADC_Channel_15, 5, ADC_SampleTime_144Cycles);	  //DM
   ADC_RegularChannelConfig(ADC3, ADC_Channel_14, 6, ADC_SampleTime_144Cycles);	  //ADC_ID (M2)
   ADC_RegularChannelConfig(ADC3, ADC_Channel_9, 7, ADC_SampleTime_144Cycles);	  //ADC_VBUS_AB (M1)
   ADC_RegularChannelConfig(ADC3, ADC_Channel_8, 8, ADC_SampleTime_144Cycles);	  //ADC_LOAD_V


   //ADC_ExternalTrigConvCmd(ADC3,ENABLE);

  /* Enable DMA request after last transfer (Single-ADC mode) */
  ADC_DMARequestAfterLastTransferCmd(ADC3,ENABLE);		//要是能，否则第二次启动不了DMA.
     /* Enable ADC3 DMA */
  ADC_DMACmd(ADC3,ENABLE);
    /* Enable ADC3 */
  ADC_Cmd(ADC3, ENABLE);

  //启动一次转换，结果进入 ADC_ConvertedBuff0
  //ADC_SoftwareStartConv(ADC3);



}





/***********************************/
/*	sDAC_SendWord				  */
/*  向SPI1发送入一个32bit数据	   */
/***********************************/ 

void sDAC_SendWord(uint32_t Word)
{
  uint16_t HalfWord = (Word & 0xffff0000)>>16;

  /*DAC_SYNC = LOW*/
   sDAC_CS_LOW();

  /*!< Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(sDAC_SPI, SPI_I2S_FLAG_TXE) == RESET);
  /*!< Send HIGH Half Word through the sFLASH peripheral */
  SPI_I2S_SendData(sDAC_SPI, HalfWord);
  
  HalfWord = Word; 
   /*!< Send Low Half Word through the sFLASH peripheral */
  while (SPI_I2S_GetFlagStatus(sDAC_SPI, SPI_I2S_FLAG_TXE) == RESET);
  SPI_I2S_SendData(sDAC_SPI, HalfWord);
   
   /*wait BSY = reset*/

  while (SPI_I2S_GetFlagStatus(sDAC_SPI, SPI_I2S_FLAG_BSY) == SET);

  /*DAC_SYNC = HIGH*/
   sDAC_CS_HIGH();

}


/*********************************************/
/*	sDAC_CMD					   			*/
/*  此程序将有关参数整合成一个32bit数据	   */
/*	然后调用sDAC_SendWord,将此数据写入DAC7568*/
/*********************************************/ 

void sDAC_CMD(uint32_t Dac_Control,uint32_t Dac_Addr,uint16_t Dac_Data)
{
 uint32_t tmp_word=Dac_Data;
 tmp_word = ((tmp_word)<<8)|Dac_Control|Dac_Addr;
 sDAC_SendWord(tmp_word) ;

}



//DAC7568 
void sDAC_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  sDAC_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(sDAC_SPI, &SPI_InitStructure);

  /*!< Enable the sFLASH_SPI  */
  SPI_Cmd(sDAC_SPI, ENABLE);

  sDAC_CMD(DAC_CMD_SOFT_RESET,0,0) ;
}

/*初始化TIMER x*/
void TIM_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  uint16_t PrescalerValue = 0;
  
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 gloabal Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

   /* ---------------------------------------------------------------
    TIM3 Configuration: Output Compare Timing Mode:
    TIM3 counter clock at 1 MHz(1us)
     - CC1 update rate = TIM3 counter clock / CCR1_Val = 200k Hz
	
      				
  --------------------------------------------------------------- */

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1;

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 65535;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);

  /* Output Compare Timing Mode configuration: Channel1 */
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = TIM3_CCR1_VAL;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

  TIM_OC1Init(TIM3, &TIM_OCInitStructure);
  TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Disable);


   
  /* TIM Interrupts enable */
 //	TIM_ITConfig(TIM3, TIM_IT_CC1|TIM_IT_CC2, ENABLE);
	TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);		   //只打开了CC1


  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE);
}

void DAC_Setting(uint8_t dac_name,int16_t dac_value)
{
   uint16_t tmpValue=0;

  	switch 	(dac_name)
	{
	 case 0:
	 		if (dac_value <=7500 && dac_value >=0)	//VBUS_V电压(0-7.5v)/3 = 0-2.5V
			tmpValue = (4096*dac_value)/(2500*3);	//3分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_E,tmpValue);	
			break;
	 case 1:  //设置EL工作电流
	 //		if(EL_MODE == CC)
		//	{
				//V4: 1.4v/A 
		 		if (dac_value <=1500 && dac_value >=0)	//
				tmpValue = (4096*7*dac_value)/(2500*5);	//
		 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_A,tmpValue); //实际设置DAC_EL_IL
	//		}
		//	else
		//	{
		//	 	if (dac_value <=5000 && dac_value >=0)	//LOAD_IV电压CV模式 1mV = 3mV
		//		tmpValue = (4096*dac_value)/(2500*3);	//3分压
		// 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);
		//	}		   
			break;				 
	 case 2:
	 		if (dac_value <=1800 && dac_value >=0)	//VBUS_IL电流0-0.75V ,0.5R
			tmpValue = (4096*7*dac_value)/(2500*5);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_C,tmpValue);	
			break;				
	 case 3: //设定EL的电压， 1/3分压
	 	//	if (dac_value <=1800 && dac_value >=0)	//LOAD_IL限流设定0-1.5V
		//	tmpValue = (4096*dac_value)/(2500*3);	//3分压
	    tmpValue = (4096*dac_value)/(2500*3);//3分压	
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);	
			break;				 
	 case 4:
	 		if (dac_value <=2500 && dac_value >=0)	//CM_V 电压0-2.5V
			tmpValue = (4096*dac_value)/(2500);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_D,tmpValue);	
			break;				 
	 case 5:
	 		if (dac_value <=6000 && dac_value >=0)	//DP电压0-3.6V ,2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_H,tmpValue);	
			break;				 
	 case 6:
			if (dac_value <=6000 && dac_value >=0)	//DM电压0-3.6V ，2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_F,tmpValue);	
			break;
	 default:
	   
	 		break;			
	}

}
 
//上位机软件，DP/DM是反的
void DAC_Setting_link(uint8_t dac_name,int16_t dac_value)
{
   uint16_t tmpValue=0;

  	switch 	(dac_name)
	{
	 case 0:
	 		if (dac_value <=7500 && dac_value >=0)	//VBUS_V电压(0-7.5v)/3 = 0-2.5V
			tmpValue = (4096*dac_value)/(2500*3);	//3分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_E,tmpValue);	
			break;
	 case 1:  //设置EL工作电流
	 //		if(EL_MODE == CC)
		//	{
				//V4: 1.4v/A 
		 		if (dac_value <=1500 && dac_value >=0)	//
				tmpValue = (4096*7*dac_value)/(2500*5);	//
		 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_A,tmpValue); //实际设置DAC_EL_IL
	//		}
		//	else
		//	{
		//	 	if (dac_value <=5000 && dac_value >=0)	//LOAD_IV电压CV模式 1mV = 3mV
		//		tmpValue = (4096*dac_value)/(2500*3);	//3分压
		// 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);
		//	}		   
			break;				 
	 case 2:
	 		if (dac_value <=1800 && dac_value >=0)	//VBUS_IL电流0-0.75V ,0.5R
			tmpValue = (4096*7*dac_value)/(2500*5);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_C,tmpValue);	
			break;				
	 case 3: //设定EL的电压， 1/3分压
	 	//	if (dac_value <=1800 && dac_value >=0)	//LOAD_IL限流设定0-1.5V
		//	tmpValue = (4096*dac_value)/(2500*3);	//3分压
	    tmpValue = (4096*dac_value)/(2500*3);//3分压	
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);	
			break;				 
	 case 4:
	 		if (dac_value <=2500 && dac_value >=0)	//CM_V 电压0-2.5V
			tmpValue = (4096*dac_value)/(2500);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_D,tmpValue);	
			break;				 
	 case 5:
	 		if (dac_value <=6000 && dac_value >=0)	//DM电压0-3.6V ,2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_F,tmpValue);	
			break;				 
	 case 6:
			if (dac_value <=6000 && dac_value >=0)	//DP电压0-3.6V ，2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_H,tmpValue);	
			break;
	 default:
	   
	 		break;			
	}

}


///////////SYSTICK处理

void SysTickDisable(void)
{
  /*由于DFU引导程序中，已经设置并打开了SYSTICK,
    跳转到应用程序后，在Rtos初始化之前，必须先把
	systick控制寄存器恢复成初始值0x0.
	否则就需要修改DFU引导程序。
											*/
 SYSTICK_CSR =0x00; //关闭systick

}



void UcpetBoardInit(void)
{
   SysTickDisable();


   
    GPIO_Configuration();

   //ADC
   Muti_CH_ADC_Init();
     /* 初始化SPI DAC7568 */ 
  sDAC_Init();
  sDAC_CMD(DAC_CMD_SOFT_RESET,0,0) ;
  sDAC_CMD(DAC_CMD_INTERNAL_REF_CMD_4,0,0) ;	// NEW FOR V4


   //初始化ADC矫正数组
   LoadCorData();

   //初始化ADC buff
  // AdcBuffInit();

   //tim3 for ran time stat
   //TIM_Configuration() ;

   GPIO_Setting(SW_LED1_G,OFF);
   GPIO_Setting(SW_LED2_G,OFF);

  	/*使能ADC_LOAD_I 的输入,用于防止在未正确配置运放时，输出高压损害ADC*/
   GPIO_Setting(SW_ADC_LOAD_I_EN, ON);
   	/*使能ADC_VBUS_I 的输入,用于防止在未正确配置运放时，输出高压损害ADC*/
   GPIO_Setting(SW_ADC_VBUS_I_EN, ON);
   /*DP,DM缺省设置*/
   GPIO_Setting(RSW_DMDP_TEST,OFF);
   /*使用M2来检测ID_AB*/
   GPIO_Setting(SW_M_ID,ON);
    /*使用M1来检测VBUS_AB*/
  GPIO_Setting(SW_VBUS_AB_M1,ON);

  //if(GPIO_Reading(RSW_AGND_TO_GND_AB) == 0)
 // {
    GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);
	//UCPET 初始化为完成时，亮红灯
	GPIO_Setting(SW_LED2_R,ON);
//   }

   SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);
   //SetLoadCurrentRange(LO_RANGE)	;
   //SetVbusCurrentRange(LO_RANGE)	;

   SetAdcDataUploadState(TRUE);

      	//初始化eeprom，并load pre_defined_para
   EE_Init();
   LoadPreDefinedPara();
  
   /*初始化FS USB*/
   USB_LINK_Init();

	//初始化完成后，关闭红灯
   GPIO_Setting(SW_LED2_R,OFF);
   GPIO_Setting(SW_LED2_G,ON);


}



void CopyAdcData2PorcessBuf(void)
{
  memcpy((uint8_t *)ADC_ProcessBuff,(uint8_t *)ADC_ConvertedBuff0,16);

}

/*
void GetRunTimeStats(void)
{
	signed char charBuf[256];
   //	 vTaskGetRunTimeStats(charBuf)	  ;
 	 Report_MSG((char*)charBuf)	;

}
*/

void T3_CCR1_Handle(void)
{
   if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
  {
    TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);

//	ulHighFreqTimerTicks++;


	//capture = TIM_GetCapture1(TIM3);
    TIM_SetCompare1(TIM3, TIM_GetCapture1(TIM3) + TIM3_CCR1_VAL);
  }

}


uint8_t GetLoadCurrentRange()
{
	 return   LoadCurrentRange;
}

void SetLoadCurrentRange(uint8_t range)
{
	  if(range == LO_RANGE)
	  {
	  GPIO_Setting(SW_LOAD_CF_switch, ON);
	  LoadCurrentRange = LO_RANGE;
	  }
	  else if(range == HI_RANGE)
	  {
 	   GPIO_Setting(SW_LOAD_CF_switch, OFF);
	   LoadCurrentRange = HI_RANGE;
	  }	
}

uint8_t GetVbusCurrentRange()
{
	  return VbusCurrentRange;
}

void SetVbusCurrentRange(uint8_t range)
{

	  if(range == LO_RANGE)
	  {
	  GPIO_Setting(SW_VBUS_CF_switch, ON);
	  VbusCurrentRange = LO_RANGE;
	  }
	  else if(range == HI_RANGE)
	  {
 	   GPIO_Setting(SW_VBUS_CF_switch, OFF);
	   VbusCurrentRange = HI_RANGE;
	  }	

}

void SetCurrentRangeMode(uint8_t name,uint8_t mode)
{
	if(name == ADC_LOAD_I)
	 {
	 LoadCurrentRangeMode = mode;
	 SetLoadCurrentRange(mode);
	 }
	 else if (name == ADC_VBUS_I)
	 {
	 VbusCurrentRangeMode = mode;
	 SetVbusCurrentRange(mode)	;
	 }
}

uint8_t GetCurrentRangeMode(uint8_t name)
{
	 uint8_t mode = AUTO_RANGE;
     if(name == ADC_LOAD_I)
	 {
	 mode = LoadCurrentRangeMode;
	 }
	 else if (name == ADC_VBUS_I)
	 {
	 mode = VbusCurrentRangeMode;
	 }
	 return  mode;

}

//reset link

void ResetUsbLink(void)
{
 	 DCD_DevDisconnect(&g_USB_link_dev);
	 vTaskDelay(500);
	 DCD_DevConnect(&g_USB_link_dev);
}




void ID_RES_Setting(uint8_t res)
{

	   GPIO_Setting(SW_ID_GND,0);	//ID和GND_AB间加载0欧姆	  RES_Rid_GND
	   GPIO_Setting(SW_ID_RES_1,0);	//ID和GND_AB间加载1K	  RES_Rid_GND_max
	   GPIO_Setting(SW_ID_RES_2,0);	//ID和GND_AB间加载35K	  RES_Rid_C_min
	   GPIO_Setting(SW_ID_RES_3,0);	//ID和GND_AB间加载39K	  RES_Rid_C_max
	   GPIO_Setting(SW_ID_RES_4,0); //ID和GND_AB间加载65K	  RES_Rid_B_min
	   GPIO_Setting(SW_ID_RES_5,0);	//ID和GND_AB间加载72K	  RES_Rid_B_max
	   GPIO_Setting(SW_ID_RES_6,0);	//ID和GND_AB间加载119K	  RES_Rid_A_min
	   GPIO_Setting(SW_ID_RES_7,0);	//ID和GND_AB间加载132K	  RES_Rid_A_max
	   GPIO_Setting(SW_ID_RES_8,0);	//ID和GND_AB间加载220K	  RES_Rid_FLOAT
	    switch (res)
   	{
	   case RES_Rid_GND:
	  	 	GPIO_Setting(SW_ID_GND,1);	
	   		break;
	   case RES_Rid_GND_max:
	  	 	GPIO_Setting(SW_ID_RES_1,1);	
	   		break;
	   case RES_Rid_C_min:
	 		GPIO_Setting(SW_ID_RES_2,1);	
			break;	   
       case RES_Rid_C_max:
	 		GPIO_Setting(SW_ID_RES_3,1);	
			break;	   
	   case RES_Rid_B_min:
	 		GPIO_Setting(SW_ID_RES_4,1);
			break;
	   case RES_Rid_B_max :
	   		GPIO_Setting(SW_ID_RES_5,1);
			break;
	   case RES_Rid_A_min :	
			GPIO_Setting(SW_ID_RES_6,1);
			break;
	   case RES_Rid_A_max :	
			GPIO_Setting(SW_ID_RES_7,1);
			break;
	   case RES_Rid_FLOAT :	
			GPIO_Setting(SW_ID_RES_8,0);
			break;
	   case RES_Rid_OPEN :	
			
			break;
	   default:	   
	 		break;
	 }

}

void VBUS_CAP_Setting(uint8_t cap)
{
	   GPIO_Setting(RSW_VBUS_CAP_1,0);	//150nF
	   GPIO_Setting(RSW_VBUS_CAP_2,0);	//900nF
	   GPIO_Setting(RSW_VBUS_CAP_3,0);	//1uF
	   GPIO_Setting(RSW_VBUS_CAP_4,0);	//5.5uF
	   GPIO_Setting(RSW_VBUS_CAP_5,0); //10uF
	   GPIO_Setting(RSW_VBUS_CAP_6,0); //100uF

	    switch (cap)
   	{
	   case CAP_150nF:
	  	 	GPIO_Setting(RSW_VBUS_CAP_1,1);	
	   		break;
	   case CAP_900nF:
	  	 	GPIO_Setting(RSW_VBUS_CAP_2,1);	
	   		break;
	   case CAP_1uF:
	 		GPIO_Setting(RSW_VBUS_CAP_3,1);	
			break;	   
       case CAP_5uF5:
	 		GPIO_Setting(RSW_VBUS_CAP_4,1);	
			break;	   
	   case CAP_6uF5:
	 		GPIO_Setting(RSW_VBUS_CAP_3,1);
			GPIO_Setting(RSW_VBUS_CAP_4,1);
			break;
	   case CAP_10uF :
	   		GPIO_Setting(RSW_VBUS_CAP_5,1);
			break;
	   case CAP_100uF :
	   		GPIO_Setting(RSW_VBUS_CAP_6,1);
			break;
	   case CAP_NONE :
	  	    break;	
	
	   default:	   
	 		break;
	 }

}

void PULL_DOWN_RES_Setting(uint8_t res)
{
    
	GPIO_Setting(SW_VBUS_LKG,0);  //SW_VBUS_LKG和SW_VBUS_PD互斥
   	GPIO_Setting(SW_VBUS_PD,0);


	    switch (res)
   	{
	   case RES_10K:
	  	 	GPIO_Setting(SW_VBUS_PD,1);	
	   		break;
	   case RES_2K:
	  	 	GPIO_Setting(SW_VBUS_LKG,1);	
	   		break;
	   case RES_NONE:
	 	
			break;	   
	
	   default:	   
	 		break;
	 }

}


void DAC_DM_Output_Res(uint8_t res)
{
       GPIO_Setting(SW_DCD_RES_1,0);	//DAC_DM + LOW to DM
	   GPIO_Setting(SW_DCD_RES_2,0);	//DAC_DM + 200R to DM
	   GPIO_Setting(SW_DCD_RES_3,0);	//DAC_DM + 3K9 to DM
	   GPIO_Setting(SW_DCD_RES_4,0);	//DAC_DM + 15K to DM
	   GPIO_Setting(SW_DCD_RES_5,0);	//DAC_DM + 100K to DM
	   GPIO_Setting(SW_DCD_RES_6,0);	//DAC_DM + ??? to DM
   switch (res)
   	{
	   case RES_1K5:
	  	 	GPIO_Setting(SW_DCD_RES_1,1);	
	   		break;
	   case RES_200R:
	  	 	GPIO_Setting(SW_DCD_RES_2,1);	
	   		break;
	   case RES_3K9:
	  	 	GPIO_Setting(SW_DCD_RES_3,1);	
	   		break;
	   case RES_15K:
	 		GPIO_Setting(SW_DCD_RES_4,1);	
			break;	   
       case RES_100K:
	 		GPIO_Setting(SW_DCD_RES_5,1);	
			break;	   
	   case RES_unused:
	 		GPIO_Setting(SW_DCD_RES_6,1);
	   case RES_NONE :	
			break;
	   default:
	   
	 		break;

	 }
}

// 2020-07-23: fixed to RES_7==> 200R , RES_8==> 1K5
void DAC_DP_Output_Res(uint8_t res)
{
     GPIO_Setting(SW_DCD_RES_8,0);	//DAC_DP + 1K5 to DP
	   GPIO_Setting(SW_DCD_RES_7,0);	//DAC_DP + 200R to DP
	   GPIO_Setting(SW_DCD_RES_9,0);	//DAC_DP + 15K to DP
	   GPIO_Setting(SW_DCD_RES_10,0);	//DAC_DP + 100K to DP
	   GPIO_Setting(RSW_DCD_CAP,0);		//DAC_DP + 1nF CAP to DM

   switch (res)
   	{
	   case RES_200R:
	  	 	GPIO_Setting(SW_DCD_RES_7,1);	
	   		break;
	   case RES_1K5:
	  	 	GPIO_Setting(SW_DCD_RES_8,1);	
	   		break;
	   case RES_15K:
	 		GPIO_Setting(SW_DCD_RES_9,1);	
			break;	   
       case RES_100K:
	 		GPIO_Setting(SW_DCD_RES_10,1);	
			break;	   
	   case CAP_1nF:
	 		GPIO_Setting(RSW_DCD_CAP,1);
	   case RES_NONE :	
			break;
	   default:	   
	 		break;
	 }
}




void BuzzerOn(uint16_t duration)
{
   	GPIO_Setting(SW_Buzzer,1);
	vTaskDelay(duration);
	GPIO_Setting(SW_Buzzer,0);
}

void powerDM(uint16_t milVol, uint8_t resId  )
{
	DAC_Setting(DAC_DM , milVol);
	DAC_DM_Output_Res(resId);

}

void powerDP(uint16_t milVol, uint8_t resId  )
{
	 DAC_Setting(DAC_DP , milVol);
	 DAC_DP_Output_Res(resId);

 }

void prepareTest(void)
{
	WatchBlockInit();  //初始化watch block
	GPIO_Setting(SW_VBUS_AB_M1,ON);
	SetAdcDataUploadState(ENABLE); //start report ADC_VAL
}

void prepareVBusForPD(void)
{
  // Delay(20);
   //VBUS_CAP_Setting(CAP_10uF);
   PULL_DOWN_RES_Setting(RES_10K);
   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
   //Delay(24);
   DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
   //Delay(20);
   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
  // Delay(10);//delay 5ms 等待VBUG_GEN 输出稳定
}



void turnOffVBus(void)
{
  	  VBUS_CAP_Setting(CAP_NONE); //将BYPASS电容断掉	  	  
	  DAC_Setting(DAC_VBUS_V,0);  //VBUS_V = 0
	  DAC_Setting(DAC_VBUS_IL,0);
	  GPIO_Setting(RSW_VBUS_AB,OFF);
	  PULL_DOWN_RES_Setting(RES_NONE) ; 
	  
}

void Set_EL_Mode(uint8_t mode)
{
	/*if (mode == CC)
	{
	GPIO_Setting(RSW_EL_MODE, OFF);
	EL_MODE = CC;
	}
	else
	{
	GPIO_Setting(RSW_EL_MODE, ON);
	EL_MODE = CV;
	}*/
}


/*****************USB TEST DEVICE AND HOST *************************************************/



void Init_Test_Host_HS(void)
{
/*初始化HS USB host*/	
USBH_at_FS = 0;		
USBH_Init(&USB_OTG_HS_Core, 
        USB_OTG_HS_CORE_ID,
        &USB_Host,
        &USBH_Enum_cb, 
        &USR_USBH_MSC_cb);
}
void Init_Test_Host_FS(void)
{

/*初始化FS USB host*/
USBH_at_FS = 1;//U			
USBH_Init(&USB_OTG_HS_Core, 
        USB_OTG_HS_CORE_ID,
        &USB_Host,
        &USBH_Enum_cb, 
        &USR_USBH_MSC_cb);
}

void DeInit_Test_Host(void)
{
USBH_Handle_Enable();	
USBH_DeInit(&USB_OTG_HS_Core, &USB_Host);


}

void Init_Test_Device_HS(void)
{
/*初始化HS USB dev*/
USBD_at_FS = 0;
USBD_Init(&USB_OTG_HS_Core,
         USB_OTG_HS_CORE_ID,
        &USR_TEST_DEVICE_desc,             
        &USBD_TEST_DEVICE_cb, 
        &USR_TEST_DEVICE_cb);
}

void Init_Test_Device_FS(void)
{
/*初始化HS USB dev*/
USBD_at_FS = 1;
USBD_Init(&USB_OTG_HS_Core,
         USB_OTG_HS_CORE_ID,
        &USR_TEST_DEVICE_desc,             
        &USBD_TEST_DEVICE_cb, 
        &USR_TEST_DEVICE_cb);
}

void USBH_PROCESS(void)
{
 	 if( USBH_Handle_Flag == 1)
    {
      USBH_Process(&USB_OTG_HS_Core, &USB_Host); 
    }
	// HOST_CMD_handle();	
	
}

uint8_t Get_PD_bMaxPower (void)
{
  return  USB_Host.device_prop.Cfg_Desc.bMaxPower ;
}

void ReportDutInfo(void)
{

	sprintf(charBuf,"VID: 0x%04X | PID:0x%04X ",USB_Host.device_prop.Dev_Desc.idVendor,\
							USB_Host.device_prop.Dev_Desc.idProduct) ;	
    Report_MSG(charBuf)  ;

    sprintf(charBuf,"Manufacturer : %s", (char *)HostDev_Manufacturer) ;	
    Report_MSG(charBuf)	 ;
    sprintf(charBuf,"Product : %s", (char *)HostDev_ProductString) ;	
    Report_MSG(charBuf)	 ;
    sprintf(charBuf,"Serial Number : %s", (char *)HostDev_SerialNumString) ;	
    Report_MSG(charBuf)	 ;





	   //USB_Host.device_prop.Dev_Desc.idVendor
	//   USB_Host.device_prop.Dev_Desc.idProduct



}

uint8_t USBH_Verify_Communication(void)
{ 		   
	   return  USBH_Get_DevDesc(&USB_OTG_HS_Core, &USB_Host, 8)  ;
}

// used for after deleted test task, reset measureing hardware.
void MEAS_HW_RST(void)
 {  
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);
	DAC_Setting(DAC_VBUS_V , 0);
	DAC_Setting(DAC_LOAD_I , 0);
	DAC_Setting(DAC_VBUS_IL , 0);
	DAC_Setting(DAC_LOAD_V , 0);
	DAC_Setting(DAC_CM_V , 0);
	Delay(1000);
	GPIO_Setting(RSW_VBUS_AB,OFF);
	GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);
	GPIO_Setting(SW_VBUS_AB_M1,ON);
	GPIO_Setting(RSW_DMDP_TEST,OFF);
	GPIO_Setting(RSW_VBUS_ACC,OFF);
	GPIO_Setting(RSW_VBUS_CHG,OFF);
	GPIO_Setting(RSW_VBUS_ACC,OFF);
	GPIO_Setting(SW_ID_RES_M_1,OFF);
	GPIO_Setting(SW_ID_RES_M_2,OFF);
	VBUS_CAP_Setting(CAP_NONE);
	GPIO_Setting(SW_GND_AB_M2,OFF);
	GPIO_Setting(RSW_VCC_PHY,OFF);
	SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
	SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);		
}



/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
