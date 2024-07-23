/**
  ******************************************************************************
  * @file    UCPET_bsp.h
  * @author  MCD Application Team
  * @version V1.0
  * @date    25-FEB-2013
  * @brief   This file contains definitions for STM3210E_EVAL's Leds, push-buttons
  *          COM ports, sDAC (on SPI) and Temperature Sensor LM75 (on I2C)
  *          hardware resources.  
  ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_ll_usb.h"

  
/* Define to prevent recursive inclusion -------------------------------------*/

/*USB 全局变量*/
extern USB_OTG_CORE_HANDLE    g_USB_link_dev  ;
extern USB_OTG_CORE_HANDLE    USB_OTG_HS_Core  ;  // FOR TEST DEVICE AND HOST



/********函数输出**********************/


extern void UcpetBoardInit(void);
extern void USB_LINK_Init(void);
extern void Muti_CH_ADC_Init(void);
extern void GPIO_Configuration(void);
extern void USB_LINK_Init(void);
extern void ResetUsbLink(void);

extern void GPIO_Setting (uint8_t io_num,uint16_t para);
extern void DAC_Setting(uint8_t dac_name,int16_t dac_value);
extern void DAC_Setting_link(uint8_t dac_name,int16_t dac_value);
extern void GPIO_Toggle(uint8_t io_num);
extern void ID_RES_Setting(uint8_t res);
extern void VBUS_CAP_Setting(uint8_t cap);
extern void PULL_DOWN_RES_Setting(uint8_t res);
extern void BuzzerOn(uint16_t duration);
extern void powerDM(uint16_t milVol, uint8_t resId  );
extern void powerDP(uint16_t milVol, uint8_t resId  );
extern void prepareTest(void);
extern void prepareVBusForPD(void);
extern void turnOffVBus(void);
extern void DAC_DM_Output_Res(uint8_t res);
extern void DAC_DP_Output_Res(uint8_t res);

extern void CopyAdcData2PorcessBuf(void);
extern void GetRunTimeStats(void);
extern void T3_CCR1_Handle(void);

extern uint8_t GetLoadCurrentRange(void);
extern void SetLoadCurrentRange(uint8_t range)	;
extern uint8_t GetVbusCurrentRange(void)		;
extern void SetVbusCurrentRange(uint8_t range)	 ;
extern uint8_t GetCurrentRangeMode(uint8_t name);
extern void SetCurrentRangeMode(uint8_t name,uint8_t mode);


extern void USB_OTG_BSP_Init (USB_OTG_CORE_HANDLE *pdev);
extern void USB_OTG_BSP_uDelay (const uint32_t usec);
extern void USB_OTG_BSP_mDelay (const uint32_t msec);
extern void USB_OTG_BSP_EnableInterrupt (USB_OTG_CORE_HANDLE *pdev);
#ifdef USE_HOST_MODE
extern void USB_OTG_BSP_ConfigVBUS(USB_OTG_CORE_HANDLE *pdev);
extern void USB_OTG_BSP_DriveVBUS(USB_OTG_CORE_HANDLE *pdev,uint8_t state);
#endif

extern void Init_Test_Host_HS(void);
extern void Init_Test_Host_FS(void);
extern void Init_Test_Device_FS(void);
extern void Init_Test_Device_HS(void);
extern void DeInit_Test_Host(void);
extern void USBH_PROCESS(void);
extern uint8_t Get_PD_bMaxPower (void);
extern uint8_t USBH_Verify_Communication(void);
//extern void Set_EL_Mode(uint8_t mode);
extern void MEAS_HW_RST(void);
extern void USB_LINK_Suspend(void);



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
