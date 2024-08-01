

/*提供各种测量函数*/
#define GlobleTime xTaskGetTickCount()

#define SUCCESS 0

#define DECLARE_VARS() uint16_t tmp = 0;\
					   int32_t testResult = SUCCESS;

//release resource
#define TASK_CLEAN_UP() {  \
						tmp = 0;\
						powerDP(0,RES_NONE);\
						powerDM(0,RES_NONE);\
						DAC_Setting(DAC_VBUS_V , 0);\
						DAC_Setting(DAC_LOAD_I , 0);\
						DAC_Setting(DAC_VBUS_IL , 0);\
						DAC_Setting(DAC_LOAD_V , 0);\
						DAC_Setting(DAC_CM_V , 0);\
						Delay(1000);\
						GPIO_Setting(RSW_VBUS_AB,OFF);\
						GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);\
						GPIO_Setting(SW_VBUS_AB_M1,ON);\
						GPIO_Setting(RSW_DMDP_TEST,OFF);\
						GPIO_Setting(RSW_VBUS_ACC,OFF);\
						GPIO_Setting(RSW_VBUS_CHG,OFF);\
						GPIO_Setting(RSW_VBUS_ACC,OFF);\
						GPIO_Setting(SW_ID_RES_M_1,OFF);\
						GPIO_Setting(SW_ID_RES_M_2,OFF);\
						GPIO_Setting(SW_DCD_RES_AB,OFF);\
						VBUS_CAP_Setting(CAP_NONE);\
						GPIO_Setting(SW_GND_AB_M2,OFF);\
						GPIO_Setting(RSW_VCC_PHY,OFF);\
   						SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);\
   						SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);\
						return testResult; \
						}





//输出的函数
extern uint16_t WaitAdcValueMatch(uint8_t adc_name,uint8_t capture_type,uint16_t less_than,uint16_t more_than,uint16_t valid_times,uint32_t timeout);
extern uint16_t Get_Avarage_ADC_Value(uint8_t adc_name, uint16_t duration);

extern void WatchBlock_Handle(void);
extern void WatchBlockInit(void);
extern void enableWatchBlock(uint8_t watchBlockId, uint8_t adcId, uint8_t capMode, uint16_t realValue, uint16_t matchCnt );
extern void disableWatchBlock(uint8_t watchBlockId);
extern uint8_t IsWatchBlockTriggered(uint8_t wb_name);
extern uint16_t GetWatchBlockRecordVal(uint8_t wb_name);
extern void Delay(uint64_t t); 
extern void cmpFloatValue(float re,float stv,float diff);
extern void cmpTCValue(int16_t re,int16_t stv,int16_t diff);
/*比较re是否在stv-(stv * PER)~stv+(stv * PER)范围内，并输出结果*/
extern void cmpTCValuePer(int16_t re,int16_t stv,int16_t per);

//输出的全局变量
extern uint16_t ADC_Captured_Value;

