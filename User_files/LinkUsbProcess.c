//处理FS LINK 事件，包括接收CMD处理和数据发送处理


/* 包含头文件 *****************************************************************/
#include <string.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "LinkUsbProcess.h"
#include "PET_CMD.h"
 #include "TestScriptProcess.h"
 #include "main.h"

// #include "UCPET_bsp.h"

// #include "usb_dcd.h"
#include "usbd_conf.h"
#include "eeprom.h"
#include <stdbool.h>
#include "usbd_cdc.h"
#include "usbd_cdc_if.h"



#define MSG_BUFF_SIZE 2560	   // 上传给HOST的MSG数据缓冲区
#define MSG_BUFF_WRITE_TIMEOUT 5000
#define MSG_SEND_PERIOD 100 * portTICK_RATE_MS		  //EP1_IN 发送频率=100ms
#define ADC_BUFF_SIZE 	 28000 //上传给HOST的ADC数据缓冲区(8ch x 2byte + 12byte head) x 1000 =28000 byte

#define SCRIPT_RUNNER_PRIO   (tskIDLE_PRIORITY  + 1 )






//fs link receive queue format









/* 变量 ----------------------------------------------------------------------*/



xQueueHandle xQueueLinkUsbRecvCmd; //FS LINK CMD队列
xQueueHandle xQueueUsb3300Event;   //test dev or test host 的事件队列
xSemaphoreHandle xMutexMsgBuff; //FS LINK 发送缓冲mutex
xTaskHandle vHandleTaskRunOneScript;

char charBuf[MAX_MSG_LENTH];
//xTaskHandle vTaskRunOneScriptHandle;

extern xQueueHandle xQueueRunTestScript;



uint8_t MsgSendBuffA[MSG_BUFF_SIZE+28];
uint8_t MsgSendBuffB[MSG_BUFF_SIZE+28];
uint8_t* pCurBuffer = MsgSendBuffA;
uint32_t  MsgSendBuffOffset = 0;

uint8_t AdcSendBuffA[ADC_BUFF_SIZE];
uint8_t AdcSendBuffB[ADC_BUFF_SIZE];
uint8_t* pCurAdcSendBuffer = AdcSendBuffA;
uint32_t  AdcSendBuffOffset = 0;

uint8_t g_userHasGesture = false;
uint16_t g_userInputInfo;

static  uint8_t USB_TX_pktSn =0;
static  uint8_t USB_ADC_pktSn = 0;
static uint8_t ADC_DATA_UPLOAD = false;
static RUN_SCRIPT_CMD_FMT RunScriptCmd;

PreDefinedPara_t PreDefinedPara;
extern uint16_t VirtAddVarTab[NB_OF_VAR];

//引用ADC 全局变量
//extern __IO uint16_t ADC_ConvertedBuff0[ADC_Buf_Size];
//extern __IO uint16_t ADC_ConvertedBuff1[ADC_Buf_Size];
//extern uint16_t* pCurADC_ConvertedBuff;
extern __IO uint16_t ADC_ProcessBuff[8]; 


/* 函数声明 ------------------------------------------------------------------*/
void ReportPreDefinedPara(void);
extern void vTaskRunOneScript(void *pvParameters)  ;
extern void ReportDutInfo(void);
/**
  AddHostCmdtoQueue 
  处理fs link usb缓冲区中的数据，组合成消息发送到xQueueLinkUsbRecvCmd队列* @}
  */ 


uint8_t AddHostCmdtoQueue(uint8_t* pRecvBuff, uint16_t count)
{
	uint8_t success = false;

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

		//到此，数据有效，将向xQueueLinkUsbRecvCmd发送一条消息
		LinkCmd.cmd =  rxBuffPtr->cmd;
		LinkCmd.target = rxBuffPtr->target;
		LinkCmd.para = 	rxBuffPtr->para;

		if(xQueueSend(xQueueLinkUsbRecvCmd,&LinkCmd,10) !=pdPASS)
		{
		   //xQueueLinkUsbRecvCmd队列已满
		  GPIO_Toggle(SW_LED2_R) ;	 ;
		}

		success = true;

     }while(false);
   return success;
}

//ADC_DATA_UPLOAD变量处理

void SetAdcDataUploadState(uint8_t i)
{
  ADC_DATA_UPLOAD = i;
}

uint8_t GetAdcDataUploadState(void)
{
 return ADC_DATA_UPLOAD;
}

//load PreDefinedPara
void LoadPreDefinedPara(void)
{
	EE_ReadVariable(VirtAddVarTab[CABLE_A_RES_EEADDR],&PreDefinedPara.cable_res_a );
	EE_ReadVariable(VirtAddVarTab[CABLE_B_RES_EEADDR],&PreDefinedPara.cable_res_b );
}




/*******************************************************************************
  * @函数名称	vTaskLinkUsbCmdProcess
  * @函数说明   处理HOST发来的CMD，由xQueueLinkUsbRecvCmd队列触发
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void vTaskLinkUsbCmdProcess(void *pvParameters)
{
FS_LINK_CMD_FMT RecvCmd;

	//for Stack check
//	unsigned portBASE_TYPE uxHighWaterMark;
   
//    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

   for(;;)
   {
	 if(xQueueReceive(xQueueLinkUsbRecvCmd,&RecvCmd,portMAX_DELAY)  == pdPASS)
	 {
	   //处理收到的有效消息
		// Report_MSG(">>>>>>>>>>>>>>>>>>>  for debug  <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
	   	// sprintf(charBuf,"INFO: cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;

		//	GetRunTimeStats();  //run time stat only 须使能TIM3
	   switch(RecvCmd.cmd)
	   {
		 case  SYSTEM_RESET:
			   NVIC_SystemReset();
		 	   break;
		 
		 case 0x66:
		 	    //NVIC_SystemReset();

				//sprintf(charBuf,"REDING = %d ",GPIO_Reading(RSW_AGND_TO_GND_AB)) ;
	           //Report_MSG(charBuf)	 ;
			   
			  // NVIC_SystemReset();
				//	USB_LINK_Init();
				//ResetUsbLink();
		 	   break;	

		 case  SET_SWITCHX_POSITION:
		 case  SET_RELAYX_POSITION:
			 //for debug
		   if (RecvCmd.target == 0x5F)
				 RecvCmd.target = 0x48;
				 
		   //end 
		 	   GPIO_Setting (RecvCmd.target,RecvCmd.para);
			   break;
			   	   				   	   	
 		 case  SET_DACX_VALUE :
		 	   DAC_Setting_link(RecvCmd.target,RecvCmd.para);
			   break; 

		 case  0x40:  //upload or not ADC data
		        if(RecvCmd.para == 0xffff)
				{
				 SetAdcDataUploadState(true);
				 }
				else
				{
				 SetAdcDataUploadState(false);
				}
		       	break;

 		 case  GET_HARDWARE_VERSION :
			 
				 sprintf(charBuf,"UID(96 bits): 0x%X %X %X", *(uint32_t*) 0x1FFF7A18, \
	 				               *(uint32_t*) 0x1FFF7A14,  *(uint32_t*) 0x1FFF7A10);
				 Report_MSG(charBuf )  ;
				 Report_MSG("FW Version: V4R_2020-07-28");
				 Report_MSG("Boot Loader Version: DFU_V4");
				 Report_MSG("HD Version: UCPET_V4");


				 /*debug*/

				 // GPIO_Setting(RSW_VBUS_ACC_LD2,ON);
				 
				 /*
				 Report_MSG("=========== For Debug===========");
				 sprintf(charBuf,"Free Heap Size: %d %",  xPortGetFreeHeapSize());
				 Report_MSG(charBuf )  ;
                  */

				 break;

		  case RUN_TESTX:
		  
		         if( RunScriptCmd.id == 0)
				 { 
				  
                   RunScriptCmd.id = RecvCmd.target;
				   RunScriptCmd.para = RecvCmd.para;

				  xTaskCreate(vTaskRunOneScript,"Run One Script",240,&RunScriptCmd,SCRIPT_RUNNER_PRIO,&vHandleTaskRunOneScript);
				 }
				 else
				 {
				  Report_MSG("Another Script is running."); 				
				}		 
				  

			   break;
         
		 case STOP_CURRENT_TEST:
		       			 if( RunScriptCmd.id != 0)
				 { 
				  
				  vTaskDelete(vHandleTaskRunOneScript);

				  //report, 结果值为250, 
			  	  Report_Test_Result(RunScriptCmd.id,250)  ;
				  				
			     // reset val
				  ScriptStopped();  // 
				  MEAS_HW_RST();
				  Report_MSG("Current test aborted.  ");
				 }
				 else
				 {
				 Report_MSG("Nothing to be deleted."); 				
				 }		
                break;

		 case SEND_USER_GESTURE:
				g_userHasGesture = true;
				break;

		 case SEND_USER_INPUT_VALUE:
			     g_userInputInfo = RecvCmd.para;
				 g_userHasGesture = true;
				break;

		 case SET_PRE_DEFINED_SETTING_Y:
		 		//设置缺省的系统参数，cable_a_res, cable_b_res
				//sprintf(charBuf,"INFO:cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;
	            //Report_MSG(charBuf)	 ;

				//  注意，参数编号从1开始：1：cable_res_a, 2:cable_res_b
				if(RecvCmd.target == 1 && RecvCmd.para <2000)
				{
				 PreDefinedPara.cable_res_a =   RecvCmd.para;
				 //record this paras to eeprom
				 EE_WriteVariable(VirtAddVarTab[CABLE_A_RES_EEADDR], PreDefinedPara.cable_res_a);
				}
				else if(RecvCmd.target ==2 && RecvCmd.para <2000)
				{
				  PreDefinedPara.cable_res_b =   RecvCmd.para;
				  EE_WriteVariable(VirtAddVarTab[CABLE_B_RES_EEADDR], PreDefinedPara.cable_res_b);
				  //record this paras to eeprom
				}

	


				break;
	    
		case GET_ALL_PRE_DEFINED_SETTING:

				ReportPreDefinedPara();
				
				break;


		default:	  //unknown cmd
			    //Report_MSG("ERRO:Unknown CMD!");
			   sprintf(charBuf,"ERRO:Unknown CMD! ,cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;
	           Report_MSG(charBuf)	 ;
			   break;
	   }

	   //Toggle LED	,
	   //GPIO_Toggle(SW_LED1_G) ;
	//	sprintf(charBuf,"CMD received ,cmd = 0x%X,target= 0x%X, para = 0x%X",RecvCmd.cmd,RecvCmd.target,RecvCmd.para) ;
	   // Report_MSG(charBuf)	 ;


	
	/*	
	//for Stack check	  
	
	 uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);		
	 sprintf(charBuf,"HighWaterMark = %d", uxHighWaterMark) ;	
     Report_MSG(charBuf)	 ;
	 */
	
	 } 

   }
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

  if(xSemaphoreTake(xMutexMsgBuff,MSG_BUFF_WRITE_TIMEOUT)== pdPASS  )
	{
	//copy the message header to CurBuff
  	memcpy(pCurBuffer + MsgSendBuffOffset, &onePkt, 12);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + 12 ;	
	//copy the current
	memcpy(pCurBuffer + MsgSendBuffOffset, content, msgLen);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + msgLen ;

	xSemaphoreGive(xMutexMsgBuff) ;
	}
   else
   {
	 //写buff失败，其他程序没有Give xMutexMsgBuff
	 ;
	 GPIO_Toggle(SW_LED1_R) ;
   }


}

void waitPcResponse(void)
{
	g_userHasGesture = false;
	while(! g_userHasGesture )
	{
		vTaskDelay(50);
	}
}



void Report_MSG(char *p)
{
	AddMsgToBuff(TESTX_MSG,0,strlen(p)+ 1,(uint8_t*)p);
} 

void Report_Test_Result(uint8_t ScriptId,uint32_t Re)
{
	AddMsgToBuff(REPORT_TEST_RESULT,ScriptId,4,(uint8_t*) &Re);
}

void ReportPreDefinedPara(void)
{
   
	UP_FMT onePkt;
 

	onePkt.startFlag = UP_START_FLAG;
	onePkt.pktSn = USB_TX_pktSn++	;
	onePkt.ts = xTaskGetTickCount();//GlobleTime;	//fill current time
	onePkt.src = 0;
	onePkt.msgType = UPLOAD_ALL_PRE_DEFINED_SETTING;
	onePkt.msgLenth =	sizeof(PreDefinedPara) ;

  if(xSemaphoreTake(xMutexMsgBuff,MSG_BUFF_WRITE_TIMEOUT)== pdPASS  )
	{
	//copy the message header to CurBuff
  	memcpy(pCurBuffer + MsgSendBuffOffset, &onePkt, 12);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + 12 ;	
	//copy the current
	

	memcpy(pCurBuffer + MsgSendBuffOffset, &PreDefinedPara, onePkt.msgLenth);	//消息头长12 byte
	MsgSendBuffOffset = MsgSendBuffOffset + onePkt.msgLenth ;

	xSemaphoreGive(xMutexMsgBuff) ;
	}
   else
   {
	 //写buff失败，其他程序没有Give xMutexMsgBuff
	 ;
	 GPIO_Toggle(SW_LED1_R) ;
   }

}


uint16_t askHostToInputVal(const char* p)
{
  AddMsgToBuff(ASK_HOST_INPUT_VALUE,0,strlen(p)+ 1,(uint8_t *)p);
  waitPcResponse();
  return  g_userInputInfo;
}

void WaitHostToContinue(const char* p)
{
   AddMsgToBuff(NEED_CLICK_OK_TO_CONTINUE,0,strlen(p)+ 1,(uint8_t *)p);
   waitPcResponse();
}




/*******************************************************************************
  * @函数名称	SendMsgToHost
  * @函数说明   将当前缓冲区中的数据通过EP1_IN 发送给HOST ,每  MSG_SEND_PERIOD MS触发一次
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void vTaskSendMsgToHost(void * pvParameters)
{
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
		    if(xSemaphoreTake(xMutexMsgBuff,MSG_BUFF_WRITE_TIMEOUT)== pdPASS  )
		    {
			   sendLen =  MsgSendBuffOffset;
			   pTemp = 	 pCurBuffer;
			   
				//switch buffer	,give Semaphore
			   pCurBuffer = pCurBuffer == MsgSendBuffA ? MsgSendBuffB : MsgSendBuffA;
			   MsgSendBuffOffset = 0;
			   xSemaphoreGive(xMutexMsgBuff) ;	
			   
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
}

//////////////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************
  * @函数名称	SendAdcDataToHost
  * @函数说明   将当前缓冲区中的ADC数据通过EP2_IN 发送给HOST 
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/
void SendAdcDataToHost(void)
{
	   uint8_t* pTemp;//= pCurBuffer;

		// if(DCD_GetEPStatus(&g_USB_link_dev,EP_ADC_IN) == USB_OTG_EP_TX_VALID )
		{
		   //EP valid

			   pTemp =  pCurAdcSendBuffer;

				//switch buffer	
			   pCurAdcSendBuffer = pCurAdcSendBuffer == AdcSendBuffA ? AdcSendBuffB : AdcSendBuffA;
			   AdcSendBuffOffset = 0;
			   
			   //send
			   //Report_MSG("Data Start sending...")	;
			//    DCD_EP_Tx(&g_USB_link_dev,EP_ADC_IN, (uint8_t*)(pTemp), ADC_BUFF_SIZE);	
			while(CDC_Transmit_HS((uint8_t*)(pTemp), ADC_BUFF_SIZE, EP_ADC_IN) != USBD_OK) {
				vTaskDelay(180U);
			}
	
	    }
}





/*******************************************************************************
  * @函数名称	AddDATAToAdcBuff
  * @函数说明   将ADC转换结果数据发送到AdcBuff,
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
*******************************************************************************/

void AddDataToAdcSendBuff(void)
 {

	UP_FMT onePkt;
//	uint8_t p;
	uint8_t dp_data_l, dp_data_h;
	

	onePkt.startFlag = UP_START_FLAG;
	onePkt.pktSn = USB_ADC_pktSn++	;
	onePkt.ts = xTaskGetTickCount();//GlobleTime;	
	onePkt.src = 0xff;
	onePkt.msgType = REPORT_ALL_ADC_VALUE;
	onePkt.msgLenth = 	16;


  //ADC数据只有一个writer,一个reader，且不会同时访问buff，因此不用Mutex
  //	{
	//copy the message header to CurBuff
  	memcpy(pCurAdcSendBuffer + AdcSendBuffOffset, &onePkt, 12);	//消息头长12 byte
	AdcSendBuffOffset = AdcSendBuffOffset + 12 ;	

	//ADC数据直接从当前的ADC_ConvertedBuff拷贝到pCurAdcSendBuffer, 长度为8CH x 2B = 16B

  /*  for(p=0;p<8;p++)
	{	
		temp = ADC_ProcessBuff[p];
		*(pCurAdcSendBuffer + AdcSendBuffOffset+2*p) =temp & 0x00ff;   //低8bit
		*(pCurAdcSendBuffer + AdcSendBuffOffset+2*p+1) =temp>>8;		//高8bit		
	}
   */
	 	memcpy(pCurAdcSendBuffer + AdcSendBuffOffset, (uint8_t *)ADC_ProcessBuff, 16);	//ADC数据长16 byte
		
		//swich ADC_DP and ADC_DM DATA
		dp_data_l = *(pCurAdcSendBuffer + AdcSendBuffOffset + 6);
		dp_data_h = *(pCurAdcSendBuffer + AdcSendBuffOffset + 7);
		*(pCurAdcSendBuffer + AdcSendBuffOffset + 6) = *(pCurAdcSendBuffer + AdcSendBuffOffset + 8);
		*(pCurAdcSendBuffer + AdcSendBuffOffset + 7) = *(pCurAdcSendBuffer + AdcSendBuffOffset + 9) ;
		*(pCurAdcSendBuffer + AdcSendBuffOffset + 8) = dp_data_l;
		*(pCurAdcSendBuffer + AdcSendBuffOffset + 9) = dp_data_h;

    //
		
		
	AdcSendBuffOffset = AdcSendBuffOffset + 16 ;
	
	
   //如果缓冲区满，则发送数据
   	if( AdcSendBuffOffset >= ADC_BUFF_SIZE )
	{
	 
	 SendAdcDataToHost();
    }

}

/*******************************************************************************
  * @函数名称	vTaskUsb3300EventProcess
  * @函数说明   处理HS USB OTG core 发来的状态信息，目前主要是将USBD or USBH 状态信息转换成MSG发出
  * @输入参数   无
  * @输出参数   无
  * @返回参数   无
  * NOTE event:
        DevInit,DevRst,DevCfg,DevSus,DevResum,DevCon,DevDiscon,
		HostInit,HostDeInit,HostDevAttach,HostDevRst,HostDevDiscon,HostDevOC,HostDevSpd,HostDevDesc,
	    HostDevAddr,HostDevCfgDesc,HostDevEnumDone 

*******************************************************************************/
void vTaskUsb3300EventProcess(void *pvParameters)
{
 USBEventQueue_FMT OneQueue;

   for(;;)
   {
	 if(xQueueReceive(xQueueUsb3300Event,&OneQueue,portMAX_DELAY)  == pdPASS)
	 {
	   //处理收到的有效消息

	   switch(OneQueue.event)
	   {

	     ///////////DEV EVENT////////////////////////////
		 case  DevInit:
			   Report_MSG("> Test Device Init") ;
		 	   break; 

		 case  DevRst:
			   Report_MSG("> Test Device Reset") ;
		 	   break;

		 case  DevCfg:
			   Report_MSG("> Test Device Configured") ;
		 	   break;

		 case  DevSus:
			   Report_MSG("> Test Device Suspended") ;
		 	   break;

		 case  DevResum:
			   Report_MSG("> Test Device Resumed") ;
		 	   break;

		 case  DevCon:
			   Report_MSG("> Test Device Connected") ;
		 	   break;

		 case  DevDiscon:
			   Report_MSG("> Test Device Disconnected") ;
		 	   break;
		/*
		 /////////////HOST EVENT////////////////////
		HostInit,HostDeInit,HostDevAttach,HostDevRst,HostDevDiscon,HostDevOC,HostDevSpd,HostDevDesc,
	    HostDevAddr,HostDevCfgDesc,HostDevEnumDone 
		 */
		 case  HostInit:
			   Report_MSG("> Init USB host...") ;
		 	   break;
		 case  HostDeInit:
			   Report_MSG("> De-Init USB host...") ;
		 	   break; 
		 case  HostDevAttach:
			   Report_MSG("> DUT Attached") ;
		 	   break;
		 case  HostDevRst:
			   Report_MSG("> Reset DUT") ;
		 	   break;
		 case  HostDevDiscon:
			   Report_MSG("> DUT Disconnected") ;
		 	   break;
		 case  HostDevOC:
			   Report_MSG("> Over Current Detected") ;
		 	   break;
		 case  HostDevSpd:
		       if(OneQueue.para == 0) Report_MSG("> High speed device detected") ;
			   else if(OneQueue.para == 1) Report_MSG("> Full speed device detected") ;
			   else if(OneQueue.para == 2) Report_MSG("> Low speed device detected") ;
			   else Report_MSG("> Error device speed!") ;
		 	   break;
		 case  HostDevDesc:
			   Report_MSG("> DUT DescAvailable") ;
		 	   break;
		 case  HostDevAddr:
			   Report_MSG("> DUT Address Available") ;
		 	   break;
		 case  HostDevCfgDesc:
			   Report_MSG("> DUT Configure Desc Available") ;
		 	   break;
		 case  HostDevEnumDone:
			   Report_MSG("> Enumeration completed") ;
			   ReportDutInfo();
		 	   break;

		 /////////////////////LINK DEV EVENT///////////////
		 case  LinkDevSus:
		       StopCurrentTest();
			  // ResetUsbLink(); 	   //不能加这个。
               // USB_LINK_Init();	  //加上这个，在运行一个test以后，拔出设备，设备会死掉
		 	   break;
		    	

		
		 default:

		       break;
		}

	  }
	}


}


void ScriptStopped(void)
{
  RunScriptCmd.id = 0;
  RunScriptCmd.para = 0;
}

void StopCurrentTest(void)
{
  if( RunScriptCmd.id != 0)
   { 		  
	 vTaskDelete(vHandleTaskRunOneScript);

		//report
		//  Report_Test_Result(RunScriptCmd.id,0)  ;				
	 // reset val
	  ScriptStopped();  //
	 }
	 MEAS_HW_RST();

	 //NVIC_SystemReset();

	

}

