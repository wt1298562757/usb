
#include <stdio.h>




#define FS_LINK_RECV_QUEUE_LENGTH 8
#define FS_LINK_RECV_SIZE sizeof(FS_LINK_CMD_FMT)
#define MAX_MSG_LENTH 512
#define USB3300_QUEUE_MAX_NUM 8
#define USB3300_QUEUE_SIZE sizeof(USBEventQueue_FMT) 


typedef struct _FS_LINK_CMD_FMT{
 	uint8_t cmd;
	uint8_t	target;
	uint16_t para;
}FS_LINK_CMD_FMT ;

typedef enum  _USB3300_Event_FMT {DevInit,DevRst,DevCfg,DevSus,DevResum,DevCon,DevDiscon,\
		HostInit,HostDeInit,HostDevAttach,HostDevRst,HostDevDiscon,HostDevOC,HostDevSpd,HostDevDesc,\
	    HostDevAddr,HostDevCfgDesc,HostDevEnumDone,LinkDevSus } USB3300_Event_FMT   ;


typedef struct _USBEventQueue_FMT{
   USB3300_Event_FMT event;
   uint16_t para;
} USBEventQueue_FMT	;



extern char charBuf[MAX_MSG_LENTH];


extern uint8_t AddHostCmdtoQueue(uint8_t* pRecvBuff, uint16_t count);
extern void vTaskLinkUsbCmdProcess(void *pvParameters);
extern void vTaskSendMsgToHost(void * pvParameters);
extern void AddDataToAdcSendBuff(void);
extern void Report_MSG(char *p);
extern void Report_Test_Result(uint8_t ScriptId,uint32_t Re);
extern void SetAdcDataUploadState(uint8_t i);
extern uint8_t GetAdcDataUploadState(void) ;
extern uint16_t askHostToInputVal(const char* p);
extern void WaitHostToContinue(const char* p);
extern void vTaskUsb3300EventProcess(void *pvParameters);
extern void ScriptStopped(void);
extern void StopCurrentTest(void);
extern void LoadPreDefinedPara(void);



