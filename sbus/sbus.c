/*
 * sbus.c
 *
 *  Created on: 2026年6月1日
 *      Author: YPY
 *  Description: SBUS 遥控信号接收解析库实现
 */

#include "sbus.h"

// SBUS 接收器实例
sbus_t sbus = {0};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     SBUS 初始化函数
// 参数说明     void
// 返回参数     void
// 使用示例     sbus_Init();
// 备注信息     需要先配置好对应的 UART 串口
//-------------------------------------------------------------------------------------------------------------------
void sbus_Init(void)
{
    // 初始化结构体
    for(uint8_t i = 0; i < SBUS_CHANNEL_COUNT; i++)
    {
        sbus.channel[i] = SBUS_CHANNEL_MID;     // 初始化为中值
    }

    for(uint8_t i = 0; i < SBUS_FRAME_LENGTH; i++)
    {
        sbus.frame[i] = 0;
        sbus.rx_buffer[i] = 0;
    }

    sbus.buffer_index = 0;
    sbus.failsafe = 0;
    sbus.framelost = 0;
    sbus.signal_lost = 1;                       // 初始状态为信号丢失
    sbus.frame_received = 0;
    sbus.init_done = 1;

    // 注意：需要根据实际硬件配置 UART 串口
    // 示例使用 UART 接收 SBUS 信号（SBUS 通常是反相逻辑，波特率 100000，8E2）
    // 请根据您的实际硬件配置相应的串口
    // uart_init(USARTx, 100000, UART_WORDLENGTH_8E2);  // SBUS 特定配置
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     解析 SBUS 原始帧数据
// 参数说明     void
// 返回参数     uint8_t 1-解析成功 0-解析失败
// 使用示例     内部调用
// 备注信息     根据 SBUS 协议解析16个通道数据
//-------------------------------------------------------------------------------------------------------------------
static uint8_t sbus_ParseFrame(void)
{
    // 检查帧头和帧尾
    if(sbus.frame[0] != SBUS_START_BYTE || sbus.frame[SBUS_FRAME_LENGTH - 1] != SBUS_END_BYTE)
    {
        return 0;   // 无效帧
    }

    // 解析16个通道数据（每个通道11位）
    // 替代解析方法（更简单直观）
    // CH1
    sbus.channel[0] = ((sbus.frame[1] | (sbus.frame[2] << 8)) & 0x07FF);
    // CH2
    sbus.channel[1] = (((sbus.frame[2] >> 3) | (sbus.frame[3] << 5)) & 0x07FF);
    // CH3
    sbus.channel[2] = (((sbus.frame[3] >> 6) | (sbus.frame[4] << 2) | (sbus.frame[5] << 10)) & 0x07FF);
    // CH4
    sbus.channel[3] = (((sbus.frame[5] >> 1) | (sbus.frame[6] << 7)) & 0x07FF);
    // CH5
    sbus.channel[4] = (((sbus.frame[6] >> 4) | (sbus.frame[7] << 4)) & 0x07FF);
    // CH6
    sbus.channel[5] = (((sbus.frame[7] >> 7) | (sbus.frame[8] << 1) | (sbus.frame[9] << 9)) & 0x07FF);
    // CH7
    sbus.channel[6] = (((sbus.frame[9] >> 2) | (sbus.frame[10] << 6)) & 0x07FF);
    // CH8
    sbus.channel[7] = (((sbus.frame[10] >> 5) | (sbus.frame[11] << 3)) & 0x07FF);
    // CH9
    sbus.channel[8] = (((sbus.frame[12] >> 0) | (sbus.frame[13] << 8)) & 0x07FF);
    // CH10
    sbus.channel[9] = (((sbus.frame[13] >> 3) | (sbus.frame[14] << 5)) & 0x07FF);
    // CH11
    sbus.channel[10] = (((sbus.frame[14] >> 6) | (sbus.frame[15] << 2) | (sbus.frame[16] << 10)) & 0x07FF);
    // CH12
    sbus.channel[11] = (((sbus.frame[16] >> 1) | (sbus.frame[17] << 7)) & 0x07FF);
    // CH13
    sbus.channel[12] = (((sbus.frame[17] >> 4) | (sbus.frame[18] << 4)) & 0x07FF);
    // CH14
    sbus.channel[13] = (((sbus.frame[18] >> 7) | (sbus.frame[19] << 1) | (sbus.frame[20] << 9)) & 0x07FF);
    // CH15
    sbus.channel[14] = (((sbus.frame[20] >> 2) | (sbus.frame[21] << 6)) & 0x07FF);
    // CH16
    sbus.channel[15] = (((sbus.frame[21] >> 5) | (sbus.frame[22] << 3)) & 0x07FF);

    // 解析标志位（第24字节）
    sbus.framelost = (sbus.frame[23] & SBUS_FLAG_FRAMELOST) ? 1 : 0;
    sbus.failsafe  = (sbus.frame[23] & SBUS_FLAG_FAILSAFE)  ? 1 : 0;
    sbus.signal_lost = (sbus.framelost || sbus.failsafe);

    return 1;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     处理串口接收到的数据（需要在串口中断中调用）
// 参数说明     data - 接收到的字节数据
// 返回参数     void
// 使用示例     在串口接收中断中调用 sbus_RxCallback(received_data);
// 备注信息     建议在串口中断服务函数中调用此函数
//-------------------------------------------------------------------------------------------------------------------
void sbus_RxCallback(uint8_t data)
{
    // 查找帧头
    if(sbus.buffer_index == 0 && data != SBUS_START_BYTE)
    {
        return;  // 等待起始字节
    }

    // 防止缓冲区溢出
    if(sbus.buffer_index >= SBUS_FRAME_LENGTH)
    {
        sbus.buffer_index = 0;
        return;
    }

    // 存储数据到缓冲区
    sbus.rx_buffer[sbus.buffer_index++] = data;

    // 检查是否接收完一帧
    if(sbus.buffer_index >= SBUS_FRAME_LENGTH)
    {
        // 复制到帧缓冲区
        for(uint8_t i = 0; i < SBUS_FRAME_LENGTH; i++)
        {
            sbus.frame[i] = sbus.rx_buffer[i];
        }

        // 重置缓冲区索引
        sbus.buffer_index = 0;

        // 解析帧数据
        if(sbus_ParseFrame())
        {
            sbus.frame_received = 1;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     SBUS 处理函数（需要在主循环中周期性调用）
// 参数说明     void
// 返回参数     void
// 使用示例     while(1) { sbus_Proc(); }
// 备注信息     可以配合 RTOS 使用，建议每 5-10ms 调用一次
//-------------------------------------------------------------------------------------------------------------------
void sbus_Proc(void)
{
    // 如果没有初始化，先初始化
    if(!sbus.init_done)
    {
        sbus_Init();
        return;
    }

    // 注意：实际的串口数据接收需要在串口中断中调用 sbus_RxCallback()
    // 如果您的平台没有自动的串口中断处理，需要在这里添加串口轮询接收代码

    // 示例：轮询方式接收串口数据（根据实际硬件修改）
    /*
    if(uart_query_rx_ready(SBUS_UART))
    {
        uint8_t data = uart_read_byte(SBUS_UART);
        sbus_RxCallback(data);
    }
    */

    // 如果检测到信号丢失且长时间未收到新帧，保持最后状态或输出警告
    static uint32_t last_frame_time = 0;
    static uint8_t first_frame_received = 0;

    if(sbus.frame_received)
    {
        last_frame_time = zf_sys_tick_get_ms();
        sbus.frame_received = 0;
        first_frame_received = 1;
    }
    else if(first_frame_received)
    {
        // 超过 200ms 未收到新帧，认为信号丢失
        if(zf_sys_tick_get_ms() - last_frame_time > 200)
        {
            sbus.signal_lost = 1;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     更新 SBUS 数据
// 参数说明     void
// 返回参数     uint8_t 1-有有效数据 0-无新数据
// 使用示例     if(sbus_Update()) { // 处理新数据 }
// 备注信息     返回1表示有新帧数据
//-------------------------------------------------------------------------------------------------------------------
uint8_t sbus_Update(void)
{
    if(sbus.frame_received)
    {
        sbus.frame_received = 0;
        return 1;
    }
    return 0;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取指定通道的原始 PWM 值
// 参数说明     ch - 通道号 (0-15)
// 返回参数     uint16_t - 通道 PWM 值 (172~1811)
// 使用示例     uint16_t roll = sbus_GetChannel(CHANNEL_ROLL);
// 备注信息     范围：172（最小值）-> 1024（中值）-> 1811（最大值）
//-------------------------------------------------------------------------------------------------------------------
uint16_t sbus_GetChannel(uint8_t ch)
{
    if(ch >= SBUS_CHANNEL_COUNT)
    {
        return SBUS_CHANNEL_MID;
    }
    return sbus.channel[ch];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取指定通道的百分比值
// 参数说明     ch - 通道号 (0-15)
// 返回参数     float - 百分比值 (-100.0 ~ 100.0)
// 使用示例     float throttle = sbus_GetChannelPercent(CHANNEL_THROTTLE);
// 备注信息     将原始 PWM 值映射到 -100% 到 +100% 范围
//-------------------------------------------------------------------------------------------------------------------
float sbus_GetChannelPercent(uint8_t ch)
{
    uint16_t raw;
    float percent;

    if(ch >= SBUS_CHANNEL_COUNT)
    {
        return 0.0f;
    }

    raw = sbus.channel[ch];

    // 映射到 -100 到 100
    if(raw <= SBUS_CHANNEL_MIN)
    {
        percent = -100.0f;
    }
    else if(raw >= SBUS_CHANNEL_MAX)
    {
        percent = 100.0f;
    }
    else if(raw <= SBUS_CHANNEL_MID)
    {
        // 负值区域映射
        percent = -((float)(SBUS_CHANNEL_MID - raw) / (SBUS_CHANNEL_MID - SBUS_CHANNEL_MIN)) * 100.0f;
    }
    else
    {
        // 正值区域映射
        percent = ((float)(raw - SBUS_CHANNEL_MID) / (SBUS_CHANNEL_MAX - SBUS_CHANNEL_MID)) * 100.0f;
    }

    return percent;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     检查是否进入失控保护状态
// 参数说明     void
// 返回参数     uint8_t 1-失控保护中 0-正常
// 使用示例     if(sbus_IsFailsafe()) { // 执行失控保护操作 }
// 备注信息     失控保护时应该停止电机或执行安全动作
//-------------------------------------------------------------------------------------------------------------------
uint8_t sbus_IsFailsafe(void)
{
    return sbus.failsafe;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     检查信号是否丢失
// 参数说明     void
// 返回参数     uint8_t 1-信号丢失 0-信号正常
// 使用示例     if(sbus_IsSignalLost()) { // 等待信号恢复 }
// 备注信息     信号丢失时应该保持最后状态或进入安全模式
//-------------------------------------------------------------------------------------------------------------------
uint8_t sbus_IsSignalLost(void)
{
    return sbus.signal_lost;
}
