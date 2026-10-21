#include "BlueSerial.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

char BlueSerial_RxPacket[BLUE_RX_BUF_SIZE];
volatile uint8_t BlueSerial_RxFlag = 0;

/* 单字节接收缓冲区 */
static uint8_t Blue_RxByte;

extern UART_HandleTypeDef huart2;

void BlueSerial_Init(void)
{
    /* 开启单字节中断接收 */
    HAL_UART_Receive_IT(&huart2, &Blue_RxByte, 1);
}

/* 单字节接收完成回调（HAL 库会每收到 1 字节调用一次） */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        static uint8_t RxState = 0;
        static uint8_t pRxPacket = 0;

        if (RxState == 0)
        {
            if (Blue_RxByte == '[' && BlueSerial_RxFlag == 0)
            {
                RxState = 1;
                pRxPacket = 0;
            }
        }
        else if (RxState == 1)
        {
            if (Blue_RxByte == ']')
            {
                RxState = 0;
                BlueSerial_RxPacket[pRxPacket] = '\0';
                BlueSerial_RxFlag = 1;
            }
            else
            {
                if (pRxPacket < BLUE_RX_BUF_SIZE - 1)
                {
                    BlueSerial_RxPacket[pRxPacket++] = Blue_RxByte;
                }
            }
        }

        /* 重新开启接收（关键！） */
        HAL_UART_Receive_IT(&huart2, &Blue_RxByte, 1);
    }
}
