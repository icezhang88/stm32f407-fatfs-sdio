/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "fatfs.h"
#include "i2s.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
osSemaphoreId_t i2sDmaSemaphore;
const osSemaphoreAttr_t i2sDmaSemaphore_attr = {
  .name = "i2sDmaSemaphore"
};
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

FIL file;
#define BUFFER_SIZE 2048
uint8_t buffer[BUFFER_SIZE];  // 读取缓冲区
uint8_t sample_bytes = 2;     // 默认16位采样（2字节），后续会更新
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

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
	 i2sDmaSemaphore = osSemaphoreNew(1, 0, &i2sDmaSemaphore_attr);

  /* USER CODE END RTOS_SEMAPHORES */
//	 audioPlayerTaskHandle = osThreadNew(StartAudioPlayerTask, NULL, &audioPlayerTask_attributes);
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
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
	 /* 初始化文件管理器 */
	  initFileManager();
	  const char *filename = "1.wav";
	  /* 打开WAV文件并解析头部 */
	 FRESULT res = f_open(&file, filename, FA_READ);
	  if (res != FR_OK) {
	    printf("Failed to open WAV file\r\n");
	    // 任务出错，进入休眠
	    for(;;) {
	      osDelay(1000);
	    }
	  }

	  /* 启动第一次DMA传输 */
	  readAndPlayNextBuffer();

	  /* 任务主循环 */
	  for(;;) {
	    // 等待DMA传输完成信号
	    if (osSemaphoreAcquire(i2sDmaSemaphore, osWaitForever) == osOK) {
	      // 读取并播放下一段音频数据
	      if (!readAndPlayNextBuffer()) {
	        // 播放完成，这里可以处理循环播放或停止播放
	        printf("Audio playback completed\r\n");
	        // 如果需要循环播放，可以在这里重新打开文件
	        // openWavFile("test.wav");
	        // readAndPlayNextBuffer();
	      }
	    }
	  }

  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* I2S DMA传输完成回调函数 */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
  // 释放信号量，唤醒音频播放任务
  osSemaphoreRelease(i2sDmaSemaphore);
}

/* 读取并播放下一个缓冲区的数据 */
int readAndPlayNextBuffer() {
  uint32_t bytesRead;
  FRESULT res;

  // 读取数据
  res = f_read(&file, buffer, BUFFER_SIZE, &bytesRead);
  if (res != FR_OK || bytesRead == 0) {
    return 0;
  }

  // 启动I2S DMA传输
  if (HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)buffer, bytesRead / sample_bytes) != HAL_OK) {
    return 0;
  }

  return 1;
}

/* USER CODE END Application */

