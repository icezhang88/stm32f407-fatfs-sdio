/*
 * filemanager.c
 *
 *  Created on: Sep 19, 2025
 *      Author: Administrator
 */

#include "filemanager.h"

#define BUFFER_SIZE 2048

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {

	//printf("HAL_I2S_TxCpltCallback\r\n");
}



uint8_t initFileManager() {

	FRESULT retSD = f_mount(&SDFatFS, (TCHAR const*) SDPath, 1);
	printf("%u\r\n", retSD);
	printf("Compilation Date: %s %s\n", __DATE__, __TIME__);
	// mount SD card
	//   int retSD = f_mount(&SDFatFS, (TCHAR const *)SDPath, 1);
	if (retSD == FR_OK) {

		//
		SDCard_ShowInfo();
		ListFiles("/");
		return 1;

	} else {
		printf("!! SDcard mount filesystem error。(%d)\r\n", retSD);
		return 0;
	}
	// 不带fatfs调试函数

}

void PrintHexData(uint8_t *buffer, uint16_t len) {
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

void ListFiles(const TCHAR *path) {
	FRESULT res;
	DIR dir;               // 目录对象
	FILINFO fno;           // 文件信息结构体

	// 打开目录
	res = f_opendir(&dir, path);
	if (res != FR_OK) {
		printf("open dir failed err: %d\r\n", res);
		return;
	}

	printf("\r\ndir %s file list : \r\n", path);
	printf("----------------------------------------\r\n");

	// 循环读取目录项（f_readdir返回FR_OK且fno.fname不为空时继续）
	while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
		// 判断是否为子目录（文件名以'/'结尾，或fno.fattrib包含AM_DIR属性）
		if (fno.fattrib & AM_DIR) {
			printf(" [dir]  %s\r\n", fno.fname);  // 子目录
		} else {
			// 普通文件：输出文件名和大小（单位：字节）
			printf(" [file]  %s  (size: %ld bytes)\r\n", fno.fname, fno.fsize);
		}
	}

	printf("----------------------------------------\r\n");
	printf("dir search success\r\n");

	// 关闭目录
	f_closedir(&dir);
}

void SDCard_ShowInfo(void) {
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
void SDCard_ShowStatus(void) {
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

