#include "MPU6050.h"
#include "cmsis_os.h"

/* ==================== 软件 I2C 底层实现 ==================== */

/**
  * @brief  SCL 写电平
  */
void MyI2C_W_SCL(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, (GPIO_PinState)BitValue);
}

/**
  * @brief  SDA 写电平
  */
void MyI2C_W_SDA(uint8_t BitValue)
{
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, (GPIO_PinState)BitValue);
}

/**
  * @brief  SDA 读电平
  */
uint8_t MyI2C_R_SDA(void)
{
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11);
}

/**
  * @brief  软件 I2C GPIO 初始化
  * @note   如果 CubeMX 已经配置 PB10/PB11 为开漏+上拉，此函数可以留空或删除
  */
void MyI2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* SCL (PB10) 和 SDA (PB11) 都配置为开漏输出，带上拉 */
    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 空闲状态：两条线都拉高 */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10 | GPIO_PIN_11, GPIO_PIN_SET);
}

/**
  * @brief  I2C 起始条件
  */
void MyI2C_Start(void)
{
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(0);
}

/**
  * @brief  I2C 停止条件
  */
void MyI2C_Stop(void)
{
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

/**
  * @brief  I2C 发送一个字节
  */
void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SDA(!!(Byte & (0x80 >> i)));
        MyI2C_W_SCL(1);
        MyI2C_W_SCL(0);
    }
}

/**
  * @brief  I2C 接收一个字节
  */
uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MyI2C_W_SDA(1);                // 主机释放 SDA
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(1);
        if (MyI2C_R_SDA()) { Byte |= (0x80 >> i); }
        MyI2C_W_SCL(0);
    }
    return Byte;
}

/**
  * @brief  I2C 发送应答位
  */
void MyI2C_SendAck(uint8_t AckBit)
{
    MyI2C_W_SDA(AckBit);
    MyI2C_W_SCL(1);
    MyI2C_W_SCL(0);
}

/**
  * @brief  I2C 接收应答位
  */
uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;
    MyI2C_W_SDA(1);                // 主机释放 SDA
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}

/* ==================== MPU6050 寄存器读写 ==================== */

/**
  * @brief  向 MPU6050 写一个寄存器
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(RegAddress);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(Data);
    MyI2C_ReceiveAck();
    MyI2C_Stop();
}

/**
  * @brief  从 MPU6050 读一个寄存器
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
    uint8_t Data;

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(RegAddress);
    MyI2C_ReceiveAck();

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
    MyI2C_ReceiveAck();
    Data = MyI2C_ReceiveByte();
    MyI2C_SendAck(1);
    MyI2C_Stop();

    return Data;
}

/**
  * @brief  从 MPU6050 连续读多个寄存器
  */
void MPU6050_ReadRegs(uint8_t RegAddress, uint8_t *DataArray, uint8_t Count)
{
    uint8_t i;

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS);
    MyI2C_ReceiveAck();
    MyI2C_SendByte(RegAddress);
    MyI2C_ReceiveAck();

    MyI2C_Start();
    MyI2C_SendByte(MPU6050_ADDRESS | 0x01);
    MyI2C_ReceiveAck();
    for (i = 0; i < Count; i++)
    {
        DataArray[i] = MyI2C_ReceiveByte();
        if (i < Count - 1)
            MyI2C_SendAck(0);      // 继续读，发 ACK
        else
            MyI2C_SendAck(1);      // 最后一个字节，发 NACK
    }
    MyI2C_Stop();
}

/* ==================== MPU6050 初始化与数据读取 ==================== */

/**
  * @brief  初始化 MPU6050
  */
void MPU6050_Init(void)
{
    MyI2C_Init();
	HAL_Delay(300);
	
    MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);     // 解除休眠，选择陀螺仪X轴为时钟源
    MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);     // 6轴都不休眠
    MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x07);     // 采样率 1kHz / (1+7) = 125Hz
    MPU6050_WriteReg(MPU6050_CONFIG, 0x00);         // 数字低通滤波器：最大带宽
    MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);    // 陀螺仪量程 ±2000°/s
    MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);   // 加速度量程 ±16g
}

/**
  * @brief  读取 MPU6050 的 WHO_AM_I 寄存器（正常应返回 0x68）
  */
uint8_t MPU6050_GetID(void)
{
    return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

/**
  * @brief  读取 MPU6050 的加速度和角速度原始数据
  * @note   14 个字节连续读取：AccX, AccY, AccZ, TempH, TempL, GyroX, GyroY, GyroZ
  */
void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ,
                     int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
    uint8_t Data[14];

    MPU6050_ReadRegs(MPU6050_ACCEL_XOUT_H, Data, 14);

    *AccX  = (Data[0]  << 8) | Data[1];
    *AccY  = (Data[2]  << 8) | Data[3];
    *AccZ  = (Data[4]  << 8) | Data[5];
    /* Data[6], Data[7] 是温度，这里跳过 */
    *GyroX = (Data[8]  << 8) | Data[9];
    *GyroY = (Data[10] << 8) | Data[11];
    *GyroZ = (Data[12] << 8) | Data[13];
}
