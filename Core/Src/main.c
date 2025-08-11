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
void ListFiles(const TCHAR* path)
{
    FRESULT res;
    DIR dir;               // 目录对象
    FILINFO fno;           // 文件信息结构体

    // 打开目录
    res = f_opendir(&dir, path);
    if (res != FR_OK) {
        printf("打开目录失败！错误码: %d\r\n", res);
        return;
    }

    printf("\r\n目录 %s 下的文件列表：\r\n", path);
    printf("----------------------------------------\r\n");

    // 循环读取目录项（f_readdir返回FR_OK且fno.fname不为空时继续）
    while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
        // 判断是否为子目录（文件名以'/'结尾，或fno.fattrib包含AM_DIR属性）
        if (fno.fattrib & AM_DIR) {
            printf(" [目录]  %s\r\n", fno.fname);  // 子目录
        } else {
            // 普通文件：输出文件名和大小（单位：字节）
            printf(" [文件]  %s  (大小: %ld 字节)\r\n", fno.fname, fno.fsize);
        }
    }

    printf("----------------------------------------\r\n");
    printf("目录遍历完成\r\n");

    // 关闭目录
    f_closedir(&dir);
}
void PrintHexData(uint8_t* buffer, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++) {
        // 格式化输出单个字节（%02X表示两位16进制，不足补0）
        printf("%02X ", buffer[i]);

        // 每16个字节换行，方便查看
        if ((i + 1) % 16 == 0) {
            printf("\r\n");
        }
    }
    printf("\r\n"); // 最后补一个换行
}
void ReadPCMFile(const char* filename)
{
    FRESULT res;
    FIL file;
    UINT bytesRead;
    uint8_t buffer[512];  // 读取缓冲区，大小可根据需要调整
    uint32_t totalRead = 0;

    // 打开文件（只读模式）
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK) {
        printf("打开文件 %s 失败！错误码: %d\r\n", filename, res);
        return;
    }

    printf("成功打开文件 %s，开始读取...\r\n", filename);
    printf("文件大小: %ld 字节\r\n", f_size(&file));

    // 循环读取文件内容
    do {
        // 读取数据到缓冲区
        res = f_read(&file, buffer, sizeof(buffer), &bytesRead);
        if (res != FR_OK) {
            printf("读取文件失败！错误码: %d\r\n", res);
            break;
        }
        //PrintHexData(buffer, bytesRead);
        // 累计读取字节数
        totalRead += bytesRead;

        // 这里可以添加处理PCM数据的代码
        // 例如：通过DMA发送到DAC播放，或通过串口转发等
        // 示例：打印读取进度
        if (totalRead % (1024 * 10) == 0) {  // 每读取10KB打印一次进度
            printf("已读取: %ld KB\r\n", totalRead / 1024);
        }

    } while (bytesRead == sizeof(buffer));  // 直到读取的字节数小于缓冲区大小

    // 关闭文件
    f_close(&file);

    printf("文件读取完成，总读取字节数: %ld 字节\r\n", totalRead);
}


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
  MX_USART1_UART_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
  FRESULT retSD = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
    printf("%u\r\n",retSD);
    printf("Compilation Date: %s %s\n", __DATE__, __TIME__);
     // mount SD card
  //   int retSD = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
     if (retSD == FR_OK) {

    	 ListFiles("/");

    	 ReadPCMFile("o.pcm");

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
