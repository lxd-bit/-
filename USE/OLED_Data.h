#ifndef __OLED_DATA_H
#define __OLED_DATA_H

#include "main.h"

/* 中文字符结构体定义 */
typedef struct {
    char Index[5];     // 中文字符串索引
    uint8_t Data[32];  // 16x16 点阵数据
} ChineseCell_t;

/* 声明外部字库和图片数组，供 OLED.c 调用 */
extern const uint8_t OLED_F8x16[][16];
extern const uint8_t OLED_F6x8[][6];
extern const ChineseCell_t OLED_CF16x16[];
extern const uint8_t Diode[];

#endif

