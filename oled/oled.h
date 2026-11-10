#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <string.h>

// 屏幕参数定义
#define OLED_I2C_ADDR 0x78
#define OLED_WIDTH  128
#define OLED_HEIGHT 64

// 函数声明
void OLED_Init(void);
void OLED_Clear(void);
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

// 外部I2C句柄声明（需与CubeMX配置的I2C句柄一致）
extern I2C_HandleTypeDef hi2c1;

#endif