#include "Motor.h"

/**
  * @brief  电机驱动初始化（启动 PWM 输出）
  * @note   GPIO 和 TIM2 时基均由 CubeMX 自动初始化，这里只负责启动 PWM
  */
void Motor_Init(void)
{
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
}

/**
  * @brief  设置电机 PWM 和方向
  * @param  n:   电机编号（1 或 2）
  * @param  PWM: 速度，正负代表方向，范围 -100 ~ 100
  */
void Motor_SetPWM(uint8_t n, int16_t PWM)
{
    if (n == 1)
    {
        if (PWM >= 0)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);    // AIN1 = 1
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);  // AIN2 = 0
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, PWM);
        }
        else
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_RESET);  // AIN1 = 0
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);    // AIN2 = 1
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, -PWM);
        }
    }
    else if (n == 2)
    {
        if (PWM >= 0)
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);  // BIN1 = 0
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    // BIN2 = 1
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, PWM);
        }
        else
        {
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);    // BIN1 = 1
            HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  // BIN2 = 0
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_2, -PWM);
        }
    }
}
