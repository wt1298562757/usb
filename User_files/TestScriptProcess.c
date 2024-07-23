
/*脚本运行task, 负责依据参数调用测试脚本，返回测试结果。
  同时，也可以根据HOST的命令，终止当前的测试 。

  这个task采用queue进行触发。

*/

/* 包含头文件 *****************************************************************/
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

/* 变量 ----------------------------------------------------------------------*/

xQueueHandle xQueueRunTestScript; //FS LINK CMD队列
extern xQueueHandle xQueueRunTestScript;
extern char charBuf[MAX_MSG_LENTH];
extern fn_ptr fn_ScriptPool[MAX_SCRIPT_NUM];



/* 函数声明 ------------------------------------------------------------------*/
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



