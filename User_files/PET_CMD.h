
#define RX_CMD_LEN  8
#define TX_MSG_LEN  8
#define DOWN_START_FLAG  0X7E
#define DOWN_MSG_HANDLED 0XFF
#define DOWN_END_FLAG  0X55
#define UP_START_FLAG  0X7C
#define UP_START_FLAG_PROCESSED  0X7D
#define UP_END_FLAG  0XAA
#define MAX_PACKET_SN  0xFF
#define CONTROL_MODE_READ  0
#define CONTROL_MODE_WRITE  1
#define CONTROL_MODE_COMMAND  2
#define TARGET_ADC  0
#define TARGET_DAC  1
#define TARGET_RELAY  2
#define TARGET_TIMER  3
#define COMMAND_ENABLE  1
#define COMMAND_DISABLE  2
#define COMMAND_START  3
#define SWITCH_ON  0xFF
#define SWITCH_OFF  0x0
#define UPLOAD_CONTENT_LEN  255


#define UP_FMT_SIZE  64
#define DOWN_FMT_SIZE 8

#define DAC_VBUS_V 	0
#define DAC_LOAD_I 	1
#define DAC_VBUS_IL 2
#define DAC_LOAD_V 3
#define DAC_CM_V 	4

/*正常的定义
#define DAC_DP 		5
#define DAC_DM 		6
#define ADC_DP		 3
#define ADC_DM		 4
*/

/*DP_B,DM_B V3硬件接反了DM/DP.使用软件补救 20120406	*/
#define DAC_DM 		6
#define DAC_DP 		5


#define ADC_DM		 4
#define ADC_DP		 3
#define ADC_LOAD_V	 7
#define ADC_LOAD_I	 0
#define ADC_VBUS_V	 1
#define ADC_VBUS_I	 2

#define ADC_ID		 5
#define ADC_VBUS_AB	 6
#define ADC_ALL 9

#define X10_AMP_TH  1200  // X10状态下，测量值2000mV
#define X1_AMP_TH 	200	   //X1状态下，测量值200mV
#define HI_RANGE 	    0
#define LO_RANGE 		1
#define AUTO_RANGE      2 
#define AMP_Factor_SW_NUM 500    //X10,X1切换累计数
#define MAX_SCRIPT_NUM	  36
#define MAX_ADC_NUM  8
#define MAX_CAL_NUM	 64				   //必须是32-bit 对其
#define COR_DATA_ADDRESS  ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
//#define COR_DATA_ADDRESS ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */

#define resCapturedInRange 0xAA

#define resCapturedTimeOut 0xBB
#define resCapturedSmall 0xcc
#define resCapturedBig 0xdd
//#define CaptureInRang 1
#define CaptureInRange 1
#define CaptureOutRang 2
#define CaptureSmall 3
#define CaptureBig 4
#define CaptureBigOrSmall 5

#define TEST_FAILED 1

#define ON 1
#define OFF 0
#define TO_STM_TIME_UNIT(t) (t*10)

#define VALID_I_FOR_CMP 250	   // 5mV/0.2R = 25mA ,25x10 = 250
#define INTERNAL_RES      25     // tester 内阻 ，单位:毫欧母
#define DEF_CABLE_RES         148	 // SPECIAL CABLE B 缺省内阻

#define ADC_OFFSET_HEX  82 // V4 ADC HEX val OFFSET  = 4096x 50 /2500 = 82

/*
//gpio define V2 borad
#define	RSW_ADP_RES_1	0x72	//	
#define	RSW_ADP_RES_2	0x4A	//	
//#define	RSW_CM_Polarity	0x76	//	共模电压输出极性
#define	RSW_DCD_CAP		0x55	//	DAC_DP和DP间串1nF电容
#define	RSW_DMDP_TEST	0x28	//	用于DMDP测试模式和非测试模式的切换，低为测试模式
#define	RSW_EL_MODE		0x75	//	LOAD的工作模式CC/CV
#define	RSW_LOAD_APPLY	0x73	//	将电子负载接入到VBUS和GND_AB间
#define	RSW_OFFSET_APPLY	0x74	//	将OFFSET加载到GND_AB和GND_GEN间，
#define	RSW_USER_A		0x1A	//	用户开关A---可用于控制DUT的power
#define	RSW_VBUS_AB		0x4E	//	VBUS_AB加载
#define	RSW_VBUS_ACC	0x4F	//	VBUS_ACC加载
#define	RSW_VBUS_ACC_LD1	0x4A	//	VBUS_ACC串入10欧姆电阻再加载 继电器？
#define	RSW_VBUS_ACC_LD2	0x4B	//	VBUS_ACC串入625欧姆电阻再加载
#define	RSW_VBUS_CAP_1	0x5D	//	在VBUS和GND_AB间加载150nF电容
#define	RSW_VBUS_CAP_2	0x5F	//	在VBUS和GND_AB间加载900nF电容
#define	RSW_VBUS_CAP_3	0x5E	//	在VBUS和GND_AB间加载1uF电容
#define	RSW_VBUS_CAP_4	0x5C	//	在VBUS和GND_AB间加载5.5uF电容
#define	RSW_VBUS_CAP_5	0x2E	//	在VBUS和GND_AB间加载10uF电容
#define	RSW_VBUS_CAP_6	0x2F	//	在VBUS和GND_AB间加载100uF电容
#define	RSW_VBUS_CHG	0x72	//	VBUS_CHG加载
#define	SNS_SPEC_CAB	0x31	//	input 
#define	SW_ADC_LOAD_I_EN	0x78	//	用于防止在未正确配置运放时，输出高压损害ADC
#define	SW_ADC_VBUS_I_EN	0x37	//	用于防止在未正确配置运放时，输出高压损害ADC
#define	SW_Buzzer		0x42	//	蜂鸣器
#define	SW_DCD_RES_1	0x7B	//	DAC_DM和DM间串1k5欧姆电阻
#define	SW_DCD_RES_10	0x52	//	DAC_DP和DP间串100K欧姆电阻
#define	SW_DCD_RES_2	0x43	//	DAC_DM和DM间串200欧姆电阻
#define	SW_DCD_RES_3	0x79	//	DAC_DM和DM间串3K9欧姆电阻
#define	SW_DCD_RES_4	0x7A	//	DAC_DM和DM间串15K欧姆电阻
#define	SW_DCD_RES_5	0x7F	//	DAC_DM和DM间串100K欧姆电阻
#define	SW_DCD_RES_6	0x7C	//	DAC_DM和DM间串?欧姆电阻
#define	SW_DCD_RES_7	0x53	//	DAC_DP和DP间串1K5欧姆电阻
#define	SW_DCD_RES_8	0x29	//	DAC_DP和DP间串200欧姆电阻
#define	SW_DCD_RES_9	0x51	//	DAC_DP和DP间串15K欧姆电阻
#define	SW_DCD_RES_AB	0x61	//	DM和DP间串183R
#define	SW_DCD_RES_CHG	0x3E	//	DM_CHG短接DM_CHG
#define	SW_DFU_EN		0x19	//	input 
#define	SW_DP_VP_CHG	0x71	//	用于DP_CHG电压测量 to ADC_VUBS_AB +
#define	SW_GND_VM		0x59	//	引入GND to ADC_VBUS_AB -
#define	SW_GND_VM_AB	0x58	//	引入GND_AB to ADC_VBUS_AB -
#define	SW_ID_3300		0x3D	//	USB3300的ID输入
#define	SW_ID_ACC_LOW	0x3F	//	ID_ACC直连ID_GND
#define	SW_ID_GND		0x64	//	ID和GND_AB间加载0欧姆
#define	SW_ID_RES_1		0x60	//	ID和GND_AB间加载1K
#define	SW_ID_RES_2		0x6D	//	ID和GND_AB间加载35K
#define	SW_ID_RES_3		0x35	//	ID和GND_AB间加载39K
#define	SW_ID_RES_4		0x6B	//	ID和GND_AB间加载65K
#define	SW_ID_RES_5		0x6C	//	ID和GND_AB间加载72K
#define	SW_ID_RES_6		0x65	//	ID和GND_AB间加载119K
#define	SW_ID_RES_7		0x62	//	ID和GND_AB间加载132K
#define	SW_ID_RES_8		0x63	//	ID和GND_AB间加载220K
#define	SW_ID_RES_M_1	0x7D	//	测量ID用电阻1K
#define	SW_ID_RES_M_2	0x7E	//	测量ID用电阻82K
#define	SW_LED1			0x49	//	面板LED1
#define	SW_LED2			0x48	//	面板LED2
#define	SW_LOAD_CF_switch	0x38	//	LOAD电流检测放大倍数
#define	SW_VBUS_CF_switch	0x77	//	VBUS电流检测放大倍数
#define	SW_VBUS_LKG		0x54	//	在VBUS和GND_AB间加载2k 电阻用于模拟VBUS leakage
#define	SW_VBUS_PD		0x56	//	在VBUS和GND_AB间加载10k电阻用于模拟 PD ？？
#define	SW_VBUS_VM_AB	0x5A	//	用于VBUS_AB电压测量 to ADC_VUBS_AB -
#define	SW_VBUS_VM_ACC	0x57	//	用于VBUS_ACC电压测量 to ADC_VBUS_AB -
#define	SW_VBUS_VP_AB	0x70	//	用于VBUS_AB电压测量 to ADC_VUBS_AB +
#define	SW_VBUS_VP_ACC	0x6F	//	用于VBUS_ACC电压测量 to ADC_VBUS_AB +
#define	SW_VBUS_VP_CHG	0x6E	//	用于VBUS_CHG电压测量 to ADC_VBUS_AB +
*/


/*
//GPIO V3 borad 2012.3.6

#define		DFU_EN		0x19	//	DFU
#define		RSW_ADP_RES_1		0x72	//	
#define		RSW_ADP_RES_2		0x4A	//	
#define		RSW_CAL_I_5MA		0x36	//	
#define		RSW_CAL_I_100MA		0x37	//	
#define		SW_CAL_DP		0x1F	//	
#define		SW_CAL_VBUS_AB		0x7E	//	
#define		RSW_EL_TO_GND_AB		0x2E	//	
#define		RSW_EL_TO_VBUS_AB		0x2F	//	
#define		RSW_AGND_TO_GND_AB		0x48	//	
#define		RSW_VBUS_AB		0x49	//	
#define		RSW_VBUS_ACC		0x4C	//	
#define		RSW_VBUS_CHG		0x4B	//	
#define		RSW_VBUS_CAP_1		0x74	//	150nF
#define		RSW_VBUS_CAP_2		0x75	//	900nF
#define		RSW_VBUS_CAP_3		0x73	//	1uF
#define		RSW_VBUS_CAP_4		0x76	//	5.5uF
#define		RSW_VBUS_CAP_5		0x77	//	9uF
#define		RSW_VBUS_CAP_6		0x78	//	100uF
#define		RSW_DCD_CAP		0x4E	//	1nF to DP
#define		SW_VBUS_LKG		0x7D	//	
#define		SW_VBUS_PD		0x24	//	
#define		SW_DCD_RES_1		0x38	//	1K5 to DM
#define		SW_DCD_RES_2		0x3C	//	200R to DM
#define		SW_DCD_RES_3		0x39	//	3K9 to DM
#define		SW_DCD_RES_4		0x1A	//	15K to DM
#define		SW_DCD_RES_5		0x18	//	100K to DM
#define		SW_DCD_RES_6		0x3A	//	spare to DP
#define		SW_DCD_RES_7		0x43	//	1K5 to DP
#define		SW_DCD_RES_8		0x41	//	200R to DP
#define		SW_DCD_RES_9		0x40	//	15K to DP
#define		SW_DCD_RES_10		0x42	//	100K to DP
#define		SW_DCD_RES_AB		0x3B	//	
#define		SW_DCD_RES_CHG		0x47	//	
#define		SW_ID_RES_1		0x28	//	ID和GND_AB间加载1K
#define		SW_ID_RES_2		0x53	//	ID和GND_AB间加载35K
#define		SW_ID_RES_3		0x51	//	ID和GND_AB间加载39K
#define		SW_ID_RES_4		0x50	//	ID和GND_AB间加载65K
#define		SW_ID_RES_5		0x52	//	ID和GND_AB间加载72K
#define		SW_ID_RES_6		0x11	//	ID和GND_AB间加载119K
#define		SW_ID_RES_7		0x56	//	ID和GND_AB间加载132K
#define		SW_ID_RES_8		0x26	//	ID和GND_AB间加载220K
#define		SW_ID_GND		0x10	//	
#define		SW_ID_RES_M_1		0x29	//	
#define		SW_ID_RES_M_2		0x27	//	
#define		SW_DP_CHG_M1		0x7B	//	DP_CHG TO M1
#define		SW_VBUS_CHG_M1		0x7A	//	VBUS_CHG TO M1
#define		SW_VBUS_ACC_M1		0x79	//	VBUS_ACC TO M1
#define		SW_GND_AB_M1		0x7C	//	GND_AB TO M1
#define		SW_VBUS_AB_M1		0x7F	//	VBUS_AB TO M1
#define		SW_M_ID		0x46	//	ID TO M2
#define		SW_GND_AB_M2		0x45	//	GND_AB TO M2
#define		SW_VBUS_CF_switch		0x5E	//	
#define		SW_ADC_VBUS_I_EN		0x5B	//	
#define		SW_LOAD_CF_switch		0x5C	//	
#define		SW_ADC_LOAD_I_EN		0x5D	//	
#define		RSW_EL_MODE		0x5F	//	
#define		RSW_DMDP_TEST		0x12	//	
#define		RSW_VBUS_ACC_LD1		0x4F	//	
#define		RSW_VBUS_ACC_LD2		0x4D	//	
#define		SW_ID_ACC_LOW		0x44	//	
#define		SNS_SPEC_CAB		0x54	//	
#define		RSW_VCC_PHY		0x5A	//	
#define		SW_ID_3300		0x55	//	
#define		SW_LED1_G		0x71	//	
#define		SW_LED1_R		0x57	//	
#define		SW_LED2_G		0x58	//	
#define		SW_LED2_R		0x59	//	
#define		SW_Buzzer		0x70	//	



//end of gpio define
*/


//GPIO V4 borad 2018.3.24

#define   VBUS_V_SENSE  0x19  //DFU NEW FOR V4
#define		DFU_EN		0x60	//	DFU  NEW FOR V4
#define		RSW_ADP_RES_1		0x72	//	
#define		RSW_ADP_RES_2		0x4A	//	
#define		RSW_CAL_I_5MA		0x36	//	
#define		RSW_CAL_I_100MA		0x37	//	
#define		SW_CAL_DP		0x1F	//	
#define		SW_CAL_VBUS_AB		0x7E	//	
#define		RSW_EL_TO_GND_AB		0x2E	//	
#define		RSW_EL_TO_VBUS_AB		0x2F	//	
#define		RSW_AGND_TO_GND_AB		0x48	//	
#define		RSW_VBUS_AB		0x49	//	
#define		RSW_VBUS_ACC		0x4C	//	
#define		RSW_VBUS_CHG		0x4B	//	
#define		RSW_VBUS_CAP_1		0x74	//	150nF
#define		RSW_VBUS_CAP_2		0x75	//	900nF
#define		RSW_VBUS_CAP_3		0x73	//	1uF
#define		RSW_VBUS_CAP_4		0x76	//	5.5uF
#define		RSW_VBUS_CAP_5		0x77	//	9uF
#define		RSW_VBUS_CAP_6		0x78	//	100uF
#define		RSW_DCD_CAP		0x4E	//	1nF to DP
#define		SW_VBUS_LKG		0x7D	//	
#define		SW_VBUS_PD		0x24	//	
#define		SW_DCD_RES_1		0x38	//	1K5 to DM
#define		SW_DCD_RES_2		0x3C	//	200R to DM
#define		SW_DCD_RES_3		0x39	//	3K9 to DM
#define		SW_DCD_RES_4		0x1A	//	15K to DM
#define		SW_DCD_RES_5		0x18	//	100K to DM
#define		SW_DCD_RES_6		0x3A	//	spare to DP
#define		SW_DCD_RES_7		0x43	//	200R to DP 
#define		SW_DCD_RES_8		0x41	//	1K5 to DP
#define		SW_DCD_RES_9		0x40	//	15K to DP
#define		SW_DCD_RES_10		0x42	//	100K to DP
#define		SW_DCD_RES_AB		0x3B	//	
#define		SW_DCD_RES_CHG		0x47	//	
#define		SW_ID_RES_1		0x28	//	ID和GND_AB间加载1K
#define		SW_ID_RES_2		0x53	//	ID和GND_AB间加载35K
#define		SW_ID_RES_3		0x51	//	ID和GND_AB间加载39K
#define		SW_ID_RES_4		0x50	//	ID和GND_AB间加载65K
#define		SW_ID_RES_5		0x52	//	ID和GND_AB间加载72K
#define		SW_ID_RES_6		0x11	//	ID和GND_AB间加载119K
#define		SW_ID_RES_7		0x56	//	ID和GND_AB间加载132K
#define		SW_ID_RES_8		0x26	//	ID和GND_AB间加载220K
#define		SW_ID_GND		0x10	//	
#define		SW_ID_RES_M_1		0x29	//	
#define		SW_ID_RES_M_2		0x27	//	
#define		SW_DP_CHG_M1		0x7B	//	DP_CHG TO M1
#define		SW_VBUS_CHG_M1		0x7A	//	VBUS_CHG TO M1
#define		SW_VBUS_ACC_M1		0x79	//	VBUS_ACC TO M1
#define		SW_GND_AB_M1		0x7C	//	GND_AB TO M1
#define		SW_VBUS_AB_M1		0x7F	//	VBUS_AB TO M1
#define		SW_M_ID		0x46	//	ID TO M2
#define		SW_GND_AB_M2		0x45	//	GND_AB TO M2
#define		SW_VBUS_CF_switch		0x5E	//	
#define		SW_ADC_VBUS_I_EN		0x5B	//	
#define		SW_LOAD_CF_switch		0x5C	//	
#define		SW_ADC_LOAD_I_EN		0x5D	//	
#define		RSW_EL_MODE		0x5F	//


#define		RSW_DMDP_TEST		0x12	//	
#define		RSW_VBUS_ACC_LD1		0x4F	//	
#define		RSW_VBUS_ACC_LD2		0x4D	//	
#define		SW_ID_ACC_LOW		0x44	//	
#define		SNS_SPEC_CAB		0x54	//	
#define		RSW_VCC_PHY		0x5A	//	
#define		SW_ID_3300		0x55	//	
#define		SW_LED1_G		0x71	//	
#define		SW_LED1_R		0x57	//	
#define		SW_LED2_G		0x58	//	
#define		SW_LED2_R		0x59	//	
#define		SW_Buzzer		0x70	//	



//end of gpio define

//DAC_DM to DM res define
#define RES_NONE 0
#define RES_LOW 1
#define RES_200R 2
#define RES_3K9 3
#define RES_15K 4
#define RES_100K 5
#define RES_unused 6
#define RES_1K5 7
#define RES_Rid_GND 8
#define RES_Rid_GND_max 9
#define RES_Rid_C_min  10 
#define RES_Rid_C_max 11
#define RES_Rid_B_min 12
#define RES_Rid_B_max 13
#define RES_Rid_A_min 14
#define RES_Rid_A_max 15
#define RES_Rid_FLOAT 16
#define RES_Rid_OPEN  17
#define RES_10K 18
#define RES_2K  19

#define CC 0
#define CV 1



#define CAP_1nF 20
#define CAP_150nF 21
#define CAP_900nF 22
#define CAP_1uF 23
#define CAP_5uF5 24
#define CAP_6uF5 25
#define CAP_10uF 26
#define CAP_100uF 27
#define CAP_NONE 28

#define RID_A  29
#define RID_B	30
#define RID_C	31
#define RID_FLOAT	32
#define RID_GND		33
#define RID_UNKNOWN	34
#define ACA_Dock 35
#define ACA_A 36
#define ACA_B 37
#define ACA_C 38
#define NOT_ACA 39
#define SDP 40
#define CDP_DCP 41
#define CDP 42
#define DCP 43

#define WatchBlockNum 4
#define USB_TX_FMT_COST 12

#define NUM_OF_PRE_DEFINED_PARA 2

typedef struct 
{
	uint16_t cable_res_a;
	uint16_t cable_res_b;
}PreDefinedPara_t ;


typedef enum _PET_COMMAND{
SYSTEM_RESET,
SELF_CHECK,
SELF_CALIBRATE,
GET_SYSTEM_TIME,
GET_SYSTEM_ID,
GET_HARDWARE_VERSION,
GET_FW_ID,
FW_UPDATE, //07
SET_SWITCHX_POSITION = 0X10, 
SET_RELAYX_POSITION = 0X11,
SET_ADCX_OPER_MODE = 0X12,
SET_ADCX_TRIGGER_HIGH_LEVEL = 0X13,
SET_ADCX_TRIGGER_LOW_LEVEL = 0X14,
SET_ADCX_TRIGGER_ON_OFF = 0X15,
SET_ADCX_ON_OFF = 0X16,
SET_DACX_OPERATION_MODE = 0X17,
SET_DACX_VALUE = 0X18,
SET_DACX_ON_OFF = 0X19,
SET_PRE_DEFINED_SETTING_Y = 0X1A,
GET_SWITCHX_POSITION = 0X20,
GET_RELAYX_POSITION = 0X21,
GET_ADCX_VALUE = 0X22,
GET_ALL_PRE_DEFINED_SETTING = 0X24,
RUN_TESTX = 0X30,
STOP_CURRENT_TEST = 0X31,
SET_TESTX_PARAMETERS = 0X32,

//lishunda add
SEND_USER_GESTURE,
SEND_OPERATOR_SELECTION,
SEND_USER_INPUT_VALUE,
}PET_COMMAND;

typedef struct _DOWN_FMT{
	uint8_t startFlag;
	uint8_t pktSn;
	uint8_t cmd;
	uint8_t	target;
	uint16_t para;
	uint8_t handleFlag;
	uint8_t endFlag;
}DOWN_FMT;





#define PC_DOWN_CMD_SIZE sizeof(DOWN_FMT)
			
//typedef DOWN_FMT* PDOWN_FMT;
#define  OPERATOR_CHOSE_YES 1
#define WORD OPERATOR_CHOSE_NO 0


typedef struct _UP_FMT{
	uint8_t startFlag;
	uint8_t pktSn;
	uint8_t msgType;	//
	uint8_t src;		//refer to INFO_CATEGORY
	uint32_t msgLenth;
	uint32_t ts;
	uint8_t content[UPLOAD_CONTENT_LEN+1];
}UP_FMT;

#define UP_PKT_WRAPPER_SIZE 12
#define SIZE_OF_UPLOAD_PKT(p) (UP_PKT_WRAPPER_SIZE + (p)->msgLenth)

//typedef UP_FMT* PUP_FMT;

typedef enum _INFO_CATEGORY{
	RESP_HOST_CMD,
	SYSTEM_ID,
	HARDWARE_VER,
	FW_ID,
	SWITCH_POS,
	RELAY_POS,
	ADCX_VAL,
	ADCX_TRIGER_HIGH,
	ADCX_TRIGER_LOW,
	TESTX_MSG,
	ERROR_MSG,
	SYSTEM_TIME,
 //lishunda add
 	REPORT_ALL_ADC_VALUE, //0x0C: all ADC Value
	REPORT_TEST_RESULT,
	ASK_OPERATOR_CONFIRM,
	NEED_CLICK_OK_TO_CONTINUE,		//0x0f
	ASK_HOST_INPUT_VALUE,	//0X10
	UPLOAD_ALL_PRE_DEFINED_SETTING = 0x1A ,
}INFO_CATEGORY;

typedef struct _WatchBlock_FMT{
  uint16_t ValueHigh;
  uint16_t ValueLow;
  uint16_t ValidNum;        //连续多少个sample满足条件后，trigger
  uint16_t CaptureDuration; //单位0.1ms 
  uint16_t RecordedVaule;	//trigger时保留下的数值
  uint16_t InRangNum ;
  uint16_t BiggerNum;
  uint16_t SmallerNum;
  uint8_t ADCName;		
  uint8_t CaptureMode;		//InRang,OutRang
  uint8_t Result;		//触发flag
  uint8_t EnableFlag;       //使能flag
  uint32_t CaptureTime;        //捕获事件的时间
}WatchBlock_FMT;


//2011 8 5 add
enum TEST_SCRIPT_ID{
	B_UUT_INITIAL_POWER_UP_TEST = 1,
	DATA_CONTACT_DETECT_TEST_WITH_CURRENT_SOURCE,
	DATA_CONTACT_DETECT_TEST_NO_CURRENT_SOURCE,
	DCP_DETECTION_TEST,
	CDP_DETECTION_TEST,
	SDP_DETECTION_TEST,
	ACA_DOCK_DETECTION_TEST,
	ACA_A_DETECTION_TEST,
	ACA_B_DETECTION_TEST,
	ACA_C_DETECTION_TEST,
	ACA_GND_DETECTION_TEST,
	COMMON_MODE_TEST_FULL_SPEED,
	COMMON_MODE_TEST_HIGH_SPEED,
	DEAD_BATTERY_PROVISION_TEST,

	DCP_OVERSHOOT_AND_UNDERSHOOT_VOLTAGE_TEST,
	DCP_VOLTAGE_AND_CURRENT,
	DCP_RESISTANCE_AND_CAPACITANCE_TESTS,

	CDP_OVERSHOOT_AND_UNDERSHOOT_VOLTAGE_TEST,
	CDP_VOLTAGE_AND_CURRENT_TEST,
	CDP_HANDSHAKING_TEST,
	CDP_GROUND_OFFSET_TEST_FULL_SPEED,
	CDP_GROUND_OFFSET_TEST_HIGH_SPEED,


	SDP_ATTACHED_TO_CHARGER_PORT_NOTHING_ATTACHED_TO_ACCESSORY_PORT,
	SDP_ATTACHED_TO_CHARGER_PORT_B_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	SDP_ATTACHED_TO_CHARGER_PORT_A_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	DCP_OR_CDP_ATTACHED_TO_CHARGER_PORT_NOTHING_ATTACHED_TO_ACCESSORY_PORT,
	DCP_OR_CDP_ATTACHED_TO_CHARGER_PORT_B_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	DCP_OR_CDP_ATTACHED_TO_CHARGER_PORT_A_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	NOTHING_ATTACHED_TO_CHARGER_PORT_NOTHING_ATTACHED_TO_ACCESSORY_PORT,
	NOTHING_ATTACHED_TO_CHARGER_PORT_B_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	NOTHING_ATTACHED_TO_CHARGER_PORT_A_DEVICE_ATTACHED_TO_ACCESSORY_PORT,
	BYPASS_CAPACITANCE,

};

typedef uint16_t(* fn_ptr)(void *)	;












