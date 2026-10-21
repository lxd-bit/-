#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "OLED_Data.h"

/* OLED显示缓存，大小：8页 * 128列 = 1024字节 */
uint8_t OLED_DisplayBuf[8][128];

/* ================== 底层 I2C 操作 (HAL库移植) ================== */

/**
  * @brief  OLED写SCL引脚
  */
void OLED_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, (GPIO_PinState)BitValue);
}

/**
  * @brief  OLED写SDA引脚
  */
void OLED_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, (GPIO_PinState)BitValue);
}

/**
  * @brief  OLED GPIO初始化（注意：引脚模式需在CubeMX中配置为开漏输出+上拉）
  */
void OLED_GPIO_Init(void)
{
    uint32_t i, j;
    /* 上电后等待OLED内部复位 */
    for (i = 0; i < 1000; i++)
        for (j = 0; j < 1000; j++);
    
    /* 释放总线 */
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C起始
  */
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  */
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        OLED_W_SDA(!!(Byte & (0x80 >> i)));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);
    OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  */
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78); // 从机地址
    OLED_I2C_SendByte(0x00); // 写命令
    OLED_I2C_SendByte(Command);
    OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40); // 写数据
    for (i = 0; i < Count; i++)
    {
        OLED_I2C_SendByte(Data[i]);
    }
    OLED_I2C_Stop();
}

/* ================== 硬件控制 ================== */

/**
  * @brief  OLED初始化
  */
void OLED_Init(void)
{
    OLED_GPIO_Init();

    OLED_WriteCommand(0xAE); // 关闭显示
    OLED_WriteCommand(0xD5); // 设置时钟分频
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8); // 设置多路复用率
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3); // 设置显示偏移
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40); // 设置显示起始行
    OLED_WriteCommand(0xA1); // 设置左右方向
    OLED_WriteCommand(0xC8); // 设置上下方向
    OLED_WriteCommand(0xDA); // 设置COM硬件引脚配置
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81); // 对比度设置
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9); // 预充电周期
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB); // VCOMH取消选择级别
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4); // 全局显示开启
    OLED_WriteCommand(0xA6); // 正常显示
    OLED_WriteCommand(0x8D); // 电荷泵设置
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF); // 开启显示

    OLED_Clear();
    OLED_Update();
}

/**
  * @brief  设置光标位置
  */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    OLED_WriteCommand(0xB0 | Page);
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (X & 0x0F));
}

/* ================== 缓存更新 ================== */

/**
  * @brief  将缓存刷入OLED屏幕（非常耗时，建议100ms调用一次）
  */
void OLED_Update(void)
{
    uint8_t j;
    for (j = 0; j < 8; j++)
    {
        OLED_SetCursor(j, 0);
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
}

void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;
    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0) { Page -= 1; Page1 -= 1; }
    
    for (j = Page; j < Page1; j++)
    {
        if (X >= 0 && X <= 127 && j >= 0 && j <= 7)
        {
            OLED_SetCursor(j, X);
            OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
        }
    }
}

void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
        for (i = 0; i < 128; i++)
            OLED_DisplayBuf[j][i] = 0x00;
}

void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
                OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));
        }
    }
}

void OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j++)
        for (i = 0; i < 128; i++)
            OLED_DisplayBuf[j][i] ^= 0xFF;
}

void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
                OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);
        }
    }
}

/* ================== 显示函数 ================== */

/* 由于篇幅限制，这里省略了字库数组 OLED_F8x16 和 OLED_F6x8，请使用原文件中的字库！ */
/* 注意：原代码中的字库数组需要保留，放在本文件合适位置 */

void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == OLED_8X16)
        OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
    else if(FontSize == OLED_6X8)
        OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
}

void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint16_t i = 0;
    while (String[i] != '\0')
    {
        OLED_ShowChar(X + i * FontSize, Y, String[i], FontSize);
        i++;
    }
}

void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
        OLED_ShowChar(X + i * FontSize, Y, Number / (uint32_t)pow(10, Length - i - 1) % 10 + '0', FontSize);
}

void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;
    if (Number >= 0) { OLED_ShowChar(X, Y, '+', FontSize); Number1 = Number; }
    else { OLED_ShowChar(X, Y, '-', FontSize); Number1 = -Number; }
    for (i = 0; i < Length; i++)
        OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / (uint32_t)pow(10, Length - i - 1) % 10 + '0', FontSize);
}

void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)
    {
        SingleNumber = Number / (uint32_t)pow(16, Length - i - 1) % 16;
        if (SingleNumber < 10) OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
        else OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
    }
}

void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)
        OLED_ShowChar(X + i * FontSize, Y, Number / (uint32_t)pow(2, Length - i - 1) % 2 + '0', FontSize);
}

void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t PowNum, IntNum, FraNum;
    if (Number >= 0) OLED_ShowChar(X, Y, '+', FontSize);
    else { OLED_ShowChar(X, Y, '-', FontSize); Number = -Number; }
    IntNum = Number;
    Number -= IntNum;
    PowNum = (uint32_t)pow(10, FraLength);
    FraNum = (uint32_t)round(Number * PowNum);
    IntNum += FraNum / PowNum;
    OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
    OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
    OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;
    OLED_ClearArea(X, Y, Width, Height);
    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        for (i = 0; i < Width; i++)
        {
            if (X + i >= 0 && X + i <= 127)
            {
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0) { Page -= 1; Shift += 8; }
                if (Page + j >= 0 && Page + j <= 7)
                    OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                if (Page + j + 1 >= 0 && Page + j + 1 <= 7)
                    OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
            }
        }
    }
}

void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
    char String[256];
    va_list arg;
    va_start(arg, format);
    vsprintf(String, format, arg);
    va_end(arg);
    OLED_ShowString(X, Y, String, FontSize);
}

void OLED_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63)
        OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
}

uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >= 0 && Y <= 63)
        if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8)) return 1;
    return 0;
}

void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
    /* ... 此处省略原文件中的画线、画圆等代码，请务必从原文件复制过来 ... */
    /* 注意：原文件中使用了 pow, round 等数学函数，依赖 <math.h> */
}

