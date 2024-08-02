/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//---------------ADD WT--------------------------
#include "usbd_cdc_if.h"
//----------------------------------------------------

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
//--------------ADD WT-----------------------
uint8_t usb_ep1_rx[2048];
uint8_t usb_ep1_tx[40960];

uint8_t usb_ep3_rx[2048];
uint8_t usb_ep3_tx[40960];

uint8_t usb_ep4_rx[2048];
uint8_t usb_ep4_tx[40960];

volatile int8_t usb_ep1_rxne = RESET;
volatile int8_t usb_ep3_rxne = RESET;
volatile int8_t usb_ep4_rxne = RESET;
volatile int8_t usb_ep3_txne = RESET;
volatile int8_t usb_ep4_txne = RESET;

extern USBD_HandleTypeDef hUsbDeviceHS;
//-----------------------------------------------

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void WinUSB_Receive_HS()
{
	uint16_t i = 0;
	if (usb_ep1_rxne == SET)
	{
		i = usb_ep1_rx[0] + (usb_ep1_rx[1]<<8);
		CDC_Transmit_HS(usb_ep1_rx, i, CDC_IN_EP);
		usb_ep1_rxne = RESET;
	}
	else if(usb_ep3_txne == SET)
	{
		i = usb_ep3_rx[0] + (usb_ep3_rx[1]<<8);
		CDC_Transmit_HS(usb_ep3_rx, i, FS_LINK_IN_EP);
		usb_ep3_rxne = RESET;
	}
  else if(usb_ep4_rxne == SET)
	{
		i = usb_ep4_rx[0] + (usb_ep4_rx[1]<<8);
		CDC_Transmit_HS(usb_ep4_rx, i, EP_ADC_IN);
		usb_ep4_rxne = RESET;
	}
  else if(usb_ep3_rxne == SET)
	{
		i = usb_ep3_rx[0] + (usb_ep3_rx[1]<<8);
		CDC_Transmit_HS(usb_ep3_rx, i, FS_LINK_OUT_EP);
		usb_ep3_rxne = RESET;
	}
	
}
//------------------------------------------

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
		WinUSB_Receive_HS();
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

