
#include "stm32f2xx.h"
#include "LinkUsbProcess.h"
#include "TestScriptProcess.h"
#include "PET_CMD.h"
#include "FreeRTOS.h"
#include "task.h"

#include "MeasureProcess.h"
#include "UCPET_bsp.h"
#include "AdcProcess.h"


/*宏定义----------------------------------------------------------------------*/



/* 变量 ----------------------------------------------------------------------*/

extern uint8_t Enum_Done;
extern xTaskHandle vHandleTaskRunOneScript;


/***脚本开始*************************************************************************************/


uint16_t FN_0(void *p)
{
   Report_MSG("Running FN_0 ...") ;
   return TEST_PASSED;
}

uint16_t FN_1(void *p)
{
   uint16_t tmp=0;
   Report_MSG("Running FN_1 ...") ;
      tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,30000) ;
    if (tmp == resCapturedInRange)
   {
		//right
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("INFO: UUT did not aplly VDP_SRC within 0-3s of VBUS on");
	      	sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
		} 
   return TEST_PASSED;
}

uint16_t FN_2(void *p)
{   
    uint16_t tmp=0;
     Report_MSG("Running FN_2 ...") ;
     tmp	= WaitAdcValueMatch(ADC_DP,CaptureSmall,500,900,5,30000) ;
    if (tmp == resCapturedSmall)
   {
		//right
			Report_MSG("INFO: UUT captured small(<500mV)");
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("INFO: UUT did not capture small(<500mV)");
	      	sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
		} 
   return TEST_PASSED;
}

uint16_t FN_3(void *p)
{
    uint16_t tmp=0;
   Report_MSG("Running FN_3 ...") ;
        tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,500,900,5,30000) ;
    if (tmp == resCapturedBig)
   {
		//right
			Report_MSG("INFO: UUT captured small(>900mV)");
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("INFO: UUT did not capture small(>900mV)");
	      	sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
	}
   return TEST_FAILED;
}


uint16_t FN_4(void *p)
{
    uint16_t tmp=0;
   Report_MSG("Running FN_4 ...") ;
        tmp	= Get_Avarage_ADC_Value(ADC_DP,30000) ;

	   sprintf(charBuf,"INFO: D+= %d mV", tmp) ;
	    Report_MSG(charBuf)	 ;

   return TEST_PASSED;
}

uint16_t FN_5(void *p)
{
    uint16_t tmp=0;
    Report_MSG("Running FN_5 ...") ;

	WatchBlockInit();
	enableWatchBlock(0,ADC_DP,CaptureSmall,100,30000); //触发条件，ADC_DP < 100 mv 连续10次 
	enableWatchBlock(1,ADC_DP,CaptureBig,250,30000);   //触发条件，ADC_DP > 250 mv 连续10次 

    tmp	= Get_Avarage_ADC_Value(ADC_DP,30000) ;
	sprintf(charBuf,"INFO: D+= %d mV", tmp) ;
	Report_MSG(charBuf)	 ;

	if(IsWatchBlockTriggered(0))
	{
	   sprintf(charBuf,"INFO:WB_0 triggered,val= %d mV", GetWatchBlockRecordVal(0)) ;
	   Report_MSG(charBuf)	 ;
	}
	else
	{
	   sprintf(charBuf,"INFO:WB_0 did not triggered,val= %d mV", GetWatchBlockRecordVal(0)) ;
	   Report_MSG(charBuf)	 ;
	}

		if(IsWatchBlockTriggered(1))
	{
	   sprintf(charBuf,"INFO:WB_1 triggered,val= %d mV", GetWatchBlockRecordVal(1)) ;
	   Report_MSG(charBuf)	 ;
	}
	else
	{
	   sprintf(charBuf,"INFO:WB_1 did not triggered,val= %d mV", GetWatchBlockRecordVal(1)) ;
	   Report_MSG(charBuf)	 ;
	}


   return TEST_PASSED;
}

uint16_t FN_6(void *p)
{
    uint16_t val;

	WaitHostToContinue("Please check your mind! ") ;
	 
	val = askHostToInputVal("Please input a number :");
    sprintf(charBuf,"INFO:Inputed val = %d ",val) ;
	 Report_MSG(charBuf);
    return TEST_PASSED;
}


/********* SelfCheck ******/
//2012-12-21 修改电流输出格式为浮点形式	,修改了电容测试结果判断
//2012-12-24 全部测试项目增加了结果判断，增加了0.5/1.0/1.5A电流测试
//2014-05-29  current measuring step ,  Apply 10uF to VBUS_AB for current measuring(good for small current measuring accuracy)
//2014-09-04  relax 100uF check diff to 200 ,Apply 100uF to VBUS_AB for current measuring(good for small current measuring accuracy)

uint16_t SelfCheck(void *p)
{
	  int16_t m1,tmp,tc=0;
	  int32_t t1,t2;
	  uint32_t ts3=0;
	  float f1,fI_offset;

      Report_MSG("Self Testing...");
	 
	  	  

      //M_1 CHECKING 
	 Report_MSG("----------------------------------------");
	 Report_MSG("M_1 CHECKING ...");
	  DAC_Setting(DAC_VBUS_IL,500); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV	  
     
	  
// check 1
	  Report_MSG("1.Vbus_Gen 5v to Vbus_AB, route Vbus_AB to M_1");	
	  GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_AB_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	  Delay(500);
	  m1 = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
	  if (m1 <5025 || m1 > 4750)
	  {
	   sprintf(charBuf,"INFO:1 PASSED. M_1 = %d",m1) ;
	   Report_MSG(charBuf)	;

	   sprintf(charBuf,"INFO:I = %d",Get_Avarage_ADC_Value(ADC_VBUS_I,2000)/10) ;
	   Report_MSG(charBuf)	;
	  }
	  else
	  {
	   	sprintf(charBuf,"ERRO:1 Failed ,M_1 = %d",m1) ;
	    Report_MSG(charBuf)	;
	  }
	   GPIO_Setting(RSW_VBUS_AB,OFF);
	   GPIO_Setting(SW_VBUS_AB_M1,OFF);

//check 2
	  Report_MSG("2.Vbus_Gen 5v to Vbus_CHG, route Vbus_CHG to M_1");	
	  GPIO_Setting(RSW_VBUS_CHG,ON); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_CHG_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	  Delay(500);
	  m1 = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
	  if (m1 <5025 || m1 > 4750)
	  {
	   sprintf(charBuf,"INFO:2 PASSED. M_1 = %d",m1) ;
	   Report_MSG(charBuf)	;
	   sprintf(charBuf,"INFO:I = %d",Get_Avarage_ADC_Value(ADC_VBUS_I,2000)/10) ;
	   Report_MSG(charBuf)	;
	  }
	  else
	  {
	   	sprintf(charBuf,"ERRO:2 Failed ,M_1 = %d",m1) ;
	    Report_MSG(charBuf)	;
	  }
	  GPIO_Setting(RSW_VBUS_CHG,OFF); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_CHG_M1,OFF); //using ADC_VBUS_AB to monitor Vbus

//check 3
	  Report_MSG("3.Vbus_Gen 5v to Vbus_ACC, route Vbus_ACC to M_1");	
	  GPIO_Setting(RSW_VBUS_ACC,ON); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_ACC_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	  Delay(500);
	  m1 = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
	  if (m1 <5025 || m1 > 4750)
	  {
	   sprintf(charBuf,"INFO:3 PASSED. M_1 = %d",m1) ;
	   Report_MSG(charBuf)	;
	   sprintf(charBuf,"INFO:I = %d",Get_Avarage_ADC_Value(ADC_VBUS_I,2000)/10) ;
	   Report_MSG(charBuf)	;
	  }
	  else
	  {
	   	sprintf(charBuf,"ERRO:3 Failed ,M_1 = %d",m1) ;
	    Report_MSG(charBuf)	;
	  }
	  GPIO_Setting(RSW_VBUS_ACC,OFF); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_ACC_M1,OFF); //using ADC_VBUS_AB to monitor Vbus



//check 4
	  Report_MSG("4.Vbus_Gen 5v to GND_AB via ADP resistor(2K2), route GND_AB to M_1");	
	  GPIO_Setting(RSW_ADP_RES_2,ON); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(SW_VBUS_AB_M1,ON); //using ADC_VBUS_AB to monitor Vbus
	  Delay(500);
	  m1 = Get_Avarage_ADC_Value(ADC_VBUS_AB,2000);
	  if (m1 <5025 || m1 > 4750)
	  {
	   sprintf(charBuf,"INFO:4 PASSED. M_1 = %d",m1) ;
	   Report_MSG(charBuf)	;
	   sprintf(charBuf,"INFO:I = %d",Get_Avarage_ADC_Value(ADC_VBUS_I,2000)/10) ;
	   Report_MSG(charBuf)	;
	  }
	  else
	  {
	   	sprintf(charBuf,"ERRO:4 Failed ,M_1 = %d",m1) ;
	    Report_MSG(charBuf)	;
	  }
	  GPIO_Setting(RSW_ADP_RES_2,OFF); //Apply vbus_gen to vbus_ab   
	//  GPIO_Setting(SW_VBUS_AB_M1,OFF); //using ADC_VBUS_AB to monitor Vbus

	  //DAC_Setting(DAC_VBUS_IL,0); //VBUS_IL 限流1500mA
      //DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 5000mV

// check 5
/*	  Report_MSG("----------------------------------------");
	  Report_MSG("VBUS CAP CHECKING... ");

	  DAC_Setting(DAC_VBUS_IL,500); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
		 Delay(1500);  //wait vbus settled
	  GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab 

	 Report_MSG("Vbus_gen 10uF fixed cap: ");
	  GPIO_Setting(RSW_CAL_I_5MA,ON); //Apply 2K res to VBUS_AB AND GND_AB
	  
	  Delay(10000);	   //充电1 sec
		
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
		cmpTCValue(tc,8,8*0.2);	// Vbus_gen 10uF fixed cap
		
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }
	    Delay(10000);	

//check 6.	SW_VBUS_CAP_1
	  Report_MSG("CAP1(0.15uf,C12=0.15uF + C13=NC):");
	  GPIO_Setting(RSW_CAL_I_5MA,ON); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	  GPIO_Setting(RSW_VBUS_CAP_1,ON); //
	  Delay(10000);	   //充电1 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	      t2 = GlobleTime;
		tc = t2-t1;
		 cmpTCValue(tc,8,8*0.2);  //	0.15uf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }
	    Delay(10000);		  
//-------------
        
	  //1.	SW_VBUS_CAP_2
	   Report_MSG("CAP2(900nF,C6=0.47uF + C5=0.47uF):");
	   GPIO_Setting(RSW_VBUS_CAP_1,OFF); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000);
	  GPIO_Setting(RSW_VBUS_CAP_2,ON); //Apply 100uF 
	  Delay(10000);	   //充电1 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
	     cmpTCValue(tc,107,107*0.2);  //	900nf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }
		Delay(10000);
//-------------
	  //1.	SW_VBUS_CAP_3
	   Report_MSG("CAP3(C13=1uF):");
	   GPIO_Setting(RSW_VBUS_CAP_2,OFF); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000);
	  GPIO_Setting(RSW_VBUS_CAP_3,ON); //Apply 100uF 
	  Delay(10000);	   //充电2 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
	    cmpTCValue(tc,118,118*0.2);  //	1uf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }//-------------

	   Delay(10000);

	  //1.	SW_VBUS_CAP_4
	   Report_MSG("CAP4(5.5uF,C4=0.47uF + C10=4.7uF):");
	   GPIO_Setting(RSW_VBUS_CAP_3,OFF); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000);
	  GPIO_Setting(RSW_VBUS_CAP_4,ON); //Apply 100uF 
	  Delay(10000);	   //充电2 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
	     cmpTCValue(tc,165,165*0.2);  //	5.5uf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }//-------------
	   Delay(10000);

	  //1.	SW_VBUS_CAP_5
	   Report_MSG("CAP5(10uF,C3=10uF + C9 =NC): ");
	   GPIO_Setting(RSW_VBUS_CAP_4,OFF); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000);
	  GPIO_Setting(RSW_VBUS_CAP_5,ON); //Apply 10uF 
	  Delay(10000);	   //充电2 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
 		cmpTCValue(tc,172,172*0.2);  //	1uf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }
	   Delay(10000);

	   	  //1.	SW_VBUS_CAP_6
	   Report_MSG("CAP6(C2=100uF):");
	   GPIO_Setting(RSW_VBUS_CAP_5,OFF); //Apply 1K res to VBUS_AB AND GND_AB
	   DAC_Setting(DAC_VBUS_V,5000);
	  GPIO_Setting(RSW_VBUS_CAP_6,ON); //Apply 100uF 
	  Delay(20000);	   //充电2 sec
	  DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 0mV	 ,discharging
	  t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X VBUS = 1850 时， T = RC
	  	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,1875,1825,1,20000) ; 
	   if(tmp == resCapturedInRange)
	   {
	    t2 = GlobleTime;
		tc = t2-t1;
 			cmpTCValue(tc,1135,1135*0.2);  //	100uf
	   }
	   else
	   {
		 Report_MSG("ERRO: NOT CAPTURED 1850mV");
	   }	  //1.	SW_VBUS_CAP_2
       Delay(20000);	  
	   GPIO_Setting(RSW_VBUS_CAP_6,OFF); //Apply 100uF 


	  DAC_Setting(DAC_VBUS_IL,0); //关闭VBUS_GEN
      DAC_Setting(DAC_VBUS_V,0); //
	   GPIO_Setting(RSW_CAL_I_5MA,OFF);//关闭2K校准电阻
*/
//-------------------------------------------------------------
//ID_RES CHECK
	Report_MSG("----------------------------------------");
	 Report_MSG("ID_RES CHECKING...");
	 GPIO_Setting(SW_M_ID,ON);	 //使用ADC_ID来检测ID_AB的电压
	 GPIO_Setting(SW_ID_RES_M_1,ON);	 //DAC_DM 通过1K 加载到ID_AB   
	 DAC_Setting(DAC_DM,1000);			//

	 //CHECKING ID_AB不接电阻是否有1V电压
	 	tmp = WaitAdcValueMatch(ADC_ID,CaptureInRange,1050,950,10,10000) ; 
	   if(tmp == resCapturedInRange)
	   {
        sprintf(charBuf,"INFO:ID_AB = %d mv at OPEN",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    Report_MSG(charBuf)	;
		 //cap = (t2 - t1)/1000 ;//C=T/R (F) 
	   }
	   else
	   {
		 Report_MSG("ERRO: 1V on ID_AB not detected!,check R90(1K),R118(1K)");
	   }	  //1.	SW_VBUS_CAP_2
       Delay(20000);	  

	    GPIO_Setting(SW_ID_RES_1,ON);	 //DAC_DM 通过1K 加载到ID_AB   DM/DP接反了，所以实际上用DP
	    Report_MSG("SW_ID_RES_1(1k)...@R77");
	    Delay(2000);	
		//sprintf(charBuf,"INFO:ID_AB = %d mv (500) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),500,500*0.1);

		GPIO_Setting(SW_ID_RES_M_1,OFF);	 //DAC_DM 通过1K 加载到ID_AB   
		GPIO_Setting(SW_ID_RES_M_2,ON);	 //DAC_DM 通过82.5K 加载到ID_AB

		GPIO_Setting(SW_ID_RES_1,OFF);
		GPIO_Setting(SW_ID_RES_2,ON);	
	    Report_MSG("SW_ID_RES_2(36k)...@R74");
	    Delay(2000);	
		//sprintf(charBuf,"INFO:ID_AB = %d mv (298) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),298,298*0.1);

		GPIO_Setting(SW_ID_RES_2,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_3,ON);	 //DAC_DM 通过1K 加载到ID_AB

	    Report_MSG("SW_ID_RES_3(37k)...@R75");
	    Delay(2000);	
		//sprintf(charBuf,"INFO:ID_AB = %d mv (321) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),321,321*0.1);

		GPIO_Setting(SW_ID_RES_3,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_4,ON);	 //DAC_DM 通过1K 加载到ID_AB
	    Report_MSG("SW_ID_RES_4(67k)...@R72");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (441) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),441,441*0.1);

		GPIO_Setting(SW_ID_RES_4,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_5,ON);	 //DAC_DM 通过1K 加载到
	    Report_MSG("SW_ID_RES_5(69k)...@R73");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (466) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),466,466*0.1);

		GPIO_Setting(SW_ID_RES_5,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_6,ON);	 //DAC_DM 通过1K 加载到ID_AB
	    Report_MSG("SW_ID_RES_6(122k)...@R70");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (591) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),591,591*0.1);
		
		GPIO_Setting(SW_ID_RES_6,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_7,ON);	 //DAC_DM 通过1K 加载到
	    Report_MSG("SW_ID_RES_7(126k)...@R71");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (615) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),615,20);

		GPIO_Setting(SW_ID_RES_7,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_RES_8,ON);	 //DAC_DM 通过1K 加载到
	    Report_MSG("SW_ID_RES_8(220k)...@R76");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (727) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),727,727*0.1);

	 	GPIO_Setting(SW_ID_RES_8,OFF);	 //DAC_DM 通过1K 加载到ID_AB
		GPIO_Setting(SW_ID_GND,ON);	 //DAC_DM 通过1K 加载到ID_AB
	    Report_MSG("SW_ID_GND(0R)...@R69");
	    Delay(2000);
		//sprintf(charBuf,"INFO:ID_AB = %d mv (0) ",Get_Avarage_ADC_Value(ADC_ID,2000)) ;
	    //Report_MSG(charBuf)	;
		cmpTCValue(Get_Avarage_ADC_Value(ADC_ID,2000),30,80);

		GPIO_Setting(SW_ID_GND,OFF);	 //DAC_DP 通过1K 加载到ID_AB
		DAC_Setting(DAC_DM,0);

 Report_MSG("----------------------------------------");
 Report_MSG("DCD_RES CHECKING...");
 		 DAC_Setting(DAC_DM,1000);

 		 GPIO_Setting(SW_DCD_RES_1,ON);
		 Report_MSG("DCD_RES_1(R131=1K5 to DM)...");
		 Delay(2000);
		// sprintf(charBuf,"INFO:DP = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DP,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DM,2000),1000,50);

		 GPIO_Setting(SW_DCD_RES_1,OFF);
		 GPIO_Setting(SW_DCD_RES_2,ON);
		 Report_MSG("DCD_RES_2(R128=200R to DM)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DP = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DP,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DM,2000),1000,50);


         GPIO_Setting(SW_DCD_RES_2,OFF);
		 GPIO_Setting(SW_DCD_RES_3,ON);
		 Report_MSG("DCD_RES_3(R142=3K9 to DM)...");
		 Delay(2000);
		// sprintf(charBuf,"INFO:DP = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DP,2000)) ;
	    // Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DM,2000),1000,50);

         GPIO_Setting(SW_DCD_RES_3,OFF);
		 GPIO_Setting(SW_DCD_RES_4,ON);
		 Report_MSG("DCD_RES_4(R136=15K to DM)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DP = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DP,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DM,2000),1000,50);


         GPIO_Setting(SW_DCD_RES_4,OFF);
		 GPIO_Setting(SW_DCD_RES_5,ON);
		 Report_MSG("DCD_RES_5(R139=100K to DM)...");
		 Delay(2000);
		// sprintf(charBuf,"INFO:DP = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DP,2000)) ;
	    // Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DM,2000),1000,50);

         DAC_Setting(DAC_DP,0);
		 GPIO_Setting(SW_DCD_RES_5,OFF);
		////////////////////////////////////////////////
		 DAC_Setting(DAC_DP,1000);
		 Delay(40000);
         GPIO_Setting(SW_DCD_RES_6,ON);
		 Report_MSG("DCD_RES_6(R116=NC to DP)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DM = %d mv (30) ",Get_Avarage_ADC_Value(ADC_DM,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DP,2000),30,200);
		 //cmpTCValuePer(Get_Avarage_ADC_Value(ADC_DM,2000),30,0.5) ;

         GPIO_Setting(SW_DCD_RES_6,OFF);
		 GPIO_Setting(SW_DCD_RES_7,ON);
		 Report_MSG("DCD_RES_7(R99=200R to DP)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DM = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DM,2000)) ;
	    // Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DP,2000),1000,50);

         GPIO_Setting(SW_DCD_RES_7,OFF);
		 GPIO_Setting(SW_DCD_RES_8,ON);
		 Report_MSG("DCD_RES_8(R105=1K5 to DP)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DM = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DM,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DP,2000),1000,50);

         GPIO_Setting(SW_DCD_RES_8,OFF);
		 GPIO_Setting(SW_DCD_RES_9,ON);
		 Report_MSG("DCD_RES_9(R96=15K to DP)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DM = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DM,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DP,2000),1000,50);

		 GPIO_Setting(SW_DCD_RES_9,OFF);
		 GPIO_Setting(SW_DCD_RES_10,ON);
		 Report_MSG("DCD_RES_10(R97=100K to DP)...");
		 Delay(2000);
		 //sprintf(charBuf,"INFO:DM = %d mv (997) ",Get_Avarage_ADC_Value(ADC_DM,2000)) ;
	     //Report_MSG(charBuf)	;
		 cmpTCValue(Get_Avarage_ADC_Value(ADC_DP,2000),1000,50);

		 DAC_Setting(DAC_DP,0);
		 GPIO_Setting(SW_DCD_RES_10,OFF);

 Report_MSG("----------------------------------------");
  Report_MSG("Apply 100uF to VBUS_AB for current measuring...");

 Report_MSG("RSW_ADP_RES_1 (R30+R31 = 200R) CHECKING...");

 	  DAC_Setting(DAC_VBUS_IL,500); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV

	  GPIO_Setting(RSW_VBUS_CAP_6,ON); //apply 10uF to Vbus_ab,	  

	  Report_MSG("x.Vbus_Gen 5v apply to 200R, checking if current increase 25mA");	
	  GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab   
	  GPIO_Setting(RSW_ADP_RES_1,ON); //apply ADP 200R res
	 // Report_MSG("Set CURRENT_OUT as MODE_X10,using low range high revelution");
	 // Set_I_AMP_MODE(CURRENT_OUT,MODE_X10);
	  Delay(500);
	 // m1 = Get_Avarage_ADC_Value(ADC_VBUS_I,2000)/10;
	  f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,2000))/10 ;
	  if (f1 <30.0 && f1 > 20.0)
	  {
	   sprintf(charBuf,"INFO:x PASSED. I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;

	  }
	  else
	  {
	   	sprintf(charBuf,"ERRO:x Failed. I = %5.1f mA",f1) ;
	    Report_MSG(charBuf)	;
	  }
	  GPIO_Setting(RSW_ADP_RES_1,OFF); 
//	   
       Report_MSG("----------------------------------------");
	   Report_MSG("SW_VBUS_LKG (R80=2K)and SW_VBUS_PD (R81=10K) CHECKING...");
	   Report_MSG("x.Vbus_Gen 5v apply to 2K, checking if current increase 2.5mA");


	   fI_offset = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,10000))/10 ;
	  /*
	   if(fI_offset==0.0)
	   {
		 sprintf(charBuf,"ERRO:with empty loading, I = %5.1f mA,current offset should more than Zero",fI_offset) ;
	     Report_MSG(charBuf)	;
	   }
	   */
	   sprintf(charBuf,"INFO:with empty loading, I = %5.1f mA",fI_offset) ;
	   Report_MSG(charBuf)	;

	   GPIO_Setting(SW_VBUS_LKG,ON); 
	   Delay(5000);

	    f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,10000))/10 ;
	   sprintf(charBuf,"INFO:with 2K loading, I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+2.5,0.3);

	   GPIO_Setting(SW_VBUS_LKG,OFF); 
	   GPIO_Setting(SW_VBUS_PD,ON);
	   Delay(5000);

	    f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,10000))/10 ;
	   sprintf(charBuf,"INFO:with 10K loading, I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+0.5,0.3);

       GPIO_Setting(SW_VBUS_PD,OFF);
	    GPIO_Setting(RSW_CAL_I_5MA,ON);
	   Delay(5000);

	   	 f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,2000))/10 ;
	   sprintf(charBuf,"INFO:with RSW_CAL_I_5MA ON(R24=2K), I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+2.5,0.3);

	   GPIO_Setting(RSW_CAL_I_5MA,OFF);
	   GPIO_Setting(RSW_CAL_I_100MA,ON);
	   Delay(5000);

	    f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,2000))/10 ;
	   sprintf(charBuf,"INFO:with RSW_CAL_I_100MA ON(R23=100R), I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+50.0,5.0);

	   GPIO_Setting(RSW_CAL_I_100MA,OFF);

       Report_MSG("----------------------------------------");
	   Report_MSG("RSW_ACC_LD_1 (R32+R33=620R)and RSW_VBUS_ACC_LD2 (R34|R35|R26=10R) CHECKING...");
	   Report_MSG("x.Vbus_Gen 5v apply to 620R, checking if current increase 8mA");

	   GPIO_Setting(RSW_VBUS_AB,OFF); //
	   GPIO_Setting(RSW_VBUS_ACC,ON);
	   GPIO_Setting(RSW_VBUS_ACC_LD1,ON);
	    Delay(5000);

	    f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,2000))/10 ;;
	   sprintf(charBuf,"INFO:with 620R(R32=150R,R33=470R), I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+8.0,1);

		GPIO_Setting(RSW_VBUS_ACC_LD1,OFF);

		Report_MSG("x.Vbus_Gen 5v apply to 10R(R34-36=30R), checking if current increase 500mA");
		Report_MSG("NOTE: Will apply anther 10uF to Vbus,total = 20uF");
		//Report_MSG("Set CURRENT_OUT as MODE_X1,using HIGH range");
	    //Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
		 GPIO_Setting(RSW_VBUS_CAP_5,ON); //Apply 10uF 
	   Delay(10000);	   //充电2 sec

	    GPIO_Setting(RSW_VBUS_ACC_LD2,ON);
	    Delay(5000);

		//sprintf(charBuf,"INFO:with 10R, I = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,2000)) ;
	   //Report_MSG(charBuf)	;

	     f1 = (float)(Get_Avarage_ADC_Value(ADC_VBUS_I,9000))/10 ;
	   sprintf(charBuf,"INFO:with 10R, I = %5.1f mA",f1) ;
	   Report_MSG(charBuf)	;
	   cmpFloatValue(f1,fI_offset+500.0,50);

		GPIO_Setting(RSW_VBUS_ACC_LD2,OFF);
		GPIO_Setting(RSW_VBUS_ACC,OFF);
		DAC_Setting(DAC_VBUS_IL,0); //VBUS_IL 限流1500mA
        DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 5000mV
		 GPIO_Setting(RSW_VBUS_CAP_5,OFF); //Apply 10uF	  

 Report_MSG("----------------------------------------");
	   Report_MSG("SW_DCD_RES_AB CHECKING...");
	   Report_MSG("DAC_DM generate 1.5v to DM,enable the SW and check DM_AB votage");
	   DAC_Setting(DAC_DM,1500);
	   GPIO_Setting(SW_DCD_RES_1,ON);  //1k5 to DM
	   GPIO_Setting(SW_DCD_RES_AB,ON);
	   Delay(500);
	    m1 = Get_Avarage_ADC_Value(ADC_DM,2000);

	   if(m1<1550 && m1>1450)
	   {
	   sprintf(charBuf,"INFO:PASSED. ADC_DM = %d mV",m1);
	   Report_MSG(charBuf)	;
	   }
	   else
	   {
	   sprintf(charBuf,"ERRO:Failed. ADC_DM = %d mV",m1);
	   Report_MSG(charBuf)	;
	   }
	   DAC_Setting(DAC_DM,0);
	   GPIO_Setting(SW_DCD_RES_1,OFF);  //1k5 to DM
	   GPIO_Setting(SW_DCD_RES_AB,ON);

 
 Report_MSG("----------------------------------------");
 Report_MSG("Current internal loop CHECKING...");
  Report_MSG("Vbus_Gen 5v apply to EL , current set to 500mA");
  Report_MSG("Output cap set to 100uF + 10uF");

	  GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab  
	  GPIO_Setting(RSW_VBUS_CAP_6,ON); //Apply 100uF 
   	  DAC_Setting(DAC_VBUS_IL,1599); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	  
	   Report_MSG("Maintain 500mA loading for 5 seconds...");

	 // DAC_Setting(DAC_LOAD_IV,1800); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_LOAD_I,500); //VBUS_V = 5000mV
	  GPIO_Setting(RSW_EL_TO_VBUS_AB,ON); //Apply EL	  	 	

	  Delay(500);

    ts3 =GlobleTime;
    do{
    sprintf(charBuf,"INFO: Avarage Vbus_I = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;
	sprintf(charBuf,"INFO: Avarage Load_I = %d mA",Get_Avarage_ADC_Value(ADC_LOAD_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;

	  }while(50000>(GlobleTime - ts3))	;

	 	   
		   
	Report_MSG("Maintain 1000mA loading for 5 seconds...");

	  //DAC_Setting(DAC_LOAD_IV,1800); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_LOAD_I,1000); //VBUS_V = 5000mV
	  	 	
	  Delay(500);
    ts3 =GlobleTime;
    do{
    sprintf(charBuf,"INFO: Avarage Vbus_I = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;
	sprintf(charBuf,"INFO: Avarage Load_I = %d mA",Get_Avarage_ADC_Value(ADC_LOAD_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;

	  }while(50000>(GlobleTime - ts3))	;

	Report_MSG("Maintain 1500mA loading for 5 seconds...");

	//  DAC_Setting(DAC_LOAD_IV,1800); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_LOAD_I,1500); //VBUS_V = 5000mV
	  	 	
	  Delay(500);

    ts3 =GlobleTime;
    do{
    sprintf(charBuf,"INFO: Avarage Vbus_I = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;
	sprintf(charBuf,"INFO: Avarage Load_I = %d mA",Get_Avarage_ADC_Value(ADC_LOAD_I,5000)/10) ;	
	Report_MSG(charBuf)	 ;

	  }while(50000>(GlobleTime - ts3))	;

	

   	  DAC_Setting(DAC_VBUS_IL,0); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_VBUS_V,0); //VBUS_V = 5000mV
	  GPIO_Setting(RSW_VBUS_AB,OFF); //Apply vbus_gen to vbus_ab  
	  GPIO_Setting(RSW_VBUS_CAP_6,OFF); //Apply 10uF 

	  DAC_Setting(DAC_LOAD_I,0); //VBUS_IL 限流1500mA
      DAC_Setting(DAC_LOAD_V,0); //VBUS_V = 5000mV
	  GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF); //Apply EL



 Report_MSG("----------------------------------------");
  Report_MSG("Buzzer CHECKING...");
	    GPIO_Setting(SW_Buzzer,ON);
		Delay(20000) ;
		GPIO_Setting(SW_Buzzer,OFF);
		Delay(10000) ;
		GPIO_Setting(SW_Buzzer,ON);
		Delay(1000) ;
		GPIO_Setting(SW_Buzzer,OFF);
		Delay(1000) ;
		GPIO_Setting(SW_Buzzer,ON);
		Delay(1000) ;
		GPIO_Setting(SW_Buzzer,OFF);



 Report_MSG("CHECKING COMPLETED");

 return 0;
}
/*********END SelfCheck ******/




/***************6.3 B-UUT Initial Power-up Test **************************/
//2012-7-2 updated	,capableof ADP 部分未完成

uint16_t PD_UUTInitialPowerUpTest(void *p)
{
   	DECLARE_VARS()
	uint16_t capableOfADP;
	__IO uint32_t ts0,ts1;
	prepareTest();
	do{

	  // 1. The user enters into the PET:whether the UUT is capable of ADP whether the UUT is capable of ADP
	   Report_MSG(" 1. The user enters into the PET:whether the UUT is capable of ADP whether the UUT is capable of ADP");

	capableOfADP =	askHostToInputVal("Whether the UUT is capable of ADP?\r\nInput: 1 = YES,0 = NO");
	
		if(capableOfADP == 1)
		{
		
		//	Part 2 – For PD UUT capable of ADP
		 Report_MSG("-----------For PD UUT capable of ADP------------");
		//2. Operator: Ensure UUT connected using special cable A or, where the device does not have a micro-AB or micro-B receptacle, a suitable alternative.
		 Report_MSG("2. Operator: Ensure UUT connected using special cable A or, where the device does not have a micro-AB or micro-B receptacle, a suitable alternative.");
		 askHostToInputVal("Ensure UUT connected using special cable A or, where the device does not have a micro-AB or micro-B receptacle, a suitable alternative.");
		
		//3. UUT is either powered or is not powered. PET is not applying VBUS, and not applying capacitance between VBUS and ground, ID pin is not connected to ground.
		   
		 Report_MSG("3. UUT is either powered or is not powered. PET is not applying VBUS,");
		 Report_MSG("  and not applying capacitance between VBUS and ground, ID pin is not connected to ground.");
		 ID_RES_Setting(RES_Rid_OPEN);

		//4. Operator: Turn UUT off, if not already off.
		 Report_MSG("4. Operator: Turn UUT off, if not already off.");
		 askHostToInputVal("Turn UUT off, if not already off.");

		//5. Operator: Turn UUT on.
		 Report_MSG("5. Operator: Turn UUT on.");
		 askHostToInputVal("Turn UUT on.");

		//6. Check that UUT performs an ADP probe within TPWRUP_RDY (30 sec or as specified by vendor).
		 Report_MSG("6. Check that UUT performs an ADP probe within TPWRUP_RDY (30 sec or as specified by vendor).");
		//7. To check probe, check that VBUS goes below 0.3V and then rises above 0.5V within 10ms.
		 Report_MSG("7. To check probe, check that VBUS goes below 0.3V and then rises above 0.5V within 10ms.");
		 
		 ts0 = GlobleTime; //TPWRUP_RDY start point
		 tmp	= WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,300,5500,5,300000) ;		 //using M1 monitor VBUS
		   if(resCapturedSmall == tmp)
		   {
		  	 //right
			    ts1 = GlobleTime;
		   		sprintf(charBuf,"INFO: VBUS went below 0.3V, VBUS = %d mV ", ADC_Captured_Value) ;
				Report_MSG(charBuf)	 ;
				//check VBUS rises above 0.5v wihtin 10ms

			    tmp	= WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,300,500,5,100) ; 
				if(tmp == resCapturedBig)
				{
				  sprintf(charBuf,"INFO: VBUS rise above 0.5V within 10ms, VBUS = %d mV ", ADC_Captured_Value) ;
				  Report_MSG(charBuf)	 ;
				}
				else
				{
				  sprintf(charBuf,"INFO: VBUS did not rise above 0.5V within 10ms, VBUS = %d mV ", ADC_Captured_Value) ;
				  Report_MSG(charBuf)	 ;

				  testResult = TEST_FAILED;
			  	  Report_MSG("Stop this test...");
                  break;

				}

		   }
		   else{
				//failed
				sprintf(charBuf,"ERRO: VBUS kept being above 0.5v for 30 seconds,VBUS = %d mV ", ADC_Captured_Value) ;
				Report_MSG(charBuf)	 ;

				testResult = TEST_FAILED;
			  	Report_MSG("Stop this test...");
                break;
		   }
		
		//8. After the first probe check that either a further probe (or probes) is performed, or that D+ goes high. Wait here till D+ goes high or TPWRUP_RDY times out.
		 Report_MSG("8. After the first probe check that either a further probe (or probes) is performed, or that D+ goes high. Wait here till D+ goes high or TPWRUP_RDY times out.");
		//9. Check that D+ stays high for at least 5ms.
		 Report_MSG("9. Check that D+ stays high for at least 5ms.");
		//10. Check that D+ goes low within 10ms of start of pulse.
		 Report_MSG("10. Check that D+ goes low within 10ms of start of pulse.");
		//11. Check ADP probe is not performed for TB_SRP_FAIL min (5 sec) after start of D+ (SRP) pulse.
		 Report_MSG("11. Check ADP probe is not performed for TB_SRP_FAIL min (5 sec) after start of D+ (SRP) pulse.");
		//12. Check that ADP probe is performed within TB_SRP_FAIL max plus TB_ADP_PRB max (6.0 + 2.6 = 8.6 sec) after start of D+ (SRP) pulse.
		 Report_MSG("12. Check that ADP probe is performed within TB_SRP_FAIL max plus TB_ADP_PRB max (6.0 + 2.6 = 8.6 sec)");
		//13. Now ready for any other B_UUT test.
		 Report_MSG("13. Now ready for any other B_UUT test.");


		 }
		
		  else
		 {
		//Part 2 – For PD UUT not capable of ADP
		 Report_MSG("-----------For PD UUT not capable of ADP------------");
		//2. Operator: Ensure UUT connected using special cable A or, where the device does not have a micro-AB or micro-B receptacle, a suitable alternative
		Report_MSG("2. Operator: Ensure UUT connected using special cable A or, where the device does not ");
		Report_MSG("  have a micro-AB or micro-B receptacle, a suitable alternative.");
		 askHostToInputVal("Ensure UUT connected using special cable A or, where the device does not have a micro-AB or micro-B receptacle, a suitable alternative.");

		//3. UUT is either powered or is not powered. PET is not applying VBUS, and not applying capacitance between VBUS and ground, ID pin is not connected to ground.
		Report_MSG("3. UUT is either powered or is not powered. PET is not applying VBUS,");
		Report_MSG(" and not applying capacitance between VBUS and ground, ID pin is not connected to ground.");

		//4. Operator: Turn UUT on, if not already on.
		Report_MSG("4. Operator: Turn UUT on, if not already on.");
		askHostToInputVal("Turn UUT on, if not already on.");

		//5. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min (10kΩ) to VBUS and turns on VBUS.
		Report_MSG("5. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS.");
		//prepareVBusForPD();
		 VBUS_CAP_Setting(CAP_6uF5);
  	     PULL_DOWN_RES_Setting(RES_10K);
         GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
	     DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	     DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	

		//6. Check that D+ goes high within TPWRUP_RDY (30 sec).
		Report_MSG("6. Check that D+ goes high within TPWRUP_RDY (30 sec).");

		tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,5500,800,10,300000) ;		 //
		   if(resCapturedBig == tmp)
		   {
		  	 //right
			    ts1 = GlobleTime;
		   		sprintf(charBuf,"INFO:D+ went high within TPWRUP_RDY (30 sec), D+ = %d mV ", ADC_Captured_Value) ;
				Report_MSG(charBuf)	 ;
				//check VBUS rises above 0.5v wihtin 10ms
		   }
		   else{
				//failed
				sprintf(charBuf,"ERRO:D+ did not go high within TPWRUP_RDY (30 sec), D+ = %d mV ", ADC_Captured_Value) ;
				Report_MSG(charBuf)	 ;

				testResult = TEST_FAILED;
		   }

		//7. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
		Report_MSG("7. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");

		 DAC_Setting(DAC_VBUS_V,0); 
	     DAC_Setting(DAC_VBUS_IL,0); 
		 VBUS_CAP_Setting(CAP_NONE);
  	     PULL_DOWN_RES_Setting(RES_NONE);
         GPIO_Setting(RSW_VBUS_AB,OFF); 

		//8. Wait 5 seconds to allow disconnection to be detected.
		Report_MSG("8. Wait 5 seconds to allow disconnection to be detected.");
		Delay(50000);

		//9. Now ready for any other B_UUT test.
		Report_MSG("9. Now ready for any other B_UUT test.");

		 }
 }while(FALSE);

TASK_CLEAN_UP()

}
/***************END OF 6.3 B-UUT Initial Power-up Test *******************/






/*********************6.4 Data Contact Detect Test – With Current Source**************************/

//weiming updated on 26 oct
//2012-6-7 updated ,上报porcedure
//2012-12-05 updated , step3-4 if failed still conntinue 
//2012-12-30 updated, step2, if failed still conntinue
//2014-07-31 updated, disconnect 15K pull down Res from D+ to GND before run the second part of this
 //           test (DCD using timeout) in step 13


uint16_t PD_DataContactDetectWithCurrentSource(void *p)				   //Data Contact Detect Test – With Current Source
{

   DECLARE_VARS()
  uint64_t curTm = GlobleTime ;
//  char* testMsg = NULL;
//	  UINT tsOnVbusReach08 = 0;
	  uint32_t intervalDPRiseAbove2V = 0;
	  uint32_t ts0,ts1,ts2;
	  prepareTest();
	  //Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
      SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);

	  ts0 = 0;
	  ts1 = 0;
	  ts2 = 0;

  do{
     //curTm = GlobleTime ;
		
  		  Report_MSG(" PD - DCD with current source test");
	//DCD using IDP_SRC
	Report_MSG("----DCD using IDP_SRC----------------------------------");
//1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)
//to VBUS and turns on VBUS to 5V.
   Report_MSG("1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)");
   Report_MSG("	to VBUS and turns on VBUS to 5V.");
   prepareVBusForPD();
 							  
		
//2. Start timer when VBUS reaches 0.8V.
   Report_MSG("2. Start timer when VBUS reaches 0.8V.");

   GPIO_Setting(SW_VBUS_AB_M1,ON);//使用M1来监测VBUS
	tmp	= WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,5500,800,5,10000) ;	//10次连续有效，500ms 超时
   if(resCapturedBig == tmp){
  	 //right

      ts0 = GlobleTime;  //VBUS captured point
	  Report_MSG("INFO:VBUS rose above 0.8V.")  ;

   }else
   {
		//failed
		Report_MSG("ERRO:VBUS did not rise above 0.8V.")  ;
		Report_MSG("Stop this test...");
		testResult = TEST_FAILED;
		//break;
   }
					  	
//3. Wait for D+ to rise above 2V. (If it doesn't within 0.9 second – test fails.) This proves
//that IDP_SRC is sufficiently large to overcome the UUT's own leakage current. [PD5]
	Report_MSG("3. Wait for D+ to rise above 2V. (If it doesn't within 0.9 second – test fails.)");
	Report_MSG("	This proves that IDP_SRC is sufficiently large to overcome the UUT's own leakage current. [PD5]");


//4. Measure and report time till it does.
	Report_MSG("4. Measure and report time till it does.");

      tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,0 ,2000,5,9000) ;
   if (tmp == resCapturedBig)
   {
		//right
			ts1 = GlobleTime; //  D+ rose above 2v
		    intervalDPRiseAbove2V = ts1 - ts0	;
			sprintf(charBuf,"INFO: D+ rose above 2V within %d ms of VBUS reaching 0.8V,D+= %d mV", intervalDPRiseAbove2V/10,ADC_Captured_Value) ;	
	        Report_MSG(charBuf)  ;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	      	sprintf(charBuf,"ERRO: D+ did not rise above 2V within 0.9s of VBUS reaching 0.8V,D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			//Report_MSG("Stop this test...");
			testResult = TEST_FAILED;
		//	break;
   }

//5. After 1ms of D+ exceeding 2V, measure and report voltage on D+.
	  Report_MSG("5. After 1ms of D+ exceeding 2V, measure and report voltage on D+.");

	  Delay(10);
	  sprintf(charBuf,"INFO: D+= %d mV",Get_Avarage_ADC_Value(ADC_DP,5) ) ;
	  Report_MSG(charBuf)	 ;


//6. Connect 0.8V via 15k to D-. Check voltage at D- is in correct range for a pull-down
//value meeting the specification (i.e. that voltage is not greater than 0.498V and not less
//than 0.390V). [PD5]
	 Report_MSG("6. Connect 0.8V via 15k to D-. Check voltage at D- is in correct range for a pull-down");
	 Report_MSG("	value meeting the specification (i.e. that voltage is not greater than 0.498V and not less");
	 Report_MSG("	than 0.390V). [PD5]");
//Note: For worst case max RDM_DWN of 24.8K we need a max voltage at D- of 0.498V. 
//For worst case min RDM_DWN of 14.25K we need a min voltage at D- of 0.390V.

	 Report_MSG("Note: For worst case max RDM_DWN of 24.8K we need a max voltage at D- of 0.498V.");
	 Report_MSG("	For worst case min RDM_DWN of 14.25K we need a min voltage at D- of 0.390V.");


	powerDM( 800 , RES_15K);
	tmp =  WaitAdcValueMatch(ADC_DM,CaptureInRange,498,390 ,30,100) ;
   if (tmp == resCapturedInRange)
   {
		//right

	  sprintf(charBuf,"INFO:D- is in range of 0.390-0.498v, D-= %d mV", ADC_Captured_Value) ;
	  Report_MSG(charBuf)	 ;
	  Report_MSG("INFO:D- pulldown RDM_DWN is valid.");
	
   }else{
	  sprintf(charBuf,"ERRO:D- is NOT in range of 0.390-0.498v, D-= %d mV", ADC_Captured_Value) ;
	  Report_MSG(charBuf)	 ;
	  Report_MSG("ERRO:D- pulldown RDM_DWN is NOT valid.");
   }

//7. Disconnect components in previous step.
	 Report_MSG("7. Disconnect components in previous step.");
	powerDM( 0 , RES_NONE); 	
    //	Report_MSG("Now,Disconnect components in previous step")  ;

//8. Connect 15k from D+ to 0V.
	Report_MSG("8. Connect 15k from D+ to 0V.")  ;
    powerDP(0 , RES_15K);

//9. Wait just less than TDCD_DBNC min (<10ms = 9.5ms). [PD9]
	 Report_MSG("9. Wait just less than TDCD_DBNC min (<10ms = 9.5ms). [PD9]")  ;
	 Delay(95);
 
//10. Check voltage on D+ is less than 0.375V. This proves that IDP_SRC and leakage,
// together, are low enough to provide a low logic level on D+, even after nearly
// TDCD_DBNC min (10ms). (Voltage to be adjusted in actual test script to allow for small tester leakage.)
	 Report_MSG("10. Check voltage on D+ is less than 0.375V. This proves that IDP_SRC and leakage,")  ;
	 Report_MSG("	together, are low enough to provide a low logic level on D+, even after nearly")  ;
	 Report_MSG("	TDCD_DBNC min (10ms). (Voltage to be adjusted in actual test script to allow for");
	 Report_MSG("	small tester leakage.")  ;
//Note: Highest current sourced by UUT should be IDP_SRC max (13μA) plus VDAT_LKG max (3.6V)
//      across RDAT_LKG min (300K), equals 13 + 12 μA = 25μA. 25μA x 15K = 0.375V.
	 Report_MSG("Note: Highest current sourced by UUT should be IDP_SRC max (13uA) plus VDAT_LKG max (3.6V)")  ;
	 Report_MSG("       across RDAT_LKG min (300K), equals 13 + 12 uA = 25uA. 25uA x 15K = 0.375V.")  ;


	  tmp = Get_Avarage_ADC_Value(ADC_DP,5);
	  if(tmp<=375)
	  {
	    sprintf(charBuf,"INFO: D+= %d mV", tmp) ;
	    Report_MSG(charBuf)	 ;
		}
	  else
	  {
		sprintf(charBuf,"ERRO: D+= %d mV", tmp) ;
	    Report_MSG(charBuf)	 ;
	  }
 

//11. Disconnect everything from VBUS, and switch VBUS off.
	  Report_MSG("11. Disconnect everything from VBUS, and switch VBUS off.")  ;
	  turnOffVBus();
	  
//12. Wait 2 seconds for disconnect to be detected.	
	  Report_MSG("12. Wait 2 seconds for disconnect to be detected.")  ;
	  Delay(20000);

//DCD using Timeout
	  Report_MSG("----DCD using Timeout----------------------------------------")  ;

//13. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)
//to VBUS and turns on VBUS to 5V.
   Report_MSG("13. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)");
   Report_MSG("	to VBUS and turns on VBUS to 5V.Disconnect the 15KOhm pulldown from D+ to 0V");

   prepareVBusForPD();
   // disconnect 15K pull down Res from D+ to GND before run the second part of this
   //test (DCD using timeout).
	powerDP( 0 , RES_NONE);	   
   	powerDM( 0 , RES_NONE);

//14. Start timer when VBUS reaches 0.8V.
	Report_MSG("14. Start timer when VBUS reaches 0.8V.");

    GPIO_Setting(SW_VBUS_AB_M1,ON);//使用M1来监测VBUS
	tmp	= WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,5500,800,5,10000) ;	//10次连续有效，500ms 超时
   if(resCapturedBig == tmp){
  	 //right

      ts0 = GlobleTime;  //VBUS captured point
	  Report_MSG("INFO:VBUS rose above 0.8V.")  ;

   }else
   {
		//failed
		Report_MSG("ERRO:VBUS did not rise above 0.8V.")  ;
		Report_MSG("Stop this test...");
		testResult = TEST_FAILED;
		break;
   }

//15. Wait for D+ to rise above 2V. (If it doesn't within 0.9 second of VBUS 
//  reaching 0.8V – test fails.)this proves that IDP_SRC is sufficiently large
//  to overcome the UUT's own leakage current..
   	Report_MSG("15. Wait for D+ to rise above 2V. (If it doesn't within 0.9 second of VBUS");
	Report_MSG("	reaching 4V – test fails.) This proves that IDP_SRC is sufficiently large");
	Report_MSG("	to overcome the UUT's own leakage current. ");

   tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,0 ,2000,5,9000) ;
   if (tmp == resCapturedBig)
   {
		//right
			ts1 = GlobleTime; //  D+ rose above 2v
		    intervalDPRiseAbove2V = ts1	- ts0;
			sprintf(charBuf,"INFO: D+ rose above 2V within %d ms of VBUS reaching 0.8V,D+= %d mV", intervalDPRiseAbove2V/10,ADC_Captured_Value) ;	
	        Report_MSG(charBuf)  ;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	      	sprintf(charBuf,"ERRO: D+ did not rise above 2V within 0.9s of VBUS reaching 0.8V,D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			Report_MSG("Stop this test...");
			testResult = TEST_FAILED;
			break;
   }


//16. Check that D+ goes down to voltage in the range VDP_SRC (0.5 to 0.7V) within
//TDCD_TIMEOUT (0.3 to 0.9 sec) of D+ going high and within 1 second of VBUS reaching 0.8V. [PD6]
   //先检测0-0.3s内，不因该有Vdp_src,
	Report_MSG("16. Check that D+ goes down to voltage in the range VDP_SRC (0.5 to 0.7V) within");
	Report_MSG("	TDCD_TIMEOUT (0.3 to 0.9 sec) of the attach event.[PD6]");

   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,TO_STM_TIME_UNIT(290)) ;
    if (tmp == resCapturedInRange)
   {
		//right
		    ts2 = GlobleTime	;  //ts2 = VDP_SRC point;ts0 VBUS apply point
			if(3000 > (ts2-ts0))		//
			{
			Delay(3000 - (ts2-ts0));//补齐0.3s
			}
			sprintf(charBuf,"ERRO: D+ voltage should not in 0.5-0.7v within 0-0.3s of VBUS on,Detect VDP_SRC at %d ms.", (ts2-ts0)/10) ;	
	        Report_MSG(charBuf)  ;
   			sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("INFO: UUT did not aplly VDP_SRC within 0-0.3s of VBUS on");
	      	sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
		}  

   //17.然后在0.3-0.9秒内应该有Vdp_src
   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,6000) ;
   //	tmp =  WaitAdcValueMatch(ADC_DM,CaptureInRange,498,390 ,10,TO_STM_TIME_UNIT(900)) ;
   if (tmp == resCapturedInRange)
   {
		//right
		    intervalDPRiseAbove2V = GlobleTime - ts0	;
			sprintf(charBuf,"INFO: Detected a valid VDP_SRC on D+ within 0.3-0.9s, after %d ms of VBUS on", intervalDPRiseAbove2V/10) ;	
	        Report_MSG(charBuf)  ;
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			//delay 35ms and check again
				
			Delay(350);
			tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,50) ;
			 if (tmp == resCapturedInRange)
  		   {
		   	Report_MSG("INFO: D+ voltage is still in range of 0.5-0.7v after TVDP_SRC_ON MIN ") ;	
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
	
		   }
		    else
			{
			Report_MSG("ERRO: But D+ voltage is not in range of 0.5-0.7v after TVDP_SRC_ON MIN ") ;	
   			sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
			}

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("ERRO: Did not detect a valid VDP_SRC within 0.3-0.9s of VBUS on");
	      	sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
		}    
//17. Turn off VBUS.
	   Report_MSG("17. Turn off VBUS.");

		turnOffVBus();

//18. Wait 8 seconds, ignoring SRP pulse, for disconnect to be detected.
   Report_MSG("18. Wait 8 seconds, ignoring SRP pulse, for disconnect to be detected.");

	Delay(TO_STM_TIME_UNIT(8000));	 
	   //if run to last, test pass
	}while(FALSE);
	Report_MSG("END of PD DCD with Current Source test");

	TASK_CLEAN_UP()
}

/***************END OF 6.4 Data Contact Detect Test – With Current Source**************************/


/***************6.5 Data Contact Detect Test – No Current Source**************/
//weiming updated on 26 oct
//weiming updated on 2012.4.6 调整了0-0.3s 时间，放宽至0-0.28s
//2012-6-7 updated ,上报porcedure


uint16_t PD_DataContactDetectTestNoCurrentSrc(void *p){
    DECLARE_VARS()
	uint32_t ts0,ts2;
    prepareTest();
   //Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
   SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);

   do{
		Report_MSG(" PD - DCD no current source test");

//DCD using Timeout
Report_MSG("----DCD using Timeout----------------------------------------")  ;

//1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)
//to VBUS and turns on VBUS to 5V.
   Report_MSG("1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)");
   Report_MSG("	to VBUS and turns on VBUS to 5V.");
   prepareVBusForPD();


//2. Start timer when VBUS reaches 0.8V.
	Report_MSG("2. Start timer when VBUS reaches 0.8V.");

    GPIO_Setting(SW_VBUS_AB_M1,ON);//使用M1来监测VBUS
	tmp	= WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,5500,800,5,10000) ;	//10次连续有效，500ms 超时
   if(resCapturedBig == tmp){
  	 //right

      ts0 = GlobleTime;  //VBUS captured point
	  Report_MSG("INFO:VBUS rose above 0.8V.")  ;

   }else
   {
		//failed
		Report_MSG("ERRO:VBUS did not rise above 0.8V.")  ;
		Report_MSG("Stop this test...");
		testResult = TEST_FAILED;
		break;
   }
//3. Connect 15k resistors from 0V to D+, and from 0V to D-.
	Report_MSG("3. Connect 15k resistors from 0V to D+, and from 0V to D-.")  ;
	powerDP( 0 , RES_15K);
	powerDM( 0 , RES_15K);



//4. Check that D+ goes down to voltage in the range VDP_SRC (0.5 to 0.7V) within
//TDCD_TIMEOUT (0.3 to 0.9 sec) of D+ going high and within 1 second of VBUS reaching 0.8V. [PD6]
   //先检测0-0.3s内，不因该有Vdp_src,
	Report_MSG("4. Check that D+ goes down to voltage in the range VDP_SRC (0.5 to 0.7V) within");
	Report_MSG("	TDCD_TIMEOUT (0.3 to 0.9 sec) of the attach event. [PD6]");

   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,TO_STM_TIME_UNIT(290)) ;
    if (tmp == resCapturedInRange)
   {
		//right
		    ts2 = GlobleTime	;  //ts2 = VDP_SRC point;ts0 VBUS apply point
			if(3000 > (ts2-ts0))		//
			{
			Delay(3000 - (ts2-ts0));//补齐0.3s
			}
			sprintf(charBuf,"ERRO: D+ voltage should not in 0.5-0.7v within 0-0.3s of VBUS on,Detect VDP_SRC at %d ms.", (ts2-ts0)/10) ;	
	        Report_MSG(charBuf)  ;
   			sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("INFO: UUT did not aplly VDP_SRC within 0-0.3s of VBUS on");
	      	sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
		}  

   //17.然后在0.3-0.9秒内应该有Vdp_src
   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,6000) ;
   //	tmp =  WaitAdcValueMatch(ADC_DM,CaptureInRange,498,390 ,10,TO_STM_TIME_UNIT(900)) ;
   if (tmp == resCapturedInRange)
   {
		//right

			sprintf(charBuf,"INFO: Detected a valid VDP_SRC on D+ within 0.3-0.9s, after %d ms of VBUS on", (GlobleTime - ts0)/10) ;	
	        Report_MSG(charBuf)  ;
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			//delay 35ms and check again
				
			Delay(350);
			tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,50) ;
			 if (tmp == resCapturedInRange)
  		   {
		   	Report_MSG("INFO: D+ voltage is still in range of 0.5-0.7v after TVDP_SRC_ON MIN ") ;	
   			sprintf(charBuf,"INFO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
	
		   }
		    else
			{
			Report_MSG("ERRO: But D+ voltage is not in range of 0.5-0.7v after TVDP_SRC_ON MIN ") ;	
   			sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
			}

   }else{
	 // resCapturedTimeOut or other
	   //test failed
	   		Report_MSG("ERRO: Did not detect a valid VDP_SRC within 0.3-0.9s of VBUS on");
	      	sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
		}    

//5. Disconnect 15k pull-down resistors from D+ and D-.
  	   Report_MSG("5. Disconnect 15k pull-down resistors from D+ and D-.");
	   powerDP( 0 , RES_NONE);
       powerDM( 0 , RES_NONE);

//6. Disconnect capacitance and pull-down resistor from VBUS and switch VBUS off.
	   Report_MSG("6. Disconnect capacitance and pull-down resistor from VBUS and switch VBUS off.");

		turnOffVBus();

//7. Wait 8 seconds, ignoring SRP pulse, for disconnect to be detected.
   Report_MSG("7. Wait 8 seconds, ignoring SRP pulse, for disconnect to be detected.");

	Delay(TO_STM_TIME_UNIT(8000));	
	 
   }while(FALSE);
   Report_MSG("END OF PD-DCD No Current Source Test");
   TASK_CLEAN_UP()
}
/***************END OF 6.5 Data Contact Detect Test – No Current Source**************/


/***************6.6 DCP Detection Test**************/
//weiming
//update at 26 oct
//updated at 2012-3-26
//2012-5-31 更新至step9 待续。
//20120-6-6 根据新的procedure更新
//2012-12-30 加大输出电容到100uF，固定电流量程为高
// fix bug in PD-CDP Detection Test. ( Secondary Detection Setp 10),原来的脚本电压判断有错误。

uint16_t PD_DCP_Detection (void* p)
{
   DECLARE_VARS()
    uint16_t hasSecondaryDetection ;//= *(uint16_t*)p;
 	uint32_t ts0,ts1,ts2,ts3;  //时间戳
	uint16_t x,y;

	ts1 = 0;

   prepareTest();
   //Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
   SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);

 do{
 	
	 Report_MSG(" PD - DCP Detection Test");
	//askHostToInputVal("Does UUT support Secondary Detection ? \n0:No, 1:Yes");
	//hasSecondaryDetection =    *(uint16_t*)g_userInputInfo;

    hasSecondaryDetection = askHostToInputVal("Does UUT support Secondary Detection?\r\nInput: 1 = SUPPORT,0 = NOT SUPPORT");



	Report_MSG("----DCD----------------------------------------------------")  ;
   //1. PET connects 200R between D+ and D-.
    Report_MSG("1. PET connects 200R between D+ and D-.")  ;
   GPIO_Setting(SW_DCD_RES_AB,ON);

   //2. PET connects voltage source of 0V via 100k to D-, to prevent false detection of voltage on D+ etc.
   Report_MSG("2. PET connects voltage source of 0V via 100k to D-, to prevent false detection of voltage on D+ etc.")  ;
   DAC_Setting(DAC_DM,0);
   DAC_Setting(DAC_DP,0); //新加上
   DAC_DM_Output_Res(RES_100K);

   //3. PET applies CADP_ VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS to VB_BUS nom (5V). ID pin is left floating.
   Report_MSG("3. PET applies CADP_ VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)")  ;
   Report_MSG("	to VBUS and turns on VBUS to VB_BUS nom (5V). ID pin is left floating.")  ;

   prepareVBusForPD();
   VBUS_CAP_Setting(CAP_100uF);	  //加大电容，保证大电流稳定	

   ID_RES_Setting(RES_Rid_OPEN);
   ts0= GlobleTime;	   //ts0 = vbus valid
   
   Report_MSG("----Primary Detection----------------------------------") ;


   /* Primary Detection*/

   //4. Wait for D+ to rise above 0.5V
   Report_MSG("4. Wait for D+ to rise above 0.5V")  ;

   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,5500,500,10,9000) ;	//10次连续有效，5000ms 超时
   if (tmp == resCapturedInRange)
   {
	    ts1 = GlobleTime;//UUT start to apply VDP_SRC.	
	    Report_MSG("INFO:D+ rose above 0.5V, ")  ;
   }
   else
   {
   		sprintf(charBuf,"ERRO:D+ did not rise above 0.5V, D+= %d", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
	}

   //5. Wait 1ms for D+ to settle.
   Report_MSG("5. Wait 1ms for D+ to settle.")  ;
   Delay(10);

   //6. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V).
	Report_MSG("6. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V).");

	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp)
   {
  	 //right
   		sprintf(charBuf,"INFO: D+ voltage was valid. D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }
   else{
		//failed
		sprintf(charBuf,"ERRO: D+ voltage was not valid. D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }


   //7. Wait for slightly less than TVDPSRC_ON min (38ms).
	Report_MSG("7. Waiting until about 39ms after UUT applying VDP_SRC( this is less than TVDPSRC_ON min).");
    if(390 > (GlobleTime - ts1))
	{
		Delay(390 - (GlobleTime - ts1));
	}


   //8. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V)
   Report_MSG("8. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V)");
	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp){
  	 //right
   		sprintf(charBuf,"INFO:D+ voltage was valid. D+= %d mv", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }else{
		//failed
		sprintf(charBuf,"ERRO:D+ voltage was not valid. D+= %d mv ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }

   //9. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS
   //   going on to expire.
Report_MSG("9. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS going on to expire.") ;

 ts2 = 10000-(GlobleTime - ts0)	 ;	 //ts0 = DUT attach 
 if(ts2>0)
  {
	  tmp = WaitAdcValueMatch(ADC_DP,CaptureBigOrSmall,500,800,10,ts2 ) ;	//10次连续有效，5000ms 超时
	   if (tmp == resCapturedBig)
	   {

		  //If D+ rises instead above 0.8V, it will be regarded as a 'device connect'. 

			Report_MSG("INFO:D+ went above 0.8V.");

				if(hasSecondaryDetection > 0)
				{
					  //If secondary detection is declared to be implemented, and less than 80ms has passed since D+ first rose above 0.5V, 
			  		  //this constitutes a failure because the specified periods TVDPSRC_ON plus TVDMSRC_ON have not been completed. [PD14] 
	
					  //If secondary detection is declared to be implemented, and more than 80ms has passed since D+ first rose above 0.5V, 
			 		  //this implies that the change between driving VDP_SRC and then driving VDM_SRC was not seen by the tester. 
			          //The only possible conclusion here is to assume a pass at this stage, and to skip to 'Checking Current Draw'.
					  ts3 = GlobleTime - ts1 ; //ts1 = UUT apply VDP_SRC
					  if(ts3 <= 800)
					  {
						Report_MSG("ERRO:Secondary detection is declared to be implemented, and less than 80ms has passed since D+ first rose above 0.5V,");
						Report_MSG("ERRO:this constitutes a failure because the specified periods TVDPSRC_ON plus TVDMSRC_ON have not been completed. [PD14]");
					    sprintf(charBuf,"ERRO:The measured duration is %d ms.", ts3/10) ;
						Report_MSG(charBuf)	 ;
						testResult = TEST_FAILED;
					  }
					  else
					  {
						Report_MSG("INFO:Secondary detection is declared to be implemented, and more than 80ms has passed since D+ first rose above 0.5V,");
					    sprintf(charBuf,"INFO:The measured duration is %d ms.", ts3/10) ;
						Report_MSG(charBuf)	 ;
	
					  }
	  			}
				else
				{
					   //If secondary detection is declared not to be implemented, this is a valid procedure, 
			 		   //and the tester will report that this sequence has occurred, and skip to 'Checking Current Draw'. 
					    Report_MSG("INFO:Secondary detection is declared NOT to be implemented,this is a valid procedure,");

				}
			 	Report_MSG("INFO:Skip to 'Checking Current Draw'.")  ;
			    goto PD_DCP_CHECKING_CURRENT_DRAW;  //enumeration
  
  		}//END OF If D+ rises instead above 0.8V 
	 
	   if (tmp == resCapturedSmall)
	   {
		   //D+ goes below 0.5V:

			Report_MSG("INFO:D+ went below 0.5V.");
		    sprintf(charBuf,"INFO:Measured duration of TVDPSRC_ON is %d ms ", (GlobleTime-ts1)/10) ;
			Report_MSG(charBuf)	 ;

		   	if(hasSecondaryDetection > 0)	
			{
			   // If secondary detection is declared to be implemented, proceed with 'Secondary Detection'.
			   	Report_MSG("INFO:Secondary detection is declared to be implemented, proceed with 'Secondary Detection'.")  ;

			}
			 else
			 {
				//If secondary detection is declared not to be implemented, skip to 'Checking Current Draw'.
		    	Report_MSG("INFO:Secondary detection is declared not to be implemented, skip to 'Checking Current Draw'")  ;
			   	goto PD_DCP_CHECKING_CURRENT_DRAW;  //enumeration

			 }
			
	   }
	    if (tmp == resCapturedTimeOut)
		{  
		   //TSVLD_CON_PWD expires:
			Report_MSG("INFO:TSVLD_CON_PWD expired.")  ;
			sprintf(charBuf,"INFO:D+= %d mV ", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
		
		//Implication is that change between driving VDP_SRC, then driving VDM_SRC and then driving VDP_SRC was not seen by the tester, 
	    	//as no gaps were inserted. We can only assume that this is not a failure. Skip to 'Checking Current Draw'.
			Report_MSG("INFO:Skip to 'Checking Current Draw'.")  ;
		   goto PD_DCP_CHECKING_CURRENT_DRAW;  //enumeration
		}   
   }
   else
   {
	   Report_MSG("ERRO:internal error: ts2<0 ") ;
	   testResult = TEST_FAILED;
	   break;

   }


  //Secondary Detection
  Report_MSG("----Secondary Detection----------------------------------") ;
 //10. Wait for D- to rise above 0.5V, or above 0.8V, or for expiration of TSVLD_CON_PWD (1 sec) from VBUS going on.
   Report_MSG("10. Wait for D- to rise above 0.5V, or above 0.8V, or for expiration of TSVLD_CON_PWD (1 sec) from VBUS going on.");


   ts3 = 10000 - (GlobleTime - ts0); //设置超时时间 = 1sec - 以上步骤消耗的时间, ts0 = vbus going on
   //首先检测是否大于0.5V
   

   tmp	= WaitAdcValueMatch(ADC_DM,CaptureInRange,5500,500,10,ts3) ;	//10次连续有效，1000ms 超时
   if (tmp == resCapturedInRange)
   {
	 //D- goes above 0.5V:
    // First check if it goes between 0.7V and 0.8V within 1ms. If so this is an error.


	//  Also check if it goes above 0.8V within 1ms. If so go to 'D- goes above 0.8V'.
	//	If voltage stays within VDM_SRC (0.5 to 0.7V), assume this is secondary detection and go to Step 11.
		Report_MSG("INFO: D- rose above 0.5V.")  ;
		 ts2 = GlobleTime;	// VDM_SRC start
		 Delay(5);
		 tmp = Get_Avarage_ADC_Value(ADC_DM,5);

		 if(tmp<800 && tmp>700 ) //0.7-0.8V	 ,First check if it goes between 0.7V and 0.8V within 1ms. If so this is an error.
		 {
		 
		    sprintf(charBuf,"ERRO: D- went between 0.7V and 0.8V. D-= %d mv", tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
			goto PD_DCP_CHECKING_CURRENT_DRAW	;
		 }
		 else if(tmp>800)	//  Also check if it goes above 0.8V within 1ms. If so go to 'D- goes above 0.8V'.
		 {
		    //This will be regarded as a 'device connect'. (D+ and D- are connected via 200R.) 
			//If less than 80ms has passed since D+ first rose above 0.5V, 
			//this constitutes a failure because the specified periods TVDPSRC_ON plus TVDMSRC_ON have not been completed. [PD14]
			
			sprintf(charBuf,"INFO: D- went above 0.8V. D-= %d mv", tmp) ;
			Report_MSG(charBuf)	 ;
		    Report_MSG("INFO: This will be regarded as a 'device connect'. (D+ and D- are connected via 200R.)") ;
			if(800 < (GlobleTime - ts0))
			{
			  	sprintf(charBuf,"INFO: More than 80ms has passed since D+ first rose above 0.5V. duration = %d ms", (GlobleTime - ts0)/10) ;
				Report_MSG(charBuf)	 ;
			}
			else
			{
				sprintf(charBuf,"ERRO: TVDPSRC_ON plus TVDMSRC_ON have not been completed. [PD14], duration = %d ms", (GlobleTime - ts0)/10) ;
				Report_MSG(charBuf)	 ;
				testResult = TEST_FAILED;
			}
			goto PD_DCP_CHECKING_CURRENT_DRAW	;

		 }
		 else if(tmp<=700 && tmp>=500)		   //0.5-0.7V
		 {
		//	ts2 = GlobleTime; // VDM_SRC start
			sprintf(charBuf,"INFO: D- voltage stay within VDM_SRC, D-= %d mv", tmp) ;
			Report_MSG(charBuf)	 ;

		 }
		 else			 // RE went below 0.5v
		 {
			sprintf(charBuf,"ERRO: D- voltage went below 0.5v, D-= %d mv", tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
			goto PD_DCP_CHECKING_CURRENT_DRAW	;
		 }


   }
   else	 //	 TSVLD_CON_PWD(1s) expired
   {
	//TSVLD_CON_PWD expires: This constitutes a failure because neither VDP_SRC 
	// nor a pullup to VDP_UP through RDP_UP occurred.	

	 sprintf(charBuf,	"ERRO: TSVLD_CON_PWD(1s) expired,D- was below 0.5v D-= %d mV", ADC_Captured_Value) ;
	 Report_MSG(charBuf)	 ;
	 testResult = TEST_FAILED;

	 goto PD_DCP_CHECKING_CURRENT_DRAW	;
	 }

  // 11. Wait for slightly less than TVDMSRC_ON min (38ms).
	 	Report_MSG("11. Waiting until about 39ms after UUT applying VDM_SRC (this is less than TVDMSRC_ON min).")	 ;
	   if(GlobleTime - ts2 < 390)
	 {   
		Delay(385 - (GlobleTime - ts2));
	 }
	


//12. Check that D- voltage is still VDM_SRC (0.5 to 0.7V). [PD14], [PD15]
  	  	Report_MSG("12. Check that D- voltage is still VDM_SRC (0.5 to 0.7V). [PD14], [PD15]");
	   tmp = Get_Avarage_ADC_Value(ADC_DM,5);
	   if (tmp <=700 && tmp >= 500)
	   {
	 	sprintf(charBuf,"INFO: D- voltage was valid, D-= %d mV", tmp) ;
		Report_MSG(charBuf)	 ;
	   }
	   else
	   {
	 	sprintf(charBuf,"ERRO:D- voltage was not valid, D-= %d mV", tmp) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
	   }

//13. Wait for D- to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD max (1 sec) from VBUS going on to expire.
 	 	Report_MSG("13. Wait for D- to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD max (1 sec) from VBUS going on to expire.");
   ts3 = 10000 - (GlobleTime - ts0); //设置超时时间 = 1sec - 以上步骤消耗的时间
   tmp	= WaitAdcValueMatch(ADC_DM,CaptureBigOrSmall,500,800,5,ts3) ;	//10次连续有效，1000ms 超时
   if (tmp == resCapturedSmall)
   {
	sprintf(charBuf,"INFO:D- went below 0.5V, D-= %d mV", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;
	sprintf(charBuf,"INFO:Measured duration of TVDM_SRCON is %d ms", (GlobleTime - ts2)/10) ;
	Report_MSG(charBuf)	 ;
   }
   else if (tmp == resCapturedBig)
   {
	 sprintf(charBuf,"INFO:D- went above 0.8V, D-= %d mV", ADC_Captured_Value) ;
	 Report_MSG(charBuf)	 ;
   }
   else	 //	 TSVLD_CON_PWD(1s) expired
   {
	//skip to 'Checking Current Draw'	

	 sprintf(charBuf,"INFO: TSVLD_CON_PWD(1s) expired, D-= %d mV", ADC_Captured_Value) ;
	 Report_MSG(charBuf)	 ;	
	 }

PD_DCP_CHECKING_CURRENT_DRAW:

      
	//14. If secondary detection is declared to be implemented,
	// display message to test operator 'PD under test should now have detected DCP'. 
	//If secondary detection is declared not to be implemented, 
	//display message to test operator 'PD under test should now have detected DCP or CDP'. (This is a development aid.)
		if(hasSecondaryDetection > 0 )
		{
	  	 Report_MSG("14.PD under test should now have detected DCP")  ;
		}
		else
		{
	  	 Report_MSG("14.PD under test should now have detected DCP or CDP")  ;
		}

	//15. Check current drawn does not exceed 1.5A from now on
	  Report_MSG("15. Check current drawn does not exceed 1.5A from now on.")  ;
	  enableWatchBlock(0 , ADC_VBUS_I , CaptureBig ,	15000  , 10); //连续1ms 超过1500mA

//16. Check D+ is held at or above VDP_SRC min (0.5V) for the entire period, 
//starting TSVLD_CON_PWD max (1 sec) after VBUS was applied, that current drawn exceeds ISUSP max (2.5mA). 
//Note that the voltage drop in the cable ground will raise the apparent voltage on D+. [PD17]
	 Report_MSG("16. Check D+ is held at or above VDP_SRC min (0.5V) for the entire period,");
	 Report_MSG("	starting TSVLD_CON_PWD max (1 sec) after VBUS was applied, that current drawn exceeds ISUSP max (2.5mA).");
	 Report_MSG("	Note that the voltage drop in the cable ground will raise the apparent voltage on D+. [PD17]");


	//17. Maintain session for 30 seconds from step 14.
	 Report_MSG("17. Maintain session for 30 seconds from step 14.");
	ts1 =  GlobleTime;//记录当前时间
	//??has problem 
    tmp	= WaitAdcValueMatch(ADC_VBUS_I,CaptureInRange,15000,30,10,300000) ;	//10次连续有效，30s 超时
	if (tmp == resCapturedInRange )
	{
	      ts2 =GlobleTime;
	    do{
		x =   Get_Avarage_ADC_Value(ADC_VBUS_I,20000)/10;
	    sprintf(charBuf,"INFO: Avarage Current = %d mA",x) ;	
		Report_MSG(charBuf)	 ;

		// 16. Check D+ is held at or above VDP_SRC min (0.5V) for the entire period, starting TSVLD_CON_PWD max (1 sec) after VBUS was applied, that current drawn exceeds ISUSP max (2.5mA). Note that the voltage drop in the cable ground will raise the apparent voltage on D+. [PD17]
	
		y =   Get_Avarage_ADC_Value(ADC_DP,200);
		if (x>=3 && y<=500)
		{
		Report_MSG("ERRO: D+ is NOT held at or above VDP_SRC min(0.5V) when current drawn exceeds 2.5mA")	;
		sprintf(charBuf,"ERRO: D+ = %d mV",y) ;	
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
		}
		else
		{
		sprintf(charBuf,"INFO: D+ = %d mV",y) ;	
		Report_MSG(charBuf)	 ;
		}
		  }while(300000>(GlobleTime - ts2))	;
	/*	 //Ibus已经超过2.5mA，开始检查D+ 是否大于0.5
		 Report_MSG("INFO: Ibus exceeded 2.5mA at step 16 ,")  ;			
		 sprintf(charBuf,"INFO: Current = %d mA",ADC_Captured_Value)  ;
		 Report_MSG(charBuf)  ;
		 Report_MSG("Monitoring,Please wait...")  ;
		 ts2 = 300000 - (GlobleTime - ts1) ;
	   	 tmp = WaitAdcValueMatch(ADC_DP,CaptureBig,0,400,10,ts2) ;
		 if (tmp == resCapturedBig)
		 {
		  // D+按照要求enable VDP_SRC or pull D+ to VDP_UP，30秒未到，等待到30秒
		  
		  ts2 =  300000 - (GlobleTime - ts1);
		  Delay(ts2);
		  //report 'DUT applied Vdp_src or pull D+ to VDP_UPon D+ when crrent draw exceeds Isusp MAX(2.5mA)'
		   Report_MSG("INFO: DUT applied Vdp_src or pull D+ to VDP_UP")	;
		   sprintf(charBuf,"INFO: D+= %d mV",ADC_Captured_Value) ;
		   Report_MSG(charBuf)	 ;
		 }
		 else
		 {
		   //D+ 未按照要求拉高,30秒已到
		   //report 'Failed -- DUT NOT applied Vdp_src on D+ when crrent draw exceeds Isusp MAX(2.5mA)'
		   Report_MSG("ERRO: DUT not applied Vdp_src or pull D+ to VDP_UP")  ;
		   sprintf(charBuf,"ERRO: D+= %d mV", ADC_Captured_Value) ;	
		   Report_MSG(charBuf)	 ;
		 }
		 */
	}					
	else
	{
	 //I没有超过2.5mA，30sec已经过去。
	 //report 'Info -- DUT current draw NOT exceeds Isusp MAX(2.5mA) in 30 Secs'
	 Report_MSG("INFO:DUT current draw NOT exceeded Isusp MAX(2.5mA) in 30 Secs")  ;
	}
//	tmp_current = Get_Avarage_Value(ADC_VBUS_I,2500)  ;
//	sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10) ;	
//	Report_MSG(charBuf)	 ;

	//check watchblock[0]- ADC_VBUS_I (对应 step 15)
	if ( IsWatchBlockTriggered(0))
	{
	 //Triggered I > 1500mA
	 //report 'Failed - Max current draw exceeded 1500mA'
	 Report_MSG("ERRO: Max current draw exceeded 1500mA")  ;
	 sprintf(charBuf,"ERRO: Max(Peak) Current = %d mA",GetWatchBlockRecordVal(0)/10) ;	
	 Report_MSG(charBuf)	 ;
	  testResult = TEST_FAILED;
		
	}
	else 
	{
	 //not Trigger then stop WatchBlock[0]
	 Report_MSG("INFO: Max current draw not exceeded 1500mA")  ;
     //sprintf(charBuf,"INFO: Max(Peak) Current = %d mA",GetWatchBlockRecordVal(0)/10) ;		
	// Report_MSG(charBuf)	 ;
	}


	 //18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
	 Report_MSG("18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");
	  GPIO_Setting(RSW_VBUS_AB,OFF); 
	  DAC_Setting(DAC_VBUS_V,0);  //VBUS_V = 0
	  VBUS_CAP_Setting(CAP_NONE);
	  PULL_DOWN_RES_Setting(RES_NONE) ;

     //19. Disconnect voltage source and 100k resistor from D-.
	 Report_MSG("19. Disconnect voltage source and 100k resistor from D-.");
       DAC_DM_Output_Res(RES_NONE);

	 //20. Disconnect 200R resistor between D+ and D-.
	 Report_MSG("20. Disconnect 200R resistor between D+ and D-.");
	   GPIO_Setting(SW_DCD_RES_AB,OFF);

	 //21. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
	 Report_MSG("21. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.");

	  Delay(80000);

	  	  Report_MSG("End of PD DCP TEST")  ;
  
}while(FALSE);
   TASK_CLEAN_UP()

}
/***************END OF 6.6 DCP Detection Test**************/



/***************BEGIN OF 6.7 CDP Detection Test**************/
//2012-3-26 modified
//2012-5-2 updated - fix enumeration
//2012-5-3 upadte overall result
//2012-5-15 update re-negoation M1
//2012-5-24 show step message, 调整Pramary 检测时序；step44 电流判断修正
//2012-5-29 完成了show step message.
//2012-5-31 修改了step8,12 的等待方式，令判别更准确	;增加TVDP_SRCON测量结果显示
//2012-6-6  临时增加Tvbus_reapp 的时间调整，可以输入时间。
//2012-12-24 临时修改了Renegotiation - Checking Current Draw部分，暂不切换至PHY
//2018-2-26  step16. Wait 100us (for D- to fall).修改成了5ms，避免由于D-下降不够快导致误判。
//2018-3-7 step20,wait 1ms changed to wait 20ms follows MQP .

uint16_t PD_CDP_Detection_Test (void *p){

	DECLARE_VARS()
	uint16_t hasSecondaryDetection,Tvbus_reapp;  // = *(uint16_t*)p;
	uint32_t ts0,ts1,ts2,ts3,ts_DP_on;
	prepareTest();

	//Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
	SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);
	ts_DP_on=0;
	do{

	hasSecondaryDetection = askHostToInputVal("Does UUT support Secondary Detection?\r\nInput: 1 = SUPPORT,0 = NOT SUPPORT");

	Tvbus_reapp = askHostToInputVal("Please input Tvubs_reapp value (50-500)ms,default is 100.");

   // sprintf(charBuf,"hasSecondaryDetection = %d", hasSecondaryDetection) ;	
   // Report_MSG(charBuf)  ;


	  //Init_Test_Host_HS();
  Report_MSG(" PD - CDP Detection Test");
//DCD
   Report_MSG("----DCD----------------------------------")	;
	
//1. Switch data lines to PET test circuit. PET applies a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS to 5V. ID pin is left floating.

   Report_MSG("1. Switch data lines to PET test circuit. PET applies a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS to 5V. ID pin is left floating.");
   prepareVBusForPD();
   //VBUS_CAP_Setting(CAP_100uF);	  //加大电容，保证大电流稳定
   //GlobleTime = 0;	  //time reset
   ts0 = GlobleTime; //vbus valid point

//2. Connect 0V via 15k resistor to D+. Connect 0V via15k resistor to D-.
   Report_MSG("2. Connect 0V via 15k resistor to D+. Connect 0V via15k resistor to D-. ");
   powerDP(0 , RES_15K);
   powerDM(0 , RES_15K);
//Primary Detection
    Report_MSG("----Primary Detection----------------------------------") ;

//3. Wait for D+ to rise above 0.5V
	Report_MSG("3. Wait for D+ to rise above 0.5V");
	ts1 =  GlobleTime;		// TS for VDP_SRC
    tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,5500,500,10,9000) ;	//10次连续有效，500ms 超时
   if(resCapturedBig == tmp){
  	 //right

      ts1 = GlobleTime;  //VDP_SRC START POINT
	  Report_MSG("INFO:D+ rose above 0.5V.")  ;

   }else
   {
		//failed
		Report_MSG("ERRO:D+ did not rise above 0.5V.")  ;
		Report_MSG("Stop this test...");
		testResult = TEST_FAILED;
		break;
   }
//4. Wait 1ms for D+ to settle.
	Report_MSG("4. Wait 1ms for D+ to settle.");
	Delay(10);

//5. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V). [PD10]
    Report_MSG("5. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V).[PD10]");
//	ts2 = GlobleTime;
	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,50) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp)
   {
  	 //right
   		sprintf(charBuf,"INFO:D+ voltage was valid, D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }
   else{
		//failed
		sprintf(charBuf,"ERRO:D+ voltage was not valid, D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }
   //
//6. Check value of IDM_SINK, as follows: Change voltage on 15k resistor to D- to 0.6V. 
//Wait 1ms, then check voltage at D- is in correct range for a worst case IDM_SINK of 25uA 
//(i.e. that voltage is not greater than 0.225V). [PD10]
//????????
   	
  	 Report_MSG("6.Check value of IDM_SINK, as follows: Change voltage on 15k resistor to D- to 0.6V.")	 ;
     Report_MSG("Wait 1ms, then check voltage at D- is in correct range for a worst case IDM_SINK of 25uA ");
	 Report_MSG("(i.e. that voltage is not greater than 0.225V). [PD10]");

	powerDM(600, RES_15K);
	Delay(10);

	//ts2 = GlobleTime; // TS FOR vdm_src
	tmp	= WaitAdcValueMatch(ADC_DM,CaptureBig,0,225,5,10) ;	//10次连续有效，1ms 超时
	//sprintf(charBuf,"Wait result = %d" , tmp);
//	Report_MSG(charBuf)	 ;

   if(resCapturedBig == tmp)
   {
  	 //failed
		sprintf(charBuf,"ERRO:IDM_SINK(MIN) test Failed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }
   else
   {
		//Pass
		sprintf(charBuf,"INFO:IDM_SINK(MIN) test Passed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }

//7. Change voltage on 15k resistor to D- to 0V.
	Report_MSG("7. Change voltage on 15k resistor to D- to 0V.")	 ;
    powerDM(0, RES_15K);

//8. Wait for 17ms (together with 2ms delay above, this is less than TVDMSRC_EN max).
	 Report_MSG("8. Waiting until about 19ms after UUT applying VDP_SRC(this is less than TVDMSRC_EN max).")	 ;

     if(GlobleTime - ts1 < 190)
	 {   
		Delay(190 - (GlobleTime - ts1));//debug 时候，发现不能去17ms,要短一些比较保险
	 }

//9. Disconnect 15K resistor from D- and replace with 1K5 from 0.7V.
    Report_MSG("9. Disconnect 15K resistor from D- and replace with 1K5 from 0.7V.");
	powerDM(700, RES_1K5);
 
	ts2 = GlobleTime; //TS for apply vdm_src
//10. Wait 1ms for D- to settle.
	Report_MSG("10. Wait 1ms for D- to settle.");
	Delay(10);

//11. Measure voltage on D-. This must be above 0.4V, to prove that IDM_SINK max (175uA) is not exceeded.
   Report_MSG("11. Measure voltage on D-. This must be above 0.4V, to prove that IDM_SINK max (175uA) is not exceeded.");
 tmp = WaitAdcValueMatch(ADC_DM,CaptureBig,0,400,5,10) ;	//

   if (tmp == resCapturedBig)
   {
  	 //OK
	    sprintf(charBuf,"INFO:IDM_SINK(MAX) test Passed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;

   }else{
	//failed
	    sprintf(charBuf,"ERRO:IDM_SINK(MAX) test Failed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }



//12. Wait 19ms (together with 20ms delay above, this is less than TVDPSRC_ON min).
 	Report_MSG("12. Waiting until about 39ms after UUT applying VDP_SRC (this is less than TVDPSRC_ON min).")	 ;
	   if(GlobleTime - ts1 < 390)
	 {   
		Delay(390 - (GlobleTime - ts1)); //2012.5.31 修改了等待方式，更准确
	 }
	

//13. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V). [PD10]
	Report_MSG("13. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V). [PD10]")	 ;

	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp){
  	 //right
   		sprintf(charBuf,"INFO:D+ voltage was valid.D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }else{
		//failed
		sprintf(charBuf,"ERRO:D+ voltage was not valid.D+= %d mV , Please check the TVDP_SRC_ON... ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }



//14. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS going on to expire.
 Report_MSG("14. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS going on to expire.")	 ;
 ts3 = 10000-(GlobleTime - ts0)	 ;	 //ts0 = VBUS ON
 if(ts3>0)
  {
	  tmp = WaitAdcValueMatch(ADC_DP,CaptureBigOrSmall,500,800,10,ts3 ) ;	//10次连续有效，ts3 ms 超时
		//	Report_MSG("Disable VDM_SRC...");
	    //	powerDM(0 , RES_15K);
	    //	Delay(1);
	   if (tmp == resCapturedBig)
	   {
	  	 //If D+ rises instead above 0.8V, it will be regarded as a 'device connec,不用执行secondary detecton
		 //goto step 27
			Report_MSG("INFO:D+ went above 0.8V")  ;
			
			 if(hasSecondaryDetection > 0)
			 {
				  //If secondary detection is declared to be implemented, this is a failure, secondary detection
					//was not attempted. [PD14]
					Report_MSG("ERRO:this is a failure for secondaryd detection is declared to be implemented");
					testResult = TEST_FAILED;
			 }

			goto PD_CDP_STEP_26;  //enumeration
	   }
	   if (tmp == resCapturedSmall)
	   {
	  	 //End of primary detection, proceed from step 15.
			Report_MSG("INFO:D+ went below 0.5V")  ;

			//report TVDP_SRCON time
			 sprintf(charBuf,"INFO:Measured duration of TVDP_SRCON is %d ms", (GlobleTime - ts1)/10) ;
			Report_MSG(charBuf)	 ;
		//	if(hasSecondaryDetection <= 0)
		//	{
		//		goto PD_CDP_STEP_15 ;
		//	}		
	   }
	    if (tmp == resCapturedTimeOut)
		{  //failed
		    //Report_MSG("ERRO:D+ failed to go below 0.4V or above 0.8V")  ;
			Report_MSG("ERRO:UUT failed to connect within TSVLD_CON_PWD (1 sec). [PD8]")  ;
		    sprintf(charBuf,"ERRO:D+= %d mV ", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
	    	Report_MSG("Stop this test...");
		    testResult = TEST_FAILED;
			break;
		}   
   }
   else
   {
	   Report_MSG("ERRO:internal error: ts3<0 ") ;
	   testResult = TEST_FAILED;
	   break;

   }

//PD_CDP_STEP_15:

// 15.Disconnect 1K5 resistor from D- and replace with 15K from 0V.
    Report_MSG("15.Disconnect 1K5 resistor from D- and replace with 15K from 0V.");
	powerDM(0,RES_15K)	;

//16. Wait 100us (for D- to fall).
	// Report_MSG("16. Wait 100us (for D- to fall).");
	//Delay(1);
	 
	 	 Report_MSG("16. Wait 5ms (for D- to fall).");
	Delay(50);

//If secondary detection is declared not to be implemented, skip to Checking Current Draw.
	if(hasSecondaryDetection == 0 )
	{
	   	goto PD_CDP_STEP_26;  //enumeration  ;
	}

   //Secondary Detection
//17. Wait for D- to rise above 0.5V

   	Report_MSG("----Secondary Detection----------------------------------") ;
	Report_MSG("17. Wait for D- to rise above 0.5V");

   tmp = WaitAdcValueMatch(ADC_DM,CaptureBig,0,500,5,10000 -(GlobleTime - ts0) ) ;	//10次连续有效，9000ms 超时
   if (tmp == resCapturedBig)
   {
     ts2= GlobleTime; //VDM_SRC start point
	Report_MSG("INFO:D- rose above 0.5V")	 ;

   }else{
	//failed
	Report_MSG("ERRO:D- did not rise above 0.5V")	 ;
	Report_MSG("Stop this test...");
	testResult = TEST_FAILED;
	break;	//??????????
   } 
//18. Wait 1ms for D- to settle.
	Report_MSG("18. Wait 1ms for D- to settle.") ;
	Delay(10);

//19. Measure D- and check voltage is VDM_SRC (0.5 to 0.7V). [PD15]
   Report_MSG("19. Measure D- and check voltage is VDM_SRC (0.5 to 0.7V). [PD15]");

   tmp = WaitAdcValueMatch(ADC_DM,CaptureInRange,700,500,10,50 ) ;	//10次连续有效，500ms 超时
   if (tmp == resCapturedInRange)
   {
     sprintf(charBuf,"INFO:D- was valid, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;	

   }else{
	//failed
	sprintf(charBuf,"ERRO:D- was not valid, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;
	testResult = TEST_FAILED;
   } 
//20. Check value of IDP_SINK, as follows: Change voltage on 15k resistor to D+ to 0.6V.
// Wait 1ms, then check voltage at D+ is in correct range for a worst case IDP_SINK of 25uA
//(i.e. that voltage is not greater than 0.225V). [PD15]


	Report_MSG("20.Check value of IDP_SINK, as follows: Change voltage on 15k resistor to D+ to 0.6V.")	 ;
	//Report_MSG("    Wait 1ms, then check voltage at D+ is in correct range for a worst case IDP_SINK of 25uA");
	 //update to wait 20ms ,2018-03-07
	Report_MSG("    Wait 20ms, then check voltage at D+ is in correct range for a worst case IDP_SINK of 25uA");
	Report_MSG("   (i.e. that voltage is not greater than 0.225V). [PD15]");

	powerDP(800, RES_15K);
	Delay(200);

   tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,0,225,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedBig == tmp){
  	 //Failed
   		sprintf(charBuf,"ERRO:IDP_SINK(MIN) test Failed.D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }else{
		//Pass
		sprintf(charBuf,"INFO:IDP_SINK(MIN) test Passed.D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }

//21. Change voltage on 15k resistor to D+ to 0V.
   Report_MSG("21. Change voltage on 15k resistor to D+ to 0V.");
	powerDP(0 , RES_15K);


//22. Wait for 18ms (together with 21ms delays above, this is less than TVDMSRC_ON min).
  	Report_MSG("22. Waiting until about 39ms after UUT applying VDM_SRC (this is less than TVDMSRC_ON min).");
	if(390>(GlobleTime -ts2))
	{
	  Delay(390 - (GlobleTime -ts2));
	 }

//23. Check that D- voltage is still VDM_SRC (0.5 to 0.7V). [PD14] [PD15]
   Report_MSG("23. Check that D- voltage is still VDM_SRC (0.5 to 0.7V). [PD14] [PD15]")	 ;
   tmp = WaitAdcValueMatch(ADC_DM,CaptureInRange,700,500,5,10 ) ;	//10次连续有效，500ms 超时
   if (tmp == resCapturedInRange)
   {
     sprintf(charBuf,"INFO:D- was valid, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;

   }else{
	//failed
	sprintf(charBuf,"ERRO:D- was not valid, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;
	testResult = TEST_FAILED;
   }


//24. DO NOT Connect 0.6V to D+.
//?????????
   	Report_MSG("24.DO NOT Connect 0.6V to D+.");

//25. Wait for D- to go below 0.5V.
   Report_MSG("25.Wait for D- to go below 0.5V.")	 ;

       tmp = WaitAdcValueMatch(ADC_DM,CaptureSmall,500,1500,10,TO_STM_TIME_UNIT(5000)) ;	//10次连续有效，5000ms 超时
   if (tmp == resCapturedSmall)
   {
    sprintf(charBuf,"INFO:D- went below 0.5V, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;
		//report TVDP_SRCON time
	 sprintf(charBuf,"INFO:Measured duration of TVDM_SRCON is %d ms", (GlobleTime - ts2)/10) ;
	 Report_MSG(charBuf)	 ;

   }else{
	//failed
    sprintf(charBuf,"ERRO:D- did not go below 0.5V, D-= %d mV ", ADC_Captured_Value) ;
	Report_MSG(charBuf)	 ;
	testResult = TEST_FAILED;
   }



PD_CDP_STEP_26:
//Checking Current Draw
//26. If secondary detection is declared to be implemented, display message to test operative
//'PD under test should now have detected CDP'.
//If secondary detection is declared not to be implemented, display message to test
//operative 'PD under test should now have detected DCP or CDP'.
//(This is a development aid.)

    Report_MSG("----Checking Current Draw-----------------------------------------");

	if(hasSecondaryDetection > 0 ){
	   Report_MSG("26.PD under test should now have detected CDP")  ;
	}else{
	   Report_MSG("26.PD under test should now have detected DCP or CDP")  ;
	}

//27. Switch data lines to transceiver.
//	NOTHING TO DO HERE
  // GPIO_Setting(RSW_DMDP_TEST,ON);
    Report_MSG("27. Switch data lines to transceiver.");

//28. Check current drawn does not exceed 1.5A from now on. [PD30]
	 Report_MSG("28. Check current drawn does not exceed 1.5A from now on. [PD30]");
	 enableWatchBlock(0 , ADC_VBUS_I , CaptureBig ,	15000 , 10);

//29. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on. [PD8] [If
//secondary detection supported: PD18]  
	   //?????????
   Report_MSG("29. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on. [PD8]");
   Report_MSG("   (If secondary detection supported: [PD18] )	");

	    ts3 = 10000-(GlobleTime - ts0)	 ;	 //ts0 = VBUS ON
 	if(ts3>0)
	{
   	   tmp = WaitAdcValueMatch(ADC_DP,CaptureBig,500,2000,10,ts3) ;	//10次连续有效，5000ms 超时
	   if (tmp == resCapturedBig)
	   {
	   	Report_MSG("INFO:D+ goes high within TSVLD_CON_PWD (1 sec).")  ;
		sprintf(charBuf,"INFO:D+= %d mV,time = %d ms ", ADC_Captured_Value,(GlobleTime - ts0)/10) ;
		Report_MSG(charBuf)	 ;
	   }
	   else
	   {
		Report_MSG("ERRO:D+ does not go high within TSVLD_CON_PWD (1 sec).")  ;
	    sprintf(charBuf,"ERRO:D+= %d mV,time = %d ms ", ADC_Captured_Value,(GlobleTime - ts0)/10) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
	   }
	}
	else
	{
		Report_MSG("ERRO:internal error: ts3<0 ") ;
		 testResult = TEST_FAILED;
	}


//30. Enumerate UUT (at HS if possible), and Set Configuration 1.
     Report_MSG("30. Enumerate UUT (at HS if possible), and Set Configuration 1.");
     powerDP(0, RES_15K);
     powerDM(0, RES_15K);

   	GPIO_Setting(RSW_VCC_PHY,ON);   //power on the usb phy
    Init_Test_Host_HS();
    GPIO_Setting(RSW_DMDP_TEST,ON);

  //GPIO_Setting(RSW_DMDP_TEST,ON);
  Delay(50000) ;
//FIXIT ?????????
//31. Maintain session for 30 seconds from configuration.
	//Report_MSG("31. Maintain session for 30 seconds from configuration.");
  if(Enum_Done ==1)
   {
   // sprintf(charBuf,"INFO:PD Conneted,TSVLD_CON_PWD = %d ms",(PD_Connect_Ts - ts0)/10) ;	
//	Report_MSG(charBuf)	 ;  
  Report_MSG("31. Maintain session for 30 seconds from configuration.");

    ts3 =GlobleTime;
    do{
    sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,20000)/10) ;	
	Report_MSG(charBuf)	 ;
	  }while(300000>(GlobleTime - ts3))	;
   }
  else
   {
	 Report_MSG("ERRO: Enumerate Faild!")	 ;
	 testResult = TEST_FAILED;
   }
   //check current
   	if ( IsWatchBlockTriggered(0) )
	{
		 //Triggered I > 1500mA
		 //report 'Failed - Max current draw exceeded 1500mA'
		 sprintf(charBuf , "ERRO:Current = %d mA, Max current draw exceeded 1500mA", GetWatchBlockRecordVal(0)/10)  ;
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		
	}
	else 
	{
		 //not Trigger then stop WatchBlock[0]
		 Report_MSG("INFO:Max current draw not exceeded 1500mA")  ;
	//	 sprintf(charBuf,"INFO:Current = %d mA",ADC_Value_Convert_to_real(ADC_VBUS_I,Current_ADC_Value[ADC_VBUS_I])) ;	
	//	 Report_MSG(charBuf)	 ;
	}										  

	  VBUS_CAP_Setting(CAP_NONE); //将BYPASS电容断掉

//Checking Renegotiation
//32. Switch data lines to PET test circuit. Turn off VBUS and disconnect pull-down resistance from VBUS.
  	Report_MSG("-----Checking Renegotiation-----------------------------")  ;
	Report_MSG("32. Switch data lines to PET test circuit. Turn off VBUS and disconnect pull-down resistance from VBUS.")	;

	//Delay(50);

	  GPIO_Setting(RSW_DMDP_TEST,OFF);
	  GPIO_Setting(SW_VBUS_AB_M1,ON)	 ;	   //2012-5-15修改，原来的是GND_M1,导致step33有问题
	  turnOffVBus();
	  ts3 =  GlobleTime;

//33. Check that the PD discharges VBUS to less than VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms). [PD28]
	 Report_MSG("33. Check that the PD discharges VBUS to less than VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms). [PD28]");

	 tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,700,500,10,5000 ) ;	//10次连续有效，500ms 超时
   	if (tmp == resCapturedSmall)   //2012-3-36修改
	 {
	  Report_MSG("INFO: PD discharged VBUS to less than VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms).")  ;
	  sprintf(charBuf , "INFO:Time = %d ms[PD28]",(GlobleTime - ts3)/10)  ;
	  Report_MSG(charBuf)	 ;
	 }
	 else
	 {
	   Report_MSG("ERRO: PD did not discharg VBUS to less than VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms).")  ;
	   sprintf(charBuf , "ERRO:VBUS = %d mV",ADC_Captured_Value)  ;
	   Report_MSG(charBuf)	 ;
	   sprintf(charBuf , "ERRO: Time = %d ms [PD28]",(GlobleTime - ts3)/10)  ;
	   Report_MSG(charBuf)	 ;
	   testResult = TEST_FAILED;
	 }
//34. Wait for TVBUS_REAPP min (100ms).
    //Report_MSG("34. Wait for TVBUS_REAPP min (100ms).")  ;

	//for debug 2012-6-6////////////
	 sprintf(charBuf , "(DEBUG)34. Wait for TVBUS_REAPP min (input value is %d ms) ",Tvbus_reapp)  ;
	  Report_MSG(charBuf)	 ;
	Delay(Tvbus_reapp *10);
	//////////////////////////////


//35. Apply a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turn on VBUS to 5V.
	 Report_MSG("35. Apply a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turn on VBUS to 5V.");
	prepareVBusForPD();

	ts0 = GlobleTime;//vbus re turn on

//36. Connect 0V via 15k resistor to D+. Connect 0V via15k resistor to D-.
//Note: We will emulate an SDP this time to demonstrate that renegotiation was successful.
	 Report_MSG("36. Connect 0V via 15k resistor to D+. Connect 0V via15k resistor to D-.");
	 Report_MSG("    Note: We will emulate an SDP this time to demonstrate that renegotiation was successful.");
	 powerDP(0,RES_15K);
	 powerDM(0,RES_15K);
	 
// Renegotiation - Primary Detection
		Report_MSG("----Renegotiation - Primary Detection-------------------------------------------------") ;

//37. Wait for D+ to rise above 0.5V.
	  Report_MSG("37. Wait for D+ to rise above 0.5V.")	 ;
       tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,5500,500,10,9000) ;	//10次连续有效，900ms 超时
   if (tmp == resCapturedInRange)
   {
	    
	    Report_MSG("INFO:D+ rose above 0.5v.") ;

   }
   else
   {
		Report_MSG("ERRO:D+ did not rise above 0.5v.") ;
		testResult = TEST_FAILED;
	}

//38. Wait 1ms for D+ to settle.
	Report_MSG("38. Wait 1ms for D+ to settle.")	 ;
	Delay(10);

//39. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V). [PD10]
	Report_MSG("39. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V). [PD10]")	 ;

	   tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,10,50) ;	
   if (tmp == resCapturedInRange)
   {
	   sprintf(charBuf , "INFO: D+ voltage was valid, D+= %d mV",ADC_Captured_Value)  ;
	   Report_MSG(charBuf)	 ;	
   }
   else
   {
	   sprintf(charBuf , "ERRO: D+ voltage was not valid, D+= %d mV",ADC_Captured_Value)  ;
	   Report_MSG(charBuf)	 ;
	   testResult = TEST_FAILED;
	}
// Renegotiation - Checking Current Draw
    Report_MSG("----Renegotiation - Checking Current Draw-----------------------------------------")	 ;
//40. Switch data lines to transceiver.
	Report_MSG("40. Switch data lines to transceiver.")	 ;
    Report_MSG("[DEBUG]40. Do not Switch data lines to transceiver!!")	 ;
		
    // Report_MSG("Switch data lines to transceiver...")  ;
    //GPIO_Setting(RSW_DMDP_TEST,ON);
//41. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on.
	 Report_MSG("41. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on.")	 ;
		    ts3 = 10000-(GlobleTime - ts0)	 ;	 //ts0 = VBUS ON
 	if(ts3>0)
	{
   	   tmp = WaitAdcValueMatch(ADC_DP,CaptureBig,500,2000,10,ts3) ;	//10次连续有效，5000ms 超时
	   if (tmp == resCapturedBig)
	   {

	   ts_DP_on = GlobleTime;  //record DP pull on time.
	   	Report_MSG("INFO:[DEBUG]  D+ went high!!!")  ;


	   	Report_MSG("INFO:D+ went high within TSVLD_CON_PWD (1 sec).")  ;
		sprintf(charBuf,"INFO:D+= %d mV,time = %d ms ", ADC_Captured_Value,(GlobleTime - ts0)/10) ;
		Report_MSG(charBuf)	 ;
	   }
	   else
	   {
		Report_MSG("ERRO:D+ did not go high within TSVLD_CON_PWD (1 sec) at step 41")  ;
	    sprintf(charBuf,"ERRO:D+= %d mV,time = %d ms ", ADC_Captured_Value,(GlobleTime - ts0)/10) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
	   }
	}
	else
	{
		 Report_MSG("ERRO:Internal ERRO,ts3<0")  ;
		 testResult = TEST_FAILED;
	}
//42. Display message to test operator 'PD under test should now have detected SDP'.

//	 Report_MSG("PD under test should now have detected SDP)  ;
	/*	2012.12.24 re current draw debug.不切到phy,
	powerDP(0, RES_15K);
 	powerDM(0, RES_15K);
     GPIO_Setting(RSW_DMDP_TEST,ON);
     Delay(10000) ;
	  */
	   Report_MSG("PD under test should now have detected SDP")  ; 
	 Report_MSG("[DEBUG]: Should no enumeration message here");


//FIXIT ?????????
	   
//43. Wait for 150ms (D+ debounce time plus 50ms).
	 Report_MSG("43. Wait for 150ms (D+ debounce time plus 50ms).")  ;
	  /* Delay(1500);

	  */

	  Report_MSG("[DEBUG]43. Wait for 150ms from DP pull high.")  ;
	  if ((GlobleTime - ts_DP_on)<1500)
	  {
		  Delay(1500 -(GlobleTime - ts_DP_on) )	;
	  }
         
//44. Check that current drawn from VBUS does not exceed IUNIT (100mA).
		Report_MSG("44. Check that current drawn from VBUS does not exceed IUNIT (250ms AVG).")  ;

	   	  tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,2500);
   		if (tmp<1001)	//电流数据10倍，所以100mA 用1000表示
		{
	  	sprintf(charBuf,"INFO: Current draw did not exceed 100mA, I = %d mA  ", tmp/10) ;
		Report_MSG(charBuf)	 ;
		}
		else
		{
		sprintf(charBuf,"ERRO: Current draw exceeded 100mA, I = %d mA  ", tmp/10) ;
		Report_MSG(charBuf)	 ;
		}
   
//45. Turn off VBUS and disconnect pull-down resistance from VBUS.
	  	Report_MSG("45. Turn off VBUS and disconnect pull-down resistance from VBUS.")  ;

	   	turnOffVBus();

//46. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.

	Report_MSG("46.Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.")  ;
	Delay(TO_STM_TIME_UNIT(8000));
	DeInit_Test_Host();

 	 //  end of test
 	 Report_MSG("END OF CDP Detection Test")  ;
 	}while(FALSE);
		
    DeInit_Test_Host();
	GPIO_Setting(RSW_VCC_PHY,OFF);   //power off the usb phy
  	GPIO_Setting(RSW_DMDP_TEST,OFF);
	TASK_CLEAN_UP()
 }
/***************END OF 6.7 CDP Detection Test**************/



/***************begin OF 6.8 SDP Detection Test**************/

//2012-5-2 updated - fix enumeration
//2012-5-3 updated - fix overall result display (testResult = TEST_FAILED;)
//2012-5-31 增加message show ,增加了pri/sec 输入选择
//2012-7-2 取消INFO之前的TAB空格；异常退出时的结果设置；

uint16_t PD_SDPDetection(void *p){
  	 DECLARE_VARS()
	 uint16_t hasSecondaryDetection = *(uint16_t*)p;
	 uint32_t ts0,ts1,ts2,ts3;
//	 uint16_t x;
	prepareTest();
	//Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
	SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);

	do{
	hasSecondaryDetection = askHostToInputVal("Does UUT support Secondary Detection?\r\nInput: 1 = SUPPORT,0 = NOT SUPPORT");


	//Init_Test_Host_HS();
    Report_MSG(" PD - SDP Detection Test");
	  Report_MSG("----DCD----------------------------------------");
 //  1. PET applies a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS to 5V. ID pin is left floating.
	 Report_MSG("1. PET applies a pull-down resistor of ROTG_VBUS min (10k) to VBUS and turns on VBUS to 5V. ID pin is left floating.");
     prepareVBusForPD();
	 VBUS_CAP_Setting(CAP_10uF);	  //加大电容，保证大电流稳定
	 ID_RES_Setting(RES_Rid_OPEN);

	 ts0 = GlobleTime;	//TS FOR VBUS APPLY

//2. Connect 0V via15k resistor to D+. Connect 0V via15k resistor to D-.
   	Report_MSG("2. Connect 0V via15k resistor to D+. Connect 0V via15k resistor to D-.")  ;
	powerDP(0 , RES_15K);
	powerDM(0 , RES_15K);

//Primary Detection
	 Report_MSG("----Primary Detection----------------------")  ;
//3. Wait for D+ to rise above 0.5V
	Report_MSG("3. Wait for D+ to rise above 0.5V.");

   	tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,5500,500,10,9000) ;	//10次连续有效，500ms 超时
   	if(resCapturedBig == tmp)
	{
  	 //right

    ts1 = GlobleTime;  //VDP_SRC START POINT
    Report_MSG("INFO:D+ rose above 0.5V.") ;	

   }
   else
   {
		//failed
		sprintf(charBuf,"ERRO:D+ did not go above 0.5V, D+= %d mV ",ADC_Captured_Value)  ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
		Report_MSG("Stop this test...");

		break;
   }
//4. Wait 1ms for D+ to settle.
	Report_MSG("4. Wait 1ms for D+ to settle.");
	Delay(10);

//5. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V). [PD10]
	Report_MSG("5. Measure D+ and check voltage is VDP_SRC (0.5 to 0.7V). [PD10]");

	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp)
   {
  	 //right
   		sprintf(charBuf,"INFO: D+ voltage was valid. D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }
   else{
		//failed
		sprintf(charBuf,"ERRO: D+ voltage was not valid. D+= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }


//6. Check value of IDM_SINK, as follows: Change voltage on 15k resistor to D- to 0.6V. Wait 1ms,
// then check voltage at D- is in correct range for a worst case IDM_SINK of 25μA 
//( i.e. that voltage is not greater than 0.225V). [PD10]

	Report_MSG("6. Check value of IDM_SINK, as follows: Change voltage on 15k resistor to D- to 0.6V. Wait 1ms,")	 ;
	Report_MSG("   then check voltage at D- is in correct range for a worst case IDM_SINK of 25μA");
	Report_MSG("   ( i.e. that voltage is not greater than 0.225V). [PD10]");

	powerDM(600, RES_15K);
	Delay(10);

//	ts2 = GlobleTime;
	tmp	= WaitAdcValueMatch(ADC_DM,CaptureBig,0,225,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedBig == tmp)
   {
  	 //
   		
		sprintf(charBuf,"ERRO:IDM_SINK(MIN) test Failed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;

   }
   else
   {
		//right
		sprintf(charBuf,"INFO:IDM_SINK(MIN) test Passed.D-= %d mV ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }

//7. Change voltage on 15k resistor to D- to 0V.
	Report_MSG("7. Change voltage on 15k resistor to D- to 0V.");
	powerDM(0, RES_15K);

//8. Wait for 18ms (together with 21ms delay above, this is less than TVDPSRC_ON min).
	Report_MSG("8. Waiting until about 39ms after UUT applying VDP_SRC( this is less than TVDPSRC_ON min).");
    if(390 > (GlobleTime - ts1))
	{
		Delay(390 - (GlobleTime - ts1));
	}

//9. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V) [PD10]
	Report_MSG("9. Check that D+ voltage is still VDP_SRC (0.5 to 0.7V) [PD10]")	 ;

	tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,700,500,5,10) ;	//10次连续有效，5ms 超时
   if(resCapturedInRange == tmp){
  	 //right
   		sprintf(charBuf,"INFO:D+ voltage was valid. D+= %d mv", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
   }else{
		//failed
		sprintf(charBuf,"ERRO:D+ voltage was not valid. D+= %d mv ", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
   }

//10. Do not Connect 0.6V to D-.  
 	Report_MSG("10. Do not Connect 0.6V to D-.");

//11. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS
//going on to expire.
Report_MSG("11. Wait for D+ to go below 0.5V, or above 0.8V, or for TSVLD_CON_PWD (1 sec) from VBUS going on to expire.") ;

 ts2 = 10000-(GlobleTime - ts0)	 ;	 //ts0 = DUT attach 
 if(ts2>0)
  {
	  tmp = WaitAdcValueMatch(ADC_DP,CaptureBigOrSmall,500,800,10,ts2 ) ;	//10次连续有效，5000ms 超时
	   if (tmp == resCapturedBig)
	   {
	  	 //If D+ rises instead above 0.8V, it will be regarded as a legitimate 'device connect'. Skip to 'Checking Current Draw'. [PD8]
			Report_MSG("INFO:D+ went above 0.8V.");
			Report_MSG("INFO:It will be regarded as a legitimate 'device connect', Skip to 'Checking Current Draw'. [PD8]")  ;
			goto PD_SDP_CHECKING_CURRENT_DRAW;  //enumeration
	   }
	   if (tmp == resCapturedSmall)
	   {
	  	 //If secondary detection is declared not to be implemented, skip to 'Checking Current Draw'.
		 // If secondary detection is declared to be implemented, proceed with 'Secondary Detection'.
			Report_MSG("INFO:D+ went below 0.5V")  ;
			sprintf(charBuf,"INFO:Measured duration of TVDPSRC_ON is %d ms ", (GlobleTime-ts1)/10) ;
			Report_MSG(charBuf)	 ;
			if(hasSecondaryDetection <= 0)
			{
				Report_MSG("INFO:Secondary detection is declared not to be implemented, skip to 'Checking Current Draw'");
				goto PD_SDP_CHECKING_CURRENT_DRAW ;//enumeration
			}
					
	   }
	    if (tmp == resCapturedTimeOut)
		{  //This is a failure, as the UUT failed to connect within TSVLD_CON_PWD (1 sec). [PD8]

		    Report_MSG("ERRO:UUT failed to connect within TSVLD_CON_PWD (1 sec). [PD8]")  ;
		    sprintf(charBuf,"ERRO:D+= %d mV ", ADC_Captured_Value) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
			Report_MSG("Stop this test...");
			break;
		}   
   }
   else
   {
	   Report_MSG("ERRO:internal error: ts2<0 ") ;
	   testResult = TEST_FAILED;
	   break;

   }
 


//SECONDARY_DETECTION:
//Secondary Detection
//12. Note: If the primary detection identifies an SDP, then Secondary Detection may not
//occur. In this case there is no purpose in looking for Secondary Detection, even if it
//does occur. So we will go straight to 'Checking Current Draw'.
	if(hasSecondaryDetection > 0){
	   Report_MSG("--------Secondary Detection----------------- ");
	   Report_MSG("12. Note: If the primary detection identifies an SDP, then Secondary Detection may not occur.");
	   Report_MSG("In this case there is no purpose in looking for Secondary Detection, even if it does occur.");
	   Report_MSG("So we will go straight to 'Checking Current Draw'");
  
  	}
PD_SDP_CHECKING_CURRENT_DRAW:
//Checking Current Draw
//13. Display message to test operative 'PD under test should now have detected SDP. (This
//is a development aid.)
	Report_MSG("--------Checking Current Draw----------------- ");
	Report_MSG("13. PD under test should now have detected SDP.");


//14. Check current drawn does not exceed ICFG_MAX (500mA) from now on. [PD12], [PD31]
	Report_MSG("14. Check current drawn does not exceed ICFG_MAX (500mA) from now on. [PD12], [PD31]");

     WatchBlockInit();
	enableWatchBlock(0 , ADC_VBUS_I , CaptureBig ,	5000 , 10); 


//15. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on. [PD8]
	Report_MSG("15. Check D+ goes high within TSVLD_CON_PWD (1 sec) from VBUS turning on. [PD8]");
	ts2 = 10000- (GlobleTime - ts0);
     if (ts2 > 0)
	 {
	    tmp = WaitAdcValueMatch(ADC_DP,CaptureBig,500,2000, 10,ts2 ) ;	//10次连续有效，5000ms 超时
		   if (tmp == resCapturedBig)
		   {										  
			sprintf(charBuf,"INFO:D+ went high within %d ms from VBUS turning on",(GlobleTime - ts0)/10) ;	
			Report_MSG(charBuf)	 ;
		   }
		
		   else
		   {
			Report_MSG("ERRO: D+ did not go high within TSVLD_CON_PWD (1 sec).")  ;
			testResult = TEST_FAILED;
		   }
	 }
	 else
	 {
		Report_MSG("ERRO:Interal error. ts2<0.")  ;
		testResult = TEST_FAILED;
	 }


//16. Enumerate UUT (at HS if possible), and Set Configuration 1.

  Report_MSG("16. Enumerate UUT (at HS if possible), and Set Configuration 1.")  ;
  powerDP(0, RES_15K);
  powerDM(0, RES_15K);
  GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy
  GPIO_Setting(RSW_DMDP_TEST,ON);
  Init_Test_Host_HS();
   Delay(50000);
 //for debug



//17. Maintain session for 30 seconds from configuration.


   if(Enum_Done ==1)
   {
   // sprintf(charBuf,"INFO:PD Conneted,TSVLD_CON_PWD = %d ms",(PD_Connect_Ts - ts0)/10) ;	
//	Report_MSG(charBuf)	 ;  
  Report_MSG("17. Maintain session for 30 seconds from configuration.");

    ts3 =GlobleTime;
    do{
    sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,20000)/10) ;	
	Report_MSG(charBuf)	 ;
	  }while(300000>(GlobleTime - ts3))	;
   }
  else
   {
	 Report_MSG("ERRO: Enumerate Faild!")	 ;
	 testResult = TEST_FAILED;
   }

   //check current
   	if ( IsWatchBlockTriggered(0) )
	{
		 //Triggered I > 1500mA
		 //report 'Failed - Max current draw exceeded 1500mA'
		 sprintf(charBuf , "ERRO:Current = %d mA, Max current draw exceeded 500mA", GetWatchBlockRecordVal(0)/10)  ;
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		
	}
	else 
	{
		 //not Trigger then stop WatchBlock[0]
		 Report_MSG("INFO:Max current draw not exceeded 500mA")  ;

	}

//18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
		Report_MSG("18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.")  ;
		 ts2 = GlobleTime;
		  GPIO_Setting(RSW_DMDP_TEST,OFF);		   //switch DATA to measure current
		  GPIO_Setting(SW_VBUS_AB_M1,ON)	 ;	   //route VBUS to M1
		turnOffVBus();

//19. Check that VBUS falls below VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms).[PD28]
		 Report_MSG("19. Check that VBUS falls below VBUS_LKG max (0.7V) within TVLD_VLKG max (500ms).[PD28]");

	     tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,700, 0,10,5000) ;	//10次连续有效，5000ms 超时
		  
		   if (tmp != resCapturedSmall)
		   {
		   	//failed
				Report_MSG("ERRO: VBUS did not fall below VBUS_LKG max (0.7V)")  ;
				sprintf(charBuf,"ERRO:VBUS = %d mV, after %d ms of turning off vbus ", ADC_Captured_Value,(GlobleTime - ts2)/10 ) ;
				Report_MSG(charBuf)	 ;
				testResult = TEST_FAILED;
		   }
		   else
		   {
			  	Report_MSG("INFO: VBUS fall below VBUS_LKG max (0.7V)")  ;
				sprintf(charBuf,"INFO:VBUS = %d mV, after %d ms of turning off vbus ", ADC_Captured_Value,(GlobleTime - ts2)/10 ) ;
				Report_MSG(charBuf)	 ;
		   }
//20. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
   	Report_MSG(" Wait 8 seconds, ignoring SRP pulse, for detachment to be detected");
	Delay(80000);
//End of Test
}while(FALSE);

	DeInit_Test_Host();
	GPIO_Setting(RSW_VCC_PHY,OFF);   //power up the usb phy
  	GPIO_Setting(RSW_DMDP_TEST,OFF);

	TASK_CLEAN_UP();
 }
/***************END OF 6.8 SDP Detection Test**************/


/*********** 6.9 ACA-Dock Detection Test **********/
//2012-7-3 updated	 report porcdeure	,测试设备枚举有问题

uint16_t PD_ACA_DockDetection(void *p){
	 DECLARE_VARS()
	 uint16_t vdp,vdm,se0_flag;
	 uint32_t ts0;
	 uint8_t curIdRes=RES_Rid_A_min;
 	  prepareTest();

	do{	
	
//1. Switch data lines to PET test circuit. Set up transceiver (now not connected) to
//peripheral mode with 1k5 pull-up to 3.3V on D+, and 15k pull-down on D-.
	   Report_MSG("1. Switch data lines to PET test circuit. Set up transceiver (now not connected)");
	   Report_MSG("  to peripheral mode with 1k5 pull-up to 3.3V on D+, and 15k pull-down on D-.");
	   	
		GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy
	    Init_Test_Device_FS();
	    //powerDP(3300,RES_1K5);
	   // powerDM(0,RES_15k);

//2. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min
//(10k) to VBUS and turns on VBUS to 5V.
		Report_MSG("2. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min");
		Report_MSG("(10k) to VBUS and turns on VBUS to 5V.");

   	   	VBUS_CAP_Setting(CAP_6uF5);
	 	PULL_DOWN_RES_Setting(RES_10K);
		GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
		DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
		DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV

//3. Wait 10ms.
		Report_MSG("3. Wait 10ms.");
	  	Delay(100);

//4. Simultaneously
		Report_MSG("4. Simultaneously");

	//? Connect ID pin to ground via RID_A min (122k).
	   Report_MSG(" - Connect ID pin to ground via RID_A min (122k).") ;
	   ID_RES_Setting(curIdRes);
	//? Connect 0.6V via 200R to D-.
		Report_MSG(" - Connect 0.6V via 200R to D-.");
		powerDM(600, RES_200R);
	//? Connect 3.3V via 1K5 to D+.
		Report_MSG(" - Connect 3.3V via 1K5 to D+.");
		powerDP(3300, RES_1K5);

//5. Check that D+ remains above VLGC_HI min (2.0V) for TDCD_TIMEOUT min (0.3 sec).
		Report_MSG("5. Check that D+ remains above VLGC_HI min (2.0V) for TDCD_TIMEOUT min (0.3 sec).");

//Note: DCD using current source is not available in this case.
		Report_MSG("  Note: DCD using current source is not available in this case.");  

		tmp = WaitAdcValueMatch(ADC_DP,CaptureSmall,2000, 5000,10,3000) ;	//10次连续有效，5000ms 超时
		   if (tmp == resCapturedSmall)
		   {
		   	//failed  
			sprintf(charBuf ,"ERRO:D+ did not remain above 2.0v for 0.3s, D+ = %d mV", ADC_Captured_Value)  ;
		 	Report_MSG(charBuf);
		   }
		   else
		   {
			sprintf(charBuf ,"INFO: D+ did remain above 2.0v for 0.3s, D+ = %d mV", ADC_Captured_Value)  ;
		 	Report_MSG(charBuf);
		   }



//6. Display message to test operative 'PD under test should now have detected ACADock'.
//(This is a development aid, and may appear up to 0.4sec before actual
//detection.)
   Report_MSG("PD under test should now have detected ACADock")	 ;

//7. Check that SE0 occurs within TA_BCON_ARST max (30 sec).
   Report_MSG("7. Check that SE0 occurs within TA_BCON_ARST max (30 sec).");


//FIXIT?
       ts0 = GlobleTime;
	   se0_flag = 0;
	  //  powerDM(0, RES_NONE);
	//	powerDP(0, RES_NONE);

	   do {
			   vdp = Get_Avarage_ADC_Value(ADC_DP,5);	//10次连续
			   vdm = Get_Avarage_ADC_Value(ADC_DM,5);
		       if (vdp<800&&vdm<800)
			   {
					 vdm = Get_Avarage_ADC_Value(ADC_DM,5);	//check again
			         vdp = Get_Avarage_ADC_Value(ADC_DP,5);
					 if(vdp<800&&vdm<800)
					 {
					  se0_flag = 1;
					 }
			   }

         }while((GlobleTime - ts0 < 300000) && se0_flag ==0)	;

		 if(se0_flag ==1)  //
		 {
		  Report_MSG("INFO: SE0 detected")	 ;

		//8. On detecting SE0, switch data lines to transceiver. This has the effect of disconnecting
		//the 0.6V from D-, and replaces the test circuit pull-up with the transceiver pull-up.
		  Report_MSG("8. On detecting SE0, switch data lines to transceiver. This has the effect of disconnecting")	 ;
		  Report_MSG("the 0.6V from D-, and replaces the test circuit pull-up with the transceiver pull-up.");
		  
		 // powerDM(0, RES_15K);
		  //powerDP(3300, RES_1K5);
		  
		  GPIO_Setting(RSW_DMDP_TEST, ON);

	    //9. Check that UUT enumerates the PET within 30 seconds. PET responds as test device
		//0x1A0A/0x0200. [PD21]

		 Report_MSG("9. Check that UUT enumerates the PET within 30 seconds. PET responds as test device 0x1A0A/0x0200.");
		 Report_MSG("   PET responds as test device 0x1A0A/0x0200. [PD21] ");

		//10. From now on check current drawn and report. (If current exceeds 1.5A test will
		//terminate to protect tester connectors.) [PD30]
		 Report_MSG("10. From now on check current drawn and report. (If current exceeds 1.5A test will");
		 Report_MSG("   terminate to protect tester connectors.) [PD30]");
		 
		 	//11. Wait 10 seconds.
		 Report_MSG("11. Wait 10 seconds.");
		 	  
		ts0 = GlobleTime;
		   do{
			    sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10) ;	
				Report_MSG(charBuf)	 ;
			  }while(300000>(GlobleTime - ts0))	;

	
		 }
		 else
		 {
		  Report_MSG("ERRO: SE0 not detected.")	 ; 
		  testResult = TEST_FAILED;
		  Report_MSG("End This Test.");
	    	break;

		 }	




//12. Simulate a detach as follows:
    Report_MSG("12. Simulate a detach as follows:");
//? Switch off transceiver D+ pullup
	Report_MSG(" - Switch off transceiver D+ pullup");
//? Switch off transceiver pull-down from D-.
	Report_MSG(" - Switch off transceiver pull-down from D-.");
	
//? Disconnect RID_A from ID pin
	Report_MSG(" - Disconnect RID_A from ID pin");
	
//? Disconnect 0.6V via 200R from D-.
	Report_MSG(" - Disconnect 0.6V via 200R from D-.");
	
//? Disconnect data line test circuit 3.3V via 1K5 from D+
	Report_MSG("- Disconnect data line test circuit 3.3V via 1K5 from D+");
	
//? Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
	Report_MSG(" - Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS. ");


	GPIO_Setting(RSW_DMDP_TEST, OFF);
	ID_RES_Setting(RES_Rid_OPEN);
	powerDM(0 , RES_NONE);
	powerDP(0 , RES_NONE);
	turnOffVBus();


//13. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
	Report_MSG("13. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.");
	Delay(80000);
//14. Repeat steps 1-13 using RID_A max (126k).

		if(curIdRes != RES_Rid_A_max)
	{	
		Report_MSG("14. Repeat steps 1-13 using RID_A max (126k).");
		curIdRes = RES_Rid_A_max;
		 Report_MSG("---------------------------------------------------") ;
		 Report_MSG("Repeat testing using RID_A max");
		continue;
	}
	else
	{
	  	Report_MSG("End This Test");
		break;
	} 

//End of Test	
	}while(TRUE);

	TASK_CLEAN_UP()
}
/*********** END OF 6.9 ACA-Dock Detection Test **********/


/*********** 6.10 ACA-A Detection Test **********/
//2012-07-02 updated with new procedure and report procdure

uint16_t PD_ACA_A_Detection(void *p){
	 DECLARE_VARS()
	
	uint32_t ts0;
 	uint8_t curIdRes=RES_Rid_A_min;
	prepareTest();
	   
	do{	
	    tmp++;
	    GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy
	    Init_Test_Device_HS();
 //1. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min
//(10k) to VBUS and turns on VBUS to 5V
	  Report_MSG("1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min");
	  Report_MSG("  (10k) to VBUS and turns on VBUS to 5V");

	   VBUS_CAP_Setting(CAP_6uF5);
	   PULL_DOWN_RES_Setting(RES_10K);
	   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
	   DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV


//2. Wait 10ms to simulate plug insertion.
	 Report_MSG("2. Wait 10ms to simulate plug insertion.");
	 Delay(100);

//3. Connect ID pin to ground via RID_A min (122k).

	
  	if(curIdRes == RES_Rid_A_min )
	{
 	Report_MSG("3. Connect ID pin to ground via RID_A min (122k)...") ;
	}
	else
	{
	 Report_MSG("3. Connect ID pin to ground via RID_A Max(126k)...") ;
	}
	ID_RES_Setting(curIdRes);

//4. Display message to test operative 'PD under test should now have detected ACA-A'.
//(This is a development aid, and may appear slightly before actual detection.)

   Report_MSG("4. PD under test should now have detected ACA-A");

//5. Connect using D+.
 	Report_MSG("5. Connect using D+.");
    GPIO_Setting(RSW_DMDP_TEST, ON);

//6. Check that UUT resets and enumerates the PET within 30 seconds. PET responds as
//test device 0x1A0A/0x0200. [PD22]

	Report_MSG("6. Check that UUT resets and enumerates the PET within 30 seconds.");
	Report_MSG("  PET responds as est device 0x1A0A/0x0200. [PD22]");

//7. From now on check current drawn and report. (If current exceeds 1.5A test will
//terminate to protect tester connectors.) [PD30]
	Report_MSG("7. From now on check current drawn and report. ");
	Report_MSG(" (If current exceeds 1.5A test will terminate to protect tester connectors.)[PD30]");
    ts0 = GlobleTime;

	  do{
		    sprintf(charBuf,"INFO: Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10) ;	
			Report_MSG(charBuf)	 ;
		  }while(300000>(GlobleTime - ts0))	;

//8. Wait 10 seconds.
   Report_MSG("8. Wait 10 seconds.");

//9. Disconnect using D+.

	Report_MSG("9. Disconnect using D+.");
	GPIO_Setting(RSW_DMDP_TEST, OFF);

//10. Disconnect resistor from ID pin.
	Report_MSG("0. Disconnect resistor from ID pin.");
	ID_RES_Setting(RES_Rid_OPEN);

//11. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
	Report_MSG("11. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");
	turnOffVBus();

//12. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
    Report_MSG("12. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.");
    Delay(TO_STM_TIME_UNIT(8000	));
//13. Repeat steps 1-12 using RID_A max (126k).
   
   Report_MSG("");

	if(curIdRes != RES_Rid_A_max)
	{
		curIdRes = RES_Rid_A_max;
		 Report_MSG("---------------------------------------------------") ;
		 Report_MSG("Repeat testing using RID_A max");
		continue;
	}
	else
	{
	  	Report_MSG("End This Test");
		break;
	} 

	}while(TRUE);
	TASK_CLEAN_UP()
}
/*********** END OF 6.10 ACA-A Detection Test **********/

/*********** 6.11 ACA-B Detection Test **********/
//2012-07-02 updated with new procedure. report porcedure.

uint16_t PD_ACA_B_Detection(void* p){
   DECLARE_VARS()
   uint8_t curIdRes = RES_Rid_B_min;
   uint32_t ts0;
 	do{
	  Report_MSG("----------DCD---------------");
	//1. Switch data lines to PET test circuit.
	  Report_MSG("1. Switch data lines to PET test circuit.");

//2. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min
//(10k) to VBUS and turns on VBUS to 5V.
	Report_MSG("2. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min");
	Report_MSG("(10k) to VBUS and turns on VBUS to 5V.");
	VBUS_CAP_Setting(CAP_6uF5);
	PULL_DOWN_RES_Setting(RES_10K);
	GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
	DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV


//3. Wait 10ms to simulate plug insertion.
    Report_MSG("3. Wait 10ms to simulate plug insertion.");
	Delay(100);

//4. Connect ID pin to ground via RID_B min (67k).
	Report_MSG("4. Connect ID pin to ground via RID_B min (67k).");
	ID_RES_Setting(curIdRes);

//5. Connect 0V via 15k resistor to D+. Connect 0V via 15k resistor to D-.
	Report_MSG("5. Connect 0V via 15k resistor to D+. Connect 0V via 15k resistor to D-.");
	powerDP(0, RES_15K);
	powerDM(0, RES_15K);

//6. Start timer of TSVLD_CON_PWD (1 sec).
	Report_MSG("6. Start timer of TSVLD_CON_PWD (1 sec).");
    ts0 =GlobleTime;

//7. Wait for D+ to exceed VLGC min, or timer to expire.
    Report_MSG("7. Wait for D+ to exceed VLGC min, or timer to expire.");
	tmp = WaitAdcValueMatch(ADC_DP,CaptureBig,3000,800,2,10000) ;
	if(tmp == resCapturedBig)
	{
//D+ rises above 0.8V:
//Check that D+ goes below 0.8V within no more than TB_DATA_PLS max (10ms). This
//represents a valid SRP pulse, which we will ignore. Proceed to ' Checking Current Draw'
//Else UUT has attempted to connect – FAIL. [PD8]
        Report_MSG("INFO:D+ rise above 0.8V") ;
		tmp = WaitAdcValueMatch(ADC_DP,CaptureSmall,800,2000,2,100) ;
			if(tmp == resCapturedSmall){
			   Report_MSG("INFO: D+ go below 0.8V within no more than TB_DATA_PLS max(10ms)") ;
			   Report_MSG("INFO: It's a valid SRP pulse") ;
			   goto PD_ACA_B_CHECKING_CURRENT_DRAW;
			}else{
				//test failed
			   Report_MSG("ERRO: UUT attempted to connect - FAIL.[PD8]");
			   testResult = TEST_FAILED;
			  // meetError = TRUE;
			  // break;									
			}
	}else{
	//timer expire
		Report_MSG("INFO:TSVLD_CON_PWD(1s) expired,D+ didn't exceed VLGC min(0.8v).");
		goto PD_ACA_B_CHECKING_CURRENT_DRAW;
	}

//Timer TSVLD_CON_PWD expires:
//Proceed to ' Checking Current Draw'

//Primary Detection
Report_MSG("----------Primary Detection---------------");
//Primary detection is optional, and its parameters are measured elsewhere, so we will ignore
//it during this test.
Report_MSG("Primary detection is optional, and its parameters are measured elsewhere, so we will ignore it during this test.");

//Checking Current Draw
PD_ACA_B_CHECKING_CURRENT_DRAW:

Report_MSG("----------Checking Current Draw---------------");

//8. Display message to test operative 'PD under test should now have detected RID_B'.
//(This is a development aid.)
Report_MSG("8. PD under test should now have detected RID_B");

//9. From now on check current drawn and report. (If current exceeds 1.5A test will
//terminate to protect tester connectors.) [PD30]
	Report_MSG("9. From now on check current drawn and report. (If current exceeds 1.5A test will");
	Report_MSG("  terminate to protect tester connectors.) [PD30]");
	enableWatchBlock(0 , ADC_DP , CaptureBig ,250 , 100);

//10. Check D+ is held below VDAT_REF min (0.25V) from now on. It is possible that SRP
//pulses are attempted, these should be ignored if they do not exceed TB_DATA_PLS max
//(10ms). [PD24]
	  Report_MSG("10. Check D+ is held below VDAT_REF min (0.25V) from now on. It is possible that SRP");
	  Report_MSG("  pulses are attempted, these should be ignored if they do not exceed TB_DATA_PLS max (10ms)");
//11. Maintain session for 15 seconds from step 6.
	  Report_MSG("11. Maintain session for 15 seconds from step 6.")  ;

    do{
    sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10) ;	
	Report_MSG(charBuf)	 ;
	  }while(150000>(GlobleTime - ts0))	;


//check watchblock

if ( IsWatchBlockTriggered(0))
	{
	 //Triggered I > 1500mA
	 //report 'Failed - Max current draw exceeded 1500mA'
	 Report_MSG("ERRO: D+ was not held below VDAT_REF min (0.25V)");
	 sprintf(charBuf,"ERRO: D+ = %d mV",GetWatchBlockRecordVal(0)) ;	
	 Report_MSG(charBuf)	 ;
	  testResult = TEST_FAILED;
		
	}
	else 
	{
	 //not Trigger then stop WatchBlock[0]
	 Report_MSG("INFO: D+ was not held below VDAT_REF min (0.25V)")  ;
     sprintf(charBuf,"ERRO: D+ = %d mV",GetWatchBlockRecordVal(0)) ;	
	 Report_MSG(charBuf)	 ;
	}

//12. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
	Report_MSG("12. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");
	turnOffVBus();

//13. Disconnect 15k pulldown resistor from D+ and D-.
	Report_MSG("13. Disconnect 15k pulldown resistor from D+ and D-.");
	DAC_DP_Output_Res(RES_NONE)	;
	DAC_DM_Output_Res(RES_NONE)	;

//14. Disconnect ID pin resistor.
    Report_MSG("14. Disconnect ID pin resistor.");
	ID_RES_Setting(RES_Rid_OPEN);

//15. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected
    Report_MSG("15. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected")  ;
	Delay(TO_STM_TIME_UNIT(8000	));

//16. Repeat steps 1-15 using RID_B max (69K).
	if(curIdRes != RES_Rid_B_max){
		curIdRes = RES_Rid_B_max;
		Report_MSG("----------------------------------------");
	    Report_MSG("Repeat testing using RES_Rid_B_max");
		continue;
	}else{
		Report_MSG("End This Teset");
		break;
	}
//17. Disconnect data lines from PET test circuit.
//End of Test
  }while(TRUE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.11 ACA-B Detection Test **********/



/*********** 6.12 ACA-C Detection Test **********/
//2012-07-01 updated, report procedure

uint16_t PD_ACA_C_Detection(void* p){
   DECLARE_VARS()
   uint32_t ts0;
   uint16_t tmp2;
   uint8_t curIdRes = RES_Rid_C_min;
 	do{
	Report_MSG("--------------DCD-------------------");

	GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy
	Init_Test_Host_HS();

	//1. Switch data lines to PET test circuit.
	Report_MSG("1. Switch data lines to PET test circuit.");

//2. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min
//(10k) to VBUS and turns on VBUS to 5V
    Report_MSG("2. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min");
	Report_MSG("(10k) to VBUS and turns on VBUS to 5V.");
	VBUS_CAP_Setting(CAP_6uF5);
	PULL_DOWN_RES_Setting(RES_10K);
	GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
	DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV

//3. Wait 10ms to simulate plug insertion.
	 Report_MSG("3. Wait 10ms to simulate plug insertion.");
	Delay(100);

//4. Connect ID pin to ground via RID_C min (36k).
	Report_MSG("4. Connect ID pin to ground via RID_C min (36k).");
	ID_RES_Setting(curIdRes);

//5. Connect 0V via15k resistor to D+. Connect 0V via15k resistor to D-.
	Report_MSG("5. Connect 0V via15k resistor to D+. Connect 0V via15k resistor to D-.");
	powerDP(0, RES_15K);
	powerDM(0, RES_15K);

//6. Start timer of TSVLD_CON_PWD (1 sec).
	Report_MSG("6. Start timer of TSVLD_CON_PWD (1 sec).");
	ts0 = GlobleTime;

//7. Wait for D+ to exceed VLGC min, or timer to expire.
	Report_MSG("7. Wait for D+ to exceed VLGC min, or timer to expire.");
	tmp = WaitAdcValueMatch(ADC_DP, CaptureBig,0, 800,10,10000) ;
	if( resCapturedBig == tmp)
	{
		//OK
		//D+ rises above 0.8V:
		sprintf(charBuf,"INFO: D+ rose above 0.8V, D+= %d", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;

		//Check that D+ is high for more than 10ms. If not this is a failure (SRP not allowed).
		//[PD25]
	    Delay(100);
		tmp2 = Get_Avarage_ADC_Value(ADC_DP,10);
		if (tmp2<800)
		{
		 Report_MSG("ERRO: SRP detected, This is a failure (SRP not allowed). [PD25]");
		  testResult = TEST_FAILED;
		}
		else
		{
		sprintf(charBuf,"INFO: D+ was high for more than 10ms, D+= %d", ADC_Captured_Value) ;
		Report_MSG(charBuf)	 ;
		}
	}
	else
	{
		//Timer TSVLD_CON_PWD expires:
		//UUT failed to connect – FAIL [PD8] [PD25]	
		Report_MSG("ERRO: TSVLD_CON_PWD expires,UUT failed to connect – FAIL [PD8] [PD25]");
		testResult = TEST_FAILED;
	}


//Primary Detection
Report_MSG("----------Primary Detection---------------");
//Primary detection is optional, and its parameters are measured elsewhere, so we will ignore
//it during this test.
Report_MSG("Primary detection is optional, and its parameters are measured elsewhere, so we will ignore it during this test.");



//CHECKING_CURRENT_DRAW:
Report_MSG("----------CHECKING_CURRENT_DRAW---------------");

//8. Switch data lines back to transceiver.
	Report_MSG("8. Switch data lines back to transceiver.");
	ts0 = GlobleTime;
    GPIO_Setting(RSW_DMDP_TEST, ON);

//9. Display message to test operative 'PD under test should now have detected RID_C'.
//(This is a development aid.)

	Report_MSG("9. PD under test should now have detected RID_C");

//10. From now on check current drawn and report. (If current exceeds 1.5A test will
//terminate to protect tester connectors.) [PD30]
	Report_MSG("10. From now on check current drawn and report. (If current exceeds 1.5A test will");
	Report_MSG("  terminate to protect tester connectors.) [PD30]");
	//enableWatchBlock(0 , ADC_DP , CaptureBig ,250 , 100);

//11. Enumerate UUT and Set Configuration 1. [PD25]
//fix
	Report_MSG("11. Enumerate UUT and Set Configuration 1. [PD25]");

	
//12. Maintain session for 10 seconds from step 8.

	Report_MSG("12. Maintain session for 10 seconds from step 8.");
    do{
    sprintf(charBuf,"INFO: Avarage Current = %d mA",Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10) ;	
	Report_MSG(charBuf)	 ;
	  }while(100000>(GlobleTime - ts0))	;

	//13. Disconnect 15k pulldown resistor from D+ and D-.
	Report_MSG("13. Disconnect 15k pulldown resistor from D+ and D-.");
	DAC_DM_Output_Res(RES_NONE);
	DAC_DP_Output_Res(RES_NONE);

//14. Disconnect ID pin resistor.
	Report_MSG("14. Disconnect ID pin resistor.");
	ID_RES_Setting(RES_Rid_OPEN);

//15. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
	Report_MSG("15. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");
	turnOffVBus();
	GPIO_Setting(RSW_DMDP_TEST, OFF);

//16. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
	Report_MSG("16. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.");
	Delay(80000);

//17. Repeat steps 1-15 using RID_C max (37K)

	if(curIdRes != RES_Rid_C_max){
		curIdRes = RES_Rid_C_max;
		Report_MSG("----------------------------------------");
	    Report_MSG("Repeat testing using RES_Rid_C_max");
		continue;
	}else{
		 Report_MSG("End This Test");
		break;
	}
//End of Test
  }while(TRUE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.12 ACA-C Detection Test **********/

/*********** 6.13 ACA-GND Detection Test **********/
//2012-7-2 updated	 report porcdeure


uint16_t PD_ACA_GND_Detection(void* p){
   DECLARE_VARS()

 	do{
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_VBUS_M1 to monitor Vbus
	//1. Connect 6.5μF from VBUS to ground.
	Report_MSG("1. Connect 6.5uF from VBUS to ground.");

	VBUS_CAP_Setting(CAP_6uF5);

//2. Connect ID pin to ground via RID_GND max (1k).
	Report_MSG("2. Connect ID pin to ground via RID_GND max (1k).");
	ID_RES_Setting(RES_Rid_GND);

//3. Wait 3 seconds.
	Report_MSG("3. Wait 3 seconds.");
	Delay(TO_STM_TIME_UNIT(3000	));

//4. If VBUS is on, wait for it to go off, and stay off for 5 seconds.
	Report_MSG("4. If VBUS is on, wait for it to go off, and stay off for 5 seconds.");
	if (Get_Avarage_ADC_Value(ADC_VBUS_AB,10)>800)
	{
		 Report_MSG("INFO: Waiting VBUS off..."); 
		 tmp = WaitAdcValueMatch(ADC_VBUS_V,0, 800, 0, 100, TO_STM_TIME_UNIT(5000)) ;
		if( resCapturedSmall == tmp)
		{
			//VBus go off
			Delay(49500);
			if(Get_Avarage_ADC_Value(ADC_VBUS_AB,500)<800) 
			{
				Report_MSG("INFO:VBUS has off for 5 sec");
			 }
			 else
			 {
			   Report_MSG("ERRO: VBus did not go off after 5 sec");
			   testResult = TEST_FAILED;
			 }
		 }
		else
		{
		  Report_MSG("ERRO: VBus did not go off");
		  testResult = TEST_FAILED;
		}
	 }
	 else
	 {
		Report_MSG("INFO: VBus did go off");
	 }

//This deals with ADP and any initial test for a peripheral.
//5. Perform SRP for 7.5ms pulse.

   Report_MSG("5. Perform SRP for 7.5ms pulse.");

	powerDP(3300,RES_1K5);
	Delay(75);
	powerDP(0,RES_NONE);

//6. Check that VBUS rises above VOTG_SESS_VLD within TA_SRP_RSPNS of rising edge of
//SRP pulse. [PD26]
//fix

	 Report_MSG("6. Check that VBUS rises above VOTG_SESS_VLD within TA_SRP_RSPNS of rising edge of SRP pulse. [PD26]");
	 tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig, 800, 0, 100, TO_STM_TIME_UNIT(4900 - 75)) ;
	if( resCapturedBig == tmp)
	{
	   Report_MSG("INFO: VBUS went above 0.8v.");

	}
	else
	{
	  Report_MSG("ERRO: VBUS did not go above 0.8v.");
	  testResult = TEST_FAILED;
	}
		
//7. Disconnect RID_GND, and capacitance and resistance from VBUS.
	Report_MSG("7. Disconnect RID_GND, and capacitance and resistance from VBUS.");
	ID_RES_Setting(RES_Rid_OPEN);
	VBUS_CAP_Setting(CAP_NONE);

//8. Wait for VBUS to go off, and stay off for 5 seconds.
		Report_MSG("8. Wait for VBUS to go off, and stay off for 5 seconds.");
		 tmp = WaitAdcValueMatch(ADC_VBUS_V,CaptureSmall, 800, 0, 100, TO_STM_TIME_UNIT(5000)) ;
		if( resCapturedSmall == tmp)
		{
			//VBus go off
			Delay(49500);
			if(Get_Avarage_ADC_Value(ADC_VBUS_AB,500)<800) 
			{
				Report_MSG("INFO:VBUS did go off for 5 sec.");
			 }
			 else
			 {
			   Report_MSG("ERRO: VBus did not go off after 5 sec.");
			   testResult = TEST_FAILED;
			 }
		 }
		else
		{
		  Report_MSG("ERRO: VBus did not go off");
		  testResult = TEST_FAILED;
		}

		Report_MSG("End of This Test.")  ;
//End of Test
  }while(FALSE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.13 ACA-GND Detection Test **********/


/*********** 6.16 Dead Battery Provision Test **********/
//2012-7-3 UPDATED		to be conntinue
//2012-12-30 , 注意2.5mA电流的测量，要减去10K pull down电阻（0.5mA）
uint16_t PD_DeadBatteryProvision(void *p){
	DECLARE_VARS()
	uint32_t ts0;
	uint16_t tmp_i,tmp_dp,bMaxPower;
	float fCurrent;
	prepareTest();


do{
		tmp++;
		GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy
//	Init_Test_Host_HS();

	//repMsg("begin Dead Battery Provision Test");

   //1. PET applies CADP_VBUS max (6.5μF) and a pull-down resistor of ROTG_VBUS min (10k)
//to VBUS and turns on VBUS to 5V. ID pin is left floating.
	 Report_MSG("1. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min (10k)");
	 Report_MSG("to VBUS and turns on VBUS to 5V. ID pin is left floating.");

	 VBUS_CAP_Setting(CAP_6uF5);	 
	PULL_DOWN_RES_Setting(RES_10K);
	GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
	DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	ID_RES_Setting(RES_Rid_OPEN);


//2. Connect 0V via 15k resistor to D+. Connect 0V via 15k resistor to D-.
	Report_MSG("2. Connect 0V via 15k resistor to D+. Connect 0V via 15k resistor to D-.");
	powerDP(0,RES_15K);
	powerDM(0,RES_15K);


//3. Wait TSVLD_CON_PWD max plus a margin (1 sec + .5 sec = 1.5 sec).
	Report_MSG("3. Wait TSVLD_CON_PWD max plus a margin (1 sec + .5 sec = 1.5 sec).");
	Delay(15000);

//4. Set up a watch-block to monitor the current drawn from VBUS, and be triggered if this
//current exceeds IUNIT (100mA).
//	enableWatchBlock(0 , ADC_VBUS_I , CaptureBig ,	1000  , 10);
   

	Report_MSG("4. Set up a watch-block to monitor the current drawn from VBUS,");
	Report_MSG("  and be triggered if this current exceeds IUNIT (100mA).")  ;

	 WatchBlockInit();
	 enableWatchBlock(0 , ADC_VBUS_I , CaptureBig ,	1000 , 250); 

//5. Check that D+ is below VLGC min (0.8v). If UUT connects at any time during the
//remainder of the test, skip to Step15. We must assume that the dead battery provision
//is no longer required by the UUT.

	Report_MSG("5. Check that D+ is below VLGC min (0.8v). If UUT connects at any time during the");		
	Report_MSG("   remainder of the test, skip to Step15. We must assume that the dead battery provision")  ;
	Report_MSG("   is no longer required by the UUT.")  ;
	

     tmp_dp = Get_Avarage_ADC_Value(ADC_DP,20);		  //avarage 2ms for DP
	 if(tmp_dp < 800)
	 {
	    sprintf(charBuf,"INFO:D+ = %d mV",tmp_dp) ;	
    	Report_MSG(charBuf)  ;
	 }
	 else
	 {
		sprintf(charBuf,"INFO:D+ = %d mV",tmp_dp) ;	
    	Report_MSG(charBuf)  ;
		Report_MSG("Skip to Step15");
		goto DBP_TEST_STEP_15 ;
	 }
       

//6. Display message to test operative 'PD under test should now have detected SDP'. (This
//is a development aid.)
	 Report_MSG("6. PD under test should now have detected SDP.")  ;

	//7. For the next 30 seconds, follow the procedure in steps 8-10
	Report_MSG("7. For the next 30 seconds, follow the procedure in steps 8-10");

    Report_MSG("8. Measure the average current drawn from VBUS, sampling every 1ms for a period of 1");
    Report_MSG("  second. Record whether this value exceeds ISUSP (2.5mA).");
    Report_MSG("9. If the value does exceed ISUSP for two consecutive averages, then check that D+ is at");
	Report_MSG("   VDP_SRC (0.5V-0.7V).");

	 //10. Check that the watch-block has not been triggered.
	 Report_MSG("10. Check that the watch-block has not been triggered.");
		 //没有用WB，因为上面已有平均值判断

	ts0 = GlobleTime;
//	Set_I_AMP_MODE(CURRENT_OUT,MODE_X1);
// 发现如果在此重新设置AMP_MODE,会导致小电流测试不准确？

	while((GlobleTime - ts0) < 300000)
	{
	  	//8. Measure the average current drawn from VBUS, sampling every 1ms for a period of 1
		//second. Record whether this value exceeds ISUSP (2.5mA).
		//9. If the value does exceed ISUSP for two consecutive averages, then check that D+ is at
		//VDP_SRC (0.5V-0.7V).
        
		fCurrent =(float) Get_Avarage_ADC_Value(ADC_VBUS_I,2500)/10;  //avarage 1 msec for VBUS_I
		  fCurrent = fCurrent - 0.5; //(plus 0.5mA pull down resistor current)
		tmp_dp = Get_Avarage_ADC_Value(ADC_DP,20);		  //avarage 2ms for DP
        sprintf(charBuf,"INFO:I = %3.1f mA, D+ = %d",fCurrent,tmp_dp) ;	
    	Report_MSG(charBuf)  ;
		if(fCurrent>100.5)
		{
		 Report_MSG("ERRO: Current exceeded Iuint!") ;
		 testResult = TEST_FAILED;

		 SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
		}
		if(fCurrent>=2.5 && tmp_dp<500)
		{
		 Report_MSG("ERRO: D+ is not at Vdp_src when current exceed Isusp");
		 testResult = TEST_FAILED;
		}
		if(tmp_dp>800)
		{
		 Report_MSG("ERRO:D+ exceeded VLGC min,the DBP is no longer required") ;	
		 goto  DBP_TEST_STEP_15;
		}
		Delay(8000); 
					
	}


//11. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
		Report_MSG("11. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");
	    GPIO_Setting(RSW_VBUS_AB,OFF); 
	  DAC_Setting(DAC_VBUS_V,0);  //VBUS_V = 0
	  VBUS_CAP_Setting(CAP_NONE);
	  PULL_DOWN_RES_Setting(RES_NONE) ;
	   

//12. Disconnect 15k pull-down resistors.
		Report_MSG("12. Disconnect 15k pull-down resistors.");
  	 	powerDP(0,RES_NONE);
	   powerDM(0,RES_NONE);

//13. Wait 5 seconds for detachment to be recognized.
		Report_MSG("13. Wait 5 seconds for detachment to be recognized.");
 	 Delay(50000);
//14. Skip to End of Test
		Report_MSG("14. Skip to End of Test");
	 goto  DBP_END_TEST;




DBP_TEST_STEP_15:

   Report_MSG("---------------Enumerate UUT--------------------")  ;
//15. Wait TA_BCON_LDB min (100ms) then issue a bus reset to the B-UUT.
	
	Report_MSG("15. Wait TA_BCON_LDB min (100ms) then issue a bus reset to the B-UUT."); 	
	Delay(1000);
  	powerDP(0, RES_15K);
  	powerDM(0, RES_15K);

  GPIO_Setting(RSW_DMDP_TEST,ON);
	Init_Test_Host_HS();
	
//16. Check that PET can enumerate UUT (at HS if possible), and Set Configuration 1.
	Report_MSG("16. Check that PET can enumerate UUT (at HS if possible), and Set Configuration 1.");
    Delay(10000); //等待ENUM DONE
	 if(Enum_Done == 1)
	 {
	 	 ts0 = GlobleTime;
		 bMaxPower = 2*Get_PD_bMaxPower();
		 //17. For the next 30 seconds, check current drawn does not exceed bMaxPower.
		  Report_MSG("17. For the next 30 seconds, check current drawn does not exceed bMaxPower.");
		 while ((GlobleTime - ts0) < 300000)
	  	  {
				tmp_i = Get_Avarage_ADC_Value(ADC_VBUS_I,10000)/10;  //avarage 1 sec for VBUS_I

				if(tmp_i>bMaxPower)
				{
				 Report_MSG("ERRO: Current exceeded bMaxPower!") ;
				 sprintf(charBuf,"ERRO:I = %d mA",tmp_i) ;	
		         Report_MSG(charBuf)  ;
				}
				else
				{
				 sprintf(charBuf,"INFO:I = %d mA",tmp_i) ;	
		         Report_MSG(charBuf)  ;
				}
		   } 
	  }
	  else
	  {
			Report_MSG("ERRO: Enumeration Failed!") ;
	  }

//18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
  	  Report_MSG("18. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.");

	ID_RES_Setting(RES_Rid_OPEN);
	powerDM(0 , RES_NONE);
	powerDP(0 , RES_NONE);
	turnOffVBus();


//19. Disconnect 15k pull-down resistors.
	 Report_MSG("19. Disconnect 15k pull-down resistors.");
	   	powerDP(0, RES_NONE);
  	powerDM(0, RES_NONE);
//20. Wait 5 seconds for detachment to be recognized.

	 Report_MSG("20. Wait 5 seconds for detachment to be recognized.");
		Delay(50000);

DBP_END_TEST:

    GPIO_Setting(RSW_DMDP_TEST,OFF);
	Delay(5000);
	DeInit_Test_Host();
	Delay(5000);
	GPIO_Setting(RSW_VCC_PHY,OFF); 
	  
//End of Test
}while(FALSE);
	  TASK_CLEAN_UP()
}
/*********** end of 6.16 Dead Battery Provision Test **********/




/*********** 6.14 Common Mode Test - Full Speed **********/
//2012-5-3 updated input voltage 
//2013-7-5  增加了UUT电流检测功能，避免电流太大，导致CMD电压不正常或振荡
//2014-8-13 deleted 100mA Limit for internal use

uint16_t PD_CommonModeTestFullSpeed(void* p){
    DECLARE_VARS()
   uint32_t ts0;
   uint32_t count,ok_count;
   uint16_t offset_val = 0;
  
 	do{

	offset_val = askHostToInputVal("Please input the offset value in mV (0-450mV)");

	while( offset_val>450)
	{
	 sprintf(charBuf,"%d is not a valid value , Please input the offset value in mV (0-450mV)",offset_val);
	 offset_val = askHostToInputVal(charBuf);
	}


//1. Apply common mode offset of VGND_OFFSET (0.375 V) with PET test socket higher than
//PET transceiver ground.


   	//prepareVBusForPD();
//	Report_MSG("APPLY VBUS...") ;
   GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy

   VBUS_CAP_Setting(CAP_100uF);
   PULL_DOWN_RES_Setting(RES_10K); 
   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
   //Delay(24);
   DAC_Setting(DAC_VBUS_IL,1000); //VBUS_IL 限流100mA
   //Delay(20);
   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	 Delay(10000);				  //waiting for stablization
  /* for debug	
    Report_MSG("Checking UUT current drain...") ;
	if(Get_Avarage_ADC_Value(ADC_VBUS_I,10000)> 1000)
	{
	 Report_MSG("ERRO:Current drain is more than 100mA, Please using a full-charged battery. ") ;
	 Report_MSG("Stop this test...");
	 break;
	}
	else
	{
	  Report_MSG("INFO: Ok, Current drain is less than 100mA") ;
	}
	
    for debug*/

	//for debug
    tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,10000)	  ;
    sprintf(charBuf,"INFO:Current drain =  %d mA", tmp/10) ;	
    Report_MSG(charBuf)  ;
	//for debug

    
	Report_MSG("AGND_TO_GND_AB = OFF...") ;
	GPIO_Setting(RSW_AGND_TO_GND_AB,ON);	
    Delay(5000);

   	Report_MSG("APPLY offset...") ;
	
    //Set_EL_Mode(CV);
	DAC_Setting(DAC_LOAD_I,500);//CURRENT 
	DAC_Setting(DAC_LOAD_V,offset_val); //VOLTAGE
   //DAC_Setting(DAC_LOAD_IL,450); //VOLTAGE


    GPIO_Setting(RSW_EL_TO_GND_AB,ON);
	Delay(5000);

	GPIO_Setting(SW_GND_AB_M1,OFF);	//
    GPIO_Setting(SW_VBUS_AB_M1,ON);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,1000)	  ;
    sprintf(charBuf,"INFO:VBUS_AB =  %d mv", tmp) ;	
    Report_MSG(charBuf)  ;
	 Delay(500);
	   
    GPIO_Setting(SW_VBUS_AB_M1,OFF);
	GPIO_Setting(SW_GND_AB_M1,ON);
	 	 Delay(500);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,1000);
    sprintf(charBuf,"INFO:GND_AB =  %d mv", tmp) ;	
    Report_MSG(charBuf)  ;

		GPIO_Setting(RSW_DMDP_TEST,ON);

	Init_Test_Host_FS();


//2. Turn VBUS on to 5V.

	//prepareVBusForPD();
//3. Wait for UUT to connect
//4. Wait 100ms.   
//5. Reset and enumerate at High Speed. Check that enumeration was successful.
	//  while(Enum_Done == 0)
	//  {
	//   ;
	//   }
	  Delay(50000);
	  Report_MSG("Checking response...") ;
//6. Continue for 10 seconds, without configuring, to perform a number of standard
//requests, (eg Get Device Descriptor) checking again for failure to respond to
//transactions. This is achieved by counting the number of failures to respond to
//transactions, and comparing this to an arbitrary ceiling. (A good device would never fail
//to respond in practice.) [PD33]
	 ts0 = GlobleTime;
	 count=0;

	 ok_count =0;
	 while( 100000 >(GlobleTime - ts0))
	 {
	   count++;
	 //  if (USBH_Verify_Communication() != 0 );
	 //  {
	  //    erro++;
	  // }
	  tmp = USBH_Verify_Communication();
	  //Delay(1000);
	  if (tmp == 0)
	  {
	   ok_count++;
	  }
    //	sprintf(charBuf,"INFO:Result =  %d",tmp) ;	
    // Report_MSG(charBuf)  ;
	   Delay(400);
	 }
	tmp =ok_count*100/(count/2)	 ;
    sprintf(charBuf,"INFO:%d transfer performed", count/2) ;	
    Report_MSG(charBuf)  ;
	if( tmp == 100)
	{
	sprintf(charBuf,"INFO:succeed rate =  %d percent", tmp) ;	
    Report_MSG(charBuf)  ;
	}

	else
	{
     sprintf(charBuf,"ERRO:succeed rate =  %d percent", tmp) ;	
     Report_MSG(charBuf)  ;
	 testResult = TEST_FAILED;
	}

	//Report_MSG("> DeInit USB host...")  ;
    GPIO_Setting(RSW_DMDP_TEST,OFF);
	Delay(5000);
	DeInit_Test_Host();
	Delay(5000);

     GPIO_Setting(RSW_EL_TO_GND_AB,OFF);
      GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);
	 Delay(500);
//   	Set_EL_Mode(CC);
		GPIO_Setting(SW_GND_AB_M1,OFF);
		GPIO_Setting(RSW_VCC_PHY,OFF);   //power down the usb phy
//7. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
//8. Remove common mode offset.
//9. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
//End of Test
  }while(FALSE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.14 Common Mode Test - Full Speed **********/

/*********** 6.15 Common Mode Test - High Speed **********/

//2014-8-13 deleted 100mA Limit for internal use

uint16_t PD_CommonModeTestHighSpeed(void* p){
   DECLARE_VARS()
   uint32_t ts0;
   uint32_t count,ok_count;
   uint16_t offset_val = 0;
  
 	do{

   	 offset_val = 	askHostToInputVal("Please input the offset value in mV (0-450mV)");

	while( offset_val>450)
	{
	 sprintf(charBuf,"%d is not a valid value , Please input the offset value in mV (0-450mV)",offset_val);
	 offset_val = askHostToInputVal(charBuf);
	}


//1. Apply common mode offset of VGND_OFFSET (0.375 V) with PET test socket higher than
//PET transceiver ground.


   	//prepareVBusForPD();
//	Report_MSG("APPLY VBUS...") ;
   GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy

   VBUS_CAP_Setting(CAP_100uF);
   PULL_DOWN_RES_Setting(RES_10K); 
   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
   //Delay(24);
   DAC_Setting(DAC_VBUS_IL,1000); //VBUS_IL 限流100mA
   //Delay(20);
   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	Delay(10000);				  //waiting for stablization


  /* for debug	
    Report_MSG("Checking UUT current drain...") ;
	if(Get_Avarage_ADC_Value(ADC_VBUS_I,10000)> 1000)
	{
	 Report_MSG("ERRO:Current drain is more than 100mA, Please using a full-charged battery. ") ;
	 Report_MSG("Stop this test...");
	 break;
	}
	else
	{
	  Report_MSG("INFO: Ok, Current drain is less than 100mA") ;
	}
	
    for debug*/

	//for debug
    tmp = Get_Avarage_ADC_Value(ADC_VBUS_I,10000)	  ;
    sprintf(charBuf,"INFO:Current drain =  %d mA", tmp/10) ;	
    Report_MSG(charBuf)  ;
	//for debug
	
	
	Report_MSG("AGND_TO_GND_AB = OFF...") ;
	GPIO_Setting(RSW_AGND_TO_GND_AB,ON);	
    Delay(5000);

   	Report_MSG("APPLY offset...") ;
	
//    Set_EL_Mode(CV);
	DAC_Setting(DAC_LOAD_V,offset_val);//VOLTAGE
	DAC_Setting(DAC_LOAD_I,500);
	
    
    GPIO_Setting(RSW_EL_TO_GND_AB,ON);
	Delay(5000);

    GPIO_Setting(SW_GND_AB_M1,OFF);	//
    GPIO_Setting(SW_VBUS_AB_M1,ON);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,1000)	  ;
    sprintf(charBuf,"INFO:VBUS_AB =  %d mv", tmp) ;	
    Report_MSG(charBuf)  ;
	 Delay(500);
	   
    GPIO_Setting(SW_VBUS_AB_M1,OFF);
	GPIO_Setting(SW_GND_AB_M1,ON);
	 	 Delay(500);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,1000);
    sprintf(charBuf,"INFO:GND_AB =  %d mv", tmp) ;	
    Report_MSG(charBuf)  ;

		GPIO_Setting(RSW_DMDP_TEST,ON);

	Init_Test_Host_HS();


//2. Turn VBUS on to 5V.

	//prepareVBusForPD();
//3. Wait for UUT to connect
//4. Wait 100ms.   
//5. Reset and enumerate at High Speed. Check that enumeration was successful.
	//  while(Enum_Done == 0)
	//  {
	//   ;
	//   }
	  Delay(50000);
	  Report_MSG("Checking response...") ;
//6. Continue for 10 seconds, without configuring, to perform a number of standard
//requests, (eg Get Device Descriptor) checking again for failure to respond to
//transactions. This is achieved by counting the number of failures to respond to
//transactions, and comparing this to an arbitrary ceiling. (A good device would never fail
//to respond in practice.) [PD33]
	 ts0 = GlobleTime;
	 count=0;

	 ok_count =0;
	 while( 100000 >(GlobleTime - ts0))
	 {
	   count++;
	 //  if (USBH_Verify_Communication() != 0 );
	 //  {
	  //    erro++;
	  // }
	  tmp = USBH_Verify_Communication();
	  //Delay(1000);
	  if (tmp == 0)
	  {
	   ok_count++;
	  }
    //	sprintf(charBuf,"INFO:Result =  %d",tmp) ;	
    // Report_MSG(charBuf)  ;
	   Delay(400);
	 }
	tmp =ok_count*100/(count/2)	 ;
    sprintf(charBuf,"INFO:%d transfer performed", count/2) ;	
    Report_MSG(charBuf)  ;

	if( tmp == 100)
	{
	sprintf(charBuf,"INFO:succeed rate =  %d percent", tmp) ;	
    Report_MSG(charBuf)  ;
	}

	else
	{
     sprintf(charBuf,"ERRO:succeed rate =  %d percent", tmp) ;	
     Report_MSG(charBuf)  ;
	 testResult = TEST_FAILED;
	}

	//Report_MSG("> DeInit USB host...")  ;
    GPIO_Setting(RSW_DMDP_TEST,OFF);
	Delay(5000);
	DeInit_Test_Host();
	Delay(5000);

     GPIO_Setting(RSW_EL_TO_GND_AB,OFF);
      GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);
	 Delay(500);
//   	Set_EL_Mode(CC);
		GPIO_Setting(SW_GND_AB_M1,OFF);
		GPIO_Setting(RSW_VCC_PHY,OFF);   //power down the usb phy
//7. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
//8. Remove common mode offset.
//9. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
//End of Test
  }while(FALSE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.15 Common Mode Test - High Speed **********/



/**7.2 DCP Overshoot and Undershoot Voltage Test***/
//weiming create, lishunda 07 20 change
//2012-07-12 updated
//2014-7-28 adding message box on step 1 and 2
//2014-8-27 update  step12,16, 'Apply voltage soure'

uint16_t DCP_overshoot_undershoot (void *p)
{
    DECLARE_VARS()

	prepareTest();

   SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,HI_RANGE);
    
	do
	{
	 //1. Ensure that UUT is connected via Special Test Cable B, or its captive cable, to the PET.
	   Report_MSG("1. Ensure that UUT is connected via Special Test Cable B, or its captive cable, to the PET.");
	   WaitHostToContinue("1. Ensure that UUT is connected via Special Test Cable B, or its captive cable, to the PET.");
	
	 //2. Ensure that DCP is in an unpowered state.
	   Report_MSG("2. If UUT power is on, switch it off, then click OK.");
	    WaitHostToContinue("2. If UUT power is on, switch it off, then click OK.");


	 //3. Wait for DCP voltage to fall below 0.5V, in case it has just been switched off. (Speed up
	 //fall using current load.)
	   Report_MSG("3. Wait for DCP voltage to fall below 0.5V, in case it has just been switched off. ");
	   Report_MSG("(Speed up fall using current load.");

	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_VBUS_M1 to monitor Vbus
	
//	Set_EL_Mode(CC);
    GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	DAC_Setting(DAC_LOAD_I,500);
	//DAC_Setting(DAC_LOAD_IV,450);


	 tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,500,0,10,100000); //Wait for DCP voltage to fall below 0.5V，10s 超时
	 if (tmp != resCapturedInRange ) 

	 {
	    
		sprintf(charBuf,"ERRO: Vbus did not fall below 0.5V, Vbus = %d mV",ADC_Captured_Value );	
	 	Report_MSG(charBuf)	 ;
		Report_MSG("Stop this test...");
		testResult = TEST_FAILED;
		break;

	   }
	   else
	   {
		 sprintf(charBuf,"INFO: Vbus went below 0.5V, Vbus = %d mV",ADC_Captured_Value );
		 Report_MSG(charBuf)	 ;
	   }
	   // GPIO_Setting(RSW_LOAD_APPLY,OFF);  //turn off current load

	 //4. Set up voltage watch-block ready to capture overshoot of VCHG_OVRSHT (6.0V) on VBUS
	   Report_MSG("4. Set up voltage watch-block ready to capture overshoot of VCHG_OVRSHT (6.0V) on VBUS");

		//DAC_Setting(DAC_LOAD_IL,1500);
		DAC_Setting(DAC_LOAD_I,0);
		Delay(5000);
		WatchBlockInit();  //初始化watch block
  		enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);

	 //5. Instruct test operator to plug DCP into 'wall-socket', or
	  // to perform steps required to bring the DCP from an un-powered state to a powered one.
	//6     . Wait for operator to click 'OK'.
	 Report_MSG("5. Plug DCP into 'wall-socket', or otherwise power it up, then click OK.")  ;  

	 WaitHostToContinue("5. Plug DCP into 'wall-socket', or otherwise power it up, then click OK.");
	 Delay(30000);

	 //7. Check watch-block overshoot detector latch was not triggered. [DCP1]
	 Report_MSG("7. Check watch-block overshoot detector latch was not triggered. [DCP1]");

	 tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureInRange,60000,500,10,100000); //Wait for DCP voltage to raise over 0.5V，10s 超时
	 if (tmp == resCapturedInRange ) 
	 {
	    //GlobleTime = 0;
	    Report_MSG("INFO: Vbus is captured.");
	   }
 	 else
	 {
	   // Delay(500);
	   Report_MSG("ERRO: No Vbus is captured, Please checking the connection!");
	   Report_MSG("Stop this test...");
	   testResult = TEST_FAILED;
	   break;
	 } 

    //check watchblock[0]- ADC_VBUS_AB 
	if ( IsWatchBlockTriggered(0))
	{
	 //Triggered V> 6.0V
	 sprintf(charBuf,"ERRO: Overshoot of Vbus(Power up) is  greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 	 Report_MSG(charBuf)	 ;
	 testResult = TEST_FAILED;
	}
	else
	{
	  sprintf(charBuf,"INFO: Overshoot of Vbus(Power up) is not greater than 6.0v,Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 	  Report_MSG(charBuf)	 ;
	}

	//8. Set up voltage watch-block ready to capture undershoot of VCHG_UNDSHT (4.1V), or
	//   overshoot of VCHG_OVRSHT (6.0V) on VBUS.
	  Report_MSG("8. Set up voltage watch-block ready to capture undershoot of VCHG_UNDSHT (4.1V),");
	  Report_MSG("   or overshoot of VCHG_OVRSHT (6.0V) on VBUS.");

	 WatchBlockInit();  //初始化watch block
	 enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	 enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 

	//9. With an applied current load of IDCP_LOW min (0mA), check that VBUS average is within
	//	appropriate range VCHG (4.75V to 5.5V) over the next TVBUS_AVG max (0.25 sec).[DCP6]
	  Report_MSG("9. With an applied current load of IDCP_LOW min (0mA), check that VBUS average is within");
	  Report_MSG("   appropriate range VCHG (4.75V to 5.5V) over the next TVBUS_AVG max (0.25 sec).[DCP6]");


	// Delay(1000);	
  	// tmp = Wait_ADC_Value(ADC_VBUS_AB,CaptureInRange,5500,4750,10,100000); //	VCHG (4.75V to 5.5V) over the next TVBUS_AVG max (0.25 sec).
	 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值

	 if (tmp <=5500 && tmp>=4750 ) 
	 {
		sprintf(charBuf,"INFO: With 0mA load, Averaged Vbus = %d mV",tmp) ;
		Report_MSG(charBuf)	;	  
	 }
 	 else
	 {
		sprintf(charBuf,"ERRO: With 0mA load, Averaged Vbus = %d mV",tmp) ;
		Report_MSG(charBuf)	;
		testResult = TEST_FAILED;	
	 } 

	 //10. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1][DCP8]
	 Report_MSG("10. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1][DCP8]");

	 	if ( IsWatchBlockTriggered(0))
	{
	 //Triggered V> 6;.0V
	 sprintf(charBuf,"ERRO: Overshoot of Vbus(0mA load) is greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 	 Report_MSG(charBuf)	 ;
	 testResult = TEST_FAILED;
	}
	else
	{
	 sprintf(charBuf,"INFO: Overshoot of Vbus(0mA load) is not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 	 Report_MSG(charBuf)	 ;
	}
	disableWatchBlock(0);
   
	 if ( IsWatchBlockTriggered(1))
	{
	 //Triggered V<4.1V
	 sprintf(charBuf,"ERRO: Undershoot of Vbus(0mA load) is less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 	 Report_MSG(charBuf)	;
	 testResult = TEST_FAILED;	 ;
	}
	else
	{
	 sprintf(charBuf,"INFO: Undershoot of Vbus(0mA load) is not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1));	
 	 Report_MSG(charBuf)	 ;
	}
	disableWatchBlock(1);// disable WB1

	//11. Re-program watch-block to allow for voltage drop in cable.
      Report_MSG("11. Re-program watch-block to allow for voltage drop in cable.");

    //Emulate attaching PD

	Report_MSG("--------Emulate attaching PD---------------------------")  ;

	//12. Apply VDP_SRC nom. (0.6V) to D+.
	//  12. Apply voltage source  (0.535V) via 200R resistor to D+. 
	//     Apply voltage source (0.15V) via 15k resistor to D-.
    Report_MSG("12. Apply voltage source  (0.535V) via 200R resistor to D+.");
	Report_MSG("   Apply voltage source (0.15V) via 15k resistor to D-.");
	powerDP(535,RES_200R);
	powerDM(150,RES_15K);

	//13. Wait TVDMSRC_EN max + 1 ms (= 21ms)
	Report_MSG("13. Wait TVDMSRC_EN max + 1 ms (= 21ms)");
	Delay(210);

	//14. Check D- > VDAT_REF min (0.25V). [DCP12]
	Report_MSG("14. Check D- > VDAT_REF min (0.25V). [DCP12]");

	tmp = WaitAdcValueMatch(ADC_DM,CaptureInRange,3600,250,10,TO_STM_TIME_UNIT(5000)); 
	 if (tmp == resCapturedInRange ) 
	 {
	    sprintf(charBuf,"INFO: D- = %d mV",ADC_Captured_Value) ;	
	    Report_MSG(charBuf)	 ;

	 }
 	 else
	 {	   
        sprintf(charBuf,"ERRO: D- = %d mV",ADC_Captured_Value) ;	
	    Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
	 } 
     
	 //15. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).
	 Report_MSG("15. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).");
	 Delay(210);

	 //16. Take D+ back to 0V.
	// 16. Disconnect voltage source via 200R resistor from D+.
	//     Disconnect voltage source via 15k resistor from D-.
	Report_MSG("16. Disconnect voltage source via 200R resistor from D+.");
	Report_MSG("    Disconnect voltage source via 15k resistor from D-.");
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);

     //Load Testing
	 Report_MSG("-------------------Load Testing--------------------------");

	 //17. Apply load of IDCP_MID max (100mA) to VBUS
	 Report_MSG("17. Apply load of IDCP_MID max (100mA) to VBUS");

	  WatchBlockInit();  //初始化watch block
	 enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	 enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 	  

	 DAC_Setting(DAC_LOAD_I,100);


	 //18. Wait TDCP_UNDSHT max (10ms).
	 Report_MSG("18. Wait TDCP_UNDSHT max (10ms).");
	 Delay(100);

	 //19. Check VBUS, at DCP connector, is in range VCHG (4.75V to 5.5V), making due
	 //    allowance for voltage drop in cable (spot check voltage). [DCP6][DCP10]
	 Report_MSG("19. Check VBUS, at DCP connector, is in range VCHG (4.75V to 5.5V), making due");
	 Report_MSG("   allowance for voltage drop in cable (spot check voltage). [DCP6][DCP10]");
	   tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值	 
		 if (tmp <=5500 && tmp>=4750 ) 
		 {

			sprintf(charBuf,"INFO: With 100mA load, Averaged Vbus = %d mV",tmp) ;
			Report_MSG(charBuf)	 ;	
		 }
	 	 else
		 {	   
			sprintf(charBuf,"ERRO: With 100mA load, Averaged Vbus = %d mV",tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
		 } 

	 	//20. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1][DCP8]

	   Report_MSG("20. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1][DCP8]");
		if ( IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(0mA->100mA) was greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		}
		else
		{
		  	sprintf(charBuf,"INFO: Overshoot of Vbus(0mA->100mA) was not greater than 6.0v, Max Vbus = %d mV ",GetWatchBlockRecordVal(0) );	
 			Report_MSG(charBuf)	 ;
		}
	
		 if ( IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V									
		 sprintf(charBuf,"ERRO: Undershoot of Vbus(0mA->100mA) was less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1));	
 		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		  sprintf(charBuf,"INFO: Undershoot of Vbus(0mA->100mA) was not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		  Report_MSG(charBuf)	 ;

		}

		 WatchBlockInit();  //初始化watch block
    	enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	    enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 

		// 21. Re-program watch-block to allow for voltage drop in cable.
		 Report_MSG("21. Re-program watch-block to allow for voltage drop in cable.");

		//22. Increase load on VBUS to IDCP min (500mA), 20ms after rise to IDCP mid.
		Report_MSG("22. Increase load on VBUS to IDCP min (500mA), 20ms after rise to IDCP mid.");
		DAC_Setting(DAC_LOAD_I,500);

		//23. Wait TDCP_UNDSHT max (10ms).
		Report_MSG("23. Wait TDCP_UNDSHT max (10ms).");
		Delay(100);

		//24. Check VBUS average, at DCP connector, is in range VCHG (4.75V to 5.5V), 
		//over the next TVBUS_AVG max (0.25 sec), making due allowance for voltage drop in cable. [DCP6]
		Report_MSG("24. Check VBUS average, at DCP connector, is in range VCHG (4.75V to 5.5V),");
		Report_MSG("   over the next TVBUS_AVG max (0.25 sec), making due allowance for voltage drop in cable. [DCP6]");
		 
		  tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值	 
		 if (tmp <=5500 && tmp>=4750 ) 
		 {

			sprintf(charBuf,"INFO: With 500mA load, Averaged Vbus = %d mV",tmp) ;
			Report_MSG(charBuf)	 ;	
		 }
	 	 else
		 {	   
			sprintf(charBuf,"ERRO: With 500mA load, Averaged Vbus = %d mV",tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
		 } 
		//25. Check watch-block overshoot and undershoot detector latches were not triggered. [DCP1] [DCP9]
        Report_MSG("25. Check watch-block overshoot and undershoot detector latches were not triggered. [DCP1] [DCP9]");
		
			if (IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(100mA->500mA) was greater than 6.0v, Max Vbus = %d mV ",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		}
		else
		{
		  	sprintf(charBuf,"INFO: Overshoot of Vbus(100mA->500mA) was not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 			Report_MSG(charBuf)	 ;
		}
	
		 if ( IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V									
		 sprintf(charBuf,"ERRO: Undershoot of Vbus(100mA->500mA) was less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		  sprintf(charBuf,"INFO: Undershoot of Vbus(100mA->500mA) was not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		  Report_MSG(charBuf)	 ;

		}

	    WatchBlockInit();  //初始化watch block
    	enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	    enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 

	//26. Remove Current Load.
	 Report_MSG("26. Remove Current Load.");
	 DAC_Setting(DAC_LOAD_I,0);

	 //27. Wait 100ms
	 Report_MSG("27. Wait 100ms");
	 Delay(1000)  ;	   

	//28. Check watch-block overshoot and undershoot detector latches were not triggered.
	Report_MSG("28. Check watch-block overshoot and undershoot detector latches were not triggered.");
     
	 		if (  IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(500mA->0mA) was greater than 6.0v, Max Vbus = %d mV ",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		}
		else
		{
		  	sprintf(charBuf,"INFO: Overshoot of Vbus(500mA->0mA) was not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 			Report_MSG(charBuf)	 ;
		}
	
		 if (  IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V									
		 sprintf(charBuf,"ERRO: Undershoot of Vbus(500mA->0mA) was less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		  sprintf(charBuf,"INFO: Undershoot of Vbus(500mA->0mA) was not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1));	
 		  Report_MSG(charBuf)	 ;

		}	

	  WatchBlockInit();  //初始化watch block
   	 enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);					 	
	 enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 



		//29. Apply load of IDCP min (500mA) to VBUS.

		Report_MSG("29. Apply load of IDCP min (500mA) to VBUS.")  ; 
		DAC_Setting(DAC_LOAD_I,500);	

	    //30. Wait TDCP_UNDSHT max (10ms).
		Report_MSG("30. Wait TDCP_UNDSHT max (10ms).");
		Delay(100);


         //31. Check VBUS, at DCP connector, is in range VCHG (4.75V to 5.5V),
		 // making due allowance for voltage drop in cable (spot check voltage). [DCP1] [DCP9] [DCP11]
		Report_MSG("31. Check VBUS, at DCP connector, is in range VCHG (4.75V to 5.5V),");
		Report_MSG("   making due allowance for voltage drop in cable (spot check voltage). [DCP1] [DCP9] [DCP11]");


		//32. Check VBUS average, at DCP connector, is in range VCHG (4.75V to 5.5V), 
		//over the next TVBUS_AVG max (0.25 sec), making due allowance for voltage drop in cable. [DCP6]

		 Report_MSG("32. Check VBUS average, at DCP connector, is in range VCHG (4.75V to 5.5V),");
		 Report_MSG("   over the next TVBUS_AVG max (0.25 sec), making due allowance for voltage drop in cable. [DCP6]");
		
		 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
		
		 if (tmp <=5500 && tmp>=4750 ) 
		 {
			 sprintf(charBuf,"INFO: With 500mA load, Averaged Vbus = %d mV",tmp) ;
			 Report_MSG(charBuf)	 ;	
		 }
	 	 else
		 {
			 sprintf(charBuf,"ERRO: With 500mA load, Averaged Vbus = %d mV",tmp) ;
			 Report_MSG(charBuf)	 ;	
			 testResult = TEST_FAILED;	   
		 } 

		  
        //33. Remove Current Load.
		 Report_MSG("33. Remove Current Load.");

		DAC_Setting(DAC_LOAD_I,0);


		//34. Wait 100ms
		  Report_MSG("34. Wait 100ms");
		 Delay(1000);
		//35. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1] [DCP9] 
		 Report_MSG("35. Check watch-block overshoot and undershoot detector latches were not triggered.[DCP1] [DCP9] ");

		 		if (IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(0mA->500mA->0mA) was greater than 6.0v, Max Vbus = %d mV ",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;
		}
		else
		{
		  	sprintf(charBuf,"INFO: Overshoot of Vbus(0mA->500mA->0mA) was not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 			Report_MSG(charBuf)	 ;
		}
	
		 if (IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V									
		 sprintf(charBuf,"ERRO: Undershoot of Vbus(0mA->500mA->0mA) was less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		  sprintf(charBuf,"INFO: Undershoot of Vbus(0mA->500mA->0mA) was not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );	
 		  Report_MSG(charBuf)	 ;

		}	

    	WatchBlockInit();  //初始化watch block

        //end of test	
		 Report_MSG("End of test")  ;



		}while(0) ;

   SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);

    //reset dac and sw
	TASK_CLEAN_UP()
	

}
/***************END OF 7.2 DCP Overshoot and Undershoot Voltage Test***/


/***************7.3 DCP Voltage and Current***/
//lishunda 2011 7 20
////2014-8-27 update  step2,6 'Apply voltage soure'

uint16_t DCP_VoltageAndCurrent(void *p){
 	DECLARE_VARS()
	prepareTest();

	SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,HI_RANGE);

	do{
 //1. IDCP is initially 0mA. Check that VBUS voltage, samples taken every 1 ms and averaged
//over TVBUS_AVG max (250ms), from DCP is within VCHG (4.75 – 5.5V).[DCP6]
	 Report_MSG("1. IDCP is initially 0mA. Check that VBUS voltage, samples taken every 1 ms and averaged");
	 Report_MSG("   over TVBUS_AVG max (250ms), from DCP is within VCHG (4.75 - 5.5V).[DCP6]");



//	Set_EL_Mode(CC);
	//DAC_Setting(DAC_LOAD_IL,1600);
	DAC_Setting(DAC_LOAD_I,0);
	GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_VBUS_AB to monitor Vbus

	Delay(100);

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500 );  //250ms 平均值
	if(tmp <= 5500 && tmp >= 4750)
	{
		//right
	   	sprintf(charBuf,"INFO: DCP was within VCHG (4.75 - 5.5V), Vbus = %d mV at 0 mA load.", tmp) ;	
   		Report_MSG(charBuf)  ;
	}else{
	   	sprintf(charBuf,"ERRO: DCP was not within VCHG (4.75 – 5.5V), Vbus = %d mV at 0 mA load.", tmp) ;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED;
		 //break;
	}
//Emulate attaching PD	  	

	Report_MSG("----------Emulate attaching PD-------------------------");

//2. Apply VDP_SRC nom. (0.6V) to D+.
//  2. Apply voltage source  (0.535V) via 200R resistor to D+. 
//     Apply voltage source (0.15V) via 15k resistor to D-.
    Report_MSG("2. Apply voltage source  (0.535V) via 200R resistor to D+.");
	Report_MSG("   Apply voltage source (0.15V) via 15k resistor to D-.");
	powerDP(535,RES_200R);
	powerDM(150,RES_15K);

//3. Wait TVDMSRC_EN max + 1 ms (= 21ms)
	Report_MSG("3. Wait TVDMSRC_EN max + 1 ms (= 21ms)");
	Delay(TO_STM_TIME_UNIT(21));

//4. Check D- > VDAT_REF min (0.25V).
 	Report_MSG("4. Check D- > VDAT_REF min (0.25V).");

	tmp = WaitAdcValueMatch(ADC_DM,CaptureBig,0,250,10,TO_STM_TIME_UNIT(5000)) ;
	if( resCapturedBig == tmp){
		//OK
		sprintf(charBuf,"INFO: D- was above VDAT_REF min (0.25V), D- = %d mV", ADC_Captured_Value) ;	
   		Report_MSG(charBuf)  ;
	}else{
	    sprintf(charBuf,"ERRO: D- was not above VDAT_REF min (0.25V), D- = %d mV", ADC_Captured_Value) ;
   		Report_MSG(charBuf)  ;
	}

//5. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).
	Report_MSG("5. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).");
	Delay(210);

//6. Take D+ back to 0V.
//  6. Disconnect voltage source via 200R resistor from D+.
//     Disconnect voltage source via 15k resistor from D-.
	Report_MSG("6. Disconnect voltage source via 200R resistor from D+.");
	Report_MSG("   Disconnect voltage source via 15k resistor from D-.");
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);

//7. Apply load of IDCP min (500 mA) to VBUS.
//fixit??
    Report_MSG("Apply load of IDCP min (500 mA) to VBUS.");
	DAC_Setting(DAC_LOAD_I,500);

//8. Wait 1 sec to avoid possible transient period (overshoot and undershoot are measured
//separately).
	Report_MSG("Wait 1 sec to avoid possible transient period (overshoot and undershoot are measured separately");
	Delay(10000);

//9. Check that VBUS voltage from DCP, at DCP connector, with samples taken every 1 ms
//and averaged over TVBUS_AVG max (250ms), is within VCHG (4.75 – 5.5V), making due
//allowance for voltage drop in cable.
	Report_MSG("9. Check that VBUS voltage from DCP, at DCP connector, with samples taken every 1 ms");
	Report_MSG("   and averaged over TVBUS_AVG max (250ms), is within VCHG (4.75 – 5.5V), making due");
	Report_MSG("   allowance for voltage drop in cable.");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,25000);
	if(tmp <= 5500 && tmp >= 4750)
	{
		//right
	   	sprintf(charBuf,"INFO: Vbus was within VCHG (4.75 - 5.5V), Avaraged Vbus = %d mV at 500mA load.", tmp) ;	
   		Report_MSG(charBuf)  ;

	}else{
	   	sprintf(charBuf,"ERRO: Vbus was not within VCHG (4.75 - 5.5V), Avaraged Vbus = %d mV at 500mA load.", tmp) ;;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED;
		 //break;
	}

//10. Increase load to IDEV_CHG max (1.5A).
	Report_MSG("10. Increase load to IDEV_CHG max (1.5A).");
	DAC_Setting(DAC_LOAD_I,1500);

//11. Wait 1 sec to avoid possible overshoot.
	Report_MSG("11. Wait 1 sec to avoid possible overshoot.");
	Delay(20000);

//12. Check that VBUS voltage from DCP, at DCP connector, with samples taken every 1 ms
//and averaged over TVBUS_AVG max (250ms), is below VCHG max (5.5V), making due
//allowance for voltage drop in cable. Report the voltage measured.
	Report_MSG("12. Check that VBUS voltage from DCP, at DCP connector, with samples taken every 1 ms");
	Report_MSG("   and averaged over TVBUS_AVG max (250ms), is below VCHG max (5.5v), making due");
	Report_MSG("   allowance for voltage drop in cable. Report the voltage measured.");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp <= 5500 && tmp >= 4750)
	{
		//right
	   	sprintf(charBuf,"INFO: Vbus was within VCHG (4.75 – 5.5V), Avaraged Vbus = %d mV at 1.5A load.", tmp) ;	
   		Report_MSG(charBuf)  ;
	}else{
	   	sprintf(charBuf,"ERRO: Vbus was not within VCHG (4.75 – 5.5V), Avaraged Vbus = %d mV at 1.5A load.", tmp) ;;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED;
		 //break;
	}
//13. Disconnect the current load.
	Report_MSG("13. Disconnect the current load.");
	DAC_Setting(DAC_LOAD_I, 0);		//order ?????????
	GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);
    Delay(TO_STM_TIME_UNIT(1000));

//End of Test
	Report_MSG("End of Test") ;
	}while(FALSE);

	TASK_CLEAN_UP();
}

/***************END OF 7.2 7.3 DCP Voltage and Current***/

/***************  DCP_Handshaking  ***/
//Weiming 2011 11 17
//2012-07-12 updated 
//2014-08-27 update  step3,8,7,12 'Apply voltage soure' methord.
//2018-1-15, add delay between step 14 and 15


uint16_t DCP_Handshaking(void *p){
 	DECLARE_VARS()
	prepareTest();

	do{

	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus



//   Initial State: UUT is connected via Special Test Cable B, or its captive cable, 
//    to the PET. No load applied. DCP is switched on. Data lines switched to data measurement circuit.


//	1. Check VBUS is above VOTG_SESS_VLD max (4V). [DCP6]
	 Report_MSG("1. Check VBUS is above VOTG_SESS_VLD max (4V). [DCP6]");
	 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp  > 4000){
		//right
		Report_MSG("INFO: Detected Vbus.")  ;
	}else{
		 //wrong
		Report_MSG("ERRO: Not detect Vbus! ")  ;
	    Report_MSG("Stop this test...");
		testResult = TEST_FAILED;

		break;
	}

//	2. Wait 200ms
    Report_MSG("2. Wait 200ms");
	Delay(2000);

//	Primary Detection
    Report_MSG("-----------Primary Detection---------------------------");
   
//	3. Connect voltage source (0.6V) via 200R resistor to D+.
//  3. Apply voltage source  (0.535V) via 200R resistor to D+. 
//     Apply voltage source (0.15V) via 15k resistor to D-.
    Report_MSG("3. Apply voltage source  (0.535V) via 200R resistor to D+.");
	Report_MSG("   Apply voltage source (0.15V) via 15k resistor to D-.");
	powerDP(535,RES_200R);
	powerDM(150,RES_15K);

//	4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	Report_MSG("4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).");
	Delay(210);

//	5. Check D- voltage is in range VDM_SRC (0.5V - 0.7V). [DCP12, DCP13]
	Report_MSG("5. Check D- voltage is in range VDM_SRC (0.5V - 0.7V). [DCP12, DCP13]");
	 tmp = Get_Avarage_ADC_Value(ADC_DM,20);
	if(tmp < 700 && tmp > 500)
	{
	  sprintf(charBuf,"INFO: D- voltage was in range VDM_SRC (0.5V - 0.7V), D- = %d mV.", tmp) ;	
      Report_MSG(charBuf)  ;
	}
	else
	{
	  sprintf(charBuf,"ERRO: D- voltage was not in range VDM_SRC (0.5V - 0.7V), D- = %d mV.", tmp) ;	
      Report_MSG(charBuf)  ;
	  testResult = TEST_FAILED;
	}
//	6. Wait 20ms to complete TVDPSRC_ON.
	 Report_MSG("6. Wait 20ms to complete TVDPSRC_ON.");
	 Delay(200);

//	7. Disconnect voltage source via 200R resistor from D+.
//  7. Disconnect voltage source via 200R resistor from D+.
//     Disconnect voltage source via 15k resistor from D-.
	Report_MSG("7. Disconnect voltage source via 200R resistor from D+.");
	Report_MSG("   Disconnect voltage source via 15k resistor from D-.");
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);

//	Secondary Detection
    Report_MSG("-----------------------Secondary Detection-----------------------");

//	8. Connect voltage source (0.6V) via 200R resistor to D-.
//  8. Apply voltage source  (0.535V) via 200R resistor to D+. 
//     Apply voltage source (0.15V) via 15k resistor to D-.
    Report_MSG("8. Apply voltage source  (0.535V) via 200R resistor to D+.");
	Report_MSG("   Apply voltage source (0.15V) via 15k resistor to D-.");
	powerDP(535,RES_200R);
	powerDM(150,RES_15K);

//	9. Wait 21ms.
	Report_MSG("9. Wait 21ms.");
	Delay(210);

//	10. Check D+ voltage is in range VDM_SRC (0.5V - 0.7V). [DCP12, DCP13]
	Report_MSG("10. Check D+ voltage is in range VDM_SRC (0.5V - 0.7V). [DCP12, DCP13]");
	 tmp = Get_Avarage_ADC_Value(ADC_DP,20);
	if(tmp < 700 && tmp > 500)
	{
	  sprintf(charBuf,"INFO: D+ voltage is in range VDP_SRC (0.5V - 0.7V), D+ = %d mV.", tmp) ;	
      Report_MSG(charBuf)  ;
	}
	else
	{
	  sprintf(charBuf,"ERRO: D+ voltage is not in range VDP_SRC (0.5V - 0.7V), D+ = %d mV.", tmp) ;	
      Report_MSG(charBuf)  ;
	  testResult = TEST_FAILED;
	}
//	11. Wait 20ms to complete TVDMSRC_ON.
	Report_MSG("11. Wait 20ms to complete TVDMSRC_ON.");
	Delay(200);

//	12. Disconnect voltage source via 200R resistor from D-.
//  12. Disconnect voltage source via 200R resistor from D+.
//      Disconnect voltage source via 15k resistor from D-.
	Report_MSG("12. Disconnect voltage source via 200R resistor from D+.");
	Report_MSG("    Disconnect voltage source via 15k resistor from D-.");
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);

//	13. Wait 5 seconds for UUT to recover.
	Report_MSG("13. Wait 5 seconds for UUT to recover.");
	Delay(50000);
	Report_MSG("End Test");


	  	}while(FALSE);


	SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
    SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);

	TASK_CLEAN_UP();
}

/***************END OF DCP_Handshaking  ***/



/********* 7.4 DCP Resistance and Capacitance Tests ******/
//lishunda 2011 7 20
//20121227 修改了 setp7,powerDM(0,RES_NONE)必须将DM电阻断开，否则结果不对！
//2014-06-10 fix step6 bug in ERRO msg report. 
//2014-7-28 ref QMP log, update step1,step5 and step6. 
////2014-8-27 update  step1,5 'Apply voltage soure'

uint16_t DCP_ResAndCapTest(void* p){
	DECLARE_VARS()
	uint16_t valueDP, valueDM;
//	uint16_t tmp;

	prepareTest();


	do{

	//Emulate attaching PD
	Report_MSG("--------------Emulate attaching PD------------------");
	
//1. Apply VDP_SRC nom. (0.6V) to D+.

	Report_MSG("1. Apply voltage source(0.535V) via 200R resistor to D+.");
	Report_MSG("   Apply voltage source (0.15V) via 15k resistor to D-.");
	powerDP(535,RES_200R);
	powerDM(150,RES_15K);

//2. Wait TVDMSRC_EN max + 1 ms (= 21ms)
   Report_MSG("2. Wait TVDMSRC_EN max + 1 ms (= 21ms)");
   Delay(210);

//3. Check D- > VDAT_REF min (0.25V).
	Report_MSG("3. Check D- > VDAT_REF min (0.25V).");

	tmp = WaitAdcValueMatch(ADC_DM,CaptureBig,0,250,10,50) ;
	if( resCapturedBig == tmp)
	{
		sprintf(charBuf,"INFO: D- was above VDAT_REF min (0.25V), D- = %d mV", ADC_Captured_Value) ;	
   		Report_MSG(charBuf)  ;
	}
	else
	{
		sprintf(charBuf,"ERRO: D- was not above VDAT_REF min (0.25V), D- = %d mV", ADC_Captured_Value) ;	
   		Report_MSG(charBuf)  ;
	    testResult = TEST_FAILED;
	}


//4. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).
	Report_MSG("4. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).");
	Delay(210);

//5. Take D+ back to 0V.
	Report_MSG("5. Disconnect voltage source via 200R resistor from D+.");
	Report_MSG("   Disconnect voltage source via 15k resistor from D-.");

    powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);

//Checking Resistance between D+ and D-
	Report_MSG("----------Checking Resistance between D+ and D--------------------") ;

//6. Check that resistance from D+ to D- is less than RDCP_DAT max (200R). i.e. Connect 3.0V
//via 200R resistor to D+, connect 0V via 200R resistor to D-. Measure voltages at D+ and
//D-. The difference must be less than 1.05V.[DCP12]
	Report_MSG("6. Check that resistance from D+ to D- is less than RDCP_DAT max (200R).");
	Report_MSG("   i.e. Connect 2.0V via 200R resistor to D+, connect 0.05V via 200R resistor to D-. ");
	Report_MSG("   Measure voltages at D+ and D-. The difference must be less than 0.67V. ");


 	powerDP(2000,RES_200R);
	powerDM(50,RES_200R);

	Delay(100); //Delay 10ms
	valueDM =  Get_Avarage_ADC_Value(ADC_DM,1000); //100ms 均值
	sprintf(charBuf,"INFO: D- = %d mV", valueDM) ;	
   	Report_MSG(charBuf)  ;
	valueDP =  Get_Avarage_ADC_Value(ADC_DP,1000);
	sprintf(charBuf,"INFO: D+ = %d mV", valueDP) ;	
   	Report_MSG(charBuf)  ;

	if(valueDP - valueDM < 670)
	{
		//right
		sprintf(charBuf,"INFO: D+ and D- difference was less than 0.67V, Vdp - Vdm = %d mV ", valueDP - valueDM) ;	
   		Report_MSG(charBuf)  ;
	
	}else
	{
		sprintf(charBuf,"ERRO: D+ and D- difference was not less than 0.67V, Vdp - Vdm = %d mV ", valueDP - valueDM) ;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED;
	}

//Checking leakage from D+ or D-
    Report_MSG("----------------------------Checking leakage from D+ or D---------------");

//7. Connect D+ via 100k to 0V
	Report_MSG("7. Connect D+ via 100k to 0V");
 	powerDP(0,RES_100K)  ;
	powerDM(0,RES_NONE)	 ;


//8. Wait 2 seconds to eliminate capacitive effects.
 	Report_MSG("8. Wait 2 seconds to eliminate capacitive effects.");
	Delay(20000);

//9. Check that voltage at D+ is below 1.44V (Two RDAT_LKG (300k) in parallel, VDAT_LKG = 3.6V).
	Report_MSG("9. Check that voltage at D+ is below 1.44V (Two RDAT_LKG (300k) in parallel, VDAT_LKG = 3.6V");

	valueDP = Get_Avarage_ADC_Value(ADC_DP,1000);
	if(valueDP < 1440){
	   		sprintf(charBuf,"INFO: D+ was below 1.44V, D+ = %d mV", valueDP) ;	
   			Report_MSG(charBuf)  ;
	
	}else{

		   	sprintf(charBuf,"ERRO: D+ was not below 1.44V, D+ = %d mV", valueDP) ;	
   			Report_MSG(charBuf)  ;
			 testResult = TEST_FAILED;
	}

	//10. Connect D+ via 100K to 3.3V
	 Report_MSG("10. Connect D+ via 100K to 3.6V. Connect D- via 100K to 3.6V.");
		powerDP(3600,RES_100K);
		powerDM(3600,RES_100K);


	//DAC_Setting(DAC_DP , 3300);
	//GPIO_Setting(SW_DCD_RES_5,0);

//11. Wait 2 seconds
	Report_MSG("11. Wait 2 seconds to eliminate capacitive effects.");
	Delay(20000);

//12. Check that D+ is greater than 1.98V (Two RDAT_LKG (300k) in parallel, VDAT_LKG = 0V).
//This shows that leakage resistance is greater than RDAT_LKG / 2.
	Report_MSG("12. Check that D+ is greater than 2.65V (Two RDAT_LKG (300k) in parallel, VDAT_LKG = 0V).");
	Report_MSG("This shows that leakage resistance is greater than RDAT_LKG / 2.");

 	valueDP = Get_Avarage_ADC_Value(ADC_DP,1000);
	if(valueDP > 2650)
	{
			sprintf(charBuf,"INFO: D+ was greater than 2.65V, D+ = %d mV", valueDP) ;	
   			Report_MSG(charBuf)  ;
	
	}else
	{
		   	sprintf(charBuf,"ERRO:  D+ was not greater than 2.65V, D+ = %d mV", valueDP) ;	
   			Report_MSG(charBuf)  ;
			testResult = TEST_FAILED;
	}

//Checking Capacitance of D+ or D-
Report_MSG("------------Checking Capacitance of D+ or D-----------------------");

//13. Discharge Standard 1nF capacitor and Capacitance under Test
//Connect 0V to D+ via 1nF test capacitor. Connect 0V to D- via 200R resistor. There is a
//tested, <200R, resistor between DM and DP. This will discharge the standard 1nF
//capacitor and the capacitance under test to 0V. Wait 10ms.
	 Report_MSG("13. Discharge Standard 1nF capacitor and Capacitance under Test");
	 Report_MSG("   Connect 0V to D+ via 1nF test capacitor. Connect 0V to D- via 200R resistor. There is a");
	 Report_MSG("   tested, <200R, resistor between DM and DP. This will discharge the standard 1nF");
	 Report_MSG("   capacitor and the capacitance under test to 0V. Wait 10ms.");


	/*DP_B,DM_B V3硬件接反了DM/DP.使用软件补救 20120406，现在1nf cap在DM上	*/

	// powerDM(0,CAP_1nF)	;
	// powerDP(0,RES_200R);

	/*V4 back to DCD_CAP*/

   	 powerDP(0,CAP_1nF)	;
	   powerDM(0,RES_200R);

     Delay(100);

//14. Isolate Capacitances
//Disconnect 0V from test capacitor to isolate it. Disconnect 0V from 200R resistor.
//Note: We will now use the D+ voltage watch-block to determine whether, during the chargesharing
//process, D+ rises above 1.65V. The watch-block amplifier has a limited band-width by
//design, so that the watch-block voltage set is lower than 1.65V. The actual value can be found
//in the test script.
	 Report_MSG("14. Isolate Capacitances");
	 Report_MSG("   Disconnect 0V from test capacitor to isolate it. Disconnect 0V from 200R resistor.");
	 Report_MSG("Note: We will now use the D+ voltage watch-block to determine whether, during the chargesharing");
	 Report_MSG("process, D+ rises above 1.65V. The watch-block amplifier has a limited band-width by");
	 Report_MSG("design, so that the watch-block voltage set is lower than 1.65V. The actual value can be found");
	 Report_MSG("in the test script.");



   	// powerDM(0,CAP_1nF)	;
  	// powerDP(0,RES_NONE);
	
   	powerDP(0,CAP_1nF)	;
  	powerDM(0,RES_NONE);
		 
   	// enableWatchBlock(0 , ADC_DM , CaptureBig ,	3300  , 1); 
	 enableWatchBlock(0, ADC_DP , CaptureBig ,	3300  , 1);
//15. Share Charge Between Capacitances
//Set DP watch-block to be testing for voltage less than the value required. Connect 3.3V to
//D+ via 1nF test capacitor. Wait 1ms. This allows for charge sharing between standard 1nF
//capacitor and capacitance under test.
	 Report_MSG("15. Share Charge Between Capacitances");
	 Report_MSG("   Set DP watch-block to be testing for voltage less than the value required. Connect 3.3V to");
	 Report_MSG("   D+ via 1nF test capacitor. Wait 1ms. This allows for charge sharing between standard 1nF");
	 Report_MSG("   capacitor and capacitance under test.");

 	 //powerDP(3300,CAP_1nF);
	 DAC_Setting(DAC_DP , 3300);
	 Delay(TO_STM_TIME_UNIT(1));

//16. Read watch-block to see if voltage on DP went above 1.65V. If it did, then the capacitance
//under test is less than 1nF and therefore in specification.
	Report_MSG("16. Read watch-block to see if voltage on DP went above 1.65V. If it did, then the capacitance");
	Report_MSG("   under test is less than 1nF and therefore in specification.");
   /*
    if ( WatchBlock[0].EnableFlag == 0)
	{
	 value =	ADC_Value_Convert_to_real(ADC_DM, WatchBlock[0].RecordedVaule);
	  sprintf(charBuf,"INFO: DP went above 1.65V, DP (peak) = %d mv", value ) ;
	  Report_MSG(charBuf )  ;


	}
	else
	{
	 value =	ADC_Value_Convert_to_real(ADC_DM, WatchBlock[0].RecordedVaule);
	  sprintf(charBuf,"ERRO: DP did not go above 1.65V, DP (peak) = %d mv", value ) ;
	  Report_MSG(charBuf )  ;
	  testResult = TEST_FAILED;
	}


	 sprintf(charBuf,"INFO:C(UUT) = %d pF by fomula 1",((1300*(2780- value)/value)) ) ;
	  Report_MSG(charBuf )  ;

	  sprintf(charBuf,"INFO:C(UUT) = %d pF by fomula 2",((1300*(3250- value)/value))-235 ) ;
	  Report_MSG(charBuf )  ;
	*/
	 tmp =	GetWatchBlockRecordVal(0);

	 if( tmp > 1070)		//使用外置1nF电容时，数值在1070左右，tester空载时在2100左右
	{
	 Report_MSG("INFO: The capacitance under test is less than 1nF")  ;	
	}
	else
	{
	 Report_MSG("INFO: The capacitance under test is more than 1nF")  ;	
	}

//End of Test
	Report_MSG("End of Test");

	}while(FALSE);


   TASK_CLEAN_UP()
}

/********* END OF 7.4 DCP Resistance and Capacitance Tests ******/



//for 8 Charging Downstream Port (CDP) Compliance
 /********* 8.2 CDP Overshoot and Undershoot Voltage Test ******/
//lishunda 2011 7 20
//WEIMING 2011 12 3 modify
//2012-07-13 updated
//2013-11-11 由于自动电流量程算法，在电流超过200mA进行量程转换时，有很高的噪音。会影响overshoot/undershoot的判别，
//			因此，在进行此类脚本时，先强制将电流量程设置为HI_RANGE.

uint16_t CDP_OvershootAndUndershootTest(void* p){
 	DECLARE_VARS()
	uint16_t valueDM;
//	uint16_t x;

	prepareTest();
   SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,HI_RANGE);

	do{
//Initial State: No load applied.
	//askHostToInputVal("");

//1. Ensure that UUT is connected via Special Test Cable B to the PET.
	Report_MSG("1. Ensure that UUT is connected via Special Test Cable B to the PET.");
	WaitHostToContinue("1. Ensure that UUT is connected via Special Test Cable B to the PET.");
//2. Ensure that DCP is in an un-powered state.
//Please note that if the UUT is a hub, then it
//should be unplugged from its host in addition to
//disconnecting its power supply unit from the wall socket.

	Report_MSG("2. Ensure that DCP is in an un-powered state.");
	Report_MSG("    Please note that if the UUT is a hub, then it");
	Report_MSG("    should be unplugged from its host in addition to");
	Report_MSG("    disconnecting its power supply unit from the wall socket.");

	WaitHostToContinue("2. Ensure that DCP is in an un-powered state.\n\rPlease note that if the UUT is a hub, then it\n\rshould be unplugged from its host in addition to\r\ndisconnecting its power supply unit from the wall socket.");	                     

//3. Wait for DCP voltage to fall below VBUS_LKG max (0.7V), in case it has 
// just been switched off. (Speed up fall using 100mA current load.)
	Report_MSG("3. Wait for DCP voltage to fall below VBUS_LKG max (0.7V), in case it has");
	Report_MSG("    just been switched off. (Speed up fall using 100mA current load.)");


   	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus
//	Set_EL_Mode(CC);
    GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	//DAC_Setting(DAC_LOAD_IL,100);
	DAC_Setting(DAC_LOAD_I,100);
	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,700,0,10,100000); //Wait for DCP voltage to fall below 0.5V，2s 超时
	 if (tmp != resCapturedInRange ) 
	 {

		sprintf(charBuf,"INFO: Vbus has fallen to below 0.7v, Vbus = %d mV",ADC_Captured_Value );	
	 	Report_MSG(charBuf)	 ;
		//GPIO_Setting(RSW_LOAD_APPLY,OFF);  //turn off current load	 ?????
	  GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);
		 DAC_Setting(DAC_LOAD_I,0);
	   }
	   else
	   {
		 sprintf(charBuf,"ERRO: Vbus failed to go below 0.7v, Vbus = %d mV",ADC_Captured_Value );
		 Report_MSG(charBuf)	 ; 
		 Report_MSG("End this test...");
		 testResult = TEST_FAILED;
		 break;
	   }

//4. Set up voltage watch-block ready to capture overshoot of VCHG_OVRSHT (6.0V) on VBUS.
	Report_MSG("4. Set up voltage watch-block ready to capture overshoot of VCHG_OVRSHT (6.0V) on VBUS.");
	//DAC_Setting(DAC_LOAD_IL,1500);
	DAC_Setting(DAC_LOAD_I,0);
	Delay(1000);
	WatchBlockInit();  //初始化watch block
  	enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);

//5. Instruct test operator to perform steps required to bring the CDP from an un-powered
//state to a powered one.
Report_MSG("5/6. Now,Please Power up the UUT, then click OK.");
		//6. Wait for operator to click 'OK'.
WaitHostToContinue("5/6. Now,Please Power up the UUT, then click OK.\n\n\rIf this is a hub, then connect its own power supply to\r\nthe wall socket, plug the hub into its host, and wait for\r\nit to be enumerated, before clicking OK");

//7. Wait for VBUS to rise above VCHG min (4.75V).
	  Report_MSG("7. Wait for VBUS to rise above VCHG min (4.75V).");

	 tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,6000,4750,100,100000); //Wait for DCP voltage to raise over 0.5V，10s 超时
	 if (tmp == resCapturedBig ) 
	 {
	    //GlobleTime = 0;
		sprintf(charBuf,"INFO: Vbus has raise to above 4.75V, Vbus = %d mV",ADC_Captured_Value );
		Report_MSG(charBuf)	 ; 
	   }
 	 else
	 {
	   // Delay(500);
	   sprintf(charBuf,"ERRO: Vbus has not raise to above 4.75V, Vbus = %d mV",ADC_Captured_Value );
		Report_MSG(charBuf)	 ; 
		 Report_MSG("End this test...");
		 testResult = TEST_FAILED;
		 break;
	 } 


//8. Check watch-block overshoot detector latch was not triggered. [CDP1]
	Report_MSG("8. Check watch-block overshoot detector latch was not triggered. [CDP1]");

	if ( IsWatchBlockTriggered(0))
	{
	 //Triggered V> 6.0V
	 sprintf(charBuf,"ERRO: Overshoot of Vbus(power up) is greater than 6.0V,Max Vbus = %d mV",GetWatchBlockRecordVal(0) );	
 	 Report_MSG(charBuf)	 ;
	 testResult = TEST_FAILED;
	}
	else
	{
 	 sprintf(charBuf,"INFO: Overshoot of Vbus(power up) is not greater than 6.0V,Max Vbus = %d mV",GetWatchBlockRecordVal(0));	
 	 Report_MSG(charBuf)	 ;
	}
//9. Set up voltage watch-block ready to capture undershoot of VCHG_UNDSHT (4.1V), or
//overshoot of VCHG_OVRSHT (6.0V) on VBUS, allowing for voltage drop in cable.
	 Report_MSG("9. Set up voltage watch-block ready to capture undershoot of VCHG_UNDSHT (4.1V), or");
	 Report_MSG("    overshoot of VCHG_OVRSHT (6.0V) on VBUS, allowing for voltage drop in cable.");

 	 WatchBlockInit();  //初始化watch block
	 enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	 enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 	  	  
	 //Report_MSG("Start to capture overshoot and undershoot...")  ;

 
//10. With an applied current load of 0mA, check that VBUS is within appropriate range VCHG
//(4.75V to 5.5V). [CDP6]
	 Report_MSG("10. With an applied current load of 0mA, check that VBUS is within appropriate range VCHG");
	 Report_MSG("    (4.75V to 5.5V). [CDP6]");

	 tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值
	 if (tmp <=5500 && tmp>=4750 ) 
	 {
		sprintf(charBuf,"INFO: Averaged Vbus = %d mV at 0mA load",tmp) ;
		Report_MSG(charBuf)	;	  
	 }
 	 else
	 {
		sprintf(charBuf,"ERRO: Averaged Vbus = %d mV at 0mA load",tmp) ;
		Report_MSG(charBuf)	;
		testResult = TEST_FAILED;	
	 } 
	 //Emulate attaching PD

	Report_MSG("----------------Emulate attaching PD-------------------------")	;	

//11. Apply VDP_SRC nom. (0.6V) to D+.
	Report_MSG("11. Apply VDP_SRC nom. (0.6V) to D+.")  ;
	powerDP(600,RES_200R);

//12. Wait TVDMSRC_EN max + 1 ms (= 21ms)
	Report_MSG("12. Wait TVDMSRC_EN max + 1 ms (= 21ms)")  ;
	Delay(210);

//13. Check D- > VDAT_REF min (0.25V). 
	Report_MSG("13. Check D- > VDAT_REF min (0.25V).")  ;

	valueDM = Get_Avarage_ADC_Value(ADC_DM,20);
	if(valueDM > 250)
	{
	 sprintf(charBuf,"INFO: D- > VDAT_REF min (0.25V), D- = %d mV",valueDM) ;
	 Report_MSG(charBuf)	;
	}
	else
	{
	 sprintf(charBuf,"ERRO:  D- < VDAT_REF min (0.25V),  D- = %d mV",valueDM) ;
	 Report_MSG(charBuf)	;
	 testResult = TEST_FAILED;
	}

//14. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).
	Report_MSG("14. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).")	;
  	Delay(TO_STM_TIME_UNIT(18));

//15. Take D+ back to 0V.
	Report_MSG("15. Take D+ back to 0V.")	;
	powerDP(600,RES_NONE);

//Load Testing
	Report_MSG("--------Load Testing--------------------")	;

//16. Apply load of ICDP min (1.5A) to VBUS.

	Report_MSG("16. Apply load of ICDP min (1.5A) to VBUS.")	;
	//DAC_Setting(DAC_LOAD_IL,1800);
	DAC_Setting(DAC_LOAD_I,1500);

//17. Wait 10ms.
	Report_MSG("17. Wait 10ms.")	;
	Delay(2500);

//18. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and
//averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V to
//5.5V), making due allowance for voltage drop in cable.	
		Report_MSG("18. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and")	;
		Report_MSG("     averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V to")	;
		Report_MSG("     5.5V), making due allowance for voltage drop in cable.")	;

		//for debug
	/*
		for(x =0;x<10;x++)
		{
		tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	  
		sprintf(charBuf,"INFO: Averaged Vbus = %d mV at 1.5A load",tmp) ;
		Report_MSG(charBuf)	 ;
		}
	  */
	   tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值	 
		 if (tmp <=5500 && tmp>=4750 ) 
		 { 	
			sprintf(charBuf,"INFO: Averaged Vbus = %d mV at 1.5A load",tmp) ;
			Report_MSG(charBuf)	 ;
		 }
	 	 else
		 {
			sprintf(charBuf,"ERRO: Averaged Vbus = %d mV at 1.5A load",tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;
		 } 		
		 										    
//19. Check watch-block overshoot and undershoot detector latches were not triggered.
//[CDP1] [CDP7]
		 Report_MSG("19. Check watch-block overshoot and undershoot detector latches were not triggered.")  ;
		 Report_MSG("    [CDP1] [CDP7]")  ;
  		if ( IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(0mA->1.5A) is greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		 sprintf(charBuf,"INFO: Overshoot of Vbus(0mA->1.5A) is not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0));		
		 Report_MSG(charBuf)	 ;
		}
			
		 if (IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V	
 		sprintf(charBuf,"ERRO: Undershoot of Vbus(0mA->1.5A)  is less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );		
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
		}
		else
		{
 		sprintf(charBuf,"INFO: Undershoot of Vbus(0mA->1.5A)  is not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );		
		Report_MSG(charBuf)	 ;
		}

		WatchBlockInit();  //初始化watch block
    	enableWatchBlock(0 , ADC_VBUS_AB , CaptureBig ,	6000  , 1);
	    enableWatchBlock(1 , ADC_VBUS_AB , CaptureSmall ,	4100  , 1); 

//20. Remove Current Load.

	Report_MSG("20. Remove Current Load.")	 ;
	DAC_Setting(DAC_LOAD_I,0);
	
//21. Wait 10ms
	Report_MSG("21. Wait 10ms.")	 ;
	Delay(100);

//22. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and
//averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V to
//5.5V).[CDP6]
	   	Report_MSG("22. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and")	;
		Report_MSG("     averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V to")	;
		Report_MSG("     5.5V).[CDP6]")	;


	   tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);	   //250ms 平均值	 
		 if (tmp <=5500 && tmp>=4750 ) 
		 { 	
			sprintf(charBuf,"INFO: Averaged Vbus = %d mV at 0mA load.",tmp) ;
			Report_MSG(charBuf)	 ;

		 }
	 	 else
		 {
			sprintf(charBuf,"ERRO: Averaged Vbus = %d mV at 0mA load.",tmp) ;
			Report_MSG(charBuf)	 ;
			testResult = TEST_FAILED;

		 } 	
//23. Check watch-block overshoot and undershoot detector latches were not triggered.
//[CDP1] [CDP7]
	   	Report_MSG("23. Check watch-block overshoot and undershoot detector latches were not triggered.")  ;
		 Report_MSG("    [CDP1] [CDP7]")  ;

	if ( IsWatchBlockTriggered(0))
		{
		 //Triggered V> 6.0V
		 sprintf(charBuf,"ERRO: Overshoot of Vbus(1.5A->0mA) is greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0));		
		 Report_MSG(charBuf)	 ;
		 testResult = TEST_FAILED;

		}
		else
		{
		 sprintf(charBuf,"INFO: Overshoot of Vbus(1.5A->0mA) is not greater than 6.0v, Max Vbus = %d mV",GetWatchBlockRecordVal(0) );		
		 Report_MSG(charBuf)	 ;
		}
			
		 if (IsWatchBlockTriggered(1))
		{
		 //Triggered V<4.1V	
 		sprintf(charBuf,"ERRO: Undershoot of Vbus(1.5A->0mA) is less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1) );		
		Report_MSG(charBuf)	 ;
		testResult = TEST_FAILED;
		}
		else
		{
 		sprintf(charBuf,"INFO: Undershoot of Vbus(1.5A->0mA) is not less than 4.1v, Min Vbus = %d mV",GetWatchBlockRecordVal(1));		
		Report_MSG(charBuf)	 ;
		}

	 Report_MSG("End of Test")	;

	
//End of Test

	}while(FALSE);

   SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);
	 TASK_CLEAN_UP()

}

 /********* END OF 8.2 CDP Overshoot and Undershoot Voltage Test ******/

/*********  8.3 CDP Voltage and Current Test *************/
//lishunda 2011 7 21 create
//WEIMING 2011-12-3 modify
//2012-07-13 updated
//20150626 modify step8 1sec delay  to 	0.8sec for yu hong test
//20150626 modify step8 1sec delay  to 	0.5sec for yu hong test

uint16_t CDP_VoltageAndCurrentTest(void *p){
 	DECLARE_VARS()
	prepareTest();

 SetCurrentRangeMode(ADC_VBUS_I,HI_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,HI_RANGE);
	do{
//	Set_EL_Mode(CC);
	//DAC_Setting(DAC_LOAD_IL,1600);
	DAC_Setting(DAC_LOAD_I,0);
    GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus
	
	Delay(100);

	//1. Check that VBUS voltage, samples taken every 1 ms and averaged over TVBUS_AVG max
//(250ms), from CDP is within VCHG (4.75 – 5.5V). [CDP6]
	 Report_MSG("1. Check that VBUS voltage, samples taken every 1 ms and averaged over TVBUS_AVG max");
	 Report_MSG("    (250ms), from CDP is within VCHG (4.75 - 5.5V). [CDP6]");

   	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500 );  //250ms 平均值
	if(tmp <= 5500 && tmp >= 4750)
	{
		//right
	  	sprintf(charBuf,"INFO: VBUS voltage was wihtin VCHG (4.75 - 5.5V), Vbus = %d mV at 0mA load.", tmp) ;	
   		Report_MSG(charBuf)  ;
	}else{
	   	sprintf(charBuf,"ERRO: VBUS voltage was not wihtin VCHG (4.75 - 5.5V), Vbus = %d mV at 0mA load.", tmp) ;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED; 
		 //break;
	}

//Emulate attaching PD
	Report_MSG("---------------Emulate attaching PD---------------------------");

//2. Apply VDP_SRC nom. (0.6V) to D+.
	Report_MSG("2. Apply VDP_SRC nom. (0.6V) to D+.");
	powerDP(600,RES_200R);

//3. Wait TVDMSRC_EN max + 1 ms (= 21ms)
	Report_MSG("3. Wait TVDMSRC_EN max + 1 ms (= 21ms)");
	Delay(TO_STM_TIME_UNIT(21));

//4. Check D- > VDAT_REF min (0.25V).
	Report_MSG("4. Check D- > VDAT_REF min (0.25V).");
	tmp = Get_Avarage_ADC_Value(ADC_DM,20);
	if(tmp > 250)
	{
	 sprintf(charBuf,"INFO: D- > VDAT_REF min (0.25V), D- = %d mV",tmp) ;
	 Report_MSG(charBuf)	;
	}
	else
	{
	 sprintf(charBuf,"ERRO:  D- < VDAT_REF min (0.25V),  D- = %d mV",tmp) ;
	 Report_MSG(charBuf)	;
	 testResult = TEST_FAILED;
	}


//5. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms – 20ms = 20ms).
	Report_MSG("5. Wait for 1ms more than the remainder of TVDPSRC_ON (40ms-20ms = 20ms).");
	Delay(TO_STM_TIME_UNIT(21));

//6. Take D+ back to 0V.
	Report_MSG("6. Take D+ back to 0V.");
	powerDP(0,RES_NONE);

//Load Testing
	Report_MSG("--------------Load Testing-----------------------");

//7. Apply load of ICDP min (1.5A) to VBUS.
 	Report_MSG("7. Apply load of ICDP min (1.5A) to VBUS.");
	DAC_Setting(DAC_LOAD_I,1500);


//8. Wait 1 sec to avoid possible transient period (overshoot and undershoot are measured
//separately).
	Report_MSG("8. Wait 0.5 sec to avoid possible transient period (overshoot and undershoot are measured separately).");
	Delay(TO_STM_TIME_UNIT(500));

//9. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and
//averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V to 5.5V),
//making due allowance for voltage drop in cable. [CDP6]
	Report_MSG("9. Check that VBUS voltage from CDP, at CDP connector, samples taken every 1 ms and");
	Report_MSG("    averaged over TVBUS_AVG max (250ms), is within appropriate range VCHG (4.75V-5.5V),");
	Report_MSG("    making due allowance for voltage drop in cable.[CDP6]");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp <= 5500 && tmp >= 4750)
	{
		//right
	  	sprintf(charBuf,"INFO: VBUS voltage was wihtin VCHG (4.75V-5.5V), Vbus = %d mV at 1.5A load.", tmp) ;	
   		Report_MSG(charBuf)  ;
	}else{
	   	sprintf(charBuf,"ERRO: VBUS voltage was not wihtin VCHG (4.75V-5.5V), Vbus = %d mV at 1.5A load.", tmp) ;	
   		Report_MSG(charBuf)  ;
		testResult = TEST_FAILED; 
		 //break;
	}
	//10. Disconnect the current load.
	Report_MSG("10. Disconnect the current load.");

	GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);  

	Report_MSG("End of Test")  ;

	}while(FALSE);

   SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
   SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);

	TASK_CLEAN_UP()	 ;

}
/********* END OF  8.3 CDP Voltage and Current Test ******/

/********* 8.4 CDP Handshaking Test ******/
//lishunda 2011 7 21 create
//weiming 2011-12-3 modify
//2012-07-13 updated
//2012-10-17 fix Step40 judgment
//2013-11-7 STEP 13和step 40 ，INFO和ERRO写反了，已经改正。

uint16_t CDP_HandshakingTest(void *p){
 	DECLARE_VARS()
	uint16_t vdm,dac_dm;
	uint32_t ts1,ts2;

	prepareTest();


	do{
//	Set_EL_Mode(CC);
	//DAC_Setting(DAC_LOAD_IL,1500);
	DAC_Setting(DAC_LOAD_I,0);
	GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus
	
	Delay(100);

//1. Check VBUS is above VOTG_SESS_VLD max (4V). [CDP6]
	 Report_MSG("1. Check VBUS is above VOTG_SESS_VLD max (4V). [CDP6]");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp  > 4000){
		//right
		sprintf(charBuf,"INFO: Vbus was above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
	}else{
		 //wrong
		sprintf(charBuf,"ERRO: Vbus was not above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
		testResult = TEST_FAILED;
		Report_MSG("End this test...")  ;
		break;
	}

//2. Wait 200ms
	Report_MSG("2. Wait 200ms");
   Delay(2000);


//3. Examine voltage on D-. If this is less than VDAT_REF min (0.25V), skip to step 31.
	Report_MSG("3. Examine voltage on D-. If this is less than VDAT_REF min (0.25V), skip to step 31.");
	tmp = Get_Avarage_ADC_Value(ADC_DM,50);
	if(tmp  > 250){
		//right
		//Behavior 1 – CDP maintains VDM_SRC while device not connected.

		sprintf(charBuf,"INFO: DM = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
	}else{

		sprintf(charBuf,"INFO: DM = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
		goto CDP_HANDSHAKING_BH2;
	}
//Behavior 1 – CDP maintains VDM_SRC while device not connected. Primary Detection
	Report_MSG("Behavior 1 – CDP maintains VDM_SRC while device not connected. Primary Detection");

//Primary Detection
	Report_MSG("------------Primary Detection----------------------") ;

//4. Connect voltage source (0.6V) via 200R resistor to D+.
	Report_MSG("4. Connect voltage source (0.6V) via 200R resistor to D+.");
	powerDP(600,RES_200R);
	ts1 = GlobleTime;

//5. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	Report_MSG("5. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).");
	Delay(210);

//Check VDM_SRC Gets Connected
	Report_MSG("-------------------Check VDM_SRC Gets Connected----------------------") ;

//6. Connect voltage source (0.5V) via 200R resistor to D-.
	Report_MSG("6. Connect voltage source (0.5V) via 200R resistor to D-.");
	  powerDM(500,RES_200R);


//7. While voltage across 200R is less than 50mV, decrement the D- voltage source in steps of 2mV.
// Stop if D- voltage falls below VDM_SRC min (0.5V) - FAIL. 
//Else we will now be drawing the required 250μA from VDM_SRC.
	Report_MSG("7. While voltage across 200R is less than 50mV, decrement the D- voltage source in steps of 2mV.");
	Report_MSG("Stop if D- voltage falls below VDM_SRC min (0.5V) - FAIL.");
	Report_MSG("Else we will now be drawing the required 250μA from VDM_SRC.");

	 	 //a:get VDM and check it is in 0.5-0.7
	    vdm = Get_Avarage_ADC_Value(ADC_DM,5); //voltage across 200R
		if(vdm>500 && vdm<700 )
		{
		 // ts0 = GlobleTime;// VDM_SRC ON point
		  //Report_MSG("INFO: D- is in 0.5v-0.7v with 0uA load ");
		  sprintf(charBuf,"INFO: D- = %d mV at 0 uA load", vdm ) ;
  	 	  Report_MSG(charBuf )  ; 
		  // Report_MSG("Checking VDM_SRC sourcing capability... ");
		    // Report_MSG("Checking VDM_SRC sourcing capability.00.. ");
		  //b: set DAC_DM = VDM
		    dac_dm  = 500 ;
	   
		  //c: derease DAC_DM if VDM-DAC_DM <50 and DAC_DM>500, step 5mV  ,DAC
		  
		   do 
			{
			  dac_dm = dac_dm - 2;
			  DAC_Setting(DAC_DM , dac_dm);
			  Delay(2);
			  tmp= Get_Avarage_ADC_Value(ADC_DM,10);
			}while((tmp - dac_dm )< 50 && dac_dm >10 ) ;
		  //d: report VDM and DAC_DM 

		  if(tmp>500 && tmp<700)
		  {
			  sprintf(charBuf,"INFO: D- = %d mV on at least 250 uA loading",tmp ) ;
	  	 	  Report_MSG(charBuf )  ; 
			  sprintf(charBuf,"INFO: DAC_DM = %d mV",dac_dm + 2 ) ;
	  	 	  Report_MSG(charBuf )  ;
		   }
		   else
		   {
		   	  sprintf(charBuf,"ERRO: D- = %d mV on 250 uA loading",tmp ) ;
	  	 	  Report_MSG(charBuf )  ; 
			  sprintf(charBuf,"ERRO: DAC_DM = %d mV",dac_dm + 2 ) ;
	  	 	  Report_MSG(charBuf )  ;
			 testResult = TEST_FAILED;
		   }	   
		}
		else
		{
		 Report_MSG("ERRO: D- is not in 0.5v-0.7v with 0uA load ");
		 sprintf(charBuf,"ERRO: D- = %d mV", vdm ) ;
  	 	 Report_MSG(charBuf )  ; 
		 testResult = TEST_FAILED;
		}
	  


//8. Check D- voltage is still in range VDM_SRC (0.5V – 0.7V). [CDP8]
      Report_MSG("8. Check D- voltage is still in range VDM_SRC (0.5V – 0.7V). [CDP8]");  

		  tmp= Get_Avarage_ADC_Value(ADC_DM,10);
		  if(tmp>500 && tmp<700)
		  { 
			  sprintf(charBuf,"INFO: D- was still in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 	 Report_MSG(charBuf )  ;
		   }
		   else
		   {
			 sprintf(charBuf,"ERRO: D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 	 Report_MSG(charBuf )  ;
			 testResult = TEST_FAILED;
			}

//9. Disconnect voltage source and 200R from D-.
        Report_MSG("9. Disconnect voltage source and 200R from D-.");
		powerDM(0,RES_NONE);

//10. Wait 20ms to complete TVDPSRC_ON.
		Report_MSG("10. Wait 20ms to complete TVDPSRC_ON.");
		if(GlobleTime - ts1 < 400)
	{
		Delay(400 - (GlobleTime - ts1));
	}

//11. Disconnect voltage source (0.6V) via 200R resistor from D+.
		Report_MSG("11. Disconnect voltage source (0.6V) via 200R resistor from D+.");
		 powerDP(0,RES_NONE);

//12. Wait 100ms.
		Report_MSG("12. Wait 100ms.");
		 Delay(1000);

//13. Check if D- is still at VDM_SRC (0.5V - 0.7V). If not – FAIL owing to inconsistent behavior.
		Report_MSG("13. Check if D- is still at VDM_SRC (0.5V - 0.7V). If not – FAIL owing to inconsistent behavior.");
		  tmp= Get_Avarage_ADC_Value(ADC_DM,10);
		  if(tmp>500 && tmp<700)
		  { 
			  sprintf(charBuf,"INFO: D- was still in 0.5v-0.7v, D- = %d mV", tmp ) ;

  	 	 	 Report_MSG(charBuf )  ;
		   }
		   else
		   {
			 sprintf(charBuf,"ERRO: D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 	 Report_MSG(charBuf )  ;
			 testResult = TEST_FAILED;
			}

//D- Pull-up Test
   Report_MSG("----------------D- Pull-up Test------------------------") ;

//14. Connect 3.3V via 1K5 to D-.
	Report_MSG("14. Connect 3.3V via 1K5 to D-.");
	powerDM(3300,RES_1K5);
	
 // for Mark Lai debug 2018-1-15 
  Delay(50);			

//15. Check that D- is greater than 2.2V. [CDP9]
	Report_MSG("15. Check that D- is greater than 2.2V. [CDP9]");
	tmp= Get_Avarage_ADC_Value(ADC_DM,10);
	  if(tmp>2200)
	  { 
		  sprintf(charBuf,"INFO: D- was greater than 2.2V, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO: D- was greater than 2.2V, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
		 testResult = TEST_FAILED;
		}
//16. Disconnect 3.3V via 1K5 from D-.
	Report_MSG("16. Disconnect 3.3V via 1K5 from D-.");
	 powerDM(0,RES_NONE);

//17. Wait 5 seconds for UUT to recognize disconnect.
	 Report_MSG("17. Wait 5 seconds for UUT to recognize disconnect.");
     Delay(50000);

//Primary Detection

	Report_MSG("------------------Primary Detection---------------------") ;

//18. Connect voltage source (0.6V) via 200R resistor to D+.
	Report_MSG("18. Connect voltage source (0.6V) via 200R resistor to D+.");
	powerDP(600,RES_200R);
	//ts1 = GlobleTime;
	
//19. Wait TVDPSRC_ON min (40ms)
	Report_MSG("19. Wait TVDPSRC_ON min (40ms)");
	Delay(400);

//20. Disconnect voltage source via 200R resistor from D+.
	Report_MSG("20. Disconnect voltage source via 200R resistor from D+.");
	powerDP(0,RES_NONE);

//Secondary Detection
	Report_MSG("-----------------------Secondary Detection--------------------------") ;

//21. Connect voltage source (0.6V) via 200R resistor to D-.
	Report_MSG("21. Connect voltage source (0.6V) via 200R resistor to D-.");
	powerDM(600,RES_200R);
    ts2 = GlobleTime;

//22. Wait 21ms.
	Report_MSG("22. Wait 21ms.");
	Delay(210);

//23. Check that D+ is below VDAT_REF min (0.25V)
	Report_MSG("23. Check that D+ is below VDAT_REF min (0.25V)");
  	tmp= Get_Avarage_ADC_Value(ADC_DP,10);
	  if(tmp<250)
	  { 
		  sprintf(charBuf,"INFO: D+ was below VDAT_REF min (0.25V), D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO: D+ was not below VDAT_REF min (0.25V), D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ;
		  testResult = TEST_FAILED;
		}

//24. Wait 20ms to complete TVDMSRC_ON.
	Report_MSG("24. Wait to complete TVDMSRC_ON.");
	if(GlobleTime - ts2 < 400)
	{
		Delay(400 - (GlobleTime - ts2));
	}

//25. Disconnect voltage source via 200R resistor from D-.
	Report_MSG("25. Disconnect voltage source via 200R resistor from D-.");
	powerDM(0,RES_200R);

//Check VDM_SRC Gets Disconnected
	Report_MSG("----------------Check VDM_SRC Gets Disconnected-------------------") ;

//26. Connect 3.3V via 1k5 resistor to D+.
	Report_MSG("26. Connect 3.3V via 1k5 resistor to D+.");
	powerDP(3300,RES_1K5);

//27. Wait slightly more than TVDMSRC_DIS max (20ms +1 ms = 21ms)
	Report_MSG("27. Wait slightly more than TVDMSRC_DIS max (20ms +1 ms = 21ms).");
	Delay(210);

//28. Check that D- voltage is below VDAT_REF min (0.25V). [CDP8]
	Report_MSG("28. Check that D- voltage is below VDAT_REF min (0.25V). [CDP8]");
	  tmp= Get_Avarage_ADC_Value(ADC_DM,10);
	  if(tmp<250)
	  { 
		  sprintf(charBuf,"INFO: D- was below VDAT_REF min (0.25V), D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO: D- was not below VDAT_REF min (0.25V), D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf ) ; 
		 testResult = TEST_FAILED;
		}

//29. Disconnect 3.3V via 1k5 resistor from D+
	Report_MSG("29. Disconnect 3.3V via 1k5 resistor from D+");
	powerDP(0,RES_NONE);

//30. Wait 5 seconds for UUT to recognize disconnect.
	Report_MSG("30. Wait 5 seconds for UUT to recognize disconnect.");
	  Delay(50000);
	  

	  

//End of Test – Behavior 1 Behavior
	 Report_MSG("End of Test - Behavior 1");
	 break;


CDP_HANDSHAKING_BH2:

//Behavior 2 – CDP only connects VDM_SRC in response to Primary detection.
   Report_MSG("Behavior 2 – CDP only connects VDM_SRC in response to Primary detection.");
//Primary Detection
	Report_MSG("---------------------Primary Detection----------------------");
//31. Connect voltage source (0.6V) via 200R resistor to D+.
	Report_MSG("31. Connect voltage source (0.6V) via 200R resistor to D+.");
	powerDP(600,RES_200R);
	ts1 = GlobleTime;

//32. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	Report_MSG("32. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).");
	Delay(210);
	
//Check VDM_SRC Gets Connected

	Report_MSG("------------Check VDM_SRC Gets Connected----------------------");

//33. Connect voltage source (0.5V) via 200R resistor to D-.
   Report_MSG("33. Connect voltage source (0.5V) via 200R resistor to D-.");
	//powerDM(500,RES_200R);

//34. While voltage across 200R is less than 50mV, decrement the D- voltage source in steps of 2mV. 
//Stop if D- voltage falls below VDM_SRC min (0.5V) - FAIL. 
//Else we will now be drawing the required 250μA from VDM_SRC.
   Report_MSG("34. While voltage across 200R is less than 50mV, decrement the D- voltage source in steps of 2mV.");
   Report_MSG("     Stop if D- voltage falls below VDM_SRC min (0.5V) - FAIL.");
   Report_MSG("     Else we will now be drawing the required 250uA from VDM_SRC.");

	    vdm = Get_Avarage_ADC_Value(ADC_DM,5); //voltage across 200R
		if(vdm>500 && vdm<700 )
		 {
			  sprintf(charBuf,"INFO: D- = %d mV at 0 uA load", vdm ) ;
	  	 	  Report_MSG(charBuf )  ; 
	
			  //b: set DAC_DM = VDM
			    dac_dm  = 502 ;
		    
			  //c: derease DAC_DM if VDM-DAC_DM <50 and DAC_DM>10, step 2mV 
			   do 
				{
				  dac_dm = dac_dm - 2;
				  DAC_Setting(DAC_DM , dac_dm);
				  Delay(2);
				  tmp = Get_Avarage_ADC_Value(ADC_DM,5) ;
				}while((tmp - dac_dm )< 50 && dac_dm >10 ) ;

			  if(tmp>500 && tmp<700)
			  {
				  sprintf(charBuf,"INFO: D- = %d mV on at least 250uA loading",tmp ) ;
		  	 	  Report_MSG(charBuf )  ; 
				  sprintf(charBuf,"INFO: DAC_DM = %d mV",dac_dm) ;
		  	 	  Report_MSG(charBuf )  ;
			   }
			   else
			   {
			   	  sprintf(charBuf,"ERRO: D- = %d mV at 250 uA loading",tmp) ;
		  	 	  Report_MSG(charBuf )  ; 
				  sprintf(charBuf,"ERRO: DAC_DM = %d mV",dac_dm) ;
		  	 	  Report_MSG(charBuf )  ;
				 testResult = TEST_FAILED;

			   }	   
		}
		else
		{
		 sprintf(charBuf,"ERRO: D- was not in 0.5v-0.7v with 0uA load, D- = %d mV", vdm ) ;
  	 	 Report_MSG(charBuf )  ;
		 testResult = TEST_FAILED; 
		}
	  

//35. Check D- voltage is still in range VDM_SRC (0.5V – 0.7V). [CDP8]
	 Report_MSG("35. Check D- voltage is still in range VDM_SRC (4.75 - 5.5v).[CDP8]");
	   	 tmp= Get_Avarage_ADC_Value(ADC_DM,10);
		if(tmp>500 && tmp<700)
	   { 
		  sprintf(charBuf,"INFO: D- was in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ;
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO: D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ;
		 testResult = TEST_FAILED; 
		}

//36. Disconnect voltage source and 200R from D-.
	 Report_MSG("36. Disconnect voltage source and 200R from D-.");
	   powerDM(0,RES_NONE);

//37. Wait 20ms to complete TVDPSRC_ON
	Report_MSG("37. Wait  to complete TVDPSRC_ON");
	if(GlobleTime - ts1 < 400)
	{
		Delay(400 - (GlobleTime - ts1));
	}
		

//38. Disconnect voltage source (0.6V) via 200R resistor from D+.
	 Report_MSG("38. Disconnect voltage source (0.6V) via 200R resistor from D+.");
		powerDP(0,RES_NONE);

//39. Wait 100ms.
	Report_MSG("39. Wait 100ms.");
		Delay(1000);


//		STEP 53 is an error in complaince !!!!!	?????
	 
//40. Check if D- is still at VDM_SRC (0.5V - 0.7V). If so – FAIL owing to inconsistent behavior.
	Report_MSG("40. Check if D- is still at VDM_SRC (0.5V - 0.7V). If so - FAIL owing to inconsistent behavior.");
   	 tmp= Get_Avarage_ADC_Value(ADC_DM,10);
		if(tmp>500 && tmp<700)
	   { 
		  sprintf(charBuf,"ERRO: D- was still in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ;
		 testResult = TEST_FAILED;
	   }
	   else
	   {
		  sprintf(charBuf,"INFO: D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ;
		  
		}	


//Secondary Detection
	Report_MSG("------------Secondary Detection-----------------");


//41. Connect voltage source (0.6V) via 200R resistor to D-.
	Report_MSG("41. Connect voltage source (0.6V) via 200R resistor to D-.");
	powerDM(600,RES_200R);
	ts2 = GlobleTime;

//42. Wait 21ms. 	
	Report_MSG("42. Wait 21ms.");
	Delay(210);

//43. Check that D+ is below VDAT_REF min (0.25V)
	Report_MSG("43. Check that D+ is below VDAT_REF min (0.25V).");
	 tmp= Get_Avarage_ADC_Value(ADC_DP,10);
	  if(tmp<250)
	  { 
		  sprintf(charBuf,"INFO: D+ was below VDAT_REF min (0.25V), D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO: D+ was not below VDAT_REF min (0.25V), D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
		 testResult = TEST_FAILED; 
		}
		
//44. Wait 20ms to complete TVDMSRC_ON.
	Report_MSG("44. Wait to complete TVDMSRC_ON.");		
	if(GlobleTime - ts2 < 400)
	{
		Delay(400 - (GlobleTime - ts2));
	}

//45. Disconnect voltage source via 200R resistor from D-.
	Report_MSG("45. Disconnect voltage source via 200R resistor from D-.");
	powerDM(0,RES_NONE);
	
//46. Wait 5 seconds for UUT to recover.
	Report_MSG("46. Wait 5 seconds for UUT to recover.");
	  Delay(50000);
	  

//End of Test – Behavior 2
	 Report_MSG("End of Test - Behavior 2");

	 

	}while(FALSE);


	TASK_CLEAN_UP()

}
/********* END OF  8.4 CDP Handshaking Test ******/

/********* 8.5 CDP Ground Offset Test – Full Speed ******/
//2012-07013 updated   to be continue
//2012-10-12 updated and tested


uint16_t CDP_GroundOffsetTestFullSpeed(void *p){
 	DECLARE_VARS()

    prepareTest();

	do{
	GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy

//	Set_EL_Mode(CC);

	DAC_Setting(DAC_LOAD_I,10);
		GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS

//	GPIO_Setting(RSW_CAL_I_5MA,ON);    //在VBUS_AB和VBUS_GND间加入5mA电流负载，没有使用EL,因为稳不住
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus

	
	Delay(100);
//1. Check VBUS is above VOTG_SESS_VLD max (4V).
  	Report_MSG("1. Check VBUS is above VOTG_SESS_VLD max (4V).");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp  > 4000){
		//right
		sprintf(charBuf,"INFO: Vbus was above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
	}else{
		 //wrong

		sprintf(charBuf,"ERRO: Vbus was not go above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
		testResult = TEST_FAILED; 
		Report_MSG("Stop this test...");
		break;
	}
//2. Wait 200ms.
	Report_MSG("2. Wait 200ms.");
	Delay(2000);

//Primary Detection
	Report_MSG("------------Primary Detection------------------------");

//3. Connect voltage source (0.6V) via 200R resistor to D+.
	 Report_MSG("3. Connect voltage source (0.6V) via 200R resistor to D+.");
	 powerDP(600,RES_200R);

//4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	 Report_MSG("4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).");
	 Delay(210);

//5. Check D- voltage is in range VDM_SRC (0.5V-0.7V).
	 Report_MSG("5. Check D- voltage is in range VDM_SRC (0.5V-0.7V).");

	 tmp= Get_Avarage_ADC_Value(ADC_DM,10);
	  if(tmp>500 && tmp<700)
	  { 
		  sprintf(charBuf,"INFO:  D- was in 0.5v-0.7v, D- = %d mV", tmp ) ;
  		  Report_MSG(charBuf )  ;
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO:  D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  		  Report_MSG(charBuf )  ;
		  testResult = TEST_FAILED; 
	   }

//6. Switch data lines to transceiver.
	 Report_MSG("6. Switch data lines to transceiver.");
	 // GPIO_Setting(RSW_DMDP_TEST,ON);		

//CMO Test
	Report_MSG("---------------CMO Test--------------------------");

//7. Apply common mode offset of VGND_OFFSET (0.375 V) with test socket lower than PET transceiver ground.
 	Report_MSG("7. Apply common mode offset of VGND_OFFSET (0.375 V) with test socket lower than PET transceiver ground.") ;


	/*	   FOR DEBUG   */ 
	DAC_Setting(DAC_CM_V,0);
		Delay(500);
	GPIO_Setting(RSW_AGND_TO_GND_AB,ON);   
	 	Delay(500);
	DAC_Setting(DAC_CM_V,375);
	
	//GPIO_Setting(RSW_CM_Polarity,ON);	//ON : GND_AB < AGND
	Delay(500);
		

//8. Connect using D+.
	Report_MSG("8. Connect using D+.");
	Init_Test_Device_FS();
		//Init_Test_Device_HS();
	GPIO_Setting(RSW_DMDP_TEST,ON);
	

//9. Allow PET to be reset and enumerated, responding as a hs Speed device. 
	Report_MSG("9. Allow PET to be reset and enumerated, responding as a FS Speed device.");
		Delay(100000);


//10. Disconnect using D+
	Report_MSG("10. Disconnect using D+.");
  GPIO_Setting(RSW_CAL_I_5MA,OFF); 	
	DAC_Setting(DAC_CM_V,0);
	Delay(500);
//11. Remove common mode offset.
	Report_MSG("11. Remove common mode offset.");

 
	GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);   
	GPIO_Setting(RSW_VCC_PHY,OFF);   //power OFF the usb phy

	}while(FALSE);


	 TASK_CLEAN_UP();
}
/********* END OF 8.5 CDP Ground Offset Test – Full Speed ******/

/*********8.6 CDP Ground Offset Test – High Speed ******/
//2012-07-13updated to be conntinue
//2012-10-12 updated and tested

uint16_t CDP_GroundOffsetTestHighSpeed(void *p){
  	DECLARE_VARS()

    prepareTest();

	do{
	GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy

	//Set_EL_Mode(CC);
	//DAC_Setting(DAC_LOAD_IL,25);
	DAC_Setting(DAC_LOAD_I,25);
   GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS

	//GPIO_Setting(RSW_CAL_I_5MA,ON);    //在VBUS_AB和VBUS_GND间加入5mA电流负载，没有使用EL,因为稳不住
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus

	
	Delay(100);
//1. Check VBUS is above VOTG_SESS_VLD max (4V).
  	Report_MSG("1. Check VBUS is above VOTG_SESS_VLD max (4V).");

	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp  > 4000){
		//right
		sprintf(charBuf,"INFO: Vbus was above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
	}else{
		 //wrong

		sprintf(charBuf,"ERRO: Vbus was not go above VOTG_SESS_VLD max (4V), Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
		testResult = TEST_FAILED; 
		Report_MSG("Stop this test...");
		break;
	}
//2. Wait 200ms.
	Report_MSG("2. Wait 200ms.");
	Delay(2000);

//Primary Detection
	Report_MSG("------------Primary Detection------------------------");

//3. Connect voltage source (0.6V) via 200R resistor to D+.
	 Report_MSG("3. Connect voltage source (0.6V) via 200R resistor to D+.");
	 powerDP(600,RES_200R);

//4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	 Report_MSG("4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).");
	 Delay(210);

//5. Check D- voltage is in range VDM_SRC (0.5V-0.7V).
	 Report_MSG("5. Check D- voltage is in range VDM_SRC (0.5V-0.7V).");

	 tmp= Get_Avarage_ADC_Value(ADC_DM,10);
	  if(tmp>500 && tmp<700)
	  { 
		  sprintf(charBuf,"INFO:  D- was in 0.5v-0.7v, D- = %d mV", tmp ) ;
  		  Report_MSG(charBuf )  ;
	   }
	   else
	   {
		  sprintf(charBuf,"ERRO:  D- was not in 0.5v-0.7v, D- = %d mV", tmp ) ;
  		  Report_MSG(charBuf )  ;
		  testResult = TEST_FAILED; 
	   }

//6. Switch data lines to transceiver.
	 Report_MSG("6. Switch data lines to transceiver.");
	  GPIO_Setting(RSW_DMDP_TEST,ON);		
		   Init_Test_Device_HS();

//CMO Test
	Report_MSG("---------------CMO Test--------------------------");

//7. Apply common mode offset of VGND_OFFSET (0.375 V) with test socket lower than PET transceiver ground.
 	Report_MSG("7. Apply common mode offset of VGND_OFFSET (0.375 V) with test socket lower than PET transceiver ground.") ;


	/*	   FOR DEBUG   */
		 
	DAC_Setting(DAC_CM_V,0);
		Delay(500);
	GPIO_Setting(RSW_AGND_TO_GND_AB,ON);   
	 	Delay(500);
	DAC_Setting(DAC_CM_V,350);
		 //DAC_Setting(DAC_CM_V,150);  //FOR DEBUG
		 
	
	//GPIO_Setting(RSW_CM_Polarity,ON);	//ON : GND_AB < AGND
	  Delay(5000);
		//GPIO_Setting()
	 //	Set_EL_Mode(CC);
	 //DAC_Setting(DAC_LOAD_IL,50);
  //	DAC_Setting(DAC_LOAD_IV,1800);
		//GPIO_Setting(RSW_DMDP_TEST,ON);
	//	GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS	

	  Delay(50000);
//8. Connect using D+.
	Report_MSG("8. Connect using D+.");
	
	

//9. Allow PET to be reset and enumerated, responding as a hs Speed device. 
	Report_MSG("9. Allow PET to be reset and enumerated, responding as a hs Speed device.");
	Delay(100000);

//10. Disconnect using D+
	Report_MSG("10. Disconnect using D+.");
	GPIO_Setting(RSW_VCC_PHY,OFF);   //power OFF the usb phy
	
	
//11. Remove common mode offset.
	Report_MSG("11. Remove common mode offset.");
	 GPIO_Setting(RSW_CAL_I_5MA,OFF); 	
	DAC_Setting(DAC_CM_V,0);
	Delay(500);
	GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);   
	




	}while(FALSE);


	 TASK_CLEAN_UP();
} 
/********* END OF 8.6 CDP Ground Offset Test – High Speed ******/



/********* SDP_Handshaking   ******/
//weiming 2011-12-3 created

uint16_t  SDP_Handshaking (void* p)
 {
   DECLARE_VARS()
		do{
//Initial State: UUT is connected via Special Test Cable B to the PET. No load applied. 
//SDP is switched on. Data lines switched to data measurement circuit.

//1. Check VBUS is above VOTG_SESS_VLD max (4V).
//  	Set_EL_Mode(CC);
	//DAC_Setting(DAC_LOAD_IL,1500);
	DAC_Setting(DAC_LOAD_I,0);
	GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus
	
		Delay(100);
	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500);
	if(tmp  > 4000){
		//right
		Report_MSG("INFO: Vbus is above VOTG_SESS_VLD max (4V)")  ;
		sprintf(charBuf,"INFO: Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
	}else{
		 //wrong
		Report_MSG("ERRO: Vbus is not above VOTG_SESS_VLD max (4V)")  ;
		sprintf(charBuf,"ERRO: Vbus = %d mV", tmp ) ;
  		Report_MSG(charBuf )  ;
		testResult = TEST_FAILED;
		break;
	}
//2. Wait 200ms
	 	Delay(2000);

//Primary Detection
    Report_MSG("-------------------") ;
	Report_MSG("Primary Detection");

//3. Connect voltage source (0.6V) via 200R resistor to D+.
	powerDP(600,RES_200R);

//4. Wait slightly more than TVDMSRC_EN max (20ms +1 ms = 21ms).
	Delay(210);

//5. Check that D- is below VDAT_REF min (0.25V). [SDP1]
	 tmp= Get_Avarage_ADC_Value(ADC_DM,10);
	  if(tmp<250)
	  { 
		 Report_MSG("INFO: D- is below VDAT_REF min (0.25V) at step 5") ;
		  sprintf(charBuf,"INFO: D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		 Report_MSG("ERRO: D- is NOT below VDAT_REF min (0.25V) at step 5") ;
		  sprintf(charBuf,"ERRO: D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
		 testResult = TEST_FAILED;
		}
//6. Wait 20ms to complete TVDPSRC_ON.
	Delay(200);

//7. Disconnect voltage source via 200R resistor from D+.
	powerDP(0,RES_NONE);

//Secondary Detection
    Report_MSG("-------------------") ;
	Report_MSG("Secondary Detection");
//8. Connect voltage source (0.6V) via 200R resistor to D-.
	powerDM(600,RES_200R);

//9. Wait 21ms.
	Delay(210);
//10. Check that D+ is below VDAT_REF min (0.25V). [SDP2]
	 tmp= Get_Avarage_ADC_Value(ADC_DP,10);
	  if(tmp<250)
	  { 
		 Report_MSG("INFO: D+ is below VDAT_REF min (0.25V) at step 10") ;
		  sprintf(charBuf,"INFO: D- = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
	   }
	   else
	   {
		 Report_MSG("ERRO: D+ is NOT below VDAT_REF min (0.25V) at step 10") ;
		  sprintf(charBuf,"ERRO: D+ = %d mV", tmp ) ;
  	 	 Report_MSG(charBuf )  ; 
		 testResult = TEST_FAILED;
		}
	
//11. Wait 20ms to complete TVDMSRC_ON.
	 Delay(200);

//12. Disconnect voltage source via 200R resistor from D-.
	 powerDM(0,RES_NONE);

//13. Wait 5 seconds for UUT to recover.
	 Report_MSG("Wait 5 seconds for UUT to recover...");
	 Delay(50000);

	 Report_MSG("End of Test");
//End of Test

	     }while(FALSE);

	TASK_CLEAN_UP()		;
 }

/********* END OF SDP_Handshaking ******/



/****MRP Functional Test************************************************************/
uint16_t MRP_Test(void *p)
{
// uint32_t ts0;
 uint16_t MRPI1,MRPI2,MRPI3,MRPI4,MRPI5,MRPI6,MRPI7;
 //uint16_t ;
 DECLARE_VARS();
 	
 MRPI4 = 0;
 MRPI5 = 0;
 MRPI6 = 0;
 MRPI7 = 0;

 Report_MSG("MRP Functional Test...");	 
 prepareTest();
 do{

//1. Ensure that UUT is connected via Special Test Cable B to the PET.
	Report_MSG("1. Ensure that UUT is connected via Special Test Cable B to the PET.");
	askHostToInputVal("1. Ensure that UUT is connected via Special Test Cable B to the PET.");

//2. Ensure that UUT is in an un-powered state.
	Report_MSG("2. Ensure that UUT is in an un-powered state.");
	askHostToInputVal("2. Ensure that UUT is in an un-powered state.");

//3. Wait for UUT voltage to fall below VBUS_LKG max (0.7V), in case it has just been switched off. 
//(Speed up fall using 100mA current load.)
	Report_MSG("3. Wait for UUT voltage to fall below VBUS_LKG max (0.7V), in case it has just been switched off.");
	Report_MSG("   (Speed up fall using 100mA current load.)");

   	GPIO_Setting(SW_VBUS_AB_M1,ON);   //using ADC_M1 to monitor Vbus
//	Set_EL_Mode(CC);
    GPIO_Setting(RSW_EL_TO_VBUS_AB,ON);  //apply current load to VBUS
	DAC_Setting(DAC_LOAD_I,100);
	//DAC_Setting(DAC_LOAD_IV,450);
	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,700,0,10,100000); //Wait for DCP voltage to fall below 0.5V，2s 超时
	 if (tmp != resCapturedInRange ) 
	 {

		sprintf(charBuf,"INFO: Vbus has fallen to below 0.7v, Vbus = %d mV",ADC_Captured_Value );	
	 	Report_MSG(charBuf)	 ;

		GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);  //turn off current load	 ?????
	
	   }
	   else
	   {
		 sprintf(charBuf,"ERRO: Vbus failed to go below 0.7v, Vbus = %d mV",ADC_Captured_Value );
		 Report_MSG(charBuf)	 ; 
		 Report_MSG("Abandon this test...");
		 testResult = TEST_FAILED;
		 break;
	   }



//4. Ask the test operator whether the MRP can act as a CDP.
// Record the answer. [MRPI1] If answer is 'No', then abandon test.
	Report_MSG("4. Ask the test operator whether the MRP can act as a CDP.");
	Report_MSG("   Record the answer. [MRPI1] If answer is 'No', then abandon test.");

    MRPI1 = askHostToInputVal("Can UUT act as a CDP?\r\nInput: 1 = YES,0 = NO");

	  if (MRPI1 == 0 ) 
	 {
		 Report_MSG("Abandon this test...");
		 testResult = TEST_FAILED;
		 break;
	   }


//5. Ask the test operator whether the MRP can act as a SDP. 
//Record the answer. [MRPI2]
	Report_MSG("5. Ask the test operator whether the MRP can act as a SDP.");
	Report_MSG("   Record the answer. [MRPI2]");

    MRPI2 = askHostToInputVal("Can UUT act as a SDP?\r\nInput: 1 = YES,0 = NO");
	

//6. Ask the test operator whether the MRP can act as a DCP. 
//Record the answer. [MRPI3]
	Report_MSG("6. Ask the test operator whether the MRP can act as a DCP.");
	Report_MSG("   Record the answer. [MRPI3]");

    MRPI3 = askHostToInputVal("Can UUT act as a DCP?\r\nInput: 1 = YES,0 = NO");


//7. If neither MRPI2 nor MRPI3 were answered 'Yes', then abandon test.
	Report_MSG("7. If neither MRPI2 nor MRPI3 were answered 'Yes', then abandon test.");
	 if (MRPI2 == 0 && MRPI3 ==0 ) 
	 {
		 Report_MSG("Abandon this test...");
		 testResult = TEST_FAILED;
		 break;
	   }


//8. If MRPI2 was answered 'No', skip to step 11.
   Report_MSG("8. If MRPI2 was answered 'No', skip to step 11.");
	if(MRPI2 == 0)
	{
	 goto MRP_STEP_11;
	}

//9. Ask the test operator whether the MRP forces detection renegotiation 
//when changing from a CDP to an SDP role. Record the answer. [MRPI4]
	Report_MSG("9. Ask the test operator whether the MRP forces detection renegotiation ");
	Report_MSG("    when changing from a CDP to an SDP role. Record the answer. [MRPI4]");

   MRPI4 =  askHostToInputVal("Does the MRP force detection renegotiation when changing rom a CDP to an SDP role?\r\nInput: 1 = YES,0 = NO");


//10. Ask the test operator whether the MRP forces detection renegotiation 
//when changing from an SDP to a CDP role. Record the answer. [MRPI5]
   	Report_MSG("10. Ask the test operator whether the MRP forces detection renegotiation");
	Report_MSG("    when changing from an SDP to a CDP role. Record the answer. [MRPI5]");

    MRPI5 = askHostToInputVal("Does the MRP force detection renegotiation when changing rom a SDP to an CDP role?\r\nInput: 1 = YES,0 = NO");


MRP_STEP_11:

//11. If MRPI3 was answered 'No', skip to step 14.
     Report_MSG("11. If MRPI3 was answered 'No', skip to step 14.");
	if(MRPI3 == 0)
	{
	 goto MRP_STEP_14;
	}
	

//12. Ask the test operator whether the MRP forces detection renegotiation 
//when changing from a CDP to a DCP role. Record the answer. [MRPI6]
	Report_MSG("12. Ask the test operator whether the MRP forces detection renegotiation");
	Report_MSG("    when changing from a CDP to a DCP role. Record the answer. [MRPI6]");

    MRPI6 = askHostToInputVal("Does the MRP force detection renegotiation when changing rom a CDP to a DCP role?\r\nInput: 1 = YES,0 = NO");


//13. Ask the test operator whether the MRP forces detection renegotiation
// when changing from a DCP to a CDP role. Record the answer. [MRPI7]	
	Report_MSG("13. Ask the test operator whether the MRP forces detection renegotiation");
	Report_MSG("    when changing from a DCP to a CDP role. Record the answer. [MRPI7]");

    MRPI7 = askHostToInputVal("Does the MRP forces detection renegotiation when changing rom a DCP to a CDP role?\r\nInput: 1 = YES,0 = NO");


MRP_STEP_14:
//14. If none of MRPI4, MRPI5, MRPI6, MRPI7 were answered 'Yes', then abandon test.
	Report_MSG("13. Ask the test operator whether the MRP forces detection renegotiation");
		 if (MRPI4 == 0 && MRPI5 ==0 && MRPI6 ==0&& MRPI7 ==0 )
	 {
		 Report_MSG("Abandon this test...");
		 testResult = TEST_FAILED;
		 break;
	   }

//15. Instruct test operator to perform steps required to bring the UUT from
// an un-powered state to a powered one, such that it will be acting in a CDP role,
//  and then to indicate that this has occurred.
   Report_MSG("15. Instruct test operator to perform steps required to bring the UUT from");
   Report_MSG("     an un-powered state to a powered one, such that it will be acting in a CDP role,");
   Report_MSG("     and then to indicate that this has occurred.");

   

//16. Wait for operator to click 'OK'.
   Report_MSG("16. Wait for operator to click 'OK'.");
   askHostToInputVal("Please Power up the UUT, such that it will be acting in a CDP role...");
	 Delay(5000);

//17. Wait for VBUS to rise above VCHG min (4.75V).
   Report_MSG("17. Wait for VBUS to rise above VCHG min (4.75V). ");

   tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureBig,0,4750,10,50000); 
   	 if (tmp == resCapturedBig ) 
	 {

		sprintf(charBuf,"INFO: Vbus has rise above VCHG min (4.75V), Vbus = %d mV",ADC_Captured_Value );	
	 	Report_MSG(charBuf)	 ;	
	   }
	   else
	   {
		 sprintf(charBuf,"ERRO: Vbus did not rise above VCHG min (4.75V), Vbus = %d mV",ADC_Captured_Value );
		 Report_MSG(charBuf)	 ; 
		 Report_MSG("Abandon this test...");
		 testResult = TEST_FAILED;
		 break;
	   }


//18. If the answer to MRPI2 is 'No', skip to Step 24.
	Report_MSG("18. If the answer to MRPI2 is 'No', skip to Step 24.");	
	if(MRPI2 == 0)
	{
	 goto MRP_STEP_24;
	}
	

//19. Instruct test operator to perform steps required to change the role of the UUT 
//from a CDP to an SDP, and then to click 'OK' to indicate that this has been done.
   	Report_MSG("19. Instruct test operator to perform steps required to change the role of the UUT");
	Report_MSG("     from a CDP to an SDP, and then to click 'OK' to indicate that this has been done.");
  		
    WatchBlockInit();  //初始化watch block
  	enableWatchBlock(0 , ADC_VBUS_AB , CaptureSmall ,700  ,1000);//持续100ms 小于0.7v
   askHostToInputVal("Please change the role of UUT from a CDP to a SDP.\r\n Click 'OK' to indicate that this has been done. ");



//20. If the answer to MRPI4 was 'No' skip to next step; else monitor VBUS to check 
//whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms), 
//before the operator clicked 'OK' in the previous step. [MRP1]
//需要在setp19，askHostToInputVal之前，使用2个watchblock, 分别捕捉VBUS<0.7V，和VBUS>0.7V（or 1V）两个事件
    Report_MSG("20. If the answer to MRPI4 was 'No' skip to next step; else monitor VBUS to check");
	Report_MSG("     whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms),");
	Report_MSG("     before the operator clicked 'OK' in the previous step. [MRP1]");
	if(MRPI4 ==1 )
	{

			if (IsWatchBlockTriggered(0) ) 
		  {	
		 	Report_MSG("INFO: Vbus has fallen to below 0.7v for at least 100ms)")	 ;				
		   }
		   else
		   {
			 Report_MSG("ERRO:  Vbus failed to go below 0.7v for TVBUS_REAPP min (100ms)");
		//	 Report_MSG("Abandon this test...");
			 testResult = TEST_FAILED;
			// break;
		   }
	}


//21. Instruct test operator to perform steps required to change the role of the UUT 
//from an SDP to an CDP, and then to click 'OK' to indicate that this has been done.
	Report_MSG("21. Instruct test operator to perform steps required to change the role of the UUT");
	Report_MSG("    from an SDP to an CDP, and then to click 'OK' to indicate that this has been done.");
	
	WatchBlockInit();  //初始化watch block
  	enableWatchBlock(0 , ADC_VBUS_AB , CaptureSmall ,700  ,1000);//持续100ms 小于0.7v

	askHostToInputVal("Please change the role of UUT from a SDP to a CDP.\r\n Click 'OK' to indicate that this has been done. ");


//22. If the answer to MRPI5 was 'No' skip to next step; else monitor VBUS to check 
//whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms), 
//before the operator clicked 'OK' in the previous step.. [MRP2]
	 Report_MSG("20. If the answer to MRPI5 was 'No' skip to next step; else monitor VBUS to check");
	Report_MSG("     whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms),");
	Report_MSG("     before the operator clicked 'OK' in the previous step. [MRP2]");

	if(MRPI5 == 1 )
	{

			if (IsWatchBlockTriggered(0) ) 
		  {	
		 	Report_MSG("INFO: Vbus has fallen to below 0.7v for at least 100ms)")	 ;				
		   }
		   else
		   {
			 Report_MSG("ERRO:  Vbus failed to go below 0.7v for TVBUS_REAPP min (100ms)");
			 //Report_MSG("Abandon this test...");
			 testResult = TEST_FAILED;
			 //break;
		   }
	}


//23. If the answer to MRPI3 is 'No', skip to 'End of Test'.
  Report_MSG("23. If the answer to MRPI3 is 'No', skip to 'End of Test'.");

   	if(MRPI3 == 0 )
	{
       break;
	}

MRP_STEP_24:
//24. Instruct test operator to perform steps required to change the role of the UUT 
//from a CDP to a DCP, and then to click 'OK' to indicate that this has been done.
   	Report_MSG("24. Instruct test operator to perform steps required to change the role of the UUT");
	Report_MSG("    from a CDP to a DCP, and then to click 'OK' to indicate that this has been done.");
	
	WatchBlockInit();  //初始化watch block
  	enableWatchBlock(0 , ADC_VBUS_AB , CaptureSmall ,700  ,1000);//持续100ms 小于0.7v

	askHostToInputVal("Please change the role of UUT from a CDP to a DCP.\r\n Click 'OK' to indicate that this has been done. ");



//25. If the answer to MRPI6 was 'No' skip to next step; else monitor VBUS to check 
//whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms), 
//before the operator clicked 'OK' in the previous step. [MRP3]
	Report_MSG("25. If the answer to MRPI6 was 'No' skip to next step; else monitor VBUS to check");
	Report_MSG("     whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms),");
	Report_MSG("     before the operator clicked 'OK' in the previous step. [MRP3]");
		if(MRPI6 == 1 )
	{
			if (IsWatchBlockTriggered(0) ) 
		  {	
		 	Report_MSG("INFO: Vbus has fallen to below 0.7v for at least 100ms)")	 ;				
		   }
		   else
		   {
			 Report_MSG("ERRO:  Vbus failed to go below 0.7v for TVBUS_REAPP min (100ms)");
			 //Report_MSG("Abandon this test...");
			 testResult = TEST_FAILED;
			 //break;
		   }
	}



//26. Instruct test operator to perform steps required to change the role of the UUT 
//from an DCP to an CDP, and then to click 'OK' to indicate that this has been done.
   	Report_MSG("26. Instruct test operator to perform steps required to change the role of the UUT");
	Report_MSG("    from a DCP to a CDP, and then to click 'OK' to indicate that this has been done.");
	
	WatchBlockInit();  //初始化watch block
  	enableWatchBlock(0 , ADC_VBUS_AB , CaptureSmall ,700  ,1000);//持续100ms 小于0.7v

	askHostToInputVal("Please change the role of UUT from a DCP to a CDP.\r\n Click 'OK' to indicate that this has been done. ");



//27. If the answer to MRPI7 was 'No' skip to 'End of Test'; else monitor VBUS to check 
//whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms), 
//before the operator clicked 'OK' in the previous step. [MRP4]
	Report_MSG("27. If the answer to MRPI7 was 'No' skip to next step; else monitor VBUS to check");
	Report_MSG("     whether it went below VBUS_LKG max (0.7V) for TVBUS_REAPP min (100ms),");
	Report_MSG("     before the operator clicked 'OK' in the previous step. [MRP4]");
		if(MRPI7 == 1 )
	{
			if (IsWatchBlockTriggered(0)) 
		  {	
		 	Report_MSG("INFO: Vbus has fallen to below 0.7v for at least 100ms)")	 ;				
		   }
		   else
		   {
			 Report_MSG("ERRO:  Vbus failed to go below 0.7v for TVBUS_REAPP min (100ms)");
			 //Report_MSG("Abandon this test...");
			 testResult = TEST_FAILED;
			 //break;
		   }
	}


  Report_MSG("End of Test");

	}while(FALSE);	  	  

    
TASK_CLEAN_UP();


}
/****END OF MRP Functional Test************************************************************/


/**** BC12ImplementedCheck Test************************************************************/

//2013-06-04 step 7 . updated 2ms to 20ms
//2013-08-28 step 3 加载了SW_DCD_RES_AB ，但是在这个scipt中，并没有关掉，也没有在TASK_CLEAN_UP()中关掉，
//           所以会影响后续的脚本运行。现在已经在TASK_CLEAN_UP()中增加了SW_DCD_RES_AB off.
//2013-11-1  step 6  D+ rise的电压值从100mV变成200mV  ,	 D+如果没有rise above 0.5V的话 ,会继续check BC1.2 capability
//			 step 7	  D+ wait time从20ms变成1ms
//2013-11-7  STEP 8  完善了对D+的判断

uint16_t BC12ImplementedCheck(void *p)
{
	DECLARE_VARS()
	
	
	prepareTest();

do
 {
	  Report_MSG("A simple check to determine whether a PD or a CDC/DCD/SDP");
	  Report_MSG("has implemented BC 1.2 Primary Detection.");
	  
	   Report_MSG("1. Ensure UUT connected using special cable A, B or C,");
	   Report_MSG("  and is powered on where appropriate");

	   WaitHostToContinue("Ensure UUT connected using special cable A, B or C,and is powered on where appropriate");

	   Report_MSG("2.Check whether UUT is sourcing VBUS.");
	   	GPIO_Setting(SW_VBUS_AB_M1,ON);  //using ADC_VBUS_AB to monitor Vbus
	   	tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500 );  //250ms 平均值
	   if(tmp > 2000)
	   {
		  sprintf(charBuf,"INFO:Vbus = %d mv, UUT may be a SDP or CDP, ",tmp) ;
		  Report_MSG(charBuf)	;
		  goto BC12ImplementedCheck_CDPSDP ;
	   }
	   else	
	   {
		  sprintf(charBuf,"INFO:Vbus = %d mv, UUT may be a PD or UUT did not connected,",tmp) ;
		  Report_MSG(charBuf)	;
	   }



       Report_MSG("3. PET connects 200R between D+ and D-.");
	   GPIO_Setting(SW_DCD_RES_AB,ON);

	   Report_MSG("4. PET connects voltage source of 0V via 100k to D-,");
	   Report_MSG("  to prevent false detection of voltage on D+ etc.");
	   powerDM(0, RES_100K);

	   Report_MSG("5. PET applies CADP_VBUS max (6.5uF) and a pull-down resistor of ROTG_VBUS min");
	   Report_MSG("  (10k) to VBUS and turns on VBUS to 5V, ID pin is left floating.");

	   //VBUS_CAP_Setting(CAP_6uF5);
	   PULL_DOWN_RES_Setting(RES_10K);
	
	   DAC_Setting(DAC_VBUS_IL,1500); //VBUS_IL 限流1500mA
	   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
     Delay(1500);
		 GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;

	   Report_MSG("----Primary Detection----------------------------------") ;
	   Report_MSG("6.  Wait for D+ to rise above 200mV") ;
		tmp	= WaitAdcValueMatch(ADC_DP,CaptureBig,5500,200,10,50000) ;	//10次连续有效，5s 超时
		if(resCapturedBig == tmp)
		{
		//	ts1 = GlobleTime;  //VDP_SRC START POINT
	        Report_MSG("INFO:D+ rose above 200mV.")  ;
	     }
		else
		   {
		    Report_MSG("ERRO: D+ failed to rise above 200mV, no Primary Detect, no Connect");
		    testResult = TEST_FAILED;
			break;
	
		   }


	 Report_MSG("7. Wait 10 ms for D+ to settle.") ;
	 Delay(100);

BC12ImplementedCheck_WaitDpRise	:

	 Report_MSG("8. Measure D+ and check voltage is between 200mV and 2000mV.") ;
    /* tmp	= WaitAdcValueMatch(ADC_DP,CaptureInRange,2000,200,100,500) ;  // 连续30ms，50ms超时
	 if(tmp == resCapturedInRange )
	 {
		  sprintf(charBuf,"INFO:D+ was between 200mV and 2000mV in 1st checking, D+ = %d mv",ADC_Captured_Value) ;
		  Report_MSG(charBuf)	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  Report_MSG("> RESULT: BC 1.2 has been implemented.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  break;
	 }
	 //d+ 不在0.2-2V之间，可能是connect 也可能是 0,也可能之前是noise
	  */
	   tmp = Get_Avarage_ADC_Value(ADC_DP,100 );  //10ms 平均值
		if(tmp<=2000 && tmp>=200)
		{
 		//	ts1 = GlobleTime;  //VDP_SRC START POINT
		  sprintf(charBuf,"INFO:D+ was between 200mV and 2000mV in 2nd checking, D+ = %d mv",tmp) ;
		  Report_MSG(charBuf)	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  Report_MSG("> RESULT: BC 1.2 has been implemented.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  break;
         }
		else if (tmp> 2000)
		{
		  sprintf(charBuf,"INFO:D+ rose above 2000mV, D+ = %d mv",tmp) ;
		  Report_MSG(charBuf)	;
		  Report_MSG("INFO: UUT connected.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  Report_MSG("> RESULT: BC 1.2 has NOT been implemented.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  break;
		}
		else
		{
	 	Report_MSG("INFO: Appears to have been noise on D+...");
		 goto BC12ImplementedCheck_WaitDpRise ;
		 // Report_MSG("ERRO: D+ failed to rise above 200mV in 2nd checking, no Primary Detect, no Connect");
		//  testResult = TEST_FAILED;
		//  break;
		}

BC12ImplementedCheck_CDPSDP	:

/*	
 3.  Check VBUS is above VOTG_SESS_VLD max (4V).
[9.906,756]     - VBUS is at 5.116V. We may proceed.
[9.906,757] 4.  Wait 200ms.
[10.106,757]
[10.106,758] Primary Detection

[10.106,758] 5.  Connect voltage source (0.6V) via 200R resistor to D+.
[10.107,770] 6.  Wait much more than TVDMSRC_EN max (20ms + 80 ms = 100ms).
[10.207,771] 7.  Check VDM_SRC goes above 100mV.
[10.207,775]     - VDM_SRC measured as 0.606V.
 */
 Report_MSG("3.  Check VBUS is above VOTG_SESS_VLD max (4V).") ;
  tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,10 );  //250ms 平均值
		if(tmp>=4000)
		{
 		//	ts1 = GlobleTime;  //VDP_SRC START POINT
		  sprintf(charBuf,"INFO: Vbus = %d mV",tmp) ;
		  Report_MSG(charBuf)	;
         }
		else
		{
		  sprintf(charBuf,"ERRO:Vbus was not valid, Vbus = %d mv",tmp) ;
		  Report_MSG(charBuf)	;
		//  Report_MSG("Stop this test...");
		  testResult = TEST_FAILED; 
		 break;

		}
  Report_MSG("4.  Wait 200ms.") ;
  Delay(2000);

  Report_MSG("----Primary Detection----------------------------------") ;

  Report_MSG("5.  Connect voltage source (0.6V) via 200R resistor to D+.") ;
  powerDP(600,RES_200R)	;

  Report_MSG("6.  Wait much more than TVDMSRC_EN max (20ms + 80 ms = 100ms).") ;
  Delay(1000);

  Report_MSG("7.  Check VDM_SRC goes above 100mV.") ;
  tmp = Get_Avarage_ADC_Value(ADC_DM,10 );  //250ms 平均值
   if(tmp >=100)
   {
	 sprintf(charBuf,"INFO: D- went above 100mV, D- = %d mV",tmp) ;
	 Report_MSG(charBuf)	;
   }
   else
   {
     sprintf(charBuf,"INFO: D- did not go above 100mV, D- = %d mV",tmp) ;
	 Report_MSG(charBuf)	;
   }
   
   if(tmp>=100)
		{
 		  Report_MSG("----------------------------------------------------------------------------------")	;
		  Report_MSG("> RESULT: BC 1.2 has been implemented.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  
		 
         }
		else
		{
	      Report_MSG("----------------------------------------------------------------------------------")	;
		  Report_MSG("> RESULT: BC 1.2 has NOT been implemented.")	;
		  Report_MSG("----------------------------------------------------------------------------------")	;
		  
		}



 }while(FALSE);

 
	  
 TASK_CLEAN_UP();

}

/****END OF  BC12ImplementedCheck Test************************************************************/

/****BypassCapTest************************************************************/
// 2017-2-16 ,relax the MAX CAP value to 
// 2018-1-15, requie just not less 1uF.
uint16_t BypassCapTest(void *p)
{
	DECLARE_VARS()
	
	  int32_t t1,t2;
	  float fCap,fTc;
	  fTc = 0.0;
	prepareTest();

do
 {
	  Report_MSG("B-UUT Bypass Capacitance Test");
	  Report_MSG("1. Tester applies no Vbus capacitance or resistance and Vbus is off");
	  Report_MSG("2. Check that UUT is not sourcing VBUS.");
	   GPIO_Setting(SW_VBUS_AB_M1,ON);  //using ADC_VBUS_AB to monitor Vbus
	   tmp = Get_Avarage_ADC_Value(ADC_VBUS_AB,2500 );  //250ms 平均值
	   if(tmp > 150)
	   {
		    sprintf(charBuf,"ERRO: Vbus was not off,Vbus = %d mv",tmp) ;
		    Report_MSG(charBuf)	;
		   	testResult = TEST_FAILED; 
			Report_MSG("Stop this test...");
			break;
	   }
	  GPIO_Setting(RSW_CAL_I_5MA,ON); //Apply 2K res to VBUS_AB AND GND_AB
	  GPIO_Setting(SW_CAL_VBUS_AB,ON); //DAC_DM gen voltage via 0R to VBUS_AB
	  DAC_Setting(DAC_DM,1000);	       //generate 1v
	  Delay(5000);	               //充电1 sec

	  Report_MSG("3. Check that the capacitance is not less than 1uF ");
 	  GPIO_Setting(SW_CAL_VBUS_AB,OFF); //DAC_DP gen voltage via 0R to VBUS_AB
	   t1 = GlobleTime; //record begin time
	  //当电压降至 0.37 X 1v= 370mv 时， T = RC

	   	tmp = WaitAdcValueMatch(ADC_VBUS_AB,CaptureSmall,370,0,1,20000) ; 
	   if(tmp == resCapturedSmall)
	   {
		    t2 = GlobleTime;
			fTc = (float)(t2-t1)/10;	  //0.1ms div
		    fCap =  (fTc / 2);	  			//R = 2K

			//================================================
		//	sprintf(charBuf,"DEBUG: fTc Value = %3.1f ms",fTc)  ;
		//    Report_MSG(charBuf)	;
			//================================================


			if (fCap <0.10)			   // makesure cCap >=0 
			{
			   fCap =0;
			}
			else
			{
			  fCap = (fCap - 0.10)*1.085;	 //误差补偿，LCR校准
											 //Tester internal cap is about 0.10uF
			}

			fCap =(int)((fCap * 10) + 0.5) / 10.0;	//取小数点后一位
			
		//	sprintf(charBuf,"INFO: Cap Value = %3.2f uF",fCap)  ;
		   // Report_MSG(charBuf)	;
			//if(fCap<= 15.0 && fCap >=1.0)						   //暂时放宽至15
			if(fCap >=1.0)	 
			{
				sprintf(charBuf,"INFO: Cap Value = %3.1f uF",fCap)  ;
				Report_MSG(charBuf)	;
			}
			else
			{
				sprintf(charBuf,"ERRO: Cap Value = %3.1f uF",fCap)  ;
				Report_MSG(charBuf)	;
				testResult = TEST_FAILED; 
			}
			  
	   }
	   else
	   {
		 		Report_MSG("ERRO: Did not captrue the falling edge.");
				testResult = TEST_FAILED; 
	   }


	
	    Delay(10000);


 }while(FALSE);

 
	  
 TASK_CLEAN_UP();

}
/****BypassCapTest************************************************************/



/*********** 6.15 Common Mode Test - High Speed **********/
uint16_t Handset_Usb_comm(void* p){
   DECLARE_VARS()
   
   uint32_t count,ok_count;
//   uint32_t ts0;
 	do{



//1. Apply common mode offset of VGND_OFFSET (0.375 V) with PET test socket higher than
//PET transceiver ground.


   	//prepareVBusForPD();
	Report_MSG("APPLY VBUS...") ;
   GPIO_Setting(RSW_VCC_PHY,ON);   //power up the usb phy

   VBUS_CAP_Setting(CAP_100uF);
   PULL_DOWN_RES_Setting(RES_10K); 
   GPIO_Setting(RSW_VBUS_AB,ON); //Apply vbus_gen to vbus_ab;
   //Delay(24);
   DAC_Setting(DAC_VBUS_IL,1000); //VBUS_IL 限流100mA
   //Delay(20);
   DAC_Setting(DAC_VBUS_V,5000); //VBUS_V = 5000mV
	 Delay(500);

	
	
	

	GPIO_Setting(RSW_DMDP_TEST,ON);

	Init_Test_Host_HS();


//2. Turn VBUS on to 5V.

	//prepareVBusForPD();
//3. Wait for UUT to connect
//4. Wait 100ms.   
//5. Reset and enumerate at High Speed. Check that enumeration was successful.
	//  while(Enum_Done == 0)
	//  {
	//   ;
	//   }
	  Delay(30000);
	  Report_MSG("Checking response...") ;
//6. Continue for 10 seconds, without configuring, to perform a number of standard
//requests, (eg Get Device Descriptor) checking again for failure to respond to
//transactions. This is achieved by counting the number of failures to respond to
//transactions, and comparing this to an arbitrary ceiling. (A good device would never fail
//to respond in practice.) [PD33]
	 //ts0 = GlobleTime;
	 count=0;

	 ok_count =0;
	 while(1 )
	 {
	   count++;
	 //  if (USBH_Verify_Communication() != 0 );
	 //  {
	  //    erro++;
	  // }
	  tmp = USBH_Verify_Communication();
	  //Delay(1000);
	  if (tmp == 0)
	  {
	   ok_count++;
	  }
	  if((count%100) == 0)
	  {

			if((count/2) != ok_count )
			{
			 sprintf(charBuf,"ERRO:%d transfers, %d successed", count/2,ok_count) ;	
    		 Report_MSG(charBuf)  ;
			 Report_MSG("Reseting device...") ;
			 Init_Test_Host_HS();
			 Delay(30000);
			  count =0;
			  ok_count =0;
			}
			else
			{
			 sprintf(charBuf,"INFO:%d transfers, %d successed", count/2,ok_count) ;	
    		 Report_MSG(charBuf)  ;
			// ReportDutInfo();
			}

	  }
    //	sprintf(charBuf,"INFO:Result =  %d",tmp) ;	
    // Report_MSG(charBuf)  ;
	   Delay(400);

	 }

	//tmp =ok_count*100/(count/2)	 ;
//    sprintf(charBuf,"INFO:%d transfer performed", count/2) ;	
  //  Report_MSG(charBuf)  ;

	if( tmp == 100)
	{
	sprintf(charBuf,"INFO:succeed rate =  %d percent", tmp) ;	
    Report_MSG(charBuf)  ;
	}

	else
	{
     sprintf(charBuf,"ERRO:succeed rate =  %d percent", tmp) ;	
     Report_MSG(charBuf)  ;
	 testResult = TEST_FAILED;
	}

	//Report_MSG("> DeInit USB host...")  ;
    GPIO_Setting(RSW_DMDP_TEST,OFF);
	Delay(5000);
	DeInit_Test_Host();
	Delay(5000);

     GPIO_Setting(RSW_EL_TO_GND_AB,OFF);
      GPIO_Setting(RSW_AGND_TO_GND_AB,OFF);
	 Delay(500);
//   	Set_EL_Mode(CC);
		GPIO_Setting(SW_GND_AB_M1,OFF);
		GPIO_Setting(RSW_VCC_PHY,OFF);   //power down the usb phy
//7. Turn off VBUS and disconnect capacitance and pull-down resistance from VBUS.
//8. Remove common mode offset.
//9. Wait 8 seconds, ignoring SRP pulse, for detachment to be detected.
//End of Test
  }while(FALSE);

	TASK_CLEAN_UP()
 }
/*********** END OF 6.15 Common Mode Test - High Speed **********/



fn_ptr fn_ScriptPool[MAX_SCRIPT_NUM] = {
 FN_0,
 PD_UUTInitialPowerUpTest,				 //6.3
 PD_DataContactDetectWithCurrentSource,	 //6.4
 PD_DataContactDetectTestNoCurrentSrc,	 //6.5
 PD_DCP_Detection,						 //6.6
 PD_CDP_Detection_Test,					 //6.7
 PD_SDPDetection,						 //6.8
 PD_ACA_DockDetection, 					 //6.9
 PD_ACA_A_Detection,                     //6.10
 PD_ACA_B_Detection,	                 //6.11
 PD_ACA_C_Detection,				     //6.12
 PD_ACA_GND_Detection,                   //6.13   
 PD_DeadBatteryProvision,                //6.16
 PD_CommonModeTestFullSpeed,			 //6.14
 PD_CommonModeTestHighSpeed,	         //6.15

 DCP_overshoot_undershoot, 				 //7.2
 DCP_Handshaking, 
 DCP_ResAndCapTest, 					 //7.4
 DCP_VoltageAndCurrent, 				 //7.3

 CDP_OvershootAndUndershootTest, 		//8.2
 CDP_VoltageAndCurrentTest, 			//8.3
 CDP_HandshakingTest, 					//8.4
 CDP_GroundOffsetTestFullSpeed,  		//8.5
 CDP_GroundOffsetTestHighSpeed, 		//8.6

 SDP_Handshaking,
 MRP_Test,

 BC12ImplementedCheck,
 BypassCapTest,
 calibrate,
 SelfCheck,//Handset_Usb_comm,  
};

void vTaskRunOneScript(void *pvParameters)
{
 	 RUN_SCRIPT_CMD_FMT* CurrentCmd; 
     uint32_t re;

	CurrentCmd = (RUN_SCRIPT_CMD_FMT*)pvParameters;

   // sprintf(charBuf,"vTaskRunOneScript is running...,id= %d,para= %d ",RunScriptCmd->id,RunScriptCmd->para) ;
   //	Report_MSG(charBuf)	 ;
    


	re = fn_ScriptPool[CurrentCmd->id](&CurrentCmd->para)	;


	//report
	Report_Test_Result(CurrentCmd->id,re)  ;
	
    // reset val
	ScriptStopped();  // 
	MEAS_HW_RST();

    //delete itself
	vTaskDelete(NULL);
}


