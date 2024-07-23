/**
  ******************************************************************************
  * @file    usbh_msc_usr.c
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    22-July-2011
  * @brief   This file includes the usb host library user callbacks
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
#include <string.h>
#include "usbh_msc_usr.h"
#include "usbh_stdreq.h"

#include "LinkUsbProcess.h"
#include "ucpet_bsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "PET_CMD.h"

//#include "dual_func_demo.h"
//#include "lcd_log.h"
/** @addtogroup USBH_USER
* @{
*/
//extern char charBuf[512];
uint8_t HostDev_Manufacturer[64];
uint8_t HostDev_ProductString[64];
uint8_t HostDev_SerialNumString[64];

uint8_t USBH_ST,Enum_Done;


extern USBH_HOST USB_Host;
extern xQueueHandle xQueueUsb3300Event;   //test dev or test host 的事件队列


/** @addtogroup USBH_MSC_DEMO_USER_CALLBACKS
* @{
*/

/** @defgroup USBH_USR 
* @brief    This file includes the usb host stack user callbacks
* @{
*/ 

/** @defgroup USBH_USR_Private_TypesDefinitions
* @{
*/ 
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Defines
* @{
*/ 
#define IMAGE_BUFFER_SIZE    512
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Macros
* @{
*/ 
extern USB_OTG_CORE_HANDLE          USB_OTG_HS_Core;
uint8_t USBH_Handle_Flag;                
/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Variables
* @{
*/ 
 

/*  Points to the DEVICE_PROP structure of current device */
/*  The purpose of this register is to speed up the execution */

USBH_Usr_cb_TypeDef USR_USBH_MSC_cb =
{
  USBH_USR_Init,
  USBH_USR_DeInit,
  USBH_USR_DeviceAttached,
  USBH_USR_ResetDevice,
  USBH_USR_DeviceDisconnected,
  USBH_USR_OverCurrentDetected,
  USBH_USR_DeviceSpeedDetected,
  USBH_USR_Device_DescAvailable,
  USBH_USR_DeviceAddressAssigned,
  USBH_USR_Configuration_DescAvailable,
  USBH_USR_Manufacturer_String,
  USBH_USR_Product_String,
  USBH_USR_SerialNum_String,
  USBH_USR_EnumerationDone,
  USBH_USR_UserInput,
  USBH_USR_MSC_Application,
  USBH_USR_DeviceNotSupported,
  USBH_USR_UnrecoveredError
    
};

/**
* @}
*/

/** @defgroup USBH_USR_Private_Constants
* @{
*/ 
/*--------------- LCD Messages ---------------*/
const uint8_t MSG_DEV_ATTACHED[]     = "> Device Attached \n";
const uint8_t MSG_DEV_DISCONNECTED[] = "> Device Disconnected\n";
const uint8_t MSG_DEV_ENUMERATED[]   = "> Enumeration completed";
const uint8_t MSG_DEV_HIGHSPEED[]    = "> High speed device detected";
const uint8_t MSG_DEV_FULLSPEED[]    = "> Full speed device detected";
const uint8_t MSG_DEV_LOWSPEED[]     = "> Low speed device detected";
const uint8_t MSG_DEV_ERROR[]        = "> Device fault \n";

const uint8_t MSG_MSC_CLASS[]      = "> Mass storage device connected\n";

const uint8_t MSG_DISK_SIZE[]      = "> Size of the disk in MBytes: \n";
const uint8_t MSG_LUN[]            = "> LUN Available in the device:\n";
const uint8_t MSG_ROOT_CONT[]      = "> Exploring disk flash ...\n";
const uint8_t MSG_WR_PROTECT[]      = "> The disk is write protected\n";
const uint8_t MSG_MSC_UNREC_ERROR[]     = "ERRO: UNRECOVERED ERROR STATE\n";

/**
* @}
*/



/** @defgroup USBH_USR_Private_FunctionPrototypes
* @{
*/

/**
* @}
*/ 


/** @defgroup USBH_USR_Private_Functions
* @{
*/ 


/**
* @brief  USBH_USR_Init 
*         Displays the message on LCD for host lib initialization
* @param  None
* @retval None
*/
void USBH_USR_Init(void)
{
 //Report_MSG("> Init USB host...");

 USBEventQueue_FMT EventThis;

 EventThis.event = HostInit;
 USBH_Handle_Enable();

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}

}

/**
* @brief  USBH_USR_DeviceAttached 
*         Displays the message on LCD on device attached
* @param  None
* @retval None
*/
void USBH_USR_DeviceAttached(void)
{
// Report_MSG("> DUT Attached");
   USBEventQueue_FMT EventThis;
   
   	Enum_Done = 0;
    EventThis.event = HostDevAttach;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}


/**
* @brief  USBH_USR_UnrecoveredError
* @param  None
* @retval None
*/
void USBH_USR_UnrecoveredError (void)
{
 
  /* Set default screen color*/ 
 // Report_MSG("> USBH_USR_UnrecoveredError");
 // USBH_Handle_Flag = 0; 

}


/**
* @brief  USBH_DisconnectEvent
*         Device disconnect event
* @param  None
* @retval Staus
*/
void USBH_USR_DeviceDisconnected (void)
{

 // Report_MSG("> DUT Disconnected");

     USBEventQueue_FMT EventThis;
   
   	Enum_Done = 0;
    EventThis.event = HostDevDiscon;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}

    
}
/**
* @brief  USBH_USR_ResetUSBDevice 
* @param  None
* @retval None
*/
void USBH_USR_ResetDevice(void)
{
  /* callback for USB-Reset */
 // Report_MSG("> ResetDevice");
    USBEventQueue_FMT EventThis;

    EventThis.event = HostDevRst;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}


/**
* @brief  USBH_USR_DeviceSpeedDetected 
*         Displays the message on LCD for device speed
* @param  Device speed
* @retval None
*/
void USBH_USR_DeviceSpeedDetected(uint8_t DeviceSpeed)
{
    USBEventQueue_FMT EventThis;

    EventThis.event = HostDevSpd;
	EventThis.para = DeviceSpeed;
  
 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}

/**
* @brief  USBH_USR_Device_DescAvailable 
*         Displays the message on LCD for device descriptor
* @param  device descriptor
* @retval None
*/
void USBH_USR_Device_DescAvailable(void *DeviceDesc)
{ 
    USBEventQueue_FMT EventThis; 
 // USBH_DevDesc_TypeDef *hs;
 // hs = DeviceDesc;  
   //Report_MSG("> USBH_USR_Device_DescAvailable");
 // sprintf(charBuf,"> VID : 0x%04X", (uint16_t)(*hs).idVendor) ;	
 // Report_MSG(charBuf)	 ;
 // sprintf(charBuf,"> PID : 0x%04X", (uint16_t)(*hs).idProduct) ;	
 // Report_MSG(charBuf)	 ;
 

    EventThis.event = HostDevDesc;
  
 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}



/**
* @brief  USBH_USR_DeviceAddressAssigned 
*         USB device is successfully assigned the Address 
* @param  None
* @retval None
*/
void USBH_USR_DeviceAddressAssigned(void)
{
  	//Report_MSG("> USBH_USR_DeviceAddressAssigned");
	    USBEventQueue_FMT EventThis;

    EventThis.event = HostDevAddr;
  
 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}


/**
* @brief  USBH_USR_Conf_Desc 
*         Displays the message on LCD for configuration descriptor
* @param  Configuration descriptor
* @retval None
*/
void USBH_USR_Configuration_DescAvailable(USBH_CfgDesc_TypeDef * cfgDesc,
                                          USBH_InterfaceDesc_TypeDef *itfDesc,
                                          USBH_EpDesc_TypeDef *epDesc)
{

  	//USBH_CfgDesc_TypeDef *cfg;
    //cfg = cfgDesc;
 	     USBEventQueue_FMT EventThis;

    EventThis.event = HostDevCfgDesc;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
   //	sprintf(charBuf,"> bMaxPower = %d mA", ((*cfg).bMaxPower * 2)) ;	
   // Report_MSG(charBuf)	 ;


}

/**
* @brief  USBH_USR_Manufacturer_String 
*         Displays the message on LCD for Manufacturer String 
* @param  Manufacturer String 
* @retval None
*/
void USBH_USR_Manufacturer_String(void *ManufacturerString)
{
   //uint8_t Local_Buffer[64];
  memcpy(HostDev_Manufacturer,ManufacturerString,64) ;
 // sprintf(charBuf,"> Manufacturer : %s", (char *)HostDev_Manufacturer) ;	
 // Report_MSG(charBuf)	 ;
  
 // Report_MSG("> Manufacturer : %s\n", (char *)ManufacturerString);
}

/**
* @brief  USBH_USR_Product_String 
*         Displays the message on LCD for Product String
* @param  Product String
* @retval None
*/
void USBH_USR_Product_String(void *ProductString)
{

  memcpy(HostDev_ProductString,ProductString,64) ;


 // sprintf(charBuf,"> Product : %s", (char *)HostDev_ProductString) ;	
 // Report_MSG(charBuf)	 ;
 // sprintf(charBuf,"> Product : %s", (char *)ProductString) ;	
 // Report_MSG(charBuf)	 ;
}

/**
* @brief  USBH_USR_SerialNum_String 
*         Displays the message on LCD for SerialNum_String 
* @param  SerialNum_String 
* @retval None
*/
void USBH_USR_SerialNum_String(void *SerialNumString)
{

  memcpy(HostDev_SerialNumString,SerialNumString,64) ;


 // sprintf(charBuf,"> Serial Number : %s", (char *)SerialNumString) ;	
 // Report_MSG(charBuf)	 ;
  //LCD_UsrLog( "> Serial Number : %s\n", (char *)SerialNumString); 
  // sprintf(charBuf,"> Serial Number : %s", (char *)SerialNumString) ;	
  //Report_MSG(charBuf)	 ;
     
} 



/**
* @brief  EnumerationDone 
*         User response request is displayed to ask application jump to class
* @param  None
* @retval None
*/
void USBH_USR_EnumerationDone(void)
{
  
  /* Enumeration complete */
     USBEventQueue_FMT EventThis;

    EventThis.event = HostDevEnumDone;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}

   Enum_Done = 1;


} 


/**
* @brief  USBH_USR_DeviceNotSupported
*         Device is not supported
* @param  None
* @retval None
*/
void USBH_USR_DeviceNotSupported(void)
{
  //Report_MSG ("> Device not supported"); 
}  


/**
* @brief  USBH_USR_UserInput
*         User Action for application state entry
* @param  None
* @retval USBH_USR_Status : User response for key button
*/
USBH_USR_Status USBH_USR_UserInput(void)
{
  return USBH_USR_RESP_OK;
}  

/**
* @brief  USBH_USR_OverCurrentDetected
*         Over Current Detected on VBUS
* @param  None
* @retval Staus
*/
void USBH_USR_OverCurrentDetected (void)
{
  //Report_MSG ("> Overcurrent detected.");
       USBEventQueue_FMT EventThis;

    EventThis.event = HostDevOC;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}


/**
* @brief  USBH_USR_MSC_Application 
*         Demo application for mass storage
* @param  None
* @retval Staus
*/

int USBH_USR_MSC_Application(void)
{

  return(0);
}	

/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/

void USBH_USR_DeInit(void)
{
  //USBH_USR_ApplicationState = USH_USR_FS_INIT;

      USBEventQueue_FMT EventThis;

    EventThis.event = HostDeInit;

 if(xQueueSendToBackFromISR(xQueueUsb3300Event,&EventThis,pdFALSE) !=pdPASS)
	{
	//xQueueUsb3300Event队列已满
	 GPIO_Toggle(SW_LED2_R) ;
	}
}


/**
* @brief  USBH_USR_DeInit
*         Deint User state and associated variables
* @param  None
* @retval None
*/



void USBH_Handle_Enable(void)
{
 	USBH_Handle_Flag = 1;
}

void USBH_Handle_Disable(void)
{
 	USBH_Handle_Flag = 0;
}



/**
* @}
*/ 

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






























