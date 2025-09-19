#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#include "fatfs.h"

#include "stdint.h"
#include "stdio.h"
#include "i2s.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"



uint8_t initFileManager();

void PrintHexData(uint8_t *buffer, uint16_t len);

void ListFiles(const TCHAR *path);


void SDCard_ShowInfo(void);

void SDCard_ShowStatus(void);


#endif
