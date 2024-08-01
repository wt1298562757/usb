
/*******************************************************************************
** 文件名称: 	AdcProcess.c
** 版    本：  	1.0
** 工作环境: 	
** 作    者: 	WEIMING
** 生成日期: 	2013-2-27
** 功    能:	处理ADC数据

*******************************************************************************/

/* 包含头文件 *****************************************************************/
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


/* 类型声明 ------------------------------------------------------------------*/


/* 宏定义 --------------------------------------------------------------------*/


/* 变量 ----------------------------------------------------------------------*/
static int8_t ADC_cor[MAX_ADC_NUM* MAX_CAL_NUM] ;	   //ADC数据校正数组，每次上电后，从flash中读出校正数据到这个数组
static uint16_t OUT_MORE_THAN_200MA_NUM,OUT_LESS_THAN_200MA_NUM;   //电流量程状态量，小于200mA就硬件放大十倍
static uint16_t IN_MORE_THAN_200MA_NUM,IN_LESS_THAN_200MA_NUM;	   //大于200mA，硬件不放大，软件放大十倍



//extern __IO uint16_t* pCurADC_ConvertedBuff; 
extern __IO uint16_t ADC_ProcessBuff[8];
extern xSemaphoreHandle xSemphrBinAdcDataSync; //ADC数据同步用
extern PreDefinedPara_t PreDefinedPara;


/* 函数声明 ------------------------------------------------------------------*/
void LoadCorData(void);
void AdcDataProcess(void);
void vTaskAdcProcess(void * pvParameters);


 /*ADC_LOAD_I 自动量程处理*/
void  IN_I_Auto_Range(void)
{
      
	 //for IN Current
	 if(GetLoadCurrentRange() == LO_RANGE)
	 {
		   //当前为硬件放大10倍，如果原始值> 200mA (测量值2000mV，对应（2000/2500）×4096 = 3277),
		   //DAC输出0-3277，显示输出0-3277，实际值 = 显示输出/10
		   if(ADC_ProcessBuff[ADC_LOAD_I]> X10_AMP_TH )
			{
			   IN_MORE_THAN_200MA_NUM++;
			   if (IN_MORE_THAN_200MA_NUM >= AMP_Factor_SW_NUM)
			   {
			     SetLoadCurrentRange(HI_RANGE);	   //下一转换时，硬件放大1
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
				    SetLoadCurrentRange(LO_RANGE);	//下一转换时，硬件放大10 
					 IN_LESS_THAN_200MA_NUM = 0;
				  }
			 }
			 else
			 {
				IN_LESS_THAN_200MA_NUM = 0;
			 }
			 //将测量值x10 ，软件放大10倍
			 //DAC输出328-4095，软件放大十倍=> 3280-40960,实际值 再除以10。
			 ADC_ProcessBuff[ADC_LOAD_I] =   ADC_ProcessBuff[ADC_LOAD_I]*10;
	}

}

void  OUT_I_Auto_Range(void)
{
	//For OUT current
	if(GetVbusCurrentRange() == LO_RANGE)
	 {
	   //当前为10X，如果原始值> 200mA 
	   //实际值 = 显示输出/10
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
			    SetVbusCurrentRange(LO_RANGE);	//下一转换时，硬件放大10被 
				OUT_LESS_THAN_200MA_NUM = 0;
			  }
		 }
		 else
		 {
			OUT_LESS_THAN_200MA_NUM = 0;
		 }
		 //将测量值x10 ，软件放大10倍
		 //DAC输出328-4095，软件放大十倍=> 3280-40960,实际值 再除以10。
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
	 //将测量值x10 ，软件放大10倍
	//实际值 再除以10。
	ADC_ProcessBuff[ADC_LOAD_I] =   ADC_ProcessBuff[ADC_LOAD_I]*10;
  }

  if(out_mode == AUTO_RANGE)
  {
   OUT_I_Auto_Range();
   }
    else if(out_mode == HI_RANGE)
  {
	 //将测量值x10 ，软件放大10倍
	//实际值 再除以10。
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


/*根据电流方向，补偿tester内阻和calbe内阻造成的压降*/
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
 	 //TESTER输出电流时，实际电压比测试电压低 ,使用special cable A
	cmp_val = -( (r_cable_a + r_tester)*i_out/10000);	

  }
  else if(i_out < VALID_I_FOR_CMP && i_in > VALID_I_FOR_CMP)
  {
   //input current
   	//TESTER输入电流时，实际电压比测试电压高,使用special cable B
	 cmp_val = (r_cable_b + r_tester)*i_in/10000;
  }
  // i_out>VALID_I_FOR_CMP &&  i_in > VALID_I_FOR_CMP ----> 自环，不管
  else 
  {
     cmp_val = 0;
  }	
  return cmp_val;
}


/*******************************************************************************
  * @函数名称	 AdcDataProcess()
  * @函数说明   根据ADC名称，将ADC的HEX数值转换成应该显示dec值
  				
  * note：	    将8ch数据一起处理，包括HEX to dec转换，电流量程增益，数据矫正和内阻补偿
  				处理完成后的结果仍然放在ADC_ConvertedBuff中,供其他测量程序使用
*******************************************************************************/

void AdcDataProcess(void)
{
	int32_t tmp_adc;
	uint16_t cor_index;
	int32_t r_cmp = 0;


  	
	//V3:ADC_LOAD_I ,	 0.05ohm x 20倍放大 = 1 
	//V4:ADC_LOAD_I,  0.04ohm x 35倍放大 = 1.4V /A
     tmp_adc =	((ADC_ProcessBuff[ADC_LOAD_I]-ADC_OFFSET_HEX)*2500*5)/(4096*7);	  //(val x 2500 /4096 = valx500x7/4096
	// ADC_ProcessBuff[ADC_LOAD_I] =  tmp_adc;
		 if (tmp_adc < 	1 )
	 {
	   ADC_ProcessBuff[ADC_LOAD_I] = 0;		   //小于offset值，就置0
	 }
	 else
	 {										   //减去offset值
	   //ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc -  ADC_cor[ADC_VBUS_I*64 + 0];
		 ADC_ProcessBuff[ADC_LOAD_I] = tmp_adc;
	 }

	//V3: ADC_VBUS_I ,	 0.5ohm x 2倍放大 = 1
	//V4:ADC_VBUS_I,  0.04ohm x 35倍放大 = 1.4A /V
     tmp_adc =(	(ADC_ProcessBuff[ADC_VBUS_I]-ADC_OFFSET_HEX)*2500*5)/(4096*7);  //val x 2500 x 1.4/4096 = valx500x7/4096

	// if (tmp_adc < ADC_cor[ADC_VBUS_I*64 + 0] )
		  if (tmp_adc <1 )
	 {
	   ADC_ProcessBuff[ADC_VBUS_I] = 0;		   //小于offset值，就置0
	 }
	 else
	 {										   //减去offset值
	    //ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc -  ADC_cor[ADC_VBUS_I*64 + 0];
		 ADC_ProcessBuff[ADC_VBUS_I] = tmp_adc;
	 }
//电流量程处理
	 CurrentRangeProcess();



//ADC_DP,原始0-3.6v。1/2分压检测
     tmp_adc =(	(ADC_ProcessBuff[ADC_DP]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_DP] = 0;	//去除小于100mV的数值
	  }
	  else
	  {
		   cor_index = tmp_adc/100;	//每100mv矫正		   		   
		   if(tmp_adc + ADC_cor[ADC_DP*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_DP] = 0;
		   else
		   ADC_ProcessBuff[ADC_DP] = tmp_adc + ADC_cor[ADC_DP*64+cor_index];

	  }

//ADC_DM ,原始0-3.6v。1/2分压检测
      tmp_adc =((ADC_ProcessBuff[ADC_DM]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_DM] = 0;	//去除小于100mV的数值
	  }
	  else
	  {
		   cor_index = tmp_adc/100;	//每100mv矫正		   		   
		   if(tmp_adc + ADC_cor[ADC_DM*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_DM] = 0;
		   else
		   ADC_ProcessBuff[ADC_DM] = tmp_adc + ADC_cor[ADC_DM*64+cor_index];
	  }

//ADC_ID(M2) ,原始0-3.6v。1/2分压检测
      tmp_adc =((ADC_ProcessBuff[ADC_ID]-ADC_OFFSET_HEX)*2*2500)>>12;   //val x 2 x 2500/4096
	  if (tmp_adc < 30)
	  {
		 ADC_ProcessBuff[ADC_ID] = 0;	//去除小于100mV的数值
	  }
	  else
	  {
	  	   cor_index = tmp_adc/100;	//每100mv矫正		   		   
		   if(tmp_adc + ADC_cor[ADC_ID*64+cor_index] < 0)
		   ADC_ProcessBuff[ADC_ID] = 0;
		   else
		   ADC_ProcessBuff[ADC_ID] = tmp_adc + ADC_cor[ADC_ID*64+cor_index];
	  }

//ADC_VBUS_AB ,原始0-7.5v。1/3分压检测
     tmp_adc =(	(ADC_ProcessBuff[ADC_VBUS_AB]-ADC_OFFSET_HEX)*3*2500)>>12;
	 r_cmp = R_compensation(); //根据电流方向，补偿tester内阻和calbe内阻造成的压降
	 if (tmp_adc < 1)
	  {
		ADC_ProcessBuff[ADC_VBUS_AB] = 0;	//去除小于100mV的数值
	  }
	  else
	  {
	  //矫正
	  //	cor_index = tmp_adc/100;	//每100mv矫正
	    ADC_ProcessBuff[ADC_VBUS_AB] =  tmp_adc + ADC_cor[ADC_VBUS_AB*64+cor_index] + r_cmp ;	
			
			//V4
			//ADC_ProcessBuff[ADC_VBUS_AB] = tmp_adc;

	  } 

	 //ADC_LOAD_V ,	原始0-7.5v。1/3分压检测
     tmp_adc =((	ADC_ProcessBuff[ADC_LOAD_V]-ADC_OFFSET_HEX)*3*2500)>>12;
	 
		if (tmp_adc < 1)
	  {
		ADC_ProcessBuff[ADC_LOAD_V] = 0;	//
	  }
		else
		{
			ADC_ProcessBuff[ADC_LOAD_V] = tmp_adc;
			
		}

	 //ADC_VBUS_V ,	原始0-7.5v。1/3分压检测
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
   DAC_Setting(DAC_VBUS_IL,500); //VBUS_IL 限200mA
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
	cal = cal/10; //10次 均值
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
   DAC_Setting(DAC_VBUS_IL,0); //VBUS_IL 限200mA
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



	//--------------1.ADC_VBUS_AB(M1)校准---------------------
	 //清除原始矫正值
	Clear_Cor_Data();

    GPIO_Setting(SW_VBUS_AB_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	GPIO_Setting(SW_CAL_VBUS_AB,ON); //DAC_DM gen voltage via 1k to VBUS_AB
	//measure	0-4900mV是测出来的，5000-6300mv是算出来的


	for	(i=0;i<50;i++)
	{
	  
	  DAC_Setting(DAC_DM,i*100); //
	  Delay(10000);
	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,5000);
	  tmp_cor[i] = i*100 - tmp;
	  sprintf(charBuf,"INFO:M1 = %d mv when DAC = %d mv,cor = %d",tmp,i*100,tmp_cor[i]) ;
	  Report_MSG(charBuf)	;

	}
	//计算 5000-6300mv
	  s1 = tmp_cor[49] - tmp_cor[29]; //设满足线性关系，step为4900至2900mv时的平均变化幅度 （mv/100mV） 
	  s1= s1/20;

	  for(i=1;i<15;i++)	//计算tmp_cor[50] ~ tmp_cor[63]
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


	//--------------2.ADC_VBUS_I 校准---------------------
    Report_MSG("Cal ADC_VBUS_I ...");
	
	for	(i=0;i<64;i++)
	{
     tmp_cor[i]	= 0;
	 }


     //更新矫正数组。

	ADC_cor[ADC_VBUS_I*64 + 0] =  CalVbusCurrent();

    //PULL_DOWN_RES_Setting(RES_NONE);
	
	 
   	//--------------2.ADC_DM/ADC_DP 校准---------------------
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
  * @函数名称	 vTaskAdcProcess
  * @函数说明   先启动下一次ADC转换，然后处理上一次的数据
  * note：
  	 ADC完成第一次转换后，DMA将数据放入ADC_ConvertedBuff0
	 ADC完成下一次转换后，DMA将数据写入到ADC_ConvertedBuff1,DMA会自动切换目标地址
	 但使用ADC_ConvertedBuff数据时需要手动切换这两个buff

*******************************************************************************/
void vTaskAdcProcess(void * pvParameters)
{
    portTickType xLastWakeTime;
	const portTickType xPeriod = 1;  //1tick 0.1ms
	


	
	 xLastWakeTime = xTaskGetTickCount();
     
    
 for(;;)
 {
    //toggle pCurADC_ConvertedBuff
	/*note :第一次到此位置时：ADC还没启动过，而PT初始化为指向buff1,到此先翻转，指向buff0，启动第一次ADC，结果在ADC_ConvertedBuff1，
	        第二次到此位置时，ADC结果在buff1,pt翻转后，指向buff1,启动下一次ADC，记过放在buff0			
	
      
	 
	// AdcPtrToggle();
	//触发下一次ADC转换
	*/

	  //目前ADC的DMA采用单缓冲，CopyAdcData2PorcessBuf将ADC的数据拷贝到ProcessBuf,所有数据处理都基于ProcessBuf
	  CopyAdcData2PorcessBuf();

	//   ADC_SoftwareStartConv(ADC3);
	  HAL_ADC_Start(&hadc3);
	  	 
     //处理上一次ADC转换完成的数据
      AdcDataProcess();



	 //将处理好的ADC数据，放入缓冲区
	  if(GetAdcDataUploadState() == true)
	  {
	    AddDataToAdcSendBuff();
	  }

	 //处理watch blocks
	 WatchBlock_Handle();
	  
	 //发出同步信息，通知测量函数可以使用ADC数据了
	 xSemaphoreGive(xSemphrBinAdcDataSync)	;
	 	
	   
     vTaskDelayUntil(&xLastWakeTime,xPeriod);

  }
}


