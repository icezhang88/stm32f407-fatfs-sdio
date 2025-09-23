/*
 * wavPlayer.c
 *
 *  Created on: Sep 23, 2025
 *      Author: Administrator
 */


#include "wavPlayer.h"




void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {

	//printf("HAL_I2S_TxCpltCallback\r\n");
}


void testRead() {

	   FRESULT res;
	    FIL file;
	    UINT bytesRead;
	    uint8_t buffer[BUFFER_SIZE];  // 读取缓冲区
	    uint32_t totalRead = 0;
	    const char *filename = "1.wav";
	    WavHeader wavHeader;          // 新增：用于解析WAV头部
	    uint8_t sample_bytes = 2;     // 默认16位采样（2字节），后续会更新

	    // 1. 打开文件
	    res = f_open(&file, filename, FA_READ);
	    if (res != FR_OK) {
	        printf("open %s failed error code: %d\r\n", filename, res);
	        return;
	    }
	    printf("sizeof wavHead:%d\r\n",sizeof(wavHeader));
	    // 2. 解析WAV头部（关键：获取采样位数）
	    res = f_read(&file, &wavHeader, sizeof(WavHeader), &bytesRead);
	    if (res != FR_OK || bytesRead != sizeof(WavHeader)) {
	        printf("read WAV header failed! code: %d\r\n", res);
	        f_close(&file); // 修复：用f_close关闭文件
	        return;
	    }
	    // 验证PCM格式
	    if (memcmp(wavHeader.riff, "RIFF", 4) != 0 || memcmp(wavHeader.wave, "WAVE", 4) != 0 || wavHeader.audio_format != 1) {
	        printf("not PCM WAV file!\r\n");
	        f_close(&file);
	        return;
	    }
	    // 更新每个采样的字节数（修复传输长度计算依据）
	    sample_bytes = wavHeader.bits_per_sample / 8;
	    printf("WAV info: sample rate=%d, bits=%d, sample bytes=%d\r\n",
	           wavHeader.sample_rate, wavHeader.bits_per_sample, sample_bytes);

	    // 3. 查找WAV数据块（跳过头部扩展信息，关键：避免读取无效数据）
	    uint8_t chunk_id[4];
	    uint32_t chunk_size;
	    while (1) {
	        res = f_read(&file, chunk_id, 4, &bytesRead);
	        if (res != FR_OK || bytesRead != 4) break;
	        res = f_read(&file, &chunk_size, 4, &bytesRead);
	        if (res != FR_OK || bytesRead != 4) break;
	        // 找到data块，跳出循环（后续读取的就是音频数据）
	        if (memcmp(chunk_id, "data", 4) == 0) {
	            printf("find data block, size=%ld bytes\r\n", chunk_size);
	            break;
	        }
	        // 跳过非data块（如LIST块）
	        printf("data block index:%d\r\n",( f_tell(&file) + chunk_size));
	        f_lseek(&file, f_tell(&file) + chunk_size);
	    }

	    // 4. 循环读取并播放音频数据
	    while (1) {
	        // 等待I2S空闲（避免DMA冲突）
	        while (HAL_I2S_GetState(&hi2s2) == HAL_I2S_STATE_BUSY_TX) {
	            continue;
	        }

	        // 读取数据（用实际需要的字节数，而非固定BUFFER_SIZE）
	        res = f_read(&file, buffer, BUFFER_SIZE, &bytesRead);
//	        if (res != FR_OK) {
//	            printf("read data failed! code: %d\r\n", res);
//	            break;
//	        }
//
//	        // 读取到0字节 = 文件结束
//	        if (bytesRead == 0) {
//	            printf("file read complete, total read=%ld bytes\r\n", totalRead);
//	            break;
//	        }

	        // 更新总读取字节数
	        //totalRead += bytesRead;
	        //printf("read %d bytes, total=%ld\r\n", bytesRead, totalRead);

	        // 启动I2S DMA播放（修复：按采样位数计算传输个数）

	        HAL_I2S_Transmit_DMA(&hi2s2, (uint16_t*)buffer, bytesRead / sample_bytes);
	    }

	    // 5. 关闭文件（修复：用f_close）
	    f_close(&file);

}
