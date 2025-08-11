
:::highlight purple 💡
FATFS 配置输入引脚 一定要上拉 不然无法初始化
:::
![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554176/image-preview)


![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554177/image-preview)


![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554178/image-preview)


![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554179/image-preview)


![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554180/image-preview)


![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554181/image-preview)



![image.png](https://api.apifox.com/api/v1/projects/5549792/resources/554182/image-preview)




```js
/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
void SDCard_ShowInfo(void)
{
    // SD卡信息结构体变量
    HAL_SD_CardInfoTypeDef cardInfo;
    HAL_StatusTypeDef res = HAL_SD_GetCardInfo(&hsd, &cardInfo);
    if (res != HAL_OK) {
        printf("HAL_SD_GetCardInfo() error\r\n");
        return;
    }
    printf("\r\n*** HAL_SD_GetCardInfo() info ***\r\n");
    printf("Card Type= %ld\r\n", cardInfo.CardType);
    printf("Card Version= %ld\r\n", cardInfo.CardVersion);
    printf("Card Class= %ld\r\n", cardInfo.Class);
    printf("Relative Card Address= %ld\r\n", cardInfo.RelCardAdd);
    printf("Block Count= %ld\r\n", cardInfo.BlockNbr);
    printf("Block Size(Bytes)= %ld\r\n", cardInfo.BlockSize);
    printf("LogiBlockCount= %ld\r\n", cardInfo.LogBlockNbr);
    printf("LogiBlockSize(Bytes)= %ld\r\n", cardInfo.LogBlockSize);
    printf("SD Card Capacity(MB)= %ld\r\n", cardInfo.BlockNbr >> 1 >> 10);
}
// 获取SD卡当前状态
void SDCard_ShowStatus(void)
{
    // SD卡状态结构体变量
    HAL_SD_CardStatusTypeDef cardStatus;
    HAL_StatusTypeDef res = HAL_SD_GetCardStatus(&hsd, &cardStatus);
    if (res != HAL_OK) {
        printf("HAL_SD_GetCardStatus() error\r\n");
        return;
    }
    printf("\r\n*** HAL_SD_GetCardStatus() info ***\r\n");
    printf("DataBusWidth= %d\r\n", cardStatus.DataBusWidth);
    printf("CardType= %d\r\n", cardStatus.CardType);
    printf("SpeedClass= %d\r\n", cardStatus.SpeedClass);
    printf("AllocationUnitSize= %d\r\n", cardStatus.AllocationUnitSize);
    printf("EraseSize= %d\r\n", cardStatus.EraseSize);
    printf("EraseTimeout= %d\r\n", cardStatus.EraseTimeout);
}
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
int __io_putchar(int ch)
{
    /* Implementation of __io_putchar */
	/* e.g. write a character to the UART1 and Loop until the end of transmission */
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xFFFFFFFF);

    return ch;
}
int __io_getchar(void)
{
  /* Implementation of __io_getchar */
    char rxChar;

    // This loops in case of HAL timeout, but if an ok or error occurs, we continue
    while (HAL_UART_Receive(&huart1, (uint8_t *)&rxChar, 1, 0xFFFFFFFF) == HAL_TIMEOUT);

    return rxChar;
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  FRESULT retSD = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
  printf("%u\r\n",retSD);
  printf("Compilation Date: %s %s\n", __DATE__, __TIME__);
   // mount SD card
//   int retSD = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
   if (retSD == FR_OK) {
       printf("》Filesystem mount ok, now you can read/write files.\r\n");
       // 创建或者打开文件 SD_Card_test.txt
       retSD = f_open(&SDFile, "SD_Card_test.txt", FA_OPEN_ALWAYS | FA_WRITE);
       if (retSD == FR_OK) {
           printf("》open/create SD_Card_test.txt OK, write data to it.\r\n");

           // Move to end of the file to append data
           retSD = f_lseek(&SDFile, f_size(&SDFile));
           if (retSD == FR_OK) {
               f_printf(&SDFile, "SD card FATFS test.\r\n");
               printf("》write data to file OK, write bytes: SD card FATFS test.\r\n");
           } else {
               printf("!! File Write error: (%d)\n", retSD);
           }
           /* close file */
           f_close(&SDFile);
       } else {
           printf("!! open/Create file SD_Card_test.txt Fail (%d).\r\n", retSD);
       }
   } else {
       printf("!! SDcard mount filesystem error。(%d)\r\n", retSD);
   }
   // 不带fatfs调试函数
   SDCard_ShowInfo();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

```
