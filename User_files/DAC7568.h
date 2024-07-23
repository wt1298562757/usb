#ifndef __DAC7568__
#define __DAC7568__


#include <stdint.h>

/**
  * @brief  DAC SPI Interface pins
  */  
#define sDAC_SPI                           SPI1
#define sDAC_SPI_Handle                    hspi1
#define sDAC_SPI_CLK                       RCC_APB2Periph_SPI1
#define sDAC_SPI_CLK_INIT                  RCC_APB2PeriphClockCmd


#define sDAC_SPI_SCK_PIN               GPIO_Pin_5                  /* PA.05 */
#define sDAC_SPI_SCK_GPIO_PORT         GPIOA                       /* GPIOA */
#define sDAC_SPI_SCK_GPIO_CLK          RCC_AHB1Periph_GPIOB					
#define sDAC_SPI_SCK_SOURCE            GPIO_PinSource5
#define sDAC_SPI_SCK_AF                GPIO_AF_SPI1

#define sDAC_SPI_MOSI_PIN              GPIO_Pin_7                  /* PA.07 */
#define sDAC_SPI_MOSI_GPIO_PORT        GPIOA                       /* GPIOA */
#define sDAC_SPI_MOSI_GPIO_CLK         RCC_AHB1Periph_GPIOA
#define sDAC_SPI_MOSI_SOURCE           GPIO_PinSource7
#define sDAC_SPI_MOSI_AF  			   GPIO_AF_SPI1

#define sDAC_CS_PIN                    GPIO_PIN_4                 /* PA.04 */
#define sDAC_CS_GPIO_PORT              GPIOA                       /* GPIOA */
#define sDAC_CS_GPIO_CLK               RCC_AHB1Periph_GPIOA

#define sDAC_CS_LOW()       HAL_GPIO_WritePin(sDAC_CS_GPIO_PORT, sDAC_CS_PIN, GPIO_PIN_RESET);
#define sDAC_CS_HIGH()      HAL_GPIO_WritePin(sDAC_CS_GPIO_PORT, sDAC_CS_PIN, GPIO_PIN_SET);





//DAC DEFS

#define DAC_CMD_WRITE_SELECTED		0x00000000 	 /*Write to Selected DAC Input Register*/
#define DAC_CMD_UPDATE_SELECTED		0x01000000 	 /*Update Selected DAC Registers*/
#define DAC_CMD_UPDATE_ALL  		0x02000000 	 /*Write to Selected DAC Input Register and Update All DAC Registers*/
#define DAC_CMD_UPDATE_RESPECTIVE  	0x03000000 	 /*Write to Selected DAC Input Register and Update Respective DAC Register*/
#define DAC_CMD_CLEARE_TO_ZERO		0x05000000 	 	/*Write to clear code register; clear to zero scale*/
#define DAC_CMD_CLEARE_TO_2V5	0x05000001 	 /*Write to clear code register; clear to midscale*/
#define DAC_CMD_CLEARE_TO_5V	0x05000002 	 /*Write to clear code register; clear to FULLSCALE*/
#define DAC_CMD_CLEARE			0x05000000 	 /*Write to clear code register; ignore CLR pin	  */
#define DAC_CMD_WRITE_TO_LDAC		0x06000000 	 /*Write to LDAC Register*/
#define DAC_CMD_SOFT_RESET  		0x07000000 	 /*Write to LDAC Register*/
#define DAC_CMD_PWR_OPERATION  		    0x04000000 	 /*Power-Down Commands*/
#define DAC_CMD_INTERNAL_REF_CMD_1  	0x08000000 	 /*Power down internal reference - static mode*/
#define DAC_CMD_INTERNAL_REF_CMD_2		0x08000001 	 /*Power up internal reference - static mode*/
#define DAC_CMD_INTERNAL_REF_CMD_3		0x09080000 	 /*Power up internal reference - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_4	    0x090A0000 	 /*Power up internal reference - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_5		0x090C0000 	 /*Power down internal reference all the time regardless of state of DACs - flexible mode*/
#define DAC_CMD_INTERNAL_REF_CMD_6		0x09000000 	 /*Switching internal reference mode from flexible mode to static mode*/
#define DAC_CH_A                		0x00000000 	 /*SELECT DAC Channel A*/
#define DAC_CH_B                		0x00100000 	 /*SELECT DAC Channel B*/
#define DAC_CH_C                		0x00200000 	 /*SELECT DAC Channel C*/
#define DAC_CH_D                		0x00300000 	 /*SELECT DAC Channel D*/
#define DAC_CH_E                		0x00400000 	 /*SELECT DAC Channel E*/
#define DAC_CH_F                		0x00500000 	 /*SELECT DAC Channel F*/
#define DAC_CH_G                		0x00600000 	 /*SELECT DAC Channel G*/
#define DAC_CH_H                		0x00700000 	 /*SELECT DAC Channel H*/
#define DAC_CH_ALL                		0x00F00000 	 /*Broadcast mode - Write to all DAC channels*/


extern void DAC_Setting(uint8_t dac_name,int16_t dac_value);
extern void DAC_Setting_link(uint8_t dac_name,int16_t dac_value);

#endif /* __DAC7568__ */