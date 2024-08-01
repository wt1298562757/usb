
/*******************************************************************************
** �ļ�����: 	AdcProcess.c
** ��    ����  	1.0
** ��������: 	
** ��    ��: 	WEIMING
** ��������: 	2013-2-27
** ��    ��:	����ADC����

*******************************************************************************/

/* ����ͷ�ļ� *****************************************************************/
#include <string.h>
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stdbool.h"


#include "PET_CMD.h"
// #include "UCPET_bsp.h"
#include "LinkUsbProcess.h"
#include "flash_if.h"
#include "MeasureProcess.h"
#include "gpio_control.h"
#include "adc.h"


/* �������� ------------------------------------------------------------------*/


/* �궨�� --------------------------------------------------------------------*/


/* ���� ----------------------------------------------------------------------*/
static int8_t ADC_cor[MAX_ADC_NUM* MAX_CAL_NUM] ;	   //ADC����У�����飬ÿ���ϵ�󣬴�flash�ж���У�����ݵ��������
static uint16_t OUT_MORE_THAN_200MA_NUM,OUT_LESS_THAN_200MA_NUM;   //��������״̬����С��200mA��Ӳ���Ŵ�ʮ��
static uint16_t IN_MORE_THAN_200MA_NUM,IN_LESS_THAN_200MA_NUM;	   //����200mA��Ӳ�����Ŵ�����Ŵ�ʮ��



//extern __IO uint16_t* pCurADC_ConvertedBuff; 
extern __IO uint16_t ADC_ProcessBuff[8];
extern xSemaphoreHandle xSemphrBinAdcDataSync; //ADC����ͬ����
extern PreDefinedPara_t PreDefinedPara;


/* �������� ------------------------------------------------------------------*/
void LoadCorData(void);
void AdcDataProcess(void);
void vTaskAdcProcess(void * pvParameters);


 /*ADC_LOAD_I �Զ����̴���*/
void  IN_I_Auto_Range(void)
{
      
	 //for IN Current
	 if(GetLoadCurrentRange() == LO_RANGE)
	 {
		   //��ǰΪӲ���Ŵ�10�������ԭʼֵ> 200mA (����ֵ2000mV����Ӧ��2000/2500����4096 = 3277),
		   //DAC���0-3277����ʾ���0-3277��ʵ��ֵ = ��ʾ���/10
		   if(ADC_ProcessBuff[ADC_LOAD_I]> X10_AMP_TH )
			{
			   IN_MORE_THAN_200MA_NUM++;
			   if (IN_MORE_THAN_200MA_NUM >= AMP_Factor_SW_NUM)
			   {
			     SetLoadCurrentRange(HI_RANGE);	   //��һת��ʱ��Ӳ���Ŵ�1
				 IN_MORE_THAN_200MA_NUM = 0;
				} 		 	 
			}
			else
			{
			   IN_MORE_THAN_200MA_NUM = 0;
			}
	 }
	 else  //I_AMP = LOW
	{
	    	 if(ADC_ProcessBuff[ADC_LOAD_I]< X1_AMP_TH )
			 {
		          IN_LESS_THAN_200MA_NUM++;
				  if(IN_LESS_THAN_200MA_NUM >= AMP_Factor_SW_NUM)
				  {
				    SetLoadCurrentRange(LO_RANGE);	//��һת��ʱ��Ӳ���Ŵ�10 
					 IN_LESS_THAN_200MA_NUM = 0;
				  }
			 }
			 else
			 {
				IN_LESS_THAN_200MA_NUM = 0;
			 }
			 //������ֵx10 ������Ŵ�10��
			 //DAC���328-4095������Ŵ�ʮ��=> 3280-40960,ʵ��ֵ �ٳ���10��
			 ADC_ProcessBuff[ADC_LOAD_I] =   ADC_ProcessBuff[ADC_LOAD_I]*10;
	}

}

void  OUT_I_Auto_Range(void)
{
	//For OUT current
	if(GetVbusCurrentRange() == LO_RANGE)
	 {
	   //��ǰΪ10X�����ԭʼֵ> 200mA 
	   //ʵ��ֵ = ��ʾ���/10
	   if(ADC_ProcessBuff[ADC_VBUS_I]> X10_AMP_TH )
		{
		   OUT_MORE_THAN_200MA_NUM++;
		   if (OUT_MORE_THAN_200MA_NUM >= AMP_Factor_SW_NUM)
		   {
		     SetVbusCurrentRange(HI_RANGE);
			 OUT_MORE_THAN_200MA_NUM = 0;
			} 		 	 
		}
		else
		{
		   OUT_MORE_THAN_200MA_NUM = 0;
		}
	 }
	 else
	{
    	 if(ADC_ProcessBuff[ADC_VBUS_I]< X1_AMP_TH )
		 {
	          OUT_LESS_THAN_200MA_NUM++;
			  if(OUT_LESS_THAN_200MA_NUM >= AMP_Factor_SW_NUM)
			  {
			    SetVbusCurrentRange(LO_RANGE);	//��һת��ʱ��Ӳ���Ŵ�10�� 
				OUT_LESS_THAN_200MA_NUM = 0;
			  }
		 }
		 else
		 {
			OUT_LESS_THAN_200MA_NUM = 0;
		 }
		 //������ֵx10 ������Ŵ�10��
		 //DAC���328-4095������Ŵ�ʮ��=> 3280-40960,ʵ��ֵ �ٳ���10��
		 ADC_ProcessBuff[ADC_VBUS_I] =   ADC_ProcessBuff[ADC_VBUS_I]*10; 

	}

}

void CurrentRangeProcess()
{
  uint8_t in_mode,out_mode;
  
  in_mode =  GetCurrentRangeMode(ADC_LOAD_I);
  out_mode = GetCurrentRangeMode(ADC_VBUS_I) ;

  if( in_mode == AUTO_RANGE)
  {
   IN_I_Auto_Range();
  }
  else if(in_mode == HI_RANGE)
  {
	 //������ֵx10 ������Ŵ�10��
	//ʵ��ֵ �ٳ���10��
	ADC_ProcessBuff[ADC_LOAD_I] =   ADC_ProcessBuff[ADC_LOAD_I]*10;
  }

  if(out_mode == AUTO_RANGE)
  {
   OUT_I_Auto_Range();
   }
    else if(out_mode == HI_RANGE)
  {
	 //������ֵx10 ������Ŵ�10��
	//ʵ��ֵ �ٳ���10��
	ADC_ProcessBuff[ADC_VBUS_I] =   ADC_ProcessBuff[ADC_VBUS_I]*10;
  }

}





void Clear_Cor_Data(void)
{

	memset((uint8_t *)ADC_cor,0,512);
}




void LoadCorData(void)
{
    Clear_Cor_Data();
	// FLASH_If_Load_Cor_Data(ADC_cor);
}


/*���ݵ������򣬲���tester�����calbe������ɵ�ѹ��*/
int32_t R_compensation(void)
{
 
  uint16_t i_out, i_in ;
  int32_t cmp_val;
  uint32_t r_tester;
  uint16_t r_cable_a,r_cable_b;

  r_cable_a = PreDefinedPara.cable_res_a;
  r_cable_b = PreDefinedPara.cable_res_b;
  r_tester = INTERNAL_RES ;

  i_out = ADC_ProcessBuff[ADC_VBUS_I];		//adc_vbus_i
  i_in =  ADC_ProcessBuff[ADC_LOAD_I];		//adc_load_i

  if(i_out > VALID_I_FOR_CMP && i_in < VALID_I_FOR_CMP)  
  {
   //output current
 	 //TESTER�������ʱ��ʵ�ʵ�ѹ�Ȳ��Ե�ѹ�� ,ʹ��special cable A
	cmp_val = -( (r_cable_a + r_tester)*i_out/10000);	

  }
  else if(i_out < VALID_I_FOR_CMP && i_in > VALID_I_FOR_CMP)
  {
   //input current
   	//TESTER�������ʱ��ʵ�ʵ�ѹ�Ȳ��Ե�ѹ��,ʹ��special cable B
	 cmp_val = (r_cable_b + r_tester)*i_in/10000;
  }
  // i_out>VALID_I_FOR_CMP &&  i_in > VALID_I_FOR_CMP ----> �Ի�������
  else 
  {
     cmp_val = 0;
  }	
  return cmp_val;
}


/*******************************************************************************
  * @��������	 AdcDataProcess()
  * @����˵��   ����ADC���ƣ���ADC��HEX��ֵת����Ӧ����ʾdecֵ
  				
  * note��	    ��8ch����һ��������HEX to decת���������������棬���ݽ��������貹��
  				������ɺ�Ľ����Ȼ����ADC_ConvertedBuff��,��������������ʹ��
*******************************************************************************/

void AdcDataProcess(void)
{
	int32_t tmp_adc;
	uint16_t cor_index;
	int32_t r_cmp = 0;


  	
	//V3:ADC_LOAD_I ,	 0.05ohm x 20���Ŵ� = 1 
	//V4:ADC_LOAD_I,  0.04ohm x 35���Ŵ� = 1.4V /A
     tmp_adc =	((ADC_ProcessBuff[ADC_LOAD_I]-ADC_OFFSET_HEX)*2500*5)/(4096*7);	  //(val x 2500 /4096 = valx500x7/4096
	// ADC_ProcessBuff[ADC_LOAD_I] =  tmp_adc;
		 if (tmp_adc < 	1 )
	 {
	   ADC_ProcessBuff[ADC_LOAD_I] = 0;		   //С��offsetֵ������0
	 }
	 else
	 {										   //��ȥoffsetֵ
	   //ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc -  ADC_cor[ADC_VBUS_I*64 + 0];
		 ADC_ProcessBuff[ADC_LOAD_I] = tmp_adc;
	 }

	//V3: ADC_VBUS_I ,	 0.5ohm x 2���Ŵ� = 1
	//V4:ADC_VBUS_I,  0.04ohm x 35���Ŵ� = 1.4A /V
     tmp_adc =(	(ADC_ProcessBuff[ADC_VBUS_I]-ADC_OFFSET_HEX)*2500*5)/(4096*7);  //val x 2500 x 1.4/4096 = valx500x7/4096

	// if (tmp_adc < ADC_cor[ADC_VBUS_I*64 + 0] )
		  if (tmp_adc <1 )
	 {
	   ADC_ProcessBuff[ADC_VBUS_I] = 0;		   //С��offsetֵ������0
	 }
	 else
	 {										   //��ȥoffsetֵ
	    //ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc -  ADC_cor[ADC_VBUS_I*64 + 0];
		 ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc;
	 }
//�������̴���
	 CurrentRangeProcess();



//ADC_DP,ԭʼ0-3.6v��1/2��ѹ���
     tmp_adc =(	(ADC_ProcessBuff[ADC_DP]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_DP] = 0;	//ȥ��С��100mV����ֵ
	  }
	  else
	  {
		   cor_index = tmp_adc/100;	//ÿ100mv����		   		   
		   if(tmp_adc + ADC_cor[ADC_DP*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_DP] = 0;
		   else
		   ADC_ProcessBuff[ADC_DP] = tmp_adc + ADC_cor[ADC_DP*64+cor_index];

	  }

//ADC_DM ,ԭʼ0-3.6v��1/2��ѹ���
      tmp_adc =((ADC_ProcessBuff[ADC_DM]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_DM] = 0;	//ȥ��С��100mV����ֵ
	  }
	  else
	  {
		   cor_index = tmp_adc/100;	//ÿ100mv����		   		   
		   if(tmp_adc + ADC_cor[ADC_DM*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_DM] = 0;
		   else
		   ADC_ProcessBuff[ADC_DM] = tmp_adc + ADC_cor[ADC_DM*64+cor_index];
	  }

//ADC_ID(M2) ,ԭʼ0-3.6v��1/2��ѹ���
      tmp_adc =((ADC_ProcessBuff[ADC_ID]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_ID] = 0;	//ȥ��С��100mV����ֵ
	  }
	  else
	  {
	  	   cor_index = tmp_adc/100;	//ÿ100mv����		   		   
		   if(tmp_adc + ADC_cor[ADC_ID*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_ID] = 0;
		   else
		   ADC_ProcessBuff[ADC_ID] = tmp_adc + ADC_cor[ADC_ID*64+cor_index];
	  }

//ADC_VBUS_AB ,ԭʼ0-7.5v��1/3��ѹ���
     tmp_adc =(	(ADC_ProcessBuff[ADC_VBUS_AB]-ADC_OFFSET_HEX)*3*2500)>>12;
	 r_cmp = R_compensation(); //���ݵ������򣬲���tester�����calbe������ɵ�ѹ��
	 if (tmp_adc < 1)
	  {
		ADC_ProcessBuff[ADC_VBUS_AB] = 0;	//ȥ��С��100mV����ֵ
	  }
	  else
	  {
	  //����
	  //	cor_index = tmp_adc/100;	//ÿ100mv����
	    ADC_ProcessBuff[ADC_VBUS_AB] =  tmp_adc + ADC_cor[ADC_VBUS_AB*64+cor_index] + r_cmp ;	
			
			//V4
			//ADC_ProcessBuff[ADC_VBUS_AB] = tmp_adc;

	  } 

	 //ADC_LOAD_V ,	ԭʼ0-7.5v��1/3��ѹ���
     tmp_adc =((	ADC_ProcessBuff[ADC_LOAD_V]-ADC_OFFSET_HEX)*3*2500)>>12;
	 
		if (tmp_adc < 1)
	  {
		ADC_ProcessBuff[ADC_LOAD_V] = 0;	//
	  }
		else
		{
			ADC_ProcessBuff[ADC_LOAD_V] = tmp_adc;
			
		}

	 //ADC_VBUS_V ,	ԭʼ0-7.5v��1/3��ѹ���
     tmp_adc =(	(ADC_ProcessBuff[ADC_VBUS_V]-ADC_OFFSET_HEX)*3*2500)>>12;
		if (tmp_adc < 1)
	  {
		ADC_ProcessBuff[ADC_VBUS_V] = 0;	//
	  }
		else
		{
			ADC_ProcessBuff[ADC_VBUS_V] = tmp_adc;
			
		}
   

}


/****************CAL FUNCTIONS******************/
uint8_t CalVbusCurrent(void)
{
   uint16_t i,d,tmp;
   uint16_t cal=0;
   d=0;

   Report_MSG("Vbus_gen 5000mV to Vbus_ab")	;
   GPIO_Setting(SW_VBUS_AB_M1,ON); //using ADC_VBUS_AB to monitor Vbus
   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
   DAC_Setting(DAC_VBUS_IL,500); //VBUS_IL ��200mA
   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
   GPIO_Setting(RSW_VBUS_CAP_6,ON); //apply 100uF to Vbus_ab,
   Delay(10000);

    tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,20000);
    sprintf(charBuf,"INFO:VBUS_AB = %d mV",tmp) ;
	Report_MSG(charBuf)	;

	Report_MSG("Adjust Vbus_ab to 5000mV")	;
	
	if(tmp<4998)
	{ 
	   d=0;
	    do
		{ 
		 d = d+2;
	     DAC_Setting(DAC_VBUS_V,(5000 + d));
		 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
		 sprintf(charBuf,"INFO:DAC_VBUS_V = %d,VBUS_AB = %d mV",5000+d,tmp) ;
	     Report_MSG(charBuf)	;
		}while(tmp<4998 && d<50)  ;
	}
	else if(tmp>5002)
	{
	    d=0;
	    do
		{ 
		 d = d+2;
	     DAC_Setting(DAC_VBUS_V,(5000 - d));
		 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
		 sprintf(charBuf,"INFO:DAC_VBUS_V = %d,VBUS_AB = %d mV",5000-d,tmp) ;
	     Report_MSG(charBuf)	;
		}while(tmp<5002 && d<50) ;
	}

    Report_MSG("Report 0mA loading current for 10 times")	;
	for (i=0;i<10;i++)
	{
	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,20000);
	  sprintf(charBuf,"INFO:I = %d mA",tmp) ;
	  Report_MSG(charBuf)	;
	  cal = tmp + cal;
	}  
	cal = cal/10; //10�� ��ֵ
    sprintf(charBuf,"INFO:Average I = %d mA",cal) ;
	Report_MSG(charBuf)	;
	 Report_MSG("Report 2.5mA loading current for 10 times")	;
	 GPIO_Setting(RSW_CAL_I_5MA,ON); //Apply vbus_gen to vbus_ab;
	  Delay(10000);
	for (i=0;i<10;i++)
	{
	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,20000);
	  sprintf(charBuf,"INFO:I = %d mA",tmp) ;
	  Report_MSG(charBuf)	;
	}  	  
	GPIO_Setting(RSW_CAL_I_5MA,OFF); 
	
	Report_MSG("Apply 50mA load ")	;
	GPIO_Setting(RSW_CAL_I_100MA,ON); //Apply vbus_gen to vbus_ab;
	 Delay(10000);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,20000);
    sprintf(charBuf,"INFO:VBUS_AB = %d mV",tmp) ;
	Report_MSG(charBuf)	;

    Report_MSG("Adjust Vbus_ab to 5000mV")	;	
	if(tmp<4998)
	{ 
	   d=0;
	    do
		{ 
		 d = d+2;
	     DAC_Setting(DAC_VBUS_V,(5000+d));
			Delay(1500);
		 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
		 sprintf(charBuf,"INFO:DAC_VBUS_V = %d,VBUS_AB = %d mV",5000+d,tmp) ;
	     Report_MSG(charBuf)	;
		}while(tmp<4998 && d<50) ;
	}
	else if(tmp>5002)
	{
	    d=0;
	    do
		{ 
		 d = d+2;
	     DAC_Setting(DAC_VBUS_V,(5000 - d));
			Delay(1500);
		 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
		 sprintf(charBuf,"INFO:DAC_VBUS_V = %d,VBUS_AB = %d mV",5000-d,tmp) ;
	     Report_MSG(charBuf)	;
		}while(tmp<5002 && d<50) ;
	 }

	Report_MSG("Report 50mA loading current for 10 times")	;
	for (i=0;i<10;i++)
	{
	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,20000);
	  sprintf(charBuf,"INFO:I = %d mA",tmp) ;
	  Report_MSG(charBuf)	;
	}  	  
	GPIO_Setting(RSW_CAL_I_100MA,OFF);	  
	 
   GPIO_Setting(RSW_VBUS_AB,OFF); //Apply vbus_gen to vbus_ab;
   DAC_Setting(DAC_VBUS_IL,0); //VBUS_IL ��200mA
   DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 5000mV

   return cal;
}

//20140905, apply 100uF to vbus before cal vbus_i
uint16_t calibrate(void *p)
{

	uint16_t i;
	uint16_t tmp;
	float s1,s2;
	int8_t tmp_cor[64];
   
//#ifdef TEST_REP_MSG
//	 testStrs(NULL);
//	 return 1;
//#endif
    
	//init tmp_cor
	for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }



	//--------------1.ADC_VBUS_AB(M1)У׼---------------------
	 //���ԭʼ����ֵ
	Clear_Cor_Data();

    GPIO_Setting(SW_VBUS_AB_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	GPIO_Setting(SW_CAL_VBUS_AB,ON); //DAC_DM gen voltage via 1k to VBUS_AB
	//measure	0-4900mV�ǲ�����ģ�5000-6300mv���������


	for	(i=0;i<50;i++)
	{
	  
	  DAC_Setting(DAC_DM,i*100); //
	  Delay(10000);
	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,5000);
	  tmp_cor[i] = i*100 - tmp;
	  sprintf(charBuf,"INFO:M1 = %d mv when DAC = %d mv,cor = %d",tmp,i*100,tmp_cor[i]) ;
	  Report_MSG(charBuf)	;

	}
	//���� 5000-6300mv
	  s1 = tmp_cor[49] - tmp_cor[29]; //���������Թ�ϵ��stepΪ4900��2900mvʱ��ƽ���仯���� ��mv/100mV�� 
	  s1= s1/20;

	  for(i=1;i<15;i++)	//����tmp_cor[50] ~ tmp_cor[63]
	  { 
	    
		s2 = s1*i	;
		tmp_cor[49+i] = tmp_cor[49]+ (int )s2		  ;
		//sprintf(charBuf,"s1 = %f |s2 = %f|tmp_cor = %d",s1, s2, tmp_cor[49+i] ) ;
	   // Report_MSG(charBuf)	;
	  }


	 Report_MSG("APPLY ADC_VBUS_AB CAL VALUE...");
	
		for	(i=0;i<64;i++)
	{
	    ADC_cor[ADC_VBUS_AB*64 + i] =  tmp_cor[i];
	}


	 /*
	//for debug
    for	(i=35;i<64;i++)
	{
      sprintf(charBuf,"INFO:%d mv |Flash read = %d |tmp_cor = %d",i*100, ADC_cor[ADC_VBUS_AB*64+i], tmp_cor[i]) ;
	  Report_MSG(charBuf)	;
	}
	*/
	DAC_Setting(DAC_DM,0);	  
    GPIO_Setting(SW_VBUS_AB_M1,OFF); //using ADC_VBUS_AB to monitor Vbus
	GPIO_Setting(SW_CAL_VBUS_AB,OFF); //DAC_DM gen voltage via 1k to VBUS_AB


	//--------------2.ADC_VBUS_I У׼---------------------
    Report_MSG("Cal ADC_VBUS_I ...");
	
	for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }


     //���½������顣

	ADC_cor[ADC_VBUS_I*64 + 0] =  CalVbusCurrent();

    //PULL_DOWN_RES_Setting(RES_NONE);
	
	 
   	//--------------2.ADC_DM/ADC_DP У׼---------------------
    Report_MSG("Cal ADC_DP ...");
	 
	 for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }

	// powerDP(0,RES_1K5)	;
	  GPIO_Setting(SW_CAL_DP,ON);	 //

	  for	(i=0;i<36;i++)
	{
	  
	  DAC_Setting(DAC_DM,i*100); //
	  Delay(10000);
	  tmp = Get_Avarage_ADC_Value(ADC_DP,5000);
	  tmp_cor[i] = i*100 - tmp;
	  sprintf(charBuf,"INFO:ADC_DP = %d mv | DAC = %d mv | cor = %d",tmp,i*100,tmp_cor[i]) ;
	  Report_MSG(charBuf)	;

	}
	  GPIO_Setting(SW_CAL_DP,OFF);	 //DAC_DM VIA 1K to ADC_DP
	  DAC_Setting(DAC_DM,0);
	 	
		for	(i=0;i<36;i++)
	{
	    ADC_cor[ADC_DP*64 + i] =  tmp_cor[i];
	}


	 //CAL DM
 	 
	 for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }

	 Report_MSG("Cal ADC_DM ...");

	   powerDM(0,RES_1K5);  
	   

	  for	(i=0;i<36;i++)
	{	  
	  DAC_Setting(DAC_DM,i*100); //
	  Delay(10000);
	  tmp = Get_Avarage_ADC_Value(ADC_DM,5000);
	  tmp_cor[i] = i*100 - tmp;
	  sprintf(charBuf,"INFO:ADC_DM = %d mv | DAC = %d mv | cor = %d",tmp,i*100,tmp_cor[i]) ;
	  Report_MSG(charBuf)	;
	}
  	for	(i=0;i<36;i++)
	{
	    ADC_cor[ADC_DM*64 + i] =  tmp_cor[i];
	} 
	    powerDM(0,RES_NONE);
		

	 //--CAL ADC_ID------------------------------------------

	  Report_MSG("Cal ADC_ID ...");
	   for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }	 
	  GPIO_Setting(SW_ID_RES_M_1,ON);	 //DAC_DM VIA 1K to ID_AB
	  GPIO_Setting(SW_M_ID,ON);


	  for	(i=0;i<36;i++)					
	{	  
	  DAC_Setting(DAC_DM,i*100); //
	  Delay(10000);
	  tmp = Get_Avarage_ADC_Value(ADC_ID,5000);
	  tmp_cor[i] = i*100 - tmp;
	  sprintf(charBuf,"INFO:ADC_ID = %d mv | DAC = %d mv | cor = %d",tmp,i*100,tmp_cor[i]) ;
	  Report_MSG(charBuf)	;
	}
  	for	(i=0;i<36;i++)
	{
	    ADC_cor[ADC_ID*64 + i] =  tmp_cor[i];
	} 
	   GPIO_Setting(SW_ID_RES_M_1,OFF);
	    DAC_Setting(DAC_DM,0); //


     Report_MSG("Measure Completed.");
	//  FLASH_If_Write_Cor_Data(ADC_cor);
     Report_MSG("Write Completed.");
	//  FLASH_If_Load_Cor_Data(ADC_cor);
	 Report_MSG("Load Completed.");
						
 return 0;
}





/*******************************************************************************
  * @��������	 vTaskAdcProcess
  * @����˵��   ��������һ��ADCת����Ȼ������һ�ε�����
  * note��
  	 ADC��ɵ�һ��ת����DMA�����ݷ���ADC_ConvertedBuff0
	 ADC�����һ��ת����DMA������д�뵽ADC_ConvertedBuff1,DMA���Զ��л�Ŀ���ַ
	 ��ʹ��ADC_ConvertedBuff����ʱ��Ҫ�ֶ��л�������buff

*******************************************************************************/
void vTaskAdcProcess(void * pvParameters)
{
    portTickType xLastWakeTime;
	const portTickType xPeriod = 1;  //1tick 0.1ms
	


	
	 xLastWakeTime = xTaskGetTickCount();
     
    
 for(;;)
 {
    //toggle pCurADC_ConvertedBuff
	/*note :��һ�ε���λ��ʱ��ADC��û����������PT��ʼ��Ϊָ��buff1,�����ȷ�ת��ָ��buff0��������һ��ADC�������ADC_ConvertedBuff1��
	        �ڶ��ε���λ��ʱ��ADC�����buff1,pt��ת��ָ��buff1,������һ��ADC���ǹ�����buff0			
	
      
	 
	// AdcPtrToggle();
	//������һ��ADCת��
	*/

	  //ĿǰADC��DMA���õ����壬CopyAdcData2PorcessBuf��ADC�����ݿ�����ProcessBuf,�������ݴ�������ProcessBuf
	  CopyAdcData2PorcessBuf();

	//   ADC_SoftwareStartConv(ADC3);
	  HAL_ADC_Start(&hadc3);
	  	 
     //������һ��ADCת����ɵ�����
      AdcDataProcess();



	 //������õ�ADC���ݣ����뻺����
	  if(GetAdcDataUploadState() == true)
	  {
	    AddDataToAdcSendBuff();
	  }

	 //����watch blocks
	 WatchBlock_Handle();
	  
	 //����ͬ����Ϣ��֪ͨ������������ʹ��ADC������
	 xSemaphoreGive(xSemphrBinAdcDataSync)	;
	 	
	   
     vTaskDelayUntil(&xLastWakeTime,xPeriod);

  }
}


