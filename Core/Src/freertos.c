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
#include "tim.h"

// #include "SEGGER_RTT.h"
#include "lua.h"     // Lua数据类型与函数接口
#include "lauxlib.h" // Lua与C交互辅助函数接口
#include "lualib.h"  // Lua标准库打开接口

// #define LUA_LIB
// #include "luaconf.h" // Lua配置文件
#include "lapi.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MSG_BUFF_SIZE 2560	   // 上传给HOST的MSG数据缓冲�??
#define MAX_MSG_LENTH 512
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
void *rtos_alloc_for_lua(void *ud, void *ptr, size_t osize, size_t nsize);
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
char charBuf[MAX_MSG_LENTH];
//-----------------------------------------------

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
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
static int lua_led_on(lua_State *L)
{
    // HAL_GPIO_TogglePin(LED_SYS_GPIO_Port, LED_SYS_Pin);
    HAL_GPIO_WritePin(LED_SYS_GPIO_Port, LED_SYS_Pin, GPIO_PIN_RESET);
    return 1;
}
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void vTaskLinkUsbCmdProcess(void *argument);
void vTaskSendMsgToHost(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* Hook prototypes */
void configureTimerForRunTimeStats(void);
unsigned long getRunTimeCounterValue(void);
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName);

/* USER CODE BEGIN 1 */
/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
__weak void configureTimerForRunTimeStats(void)
{
  HAL_TIM_Base_Start_IT(&htim10);
}

__weak unsigned long getRunTimeCounterValue(void)
{
  return (unsigned long)htim10.Instance->CNT;
}
/* USER CODE END 1 */

/* USER CODE BEGIN 4 */
void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
  (void)xTask;  (void)pcTaskName;
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */
}
/* USER CODE END 4 */

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
 
static int lua_tester()
{
    printf("Lua tester runned!\r\n");
    return 1;
}

/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  // MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */



  // 创建 Lua 线程
  lua_State *L = lua_newstate(rtos_alloc_for_lua, NULL);
  if(L != NULL) {
    printf("Lua State Created.\r\n");
  } else {
    printf("Lua State Create Failed!\r\n");
  }
  lua_gc(L, LUA_GCSTOP); // 停止垃圾回收
  luaL_openlibs(L); // 打开标准库

  // 开始 Lua 任务
  luaL_dostring(L, "print('Naisu, Lua!')");

  // 自定义 C 函数库
  static const struct luaL_Reg func_lib[] = {
      {"lua_tester", lua_tester},
      {NULL, NULL}
  };
  luaL_newlib(L, func_lib); // 创建一个新的库
  lua_setglobal(L, "testlib"); // 将库设置为全局变量

  luaL_dostring(L, "print('Naisu, Lua!')");
  luaL_dostring(L, "testlib.lua_tester()"); // 通过库调用函数



  HAL_GPIO_WritePin(LED_SYS_GPIO_Port, LED_SYS_Pin, GPIO_PIN_SET);
  /* Infinite loop */
  for(;;)
  {
    HAL_GPIO_TogglePin(LED_SYS_GPIO_Port, LED_SYS_Pin);
    osDelay(250); // LED 闪烁表明运行到了此处
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
	  sprintf(charBuf,"INFO: cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;
    Report_MSG(charBuf);

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
void *rtos_alloc_for_lua(void *ud, void *ptr, size_t osize, size_t nsize) {
    (void)ud; (void)osize;  // not used
    if (nsize == 0) {
        vPortFree(ptr);
        return NULL;
    } else if (ptr == NULL) {
        return pvPortMalloc(nsize);
    } else {
        void *newPtr = pvPortMalloc(nsize);
        if (newPtr != NULL) {
            size_t minSize = (osize < nsize) ? osize : nsize;
            memcpy(newPtr, ptr, minSize);
            vPortFree(ptr);
        }
        return newPtr;
    }
}

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
			break;
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
	AddMsgToBuff(0x00,0,strlen(p)+ 1,(uint8_t*)p);
} 
/* USER CODE END Application */

