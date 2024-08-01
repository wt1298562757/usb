/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : flash_if.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : specific media access Layer for internal flash
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "PET_CMD.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
// #include "ucpet_bsp.h"


/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbytes */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbytes */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbytes */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbytes */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t StartSector = 0;
uint32_t EndSector = 0;
uint32_t i=0;



/* Private function prototypes -----------------------------------------------*/

void FLASH_If_Write_Cor_Data(int8_t* pCorData);
void FLASH_If_Load_Cor_Data(int8_t* pCorData) ;




/* Private functions ---------------------------------------------------------*/


uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_SECTOR_11;  
  }

  return sector;
}


/*******************************************************************************
* Function Name  : FLASH_If_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t FLASH_If_Init(void)
{
    /* Unlock the Flash to enable the flash control register access *************/ 
  FLASH_Unlock();
  return 1;
}

/*******************************************************************************
* Function Name  : FLASH_If_Erase
* Description    : Erase sector
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t FLASH_If_Erase(uint32_t SectorAddress)
{
  

  FLASH_Unlock();
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 

     //  be done by word 
    if (FLASH_EraseSector(GetSector(COR_DATA_ADDRESS), VoltageRange_3) != FLASH_COMPLETE)
    { 
      // Error occurred while sector erase. 
      //  User can add here some code to deal with this error
	  GPIO_Setting(SW_LED1_R,ON);

      while (1)
      {
      }
    }
 		


  FLASH_Lock();
  return 1;
}

/*******************************************************************************
* Function Name  : FLASH_If_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  FLASH_If_Write_Cor_Data(int8_t* pCorData)
{

  uint32_t idx = 0;

  uint32_t DataLength;
  uint32_t SectorAdress = COR_DATA_ADDRESS;
 

  	FLASH_If_Erase(COR_DATA_ADDRESS)	;
    
	 FLASH_Unlock();

  //¿ªÊ¼Ð´Èë

  /* Data received are Word multiple */
	DataLength = MAX_ADC_NUM* MAX_CAL_NUM;

	   for(idx=0;idx<DataLength;idx = idx + 4)
	   {
		  if( FLASH_ProgramWord(SectorAdress, *(uint32_t *)(pCorData + idx))== FLASH_COMPLETE)
		  {
		  	SectorAdress += 4;
		   }
		   else
		   {	
			 GPIO_Setting(SW_LED1_R,ON);
		//	 Delay(500);
		//	 GPIO_Setting(SW_LED1_G,OFF);
		  } 
    	  
	   }
	 //CHECK



  FLASH_Lock();

}

/*******************************************************************************
* Function Name  : FLASH_If_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : buffer address pointer
*******************************************************************************/
void FLASH_If_Load_Cor_Data(int8_t* pCorData)
{
    uint32_t idx = 0;
    uint32_t DataLength;


	//	 FLASH_Unlock();

	DataLength = MAX_ADC_NUM* MAX_CAL_NUM;

	   for(idx=0;idx<DataLength;idx = idx + 4)
	   {
		*(uint32_t*)( pCorData + idx) = *(uint32_t *)(COR_DATA_ADDRESS+idx);

	   }
     
	 //    FLASH_Lock();


}





/*******************************************************************************
* Function Name  : FLASH_If_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : buffer address pointer
*******************************************************************************/



/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
