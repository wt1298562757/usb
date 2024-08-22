/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//---------------ADD WT--------------------------
#include "usbd_cdc_if.h"
#include "LinkUsb.h"
#include "queue.h"
#include "stdbool.h"
#include "THE_CMD.h"
//----------------------------------------------------

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSG_BUFF_SIZE 2560	   // 上传给HOST的MSG数据缓冲�??

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void Report_MSG(char *p);
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
//--------------ADD WT-----------------------
uint8_t usb_ep1_rx[2048];
uint8_t usb_ep1_tx[40960];

uint8_t usb_ep3_rx[2048];
uint8_t usb_ep3_tx[40960];

uint8_t usb_ep4_rx[2048];
uint8_t usb_ep4_tx[40960];

volatile int8_t usb_ep1_rxne = RESET;
volatile int8_t usb_ep3_rxne = RESET;
volatile int8_t usb_ep4_rxne = RESET;
volatile int8_t usb_ep3_txne = RESET;
volatile int8_t usb_ep4_txne = RESET;

extern USBD_HandleTypeDef hUsbDeviceHS;

uint8_t MsgSendBuffA[MSG_BUFF_SIZE+28];
uint8_t MsgSendBuffB[MSG_BUFF_SIZE+28];
uint8_t* pCurBuffer = MsgSendBuffA;
uint32_t  MsgSendBuffOffset = 0;

static  uint8_t USB_TX_pktSn =0;
//-----------------------------------------------

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LINK_USB_CMD */
osThreadId_t LINK_USB_CMDHandle;
const osThreadAttr_t LINK_USB_CMD_attributes = {
  .name = "LINK_USB_CMD",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal2,
};
/* Definitions for LINK_USB_SEND */
osThreadId_t LINK_USB_SENDHandle;
const osThreadAttr_t LINK_USB_SEND_attributes = {
  .name = "LINK_USB_SEND",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal4,
};
/* Definitions for xQueueLinkUsbRecvCmd */
osMessageQueueId_t xQueueLinkUsbRecvCmdHandle;
const osMessageQueueAttr_t xQueueLinkUsbRecvCmd_attributes = {
  .name = "xQueueLinkUsbRecvCmd"
};
/* Definitions for xMutexMsgBuff */
osMutexId_t xMutexMsgBuffHandle;
const osMutexAttr_t xMutexMsgBuff_attributes = {
  .name = "xMutexMsgBuff"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void WinUSB_Receive_HS()
{
	uint16_t i = 0;
	if (usb_ep1_rxne == SET)
	{
		i = usb_ep1_rx[0] + (usb_ep1_rx[1]<<8);
		CDC_Transmit_HS(usb_ep1_rx, i, CDC_IN_EP);
		usb_ep1_rxne = RESET;
	}
	else if(usb_ep3_txne == SET)
	{
		i = usb_ep3_rx[0] + (usb_ep3_rx[1]<<8);
		CDC_Transmit_HS(usb_ep3_rx, i, FS_LINK_IN_EP);
		usb_ep3_rxne = RESET;
	}
  else if(usb_ep4_rxne == SET)
	{
		i = usb_ep4_rx[0] + (usb_ep4_rx[1]<<8);
		CDC_Transmit_HS(usb_ep4_rx, i, EP_ADC_IN);
		usb_ep4_rxne = RESET;
	}
  else if(usb_ep3_rxne == SET)
	{
		i = usb_ep3_rx[0] + (usb_ep3_rx[1]<<8);
		CDC_Transmit_HS(usb_ep3_rx+2, i, FS_LINK_IN_EP);
		usb_ep3_rxne = RESET;
	}
	
}
//------------------------------------------

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void vTaskLinkUsbCmdProcess(void *argument);
void vTaskSendMsgToHost(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of xMutexMsgBuff */
  xMutexMsgBuffHandle = osMutexNew(&xMutexMsgBuff_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* creation of xQueueLinkUsbRecvCmd */
  xQueueLinkUsbRecvCmdHandle = osMessageQueueNew (8, sizeof(FS_LINK_CMD_FMT), &xQueueLinkUsbRecvCmd_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LINK_USB_CMD */
  LINK_USB_CMDHandle = osThreadNew(vTaskLinkUsbCmdProcess, NULL, &LINK_USB_CMD_attributes);

  /* creation of LINK_USB_SEND */
  LINK_USB_SENDHandle = osThreadNew(vTaskSendMsgToHost, NULL, &LINK_USB_SEND_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		// WinUSB_Receive_HS();
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_vTaskLinkUsbCmdProcess */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskLinkUsbCmdProcess */
void vTaskLinkUsbCmdProcess(void *argument)
{
  /* USER CODE BEGIN vTaskLinkUsbCmdProcess */
  FS_LINK_CMD_FMT RecvCmd;
  /* Infinite loop */
  for(;;)
  {
    if(osMessageQueueGet(xQueueLinkUsbRecvCmdHandle,&RecvCmd,NULL,osWaitForever)  == osOK)
	 {
	   //处理收到的有效消息
		Report_MSG(">>>>>>>>>>>>>>>>>>>  for debug  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	   	// sprintf(charBuf,"INFO: cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;

		//	GetRunTimeStats();  //run time stat only 须使能TIM3
	   switch(RecvCmd.cmd)
	   {

     }
    }
    osDelay(1);
  }
  /* USER CODE END vTaskLinkUsbCmdProcess */
}

/* USER CODE BEGIN Header_vTaskSendMsgToHost */
/**
* @brief Function implementing the LINK_USB_SEND thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_vTaskSendMsgToHost */
void vTaskSendMsgToHost(void *argument)
{
  /* USER CODE BEGIN vTaskSendMsgToHost */
  /* Infinite loop */
  portTickType xLastWakeTime;
	const portTickType xPeriod = (180 * portTICK_RATE_MS);
	uint32_t sendLen;// = MsgSendBuffOffset ;
    uint8_t* pTemp;//= pCurBuffer;
    // UP_FMT* pCmd = (UP_FMT*)(pCurBuffer );
	
	xLastWakeTime = xTaskGetTickCount();
    
    
 for(;;)
 {
   		//sprintf(charBuf,"SendMsgTask Running ,T = %d",xTaskGetTickCount()) ;
	   // Report_MSG(charBuf)	 ;
   if( MsgSendBuffOffset > 0 )	//如果当前指针不为零，就发送
   {
		// if(DCD_GetEPStatus(&g_USB_link_dev,FS_LINK_IN_EP) == USB_OTG_EP_TX_VALID )

		   //EP valid
		    if(osMutexAcquire(xMutexMsgBuffHandle,MSG_BUFF_WRITE_TIMEOUT)== osOK  )
		    {
			   sendLen =  MsgSendBuffOffset;
			   pTemp = 	 pCurBuffer;
			   
				//switch buffer	,give Semaphore
			   pCurBuffer = pCurBuffer == MsgSendBuffA ? MsgSendBuffB : MsgSendBuffA;
			   MsgSendBuffOffset = 0;
			   osMutexRelease(xMutexMsgBuffHandle);
			   
			   //send
			//    DCD_EP_Tx(&g_USB_link_dev,FS_LINK_IN_EP, (uint8_t*)(pTemp), sendLen);		  
			   	while(CDC_Transmit_HS((uint8_t*)(pTemp), sendLen, FS_LINK_IN_EP) != USBD_OK); 	
			   
			  // Report_MSG("MsgSendBuff Toggled.");
			  // 	GPIO_Toggle(SW_LED1_R) 	;

		    }		   
			else
			{
			 //EP not valid
			  Report_MSG("ERRO: EP1_TX_NOT_VALID !");
			}	
	    
	}

     vTaskDelayUntil(&xLastWakeTime,xPeriod);
  }
  /* USER CODE END vTaskSendMsgToHost */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
bool AddHostCmdtoQueue(uint8_t* pRecvBuff, uint32_t count)
{
	bool success = false;

	DOWN_FMT *rxBuffPtr = (DOWN_FMT*)pRecvBuff;
	FS_LINK_CMD_FMT LinkCmd;

	do{
		if(count != PC_DOWN_CMD_SIZE)
		{
			break;
		}

		if(rxBuffPtr->startFlag != DOWN_START_FLAG ) 
		{
			// break;
		}

		//到此，数据有效，将向xQueueLinkUsbRecvCmd发�?�一条消�??
		LinkCmd.cmd =  rxBuffPtr->cmd;
		LinkCmd.target = rxBuffPtr->target;
		LinkCmd.para = 	rxBuffPtr->para;

		if(osMessageQueuePut(xQueueLinkUsbRecvCmdHandle,&LinkCmd,0,0) != osOK)
		{
		   //xQueueLinkUsbRecvCmd队列已满
		  // GPIO_Toggle(SW_LED2_R) ;	 ;
		}

		success = true;

     }while(false);
   return success;
}

static void AddMsgToBuff(uint8_t infoCategory, uint8_t src, uint8_t msgLen,  uint8_t* content)
 {

	UP_FMT onePkt;


	onePkt.startFlag = UP_START_FLAG;
	onePkt.pktSn = USB_TX_pktSn++	;
	onePkt.ts = xTaskGetTickCount();//GlobleTime;	//fill current time
	onePkt.src = src;
	onePkt.msgType = infoCategory;
	onePkt.msgLenth = 	msgLen;

  if(osMutexAcquire(xMutexMsgBuffHandle,MSG_BUFF_WRITE_TIMEOUT)== osOK  )
	{
	//copy the message header to CurBuff
  	memcpy(pCurBuffer + MsgSendBuffOffset, &onePkt, 12);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + 12 ;	
	//copy the current
	memcpy(pCurBuffer + MsgSendBuffOffset, content, msgLen);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + msgLen ;

	osMutexRelease(xMutexMsgBuffHandle);
	}
   else
   {
	 //写buff失败，其他程序没有Give xMutexMsgBuff
	//  ;
	//  GPIO_Toggle(SW_LED1_R) ;
   }


}

void Report_MSG(char *p)
{
	AddMsgToBuff(0x01,0,strlen(p)+ 1,(uint8_t*)p);
} 
/* USER CODE END Application */

