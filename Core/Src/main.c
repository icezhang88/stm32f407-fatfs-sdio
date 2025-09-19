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
#include "i2s.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "filemanager.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
// WAV文件头部结构
typedef struct {
    uint8_t  riff[4];         // "RIFF"
    uint32_t file_size;       // 文件大小 - 8
    uint8_t  wave[4];         // "WAVE"
    uint8_t  fmt[4];          // "fmt "
    uint32_t fmt_size;        // 格式块大小
    uint16_t audio_format;    // 音频格式 (1 = PCM)
    uint16_t num_channels;    // 声道数
    uint32_t sample_rate;     // 采样率
    uint32_t byte_rate;       // 字节率
    uint16_t block_align;     // 块对齐
    uint16_t bits_per_sample; // 每样本位数
} WavHeader;

// 音频播放控制结构体
typedef struct {
    FIL file;                 // 文件对象
    uint32_t data_start;      // 数据起始位置
    uint32_t data_size;       // 数据大小
    uint32_t bytes_played;    // 已播放字节数
    uint8_t  buffer[2048];    // 双缓冲区
    uint8_t  buffer2[2048];   // 双缓冲区
    uint8_t  current_buffer;  // 当前使用的缓冲区
    uint8_t  playing;         // 播放状态
} AudioPlayer;

int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t*) &ch, 1, 0xFFFFFFFF);
    return ch;
}

int __io_getchar(void) {
    char rxChar;
    while (HAL_UART_Receive(&huart1, (uint8_t*) &rxChar, 1, 0xFFFFFFFF) == HAL_TIMEOUT);
    return rxChar;
}

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BUFFER_SIZE 2048
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
AudioPlayer player = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s);
uint8_t WavOpen(const char *filename, WavHeader *header, AudioPlayer *player);
void WavPlay(AudioPlayer *player);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

// 解析WAV文件头部并打开文件
uint8_t WavOpen(const char *filename, WavHeader *header, AudioPlayer *player) {
    FRESULT res;
    UINT bytesRead;

    // 打开文件
    res = f_open(&player->file, filename, FA_READ);
    if (res != FR_OK) {
        printf("open file failed %s error code: %d\r\n", filename, res);
        return 0;
    }

    // 读取WAV头部
    res = f_read(&player->file, header, sizeof(WavHeader), &bytesRead);
    if (res != FR_OK || bytesRead != sizeof(WavHeader)) {
        printf("read WAVhead failed error code : %d\r\n", res);
        f_close(&player->file);
        return 0;
    }

    // 验证WAV文件
    if (memcmp(header->riff, "RIFF", 4) != 0 || memcmp(header->wave, "WAVE", 4) != 0 ||
        memcmp(header->fmt, "fmt ", 4) != 0 || header->audio_format != 1) {
        printf("not ok PCM WAV file\r\n");
        f_close(&player->file);
        return 0;
    }

    // 打印音频信息
    printf("WAV file info :\r\n");
    printf("  sample rate : %d Hz\r\n", header->sample_rate);
    printf("  channel count: %d\r\n", header->num_channels);
    printf("  bit sample : %d bits\r\n", header->bits_per_sample);

    // 查找数据块
    uint8_t chunk_id[4];
    uint32_t chunk_size;
    while (1) {
        res = f_read(&player->file, chunk_id, 4, &bytesRead);
        if (res != FR_OK || bytesRead != 4) break;

        res = f_read(&player->file, &chunk_size, 4, &bytesRead);
        if (res != FR_OK || bytesRead != 4) break;

        // 找到数据块
        if (memcmp(chunk_id, "data", 4) == 0) {
            player->data_start = f_tell(&player->file);
            player->data_size = chunk_size;
            player->bytes_played = 0;
            player->playing = 1;
            printf("find data block : %ld bytes\r\n", chunk_size);
            return 1;
        }

        // 跳过当前块
        f_lseek(&player->file, f_tell(&player->file) + chunk_size);
    }

    printf("未找到数据块！\r\n");
    f_close(&player->file);
    return 0;
}

// 填充缓冲区
uint8_t FillBuffer(AudioPlayer *player, uint8_t *buffer, uint32_t size) {
    UINT bytesRead;
    uint32_t bytesToRead = size;

    // 检查是否还有数据可读取
    if (player->bytes_played >= player->data_size) {
        return 0;
    }

    // 计算实际可读取的字节数
    if (player->bytes_played + bytesToRead > player->data_size) {
        bytesToRead = player->data_size - player->bytes_played;
    }

    // 读取数据
    FRESULT res = f_read(&player->file, buffer, bytesToRead, &bytesRead);
    if (res != FR_OK) {
        printf("读取音频数据失败！错误码: %d\r\n", res);
        return 0;
    }

    // 填充剩余空间（如果需要）
    if (bytesRead < bytesToRead) {
        memset(buffer + bytesRead, 0, bytesToRead - bytesRead);
    }

    player->bytes_played += bytesRead;
    return 1;
}

// 开始播放WAV文件
void WavPlay(AudioPlayer *player) {
    // 填充两个缓冲区
    if (!FillBuffer(player, player->buffer, BUFFER_SIZE)) {
        player->playing = 0;
        return;
    }

    if (!FillBuffer(player, player->buffer2, BUFFER_SIZE)) {
        player->playing = 0;
        return;
    }

    // 开始播放第一个缓冲区
    player->current_buffer = 0;
    HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)player->buffer, BUFFER_SIZE / 2);
}

// I2S传输完成回调函数
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {



    if (!player.playing) return;

    // 切换缓冲区
    if (player.current_buffer == 0) {
        // 填充第一个缓冲区，同时播放第二个
        if (!FillBuffer(&player, player.buffer, BUFFER_SIZE)) {
            player.playing = 0;
            return;
        }
        player.current_buffer = 1;
        HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)player.buffer2, BUFFER_SIZE / 2);
    } else {
        // 填充第二个缓冲区，同时播放第一个
        if (!FillBuffer(&player, player.buffer2, BUFFER_SIZE)) {
            player.playing = 0;
            return;
        }
        player.current_buffer = 0;
        HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)player.buffer, BUFFER_SIZE / 2);
    }

    // 检查是否播放完成
    if (player.bytes_played >= player.data_size) {
        printf("play complete\r\n");
        player.playing = 0;
        f_close(&player.file);
    }
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  WavHeader wavHeader;
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
  MX_I2S2_Init();
  /* USER CODE BEGIN 2 */
  printf("system init success，read play WAV file...\r\n");
  initFileManager();

  // 打开WAV文件并开始播放
  if (WavOpen("1.wav", &wavHeader, &player)) {
      printf("start play WAV file ...\r\n");
      WavPlay(&player);
  } else {
      printf("cant play WAV file \r\n");
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // 可以在这里添加其他处理逻辑
    HAL_Delay(100);
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
  while (1) {
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
