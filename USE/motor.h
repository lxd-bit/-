#ifndef __MOTOR_H
#define __MOTOR_H

#include "main.h"
#include "tim.h"   // 用于访问 htim2

/* 启动 PWM（在任务开始时调用一次） */
void Motor_Init(void);

/* 设置电机 PWM 和方向
 * n:   电机编号（1 或 2）
 * PWM: 速度，正负代表方向，范围 -100 ~ 100
 */
void Motor_SetPWM(uint8_t n, int16_t PWM);

#endif
