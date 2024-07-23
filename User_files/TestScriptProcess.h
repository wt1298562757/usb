


//fs link receive queue format
typedef struct _RUN_SCRIPT_CMD_FMT{
	uint8_t	id;
	uint8_t para;
}RUN_SCRIPT_CMD_FMT ;

#define TEST_PASSED 0
#define TEST_FAILED 1


#define RUN_SCRIPT_QUEUE_LENGTH 1
#define RUN_SCRIPT_CMD_SIZE sizeof(RUN_SCRIPT_CMD_FMT)





void vTaskRunTestScript(void *pvParameters);
