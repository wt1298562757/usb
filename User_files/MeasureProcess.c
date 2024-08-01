


/*******************************************************************************
** 文件名称: 	MeasureProcess.c
** 版    本：  	1.0
** 工作环境: 	
** 作    者: 	WEIMING
** 生成日期: 	2013-4-10
** 功    能:	基于ADC数据，进行各种测量

*******************************************************************************/

/* 包含头文件 *****************************************************************/
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "PET_CMD.h"
#include "LinkUsbProcess.h"
// #include "ucpet_bsp.h"


/* 类型声明 ------------------------------------------------------------------*/


/* 宏定义 --------------------------------------------------------------------*/


/* 变量 ----------------------------------------------------------------------*/
uint16_t ADC_Captured_Value = 0;
WatchBlock_FMT WatchBlock[4];	  //watchblock用于检测ADC

xSemaphoreHandle xSemphrBinAdcDataSync;
//使用此Semaphore来同步ADC数据，ADC数据预处理完成后，由vTaskAdcProcess发出xSemaphoreAdcDataSync,
//触发WaitAdcValueMatch	函数来使用数据进行测量
 
//extern __IO uint16_t* pCurADC_ConvertedBuff;
extern __IO uint16_t ADC_ProcessBuff[8];



/*****************主测量函数，可完成bigger,inrange,small,out of rang,timeout等功能*******************************************************************/
uint16_t WaitAdcValueMatch(uint8_t adc_name,uint8_t capture_type,uint16_t less_than,uint16_t more_than,uint16_t valid_times,uint32_t timeout)
{
	

	uint16_t ADC_Captured_num = 0;
	uint16_t ADC_Captured_big_num = 0;
	uint16_t ADC_Captured_small_num = 0;
//	uint16_t unmatchValueCount = 0;
	uint16_t curValue = 0;
	uint32_t timeOutTs = 0;
	uint16_t watchResult = resCapturedTimeOut;
	uint64_t ADC_avarage_InRange = 0;
	uint64_t ADC_avarage_Big = 0;
	uint64_t ADC_avarage_Small = 0 ;

	//全局变量初始化
	ADC_Captured_Value = 0;
//	ADC_avarage_vaule = 0;

	 
   timeOutTs = xTaskGetTickCount() + timeout;
	

  while(xTaskGetTickCount() < timeOutTs)
  { 
    if(xSemaphoreTake(xSemphrBinAdcDataSync,0)  == pdPASS) 
	{ 		
	curValue = 	ADC_ProcessBuff[adc_name];

	//记录每次捕获的数值，如果满足判断条件，这个值再被更新成平均值
	ADC_Captured_Value = curValue;
	 
		/*CaptureInRange*/
		if(capture_type == CaptureInRange)
		{
		  if((curValue <= less_than)&&(curValue >= more_than) )
			{
				 ADC_Captured_num++;
				 ADC_avarage_InRange = ADC_avarage_InRange + curValue;
				 if(ADC_Captured_num == valid_times)
				 {
					ADC_Captured_Value = 	ADC_avarage_InRange/valid_times;
					watchResult = resCapturedInRange;   //如果符合条件，在此退出此函数
					break;
				 }
			}
			else
			{
			 ADC_Captured_num=0;//如果遇到不在范围内的值，则计数器 ADC_Captured_num置0，下次从新开始
			 ADC_avarage_InRange =0;
			}	
		 }

		 /*CaptureOutRang*/
		 else 
		 {
			 if(capture_type == CaptureBig || CaptureBigOrSmall == capture_type)
			 {
		 		  if(curValue >= more_than)
				   {
						 ADC_Captured_big_num++;
						 ADC_Captured_small_num = 0	;
						 ADC_avarage_Big = ADC_avarage_Big + curValue;
						 if(ADC_Captured_big_num == valid_times)
						 {
								ADC_Captured_Value = 	ADC_avarage_Big/valid_times;
								watchResult = resCapturedBig; //如果符合条件，在此退出此函数
								break;
						 }
				   }
				   else
				   {
					 ADC_Captured_big_num=0;//如果遇到不在范围内的值，则计数器 ADC_Captured_num置0，下次从新开始
					 ADC_avarage_Big=0;
				  }
			
			}
			if(capture_type == CaptureSmall || CaptureBigOrSmall == capture_type)
			 {
				  if(curValue <= less_than )
				   {
					  ADC_Captured_small_num++;
					  ADC_Captured_big_num = 0;
					  ADC_avarage_Small = ADC_avarage_Small + curValue;	
					  if(ADC_Captured_small_num == valid_times)
						 {
							 ADC_Captured_Value = ADC_avarage_Small/valid_times;
						 	watchResult = resCapturedSmall; //如果符合条件，在此退出此函数
							 break;
						 }
				  	}
					else
					{
					   	//++ unmatchValueCount ;
						ADC_Captured_small_num = 0;
						ADC_avarage_Small =0;
				    }
				
			 }
	
				
		 }
		//等待下一个ADC转换结束后再处理，0.1ms < 实际延时 < 0.2ms。若使用RTOS,则不用此延时。
   		vTaskDelay(1);		
		//i++;
		} //semaphore end
   
  }

  return watchResult;
}


/**Get_Avarage_ADC_Value **/

uint16_t Get_Avarage_ADC_Value(uint8_t adc_name, uint16_t duration)
 {
    uint64_t avarage_vaule = 0;
	uint64_t timeOutTs = 0;	 	
	uint32_t rec_num = 0;	
   
	//设置超时倒数计数器,单位为100us  
	timeOutTs = xTaskGetTickCount() + duration;

  while(xTaskGetTickCount() < timeOutTs)
  {
     if(xSemaphoreTake(xSemphrBinAdcDataSync,0)  == pdPASS) 
	{
	 	 rec_num++;
		 avarage_vaule = avarage_vaule + ADC_ProcessBuff[adc_name];   
    }
	 vTaskDelay(1);			
  }
 
	return	avarage_vaule/rec_num;
 }



void SingleWBInit(uint8_t wb_name)
{
  WatchBlock[wb_name].ValueHigh = 0;
  WatchBlock[wb_name].ValueLow  = 0;
  WatchBlock[wb_name].ValidNum   = 0;
  WatchBlock[wb_name].CaptureDuration  = 0;
  WatchBlock[wb_name].RecordedVaule  = 0;
  WatchBlock[wb_name].InRangNum  = 0;
  WatchBlock[wb_name].BiggerNum   = 0;
  WatchBlock[wb_name].SmallerNum	 = 0;
  WatchBlock[wb_name].CaptureMode	 = 0;
  WatchBlock[wb_name].Result  = 0;
  WatchBlock[wb_name].EnableFlag  = 0;
  WatchBlock[wb_name].CaptureTime = 0;
}

/*初始化 WatchBlock 结构体*/
void WatchBlockInit(void)
{
 uint8_t i;
 for (i=0;i<WatchBlockNum;i++)
 {
	SingleWBInit(i);
 }
}

/*处理WatchBlock*/
void WatchBlock_Handle(void)
{
	uint8_t i;

	for (i=0;i<WatchBlockNum;i++)
	{
	  if(WatchBlock[i].EnableFlag == 1)
	  {

		/*CaptureInRange
		if(WatchBlock[i].CaptureMode == CaptureInRange)
		{
		  if((pCurADC_ConvertedBuff[WatchBlock[i].ADCName]<WatchBlock[i].ValueHigh)&&(pCurADC_ConvertedBuff[WatchBlock[i].ADCName]>WatchBlock[i].ValueLow) )
			{
				 WatchBlock[i].InRangNum++;
				 WatchBlock[i].RecordedVaule += pCurADC_ConvertedBuff[WatchBlock[i].ADCName];
				 if(WatchBlock[i].InRangNum == WatchBlock[i].ValidNum)
					 {
					   WatchBlock[i].RecordedVaule = WatchBlock[i].RecordedVaule / WatchBlock[i].ValidNum ;// 保留平均值
					   WatchBlock[i].Result = resCapturedInRange;
					   WatchBlock[i].CaptureTime = xTaskGetTickCount();
					   WatchBlock[i].EnableFlag = 0;
					 }
			}
			else
			{
			 WatchBlock[i].InRangNum = 0;//如果遇到不在范围内的值，则计数器 InRangNum置0，下次从新开始
			}	
	   } */

		// smaller than ValueLow
		if (WatchBlock[i].CaptureMode == CaptureSmall) // < ValueLow
		 {
			    //保留最小值
			   if(ADC_ProcessBuff[WatchBlock[i].ADCName]<WatchBlock[i].RecordedVaule)
			   {
			   	  WatchBlock[i].RecordedVaule = ADC_ProcessBuff[WatchBlock[i].ADCName];
			   }
	
			   	//检测是否小于设定值
			   if(ADC_ProcessBuff[WatchBlock[i].ADCName]<WatchBlock[i].ValueLow)
				{
					
					 WatchBlock[i].SmallerNum++;
	
					 if(WatchBlock[i].SmallerNum == WatchBlock[i].ValidNum)
						 {
					   	  WatchBlock[i].Result = resCapturedSmall;
						  WatchBlock[i].CaptureTime = xTaskGetTickCount();
					   	  WatchBlock[i].EnableFlag = 0;
						 }
			 	}
				else
				{
				 WatchBlock[i].SmallerNum = 0;  //如果遇到不在范围内的值，则计数器置0，下次从新开始
				}
		  }

		 //bigger than Value High
		 else  // > ValueHig
		 {		
				//record the new MAX value
			   if(ADC_ProcessBuff[WatchBlock[i].ADCName] > WatchBlock[i].RecordedVaule)
			   {
			   	  WatchBlock[i].RecordedVaule = ADC_ProcessBuff[WatchBlock[i].ADCName];
			   }			  
			    if(ADC_ProcessBuff[WatchBlock[i].ADCName]>WatchBlock[i].ValueHigh)
				{
				    
					 WatchBlock[i].BiggerNum++;
					
					 if(WatchBlock[i].BiggerNum == WatchBlock[i].ValidNum)
						 {
				   	  		WatchBlock[i].Result = resCapturedBig;
							WatchBlock[i].CaptureTime = xTaskGetTickCount();
				   	  		WatchBlock[i].EnableFlag = 0;
						 }
				}
				else
				{
				 WatchBlock[i].BiggerNum = 0;  //如果遇到不在范围内的值，则计数器置0，下次从新开始
				}
				
			}		
	  }	 //	if(WatchBlock[i].EnableFlag == 1)
	} //for

}


void disableWatchBlock(uint8_t watchBlockId)
{
	 WatchBlock[watchBlockId].EnableFlag = DISABLE;
}

//设置一个WB，一般在一段时间以后回来看结果，是否trigger
//只用于超限的测量，所以capMode 只有CaptureBig和CaptrueSmall
void enableWatchBlock(uint8_t watchBlockId, uint8_t adcId, uint8_t capMode, uint16_t realValue, uint16_t matchCnt )
{
   
     SingleWBInit(watchBlockId)	 ;

     WatchBlock[watchBlockId].ADCName = adcId	 ;
	 WatchBlock[watchBlockId].CaptureMode = capMode;	 

	 switch(capMode)
	 {
		 case CaptureBig:
			  WatchBlock[watchBlockId].ValueHigh = realValue;
			  WatchBlock[watchBlockId].RecordedVaule = 0;
			   break;
		 case CaptureSmall:
			  WatchBlock[watchBlockId].ValueLow = realValue;
			  WatchBlock[watchBlockId].RecordedVaule = 0xFFFF;
		 	  break;
	 }	 
	 WatchBlock[watchBlockId].ValidNum = matchCnt;  
	   //in order to keep the value at first time
	 WatchBlock[watchBlockId].EnableFlag = ENABLE; //启动WB
}

uint8_t IsWatchBlockTriggered(uint8_t wb_name)
{
 return	!WatchBlock[wb_name].EnableFlag ;
} 

uint16_t GetWatchBlockRecordVal(uint8_t wb_name)
{
  return WatchBlock[wb_name].RecordedVaule;
}

void Delay(uint64_t i)
{
 vTaskDelay(i);
}

/*比较re是否在stv-diff~stv+diff范围内，并输出结果*/
void cmpTCValue(int16_t re,int16_t stv,int16_t diff)
{
	if((re<=(stv + diff)) && (re>=(stv-diff)) )  //在范围内
	{
	   sprintf(charBuf,"INFO:PASSED.result = %d, standValue = %d, diff = %d",re,stv,re-stv) ;
	   Report_MSG(charBuf)	;
	}
	else
	{
	   sprintf(charBuf,"ERRO:Failed.result = %d, standValue = %d, diff = %d",re,stv,re-stv) ;
	   Report_MSG(charBuf)	;
	}
	
}

/*比较re是否在stv-(stv * PER)~stv+(stv * PER)范围内，并输出结果*/
void cmpTCValuePer(int16_t re,int16_t stv,int16_t per)
{
    float dev;
	dev =  100*(re-stv)/stv;
//	dev = -12.88;
	if((re<=stv *(1+ per)) && (re>=stv*(1- per)) )  //在范围内
	{
	   
	  // sprintf(charBuf,"INFO:PASSED.result = %d, standValue = %d, dev = %f",re,stv,re>=stv?(re-stv)/stv:-(stv-re)/stv) ;
	  sprintf(charBuf,"INFO:PASSED.result = %d, standValue = %d, dev = %4.2f'%'",re,stv,dev) ;
	   Report_MSG(charBuf)	;
	}
	else
	{
	    sprintf(charBuf,"ERRO:Failed.result = %d, standValue = %d, dev = %4.2f'%'",re,stv,dev) ;
	   Report_MSG(charBuf)	;
	}
	
}

void cmpFloatValue(float re,float stv,float diff)
{
	if((re<=(stv + diff)) && (re>=(stv-diff)) )  //在范围内
	{
	   sprintf(charBuf,"INFO:PASSED.result = %5.1f, standValue = %5.1f, diff = %5.1f",re,stv,re-stv) ;
	   Report_MSG(charBuf)	;
	}
	else
	{
	   sprintf(charBuf,"ERRO:Failed.result = %5.1f, standValue = %5.1f, diff = %5.1f",re,stv,re-stv) ;
	   Report_MSG(charBuf)	;
	}
	
}





