#include "LED.h"


/**
  * @brief  点亮 LED (低电平点亮)
  */
void LED_ON(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
}

/**
  * @brief  熄灭 LED
  */
void LED_OFF(void)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

/**
  * @brief  翻转 LED 状态
  */
void LED_Turn(void)
{
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}
