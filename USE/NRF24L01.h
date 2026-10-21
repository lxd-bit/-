#ifndef __NRF24L01_H
#define __NRF24L01_H

#include "main.h"

/* 寄存器地址定义（和原代码一致） */
#define NRF24L01_CONFIG         0x00
#define NRF24L01_EN_AA          0x01
#define NRF24L01_EN_RXADDR      0x02
#define NRF24L01_SETUP_AW       0x03
#define NRF24L01_SETUP_RETR     0x04
#define NRF24L01_RF_CH          0x05
#define NRF24L01_RF_SETUP       0x06
#define NRF24L01_STATUS         0x07
#define NRF24L01_RX_ADDR_P0     0x0A
#define NRF24L01_TX_ADDR        0x10
#define NRF24L01_RX_PW_P0       0x11
#define NRF24L01_FIFO_STATUS    0x17

#define NRF24L01_R_REGISTER     0x00
#define NRF24L01_W_REGISTER     0x20
#define NRF24L01_R_RX_PAYLOAD   0x61
#define NRF24L01_W_TX_PAYLOAD   0xA0
#define NRF24L01_FLUSH_TX       0xE1
#define NRF24L01_FLUSH_RX       0xE2
#define NRF24L01_NOP            0xFF

/* 数据包宽度 */
#define NRF24L01_TX_PACKET_WIDTH    32
#define NRF24L01_RX_PACKET_WIDTH    32

/* 全局变量，供 Comm_Task 解析使用 */
extern uint8_t NRF24L01_RxPacket[NRF24L01_RX_PACKET_WIDTH];
extern uint8_t NRF24L01_TxPacket[NRF24L01_TX_PACKET_WIDTH];
extern uint8_t NRF24L01_TxAddress[5];
extern uint8_t NRF24L01_RxAddress[5];

/* 接口函数 */
void NRF24L01_Init(void);
uint8_t NRF24L01_Send(void);
uint8_t NRF24L01_Receive(void);

#endif
