/*
 * wavPlayer.h
 *
 *  Created on: Sep 23, 2025
 *      Author: Administrator
 */

#ifndef INC_WAVPLAYER_H_
#define INC_WAVPLAYER_H_

#include "fatfs.h"
#include "stdint.h"
#include "stdio.h"
#include "i2s.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"

#define BUFFER_SIZE 2048







typedef struct {
	uint8_t riff[4];         // "RIFF"
	uint32_t file_size;       // 文件大小 - 8
	uint8_t wave[4];         // "WAVE"
	uint8_t fmt[4];          // "fmt "
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
	uint8_t buffer[2048];    // 双缓冲区
	uint8_t buffer2[2048];   // 双缓冲区
	uint8_t current_buffer;  // 当前使用的缓冲区
	uint8_t playing;         // 播放状态
} AudioPlayer;

void testRead();



 FRESULT res;
 FIL file;
 UINT bytesRead;
 uint8_t buffer[BUFFER_SIZE];  // 读取缓冲区
 uint32_t totalRead = 0;
 const char *filename = "1.wav";
 WavHeader wavHeader;          // 新增：用于解析WAV头部
 uint8_t sample_bytes = 2;     // 默认16位采样（2字节），后续会更新

 int playFlag=0;




#endif /* INC_WAVPLAYER_H_ */
