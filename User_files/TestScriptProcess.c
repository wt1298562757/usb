
/*�ű�����task, �������ݲ������ò��Խű������ز��Խ����
  ͬʱ��Ҳ���Ը���HOST�������ֹ��ǰ�Ĳ��� ��

  ���task����queue���д�����

*/

/* ����ͷ�ļ� *****************************************************************/
#include <string.h>
#include <stdio.h>

#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "LinkUsbProcess.h"
#include "PET_CMD.h"
#include "UCPET_bsp.h"
#include "TestScriptProcess.h"

/* ���� ----------------------------------------------------------------------*/

xQueueHandle xQueueRunTestScript; //FS LINK CMD����
extern xQueueHandle xQueueRunTestScript;
extern char charBuf[MAX_MSG_LENTH];
extern fn_ptr fn_ScriptPool[MAX_SCRIPT_NUM];



/* �������� ------------------------------------------------------------------*/
void vTaskRunTestScript(void *pvParameters)
{
 RUN_SCRIPT_CMD_FMT RunScriptCmd;
 uint32_t re;
 
 for(;;)
   {
	 if(xQueueReceive(xQueueRunTestScript,&RunScriptCmd,portMAX_DELAY)  == pdPASS)
	 {
		   	sprintf(charBuf,"Run Script Id = %d, Para = %d",RunScriptCmd.id,RunScriptCmd.para) ;
			Report_MSG(charBuf)	 ;
             
		
	 }
   }
}



