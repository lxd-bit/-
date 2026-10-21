#include "NRF24L01.h"
#include <string.h>

/* ==================== 全局变量 ==================== */
uint8_t NRF24L01_TxAddress[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
uint8_t NRF24L01_RxAddress[5] = {0x11, 0x22, 0x33, 0x44, 0x55};
uint8_t NRF24L01_TxPacket[NRF24L01_TX_PACKET_WIDTH];
uint8_t NRF24L01_RxPacket[NRF24L01_RX_PACKET_WIDTH];

/* ==================== 引脚操作 ==================== */

void NRF24L01_W_CE(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, (GPIO_PinState)BitValue);
}

void NRF24L01_W_CSN(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, (GPIO_PinState)BitValue);
}

void NRF24L01_W_SCK(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_3, (GPIO_PinState)BitValue);
}

void NRF24L01_W_MOSI(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, (GPIO_PinState)BitValue);
}

uint8_t NRF24L01_R_MISO(void)
{
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);
}

/* ==================== SPI 时序 ==================== */

uint8_t NRF24L01_SPI_SwapByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (Byte & 0x80) NRF24L01_W_MOSI(1);
        else             NRF24L01_W_MOSI(0);
        Byte <<= 1;
        
        NRF24L01_W_SCK(1);
        if (NRF24L01_R_MISO()) Byte |= 0x01;
        NRF24L01_W_SCK(0);
    }
    return Byte;
}

/* ==================== 寄存器读写 ==================== */

uint8_t NRF24L01_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_REGISTER | RegAddress);
    Data = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);
    return Data;
}

void NRF24L01_ReadRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_REGISTER | RegAddress);
    for (i = 0; i < Count; i++) DataArray[i] = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);
}

void NRF24L01_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_REGISTER | RegAddress);
    NRF24L01_SPI_SwapByte(Data);
    NRF24L01_W_CSN(1);
}

void NRF24L01_WriteRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_REGISTER | RegAddress);
    for (i = 0; i < Count; i++) NRF24L01_SPI_SwapByte(DataArray[i]);
    NRF24L01_W_CSN(1);
}

void NRF24L01_ReadRxPayload(uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_R_RX_PAYLOAD);
    for (i = 0; i < Count; i++) DataArray[i] = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);
}

void NRF24L01_WriteTxPayload(uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_W_TX_PAYLOAD);
    for (i = 0; i < Count; i++) NRF24L01_SPI_SwapByte(DataArray[i]);
    NRF24L01_W_CSN(1);
}

void NRF24L01_FlushTx(void)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_FLUSH_TX);
    NRF24L01_W_CSN(1);
}

void NRF24L01_FlushRx(void)
{
    NRF24L01_W_CSN(0);
    NRF24L01_SPI_SwapByte(NRF24L01_FLUSH_RX);
    NRF24L01_W_CSN(1);
}

uint8_t NRF24L01_ReadStatus(void)
{
    uint8_t Status;
    NRF24L01_W_CSN(0);
    Status = NRF24L01_SPI_SwapByte(NRF24L01_NOP);
    NRF24L01_W_CSN(1);
    return Status;
}

/* ==================== 模式切换 ==================== */

void NRF24L01_Rx(void)
{
    uint8_t Config;
    NRF24L01_W_CE(0);
    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) return;
    Config |= 0x03;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);
    NRF24L01_W_CE(1);
}

void NRF24L01_Tx(void)
{
    uint8_t Config;
    NRF24L01_W_CE(0);
    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    if (Config == 0xFF) return;
    Config |= 0x02;
    Config &= ~0x01;
    NRF24L01_WriteReg(NRF24L01_CONFIG, Config);
    NRF24L01_W_CE(1);
}

/* ==================== 初始化 ==================== */

void NRF24L01_GPIO_Init(void)
{
    /* ⚠️ 禁用 JTAG，释放 PA15/PB3/PB4 */
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
    
    /* CubeMX 里已经把引脚配好了，这里只设初始电平 */
    NRF24L01_W_CE(0);
    NRF24L01_W_CSN(1);
    NRF24L01_W_SCK(0);
    NRF24L01_W_MOSI(0);
}

void NRF24L01_Init(void)
{
    NRF24L01_GPIO_Init();
    
    NRF24L01_WriteReg(NRF24L01_CONFIG, 0x08);
    NRF24L01_WriteReg(NRF24L01_EN_AA, 0x3F);
    NRF24L01_WriteReg(NRF24L01_EN_RXADDR, 0x01);
    NRF24L01_WriteReg(NRF24L01_SETUP_AW, 0x03);
    NRF24L01_WriteReg(NRF24L01_SETUP_RETR, 0x03);
    NRF24L01_WriteReg(NRF24L01_RF_CH, 0x02);
    NRF24L01_WriteReg(NRF24L01_RF_SETUP, 0x0E);
    NRF24L01_WriteReg(NRF24L01_RX_PW_P0, NRF24L01_RX_PACKET_WIDTH);
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);
    
    NRF24L01_FlushTx();
    NRF24L01_FlushRx();
    NRF24L01_WriteReg(NRF24L01_STATUS, 0x70);
    
    NRF24L01_Rx();
}

/* ==================== 发送和接收 ==================== */

uint8_t NRF24L01_Send(void)
{
    uint8_t Status;
    uint8_t SendFlag;
    uint32_t Timeout;
    
    NRF24L01_WriteRegs(NRF24L01_TX_ADDR, NRF24L01_TxAddress, 5);
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_TxAddress, 5);
    NRF24L01_WriteTxPayload(NRF24L01_TxPacket, NRF24L01_TX_PACKET_WIDTH);
    NRF24L01_Tx();
    
    Timeout = 10000;
    while (1)
    {
        Status = NRF24L01_ReadStatus();
        Timeout--;
        if (Timeout == 0) { SendFlag = 4; NRF24L01_Init(); break; }
        
        if ((Status & 0x30) == 0x30) { SendFlag = 3; NRF24L01_Init(); break; }
        else if ((Status & 0x10) == 0x10) { SendFlag = 2; NRF24L01_Init(); break; }
        else if ((Status & 0x20) == 0x20) { SendFlag = 1; break; }
    }
    
    NRF24L01_WriteReg(NRF24L01_STATUS, 0x30);
    NRF24L01_FlushTx();
    NRF24L01_WriteRegs(NRF24L01_RX_ADDR_P0, NRF24L01_RxAddress, 5);
    NRF24L01_Rx();
    
    return SendFlag;
}

uint8_t NRF24L01_Receive(void)
{
    uint8_t Status, Config;
    uint8_t ReceiveFlag;
    
    Status = NRF24L01_ReadStatus();
    Config = NRF24L01_ReadReg(NRF24L01_CONFIG);
    
    if ((Config & 0x02) == 0x00)
    {
        ReceiveFlag = 3;
        NRF24L01_Init();
    }
    else if ((Status & 0x30) == 0x30)
    {
        ReceiveFlag = 2;
        NRF24L01_Init();
    }
    else if ((Status & 0x40) == 0x40)
    {
        ReceiveFlag = 1;
        NRF24L01_ReadRxPayload(NRF24L01_RxPacket, NRF24L01_RX_PACKET_WIDTH);
        NRF24L01_WriteReg(NRF24L01_STATUS, 0x40);
        NRF24L01_FlushRx();
    }
    else
    {
        ReceiveFlag = 0;
    }
    
    return ReceiveFlag;
}
