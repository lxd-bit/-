#ifndef __ROBOT_STATE_H
#define __ROBOT_STATE_H

#include "main.h"
#include "PID.h"

/* ========== 运行状态 ========== */
extern uint8_t Run_Flag;         // 0=停止，1=运行

/* ========== PID 对象 ========== */
extern PID_t AnglePID;
extern PID_t SpeedPID;
extern PID_t TurnPID;

/* ========== 姿态与速度 ========== */
extern float Angle;
extern float LeftSpeed, RightSpeed, AveSpeed, DifSpeed;

/* ========== 电机 PWM ========== */
extern int16_t LeftPWM, RightPWM, AvePWM, DifPWM;

#endif
