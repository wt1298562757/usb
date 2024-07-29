#ifndef __GPIO_CONTROL_H
#define __GPIO_CONTROL_H
#include "stm32f4xx.h"
#include "freertos.h"
#include "task.h"

void GPIO_Setting (uint8_t io_num,uint16_t para);
uint8_t GPIO_Reading (uint8_t io_num);
void GPIO_Toggle(uint8_t io_num);
void powerDM(uint16_t milVol, uint8_t resId  );
void powerDP(uint16_t milVol, uint8_t resId  );
void VBUS_CAP_Setting(uint8_t cap);
void DAC_DM_Output_Res(uint8_t res);
void DAC_DP_Output_Res(uint8_t res);
void SetVbusCurrentRange(uint8_t range);
void SetLoadCurrentRange(uint8_t range);
void SetCurrentRangeMode(uint8_t name,uint8_t mode);

#endif