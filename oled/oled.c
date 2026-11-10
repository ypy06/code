#include "oled.h"
#include "OLED_Font.h"

/* 硬件配置（可根据实际修改I2C和引脚） */
#define OLED_I2C_ADDR  0x78    // OLED I2C地址（0x3C<<1）
#define OLED_I2C_TIMEOUT 100   // I2C超时时间(ms)

/**
 * @brief  I2C写入一个字节（底层封装）
 * @param  reg_addr: 寄存器/命令标识（0x00=命令，0x40=数据）
 * @param  data: 要写入的字节
 * @retval 无
 */
static void OLED_I2C_WriteByte(uint8_t reg_addr, uint8_t data)
{
    uint8_t tx_buf[2] = {reg_addr, data};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, tx_buf, 2, OLED_I2C_TIMEOUT);
}

/**
 * @brief  OLED写命令
 * @param  Command: 要写入的命令
 * @retval 无
 */
static void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_WriteByte(0x00, Command); // 0x00表示写命令
}

/**
 * @brief  OLED写数据
 * @param  Data: 要写入的数据
 * @retval 无
 */
static void OLED_WriteData(uint8_t Data)
{
    OLED_I2C_WriteByte(0x40, Data); // 0x40表示写数据
}

/**
 * @brief  OLED设置光标位置
 * @param  Y: 页地址（0~7，对应屏幕Y轴）
 * @param  X: 列地址（0~127，对应屏幕X轴）
 * @retval 无
 */
static void OLED_SetCursor(uint8_t Y, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Y);                  // 设置Y位置（页）
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));  // 设置X位置高4位
    OLED_WriteCommand(0x00 | (X & 0x0F));         // 设置X位置低4位
}

/**
 * @brief  OLED清屏
 * @param  无
 * @retval 无
 */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        for(i = 0; i < 128; i++)
        {
            OLED_WriteData(0x00);
        }
    }
}

/**
 * @brief  OLED显示一个字符
 * @param  Line: 行位置（1~4）
 * @param  Column: 列位置（1~16）
 * @param  Char: 要显示的字符（ASCII可见字符）
 * @retval 无
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{
    uint8_t i;
    // 设置光标位置（上半部分，8x16字符占2页）
    OLED_SetCursor((Line - 1) * 2, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i]); // 显示上半部分
    }
    // 设置光标位置（下半部分）
    OLED_SetCursor((Line - 1) * 2 + 1, (Column - 1) * 8);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]); // 显示下半部分
    }
}

/**
 * @brief  OLED显示字符串
 * @param  Line: 起始行（1~4）
 * @param  Column: 起始列（1~16）
 * @param  String: 要显示的字符串
 * @retval 无
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++)
    {
        OLED_ShowChar(Line, Column + i, String[i]);
    }
}

/**
 * @brief  次方函数（辅助数字显示）
 * @param  X: 底数
 * @param  Y: 指数
 * @retval X^Y的结果
 */
static uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;
    while (Y--)
    {
        Result *= X;
    }
    return Result;
}

/**
 * @brief  OLED显示十进制正数
 * @param  Line: 起始行（1~4）
 * @param  Column: 起始列（1~16）
 * @param  Number: 要显示的数字（0~4294967295）
 * @param  Length: 显示长度（1~10）
 * @retval 无
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief  OLED显示十进制带符号数
 * @param  Line: 起始行（1~4）
 * @param  Column: 起始列（1~16）
 * @param  Number: 要显示的数字（-2147483648~2147483647）
 * @param  Length: 显示长度（1~10）
 * @retval 无
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0)
    {
        OLED_ShowChar(Line, Column, '+');
        Number1 = Number;
    }
    else
    {
        OLED_ShowChar(Line, Column, '-');
        Number1 = -Number;
    }
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
    }
}

/**
 * @brief  OLED显示十六进制正数
 * @param  Line: 起始行（1~4）
 * @param  Column: 起始列（1~16）
 * @param  Number: 要显示的数字（0~0xFFFFFFFF）
 * @param  Length: 显示长度（1~8）
 * @retval 无
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10)
        {
            OLED_ShowChar(Line, Column + i, SingleNumber + '0');
        }
        else
        {
            OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
        }
    }
}

/**
 * @brief  OLED显示二进制正数
 * @param  Line: 起始行（1~4）
 * @param  Column: 起始列（1~16）
 * @param  Number: 要显示的数字（0~0xFFFF）
 * @param  Length: 显示长度（1~16）
 * @retval 无
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
    {
        OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
    }
}

/**
 * @brief  OLED初始化
 * @param  无
 * @retval 无
 */
void OLED_Init(void)
{
    HAL_Delay(100); // 上电延时（替代原有的空循环）

    // SSD1306初始化指令（与原标准库逻辑一致）
    OLED_WriteCommand(0xAE); // 关闭显示
    OLED_WriteCommand(0xD5); // 设置时钟分频比/振荡器频率
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40); // 设置显示开始行
    OLED_WriteCommand(0xA1); // 设置左右方向（0xA1正常）
    OLED_WriteCommand(0xC8); // 设置上下方向（0xC8正常）
    OLED_WriteCommand(0xDA); // 设置COM引脚配置
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); // 设置对比度
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); // 设置预充电周期
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); // 设置VCOMH级别
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4); // 整个显示开启（按显存显示）
    OLED_WriteCommand(0xA6); // 正常显示（非反显）
    OLED_WriteCommand(0x8D); // 使能充电泵
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // 开启显示

    OLED_Clear(); // 清屏
}