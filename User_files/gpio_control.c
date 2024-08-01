#include "gpio_control.h"
#include "DAC7568.h"
#include <math.h>
#include "PET_CMD.h"


//量程本地变量

static uint8_t LoadCurrentRange = HI_RANGE;	  //LoadCurrentRange，当前帧的电流量程(HI,LO)
static uint8_t VbusCurrentRange = HI_RANGE;	  //VbusCurrentRange，当前帧的电流量程(HI,LO)
static uint8_t LoadCurrentRangeMode = HI_RANGE; //LoadCurrentRangeMode, 当前系统的电流量程模式（HI,LO,AUTO）
static uint8_t VbusCurrentRangeMode = HI_RANGE;	 //VbusCurrentRangeMode, 当前系统的电流量程模式（HI,LO,AUTO）



//设置开关
void GPIO_Setting (uint8_t io_num,uint16_t para)
{

 uint16_t io_pin = 0;
 GPIO_TypeDef* gpio_port = GPIOA;
 io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;
  else 
 	return;

 if(para == 0 ) //	
   {
      HAL_GPIO_WritePin(gpio_port,io_pin,GPIO_PIN_RESET);
	//   GPIO_ResetBits(gpio_port, io_pin); 
   }
 else 
   {
      HAL_GPIO_WritePin(gpio_port,io_pin,GPIO_PIN_SET);
	//   GPIO_SetBits(gpio_port, io_pin);
   }

}

//读取开关状态  GPIO_ReadOutputDataBit
uint8_t GPIO_Reading (uint8_t io_num)
{

 uint16_t io_pin = 0;
 GPIO_TypeDef* gpio_port = GPIOA;
 io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;

    return HAL_GPIO_ReadPin(gpio_port,io_pin);
//   return  GPIO_ReadOutputDataBit(gpio_port,io_pin) ;
}



// LED_State_toggle

void GPIO_Toggle(uint8_t io_num)
{
  uint16_t io_pin = 0;
  GPIO_TypeDef* gpio_port = GPIOA;
  io_pin = (uint16_t)(pow(2,(io_num & 0x0f)));

  if(io_num>=0x10 && io_num <0x20)
	gpio_port = GPIOA;
  else if (io_num>=0x20 && io_num <0x30)
 	gpio_port = GPIOB;
  else if (io_num>=0x30 && io_num <0x40)
 	gpio_port = GPIOC;
  else if (io_num>=0x40 && io_num <0x50)
 	gpio_port = GPIOD;
  else if (io_num>=0x50 && io_num <0x60)
 	gpio_port = GPIOE;
  else if (io_num>=0x60 && io_num <0x70)
 	gpio_port = GPIOF;
  else if (io_num>=0x70 && io_num <0x80)
 	gpio_port = GPIOG;
  else 
 	return;

//    GPIO_ToggleBits(gpio_port, io_pin);
    HAL_GPIO_TogglePin(gpio_port, io_pin);
} 


void DAC_DM_Output_Res(uint8_t res)
{
       GPIO_Setting(SW_DCD_RES_1,0);	//DAC_DM + LOW to DM
	   GPIO_Setting(SW_DCD_RES_2,0);	//DAC_DM + 200R to DM
	   GPIO_Setting(SW_DCD_RES_3,0);	//DAC_DM + 3K9 to DM
	   GPIO_Setting(SW_DCD_RES_4,0);	//DAC_DM + 15K to DM
	   GPIO_Setting(SW_DCD_RES_5,0);	//DAC_DM + 100K to DM
	   GPIO_Setting(SW_DCD_RES_6,0);	//DAC_DM + ??? to DM
   switch (res)
   	{
	   case RES_1K5:
	  	 	GPIO_Setting(SW_DCD_RES_1,1);	
	   		break;
	   case RES_200R:
	  	 	GPIO_Setting(SW_DCD_RES_2,1);	
	   		break;
	   case RES_3K9:
	  	 	GPIO_Setting(SW_DCD_RES_3,1);	
	   		break;
	   case RES_15K:
	 		GPIO_Setting(SW_DCD_RES_4,1);	
			break;	   
       case RES_100K:
	 		GPIO_Setting(SW_DCD_RES_5,1);	
			break;	   
	   case RES_unused:
	 		GPIO_Setting(SW_DCD_RES_6,1);
	   case RES_NONE :	
			break;
	   default:
	   
	 		break;

	 }
}

// 2020-07-23: fixed to RES_7==> 200R , RES_8==> 1K5
void DAC_DP_Output_Res(uint8_t res)
{
     GPIO_Setting(SW_DCD_RES_8,0);	//DAC_DP + 1K5 to DP
	   GPIO_Setting(SW_DCD_RES_7,0);	//DAC_DP + 200R to DP
	   GPIO_Setting(SW_DCD_RES_9,0);	//DAC_DP + 15K to DP
	   GPIO_Setting(SW_DCD_RES_10,0);	//DAC_DP + 100K to DP
	   GPIO_Setting(RSW_DCD_CAP,0);		//DAC_DP + 1nF CAP to DM

   switch (res)
   	{
	   case RES_200R:
	  	 	GPIO_Setting(SW_DCD_RES_7,1);	
	   		break;
	   case RES_1K5:
	  	 	GPIO_Setting(SW_DCD_RES_8,1);	
	   		break;
	   case RES_15K:
	 		GPIO_Setting(SW_DCD_RES_9,1);	
			break;	   
       case RES_100K:
	 		GPIO_Setting(SW_DCD_RES_10,1);	
			break;	   
	   case CAP_1nF:
	 		GPIO_Setting(RSW_DCD_CAP,1);
	   case RES_NONE :	
			break;
	   default:	   
	 		break;
	 }
}

void powerDM(uint16_t milVol, uint8_t resId  )
{
	DAC_Setting(DAC_DM , milVol);
	DAC_DM_Output_Res(resId);

}

void powerDP(uint16_t milVol, uint8_t resId  )
{
	 DAC_Setting(DAC_DP , milVol);
	 DAC_DP_Output_Res(resId);

 }

void VBUS_CAP_Setting(uint8_t cap)
{
	   GPIO_Setting(RSW_VBUS_CAP_1,0);	//150nF
	   GPIO_Setting(RSW_VBUS_CAP_2,0);	//900nF
	   GPIO_Setting(RSW_VBUS_CAP_3,0);	//1uF
	   GPIO_Setting(RSW_VBUS_CAP_4,0);	//5.5uF
	   GPIO_Setting(RSW_VBUS_CAP_5,0); //10uF
	   GPIO_Setting(RSW_VBUS_CAP_6,0); //100uF

	    switch (cap)
   	{
	   case CAP_150nF:
	  	 	GPIO_Setting(RSW_VBUS_CAP_1,1);	
	   		break;
	   case CAP_900nF:
	  	 	GPIO_Setting(RSW_VBUS_CAP_2,1);	
	   		break;
	   case CAP_1uF:
	 		GPIO_Setting(RSW_VBUS_CAP_3,1);	
			break;	   
       case CAP_5uF5:
	 		GPIO_Setting(RSW_VBUS_CAP_4,1);	
			break;	   
	   case CAP_6uF5:
	 		GPIO_Setting(RSW_VBUS_CAP_3,1);
			GPIO_Setting(RSW_VBUS_CAP_4,1);
			break;
	   case CAP_10uF :
	   		GPIO_Setting(RSW_VBUS_CAP_5,1);
			break;
	   case CAP_100uF :
	   		GPIO_Setting(RSW_VBUS_CAP_6,1);
			break;
	   case CAP_NONE :
	  	    break;	
	
	   default:	   
	 		break;
	 }

}



void SetLoadCurrentRange(uint8_t range)
{
	  if(range == LO_RANGE)
	  {
	  GPIO_Setting(SW_LOAD_CF_switch, ON);
	  LoadCurrentRange = LO_RANGE;
	  }
	  else if(range == HI_RANGE)
	  {
 	   GPIO_Setting(SW_LOAD_CF_switch, OFF);
	   LoadCurrentRange = HI_RANGE;
	  }	
}


void SetVbusCurrentRange(uint8_t range)
{

	  if(range == LO_RANGE)
	  {
	  GPIO_Setting(SW_VBUS_CF_switch, ON);
	  VbusCurrentRange = LO_RANGE;
	  }
	  else if(range == HI_RANGE)
	  {
 	   GPIO_Setting(SW_VBUS_CF_switch, OFF);
	   VbusCurrentRange = HI_RANGE;
	  }	

}

void SetCurrentRangeMode(uint8_t name,uint8_t mode)
{
	if(name == ADC_LOAD_I)
	 {
	 LoadCurrentRangeMode = mode;
	 SetLoadCurrentRange(mode);
	 }
	 else if (name == ADC_VBUS_I)
	 {
	 VbusCurrentRangeMode = mode;
	 SetVbusCurrentRange(mode)	;
	 }
}


// used for after deleted test task, reset measureing hardware.
void MEAS_HW_RST(void)
 {  
	powerDP(0,RES_NONE);
	powerDM(0,RES_NONE);
	DAC_Setting(DAC_VBUS_V , 0);
	DAC_Setting(DAC_LOAD_I , 0);
	DAC_Setting(DAC_VBUS_IL , 0);
	DAC_Setting(DAC_LOAD_V , 0);
	DAC_Setting(DAC_CM_V , 0);
    vTaskDelay(1000);
	GPIO_Setting(RSW_VBUS_AB,OFF);
	GPIO_Setting(RSW_EL_TO_VBUS_AB,OFF);
	GPIO_Setting(SW_VBUS_AB_M1,ON);
	GPIO_Setting(RSW_DMDP_TEST,OFF);
	GPIO_Setting(RSW_VBUS_ACC,OFF);
	GPIO_Setting(RSW_VBUS_CHG,OFF);
	GPIO_Setting(RSW_VBUS_ACC,OFF);
	GPIO_Setting(SW_ID_RES_M_1,OFF);
	GPIO_Setting(SW_ID_RES_M_2,OFF);
	VBUS_CAP_Setting(CAP_NONE);
	GPIO_Setting(SW_GND_AB_M2,OFF);
	GPIO_Setting(RSW_VCC_PHY,OFF);
	SetCurrentRangeMode(ADC_VBUS_I,AUTO_RANGE);
	SetCurrentRangeMode(ADC_LOAD_I,AUTO_RANGE);		
}

uint8_t GetCurrentRangeMode(uint8_t name)
{
	 uint8_t mode = AUTO_RANGE;
     if(name == ADC_LOAD_I)
	 {
	 mode = LoadCurrentRangeMode;
	 }
	 else if (name == ADC_VBUS_I)
	 {
	 mode = VbusCurrentRangeMode;
	 }
	 return  mode;

}

uint8_t GetLoadCurrentRange()
{
	 return   LoadCurrentRange;
}

uint8_t GetVbusCurrentRange()
{
	  return VbusCurrentRange;
}


