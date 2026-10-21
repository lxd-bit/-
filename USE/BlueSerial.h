#ifndef __BLUESERIAL_H
#define __BLUESERIAL_H

#include "main.h"

/* 接收缓冲区大小 */
#define BLUE_RX_BUF_SIZE   128

/* 全局变量：接收数据（供 Comm_Task 解析） */
extern char BlueSerial_RxPacket[BLUE_RX_BUF_SIZE];
extern volatile uint8_t BlueSerial_RxFlag;

/* 对外接口 */
void BlueSerial_Init(void);
void BlueSerial_SendByte(uint8_t Byte);
void BlueSerial_SendString(char *String);
void BlueSerial_Printf(char *format, ...);

#endif
