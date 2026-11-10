/*
 * sbus.h
 *
 *  Created on: 2026年6月1日
 *      Author: YPY
 */

#ifndef SBUS_H
#define SBUS_H

#include "zf_libraries_headfile.h"

// SBUS 帧相关定义
#define SBUS_FRAME_LENGTH      25      // SBUS 帧长度
#define SBUS_CHANNEL_COUNT     16      // SBUS 通道数量（实际最多16通道）
#define SBUS_START_BYTE        0x0F    // SBUS 帧起始字节
#define SBUS_END_BYTE          0x00    // SBUS 帧结束字节

// 通道数据范围
#define SBUS_CHANNEL_MIN       172     // 最小PWM值（对应约 -100%）
#define SBUS_CHANNEL_MID       1024    // 中间PWM值（对应 0%）
#define SBUS_CHANNEL_MAX       1811    // 最大PWM值（对应 +100%）

// 标志位定义（第24字节）
#define SBUS_FLAG_FRAMELOST     0x04    // 帧丢失标志
#define SBUS_FLAG_FAILSAFE      0x08    // 失控保护标志

// 通道宏定义
#define CHANNEL_ROLL           0       // 横滚（副翼）channel1
#define CHANNEL_PITCH          1       // 俯仰（升降）channel2
#define CHANNEL_THROTTLE       2       // 油门 channel3
#define CHANNEL_YAW            3       // 偏航（方向）channel4

// SBUS 接收器结构体
typedef struct
{
    uint16_t channel[SBUS_CHANNEL_COUNT];   // 16个通道数据
    uint8_t  frame[SBUS_FRAME_LENGTH];      // 原始帧数据
    uint8_t  rx_buffer[SBUS_FRAME_LENGTH];  // 接收缓冲区
    uint8_t  buffer_index;                   // 缓冲区索引
    uint8_t  failsafe;                       // 失控保护标志
    uint8_t  framelost;                      // 帧丢失标志
    uint8_t  signal_lost;                    // 信号丢失标志
    uint8_t  frame_received;                 // 新帧接收标志
    uint8_t  init_done;                      // 初始化完成标志
} sbus_t;

// 外部变量声明
extern sbus_t sbus;

// 函数声明
void sbus_Init(void);                        // SBUS 初始化
void sbus_Proc(void);                        // SBUS 处理函数（需要在主循环调用）
uint8_t sbus_Update(void);                   // 更新SBUS数据，返回1表示有有效数据
uint16_t sbus_GetChannel(uint8_t ch);        // 获取指定通道的原始PWM值
float sbus_GetChannelPercent(uint8_t ch);    // 获取指定通道的百分比值（-100~100）
uint8_t sbus_IsFailsafe(void);               // 是否进入失控保护状态
uint8_t sbus_IsSignalLost(void);             // 是否信号丢失

// 串口回调函数（需要在串口接收中断中调用）
void sbus_RxCallback(uint8_t data);

#endif /* SBUS_H */
