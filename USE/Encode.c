// Encoder.c
#include "Encode.h"

/**
  * @brief  启动两个编码器定时器
  * @note   TIM3、TIM4 的编码器模式配置已由 CubeMX 自动生成
  */
void Encoder_Init(void)
{
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
}

/**
  * @brief  读取并清零编码器计数
  * @param  n: 1 = TIM3（左电机）, 2 = TIM4（右电机）
  * @retval 自上次调用以来转过的脉冲数（带符号）
  */
int16_t Encoder_Get(uint8_t n)
{
    int16_t Temp;
    if (n == 1)
    {
        Temp = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
        __HAL_TIM_SET_COUNTER(&htim3, 0);
        return Temp;
    }
    else if (n == 2)
    {
        Temp = -(int16_t)__HAL_TIM_GET_COUNTER(&htim4);
        __HAL_TIM_SET_COUNTER(&htim4, 0);
        return Temp;
    }
    return 0;
}
