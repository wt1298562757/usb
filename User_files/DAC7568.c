
#include "DAC7568.h"
#include "spi.h"


/***********************************/
/*	sDAC_SendWord				  */
/*  向SPI1发送入一个32bit数据	   */
/***********************************/ 

void sDAC_SendWord(uint32_t Word)
{
  uint16_t HalfWord = (Word & 0xffff0000)>>16;

  /*DAC_SYNC = LOW*/
   sDAC_CS_LOW();

  /*!< Loop while DR register in not emplty */
  while (__HAL_SPI_GET_FLAG(&sDAC_SPI_Handle, SPI_FLAG_TXE) == RESET);
  /*!< Send HIGH Half Word through the sFLASH peripheral */
  HAL_SPI_Transmit(&sDAC_SPI_Handle, (uint8_t *)&HalfWord, 1, HAL_MAX_DELAY);
  
  HalfWord = Word; 
   /*!< Send Low Half Word through the sFLASH peripheral */
  while (__HAL_SPI_GET_FLAG(&sDAC_SPI_Handle, SPI_FLAG_TXE) == RESET);
  HAL_SPI_Transmit(&sDAC_SPI_Handle, (uint8_t *)&HalfWord, 1, HAL_MAX_DELAY);
   
   /*wait BSY = reset*/

  while (__HAL_SPI_GET_FLAG(&sDAC_SPI_Handle, SPI_FLAG_BSY) == SET);

  /*DAC_SYNC = HIGH*/
   sDAC_CS_HIGH();

}


/*********************************************/
/*	sDAC_CMD					   			*/
/*  此程序将有关参数整合成一个32bit数据	   */
/*	然后调用sDAC_SendWord,将此数据写入DAC7568*/
/*********************************************/ 

void sDAC_CMD(uint32_t Dac_Control,uint32_t Dac_Addr,uint16_t Dac_Data)
{
 uint32_t tmp_word=Dac_Data;
 tmp_word = ((tmp_word)<<8)|Dac_Control|Dac_Addr;
 sDAC_SendWord(tmp_word) ;

}



void DAC_Setting(uint8_t dac_name,int16_t dac_value)
{
   uint16_t tmpValue=0;

  	switch 	(dac_name)
	{
	 case 0:
	 		if (dac_value <=7500 && dac_value >=0)	//VBUS_V电压(0-7.5v)/3 = 0-2.5V
			tmpValue = (4096*dac_value)/(2500*3);	//3分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_E,tmpValue);	
			break;
	 case 1:  //设置EL工作电流
	 //		if(EL_MODE == CC)
		//	{
				//V4: 1.4v/A 
		 		if (dac_value <=1500 && dac_value >=0)	//
				tmpValue = (4096*7*dac_value)/(2500*5);	//
		 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_A,tmpValue); //实际设置DAC_EL_IL
	//		}
		//	else
		//	{
		//	 	if (dac_value <=5000 && dac_value >=0)	//LOAD_IV电压CV模式 1mV = 3mV
		//		tmpValue = (4096*dac_value)/(2500*3);	//3分压
		// 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);
		//	}		   
			break;				 
	 case 2:
	 		if (dac_value <=1800 && dac_value >=0)	//VBUS_IL电流0-0.75V ,0.5R
			tmpValue = (4096*7*dac_value)/(2500*5);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_C,tmpValue);	
			break;				
	 case 3: //设定EL的电压， 1/3分压
	 	//	if (dac_value <=1800 && dac_value >=0)	//LOAD_IL限流设定0-1.5V
		//	tmpValue = (4096*dac_value)/(2500*3);	//3分压
	    tmpValue = (4096*dac_value)/(2500*3);//3分压	
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);	
			break;				 
	 case 4:
	 		if (dac_value <=2500 && dac_value >=0)	//CM_V 电压0-2.5V
			tmpValue = (4096*dac_value)/(2500);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_D,tmpValue);	
			break;				 
	 case 5:
	 		if (dac_value <=6000 && dac_value >=0)	//DP电压0-3.6V ,2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_H,tmpValue);	
			break;				 
	 case 6:
			if (dac_value <=6000 && dac_value >=0)	//DM电压0-3.6V ，2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_F,tmpValue);	
			break;
	 default:
	   
	 		break;			
	}

}
 
//上位机软件，DP/DM是反的
void DAC_Setting_link(uint8_t dac_name,int16_t dac_value)
{
   uint16_t tmpValue=0;

  	switch 	(dac_name)
	{
	 case 0:
	 		if (dac_value <=7500 && dac_value >=0)	//VBUS_V电压(0-7.5v)/3 = 0-2.5V
			tmpValue = (4096*dac_value)/(2500*3);	//3分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_E,tmpValue);	
			break;
	 case 1:  //设置EL工作电流
	 //		if(EL_MODE == CC)
		//	{
				//V4: 1.4v/A 
		 		if (dac_value <=1500 && dac_value >=0)	//
				tmpValue = (4096*7*dac_value)/(2500*5);	//
		 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_A,tmpValue); //实际设置DAC_EL_IL
	//		}
		//	else
		//	{
		//	 	if (dac_value <=5000 && dac_value >=0)	//LOAD_IV电压CV模式 1mV = 3mV
		//		tmpValue = (4096*dac_value)/(2500*3);	//3分压
		// 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);
		//	}		   
			break;				 
	 case 2:
	 		if (dac_value <=1800 && dac_value >=0)	//VBUS_IL电流0-0.75V ,0.5R
			tmpValue = (4096*7*dac_value)/(2500*5);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_C,tmpValue);	
			break;				
	 case 3: //设定EL的电压， 1/3分压
	 	//	if (dac_value <=1800 && dac_value >=0)	//LOAD_IL限流设定0-1.5V
		//	tmpValue = (4096*dac_value)/(2500*3);	//3分压
	    tmpValue = (4096*dac_value)/(2500*3);//3分压	
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_B,tmpValue);	
			break;				 
	 case 4:
	 		if (dac_value <=2500 && dac_value >=0)	//CM_V 电压0-2.5V
			tmpValue = (4096*dac_value)/(2500);	//不分压
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_D,tmpValue);	
			break;				 
	 case 5:
	 		if (dac_value <=6000 && dac_value >=0)	//DM电压0-3.6V ,2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_F,tmpValue);	
			break;				 
	 case 6:
			if (dac_value <=6000 && dac_value >=0)	//DP电压0-3.6V ，2倍放大
			tmpValue = (4096*dac_value)/(2500*2);
	 		sDAC_CMD(DAC_CMD_UPDATE_RESPECTIVE,DAC_CH_H,tmpValue);	
			break;
	 default:
	   
	 		break;			
	}

}

