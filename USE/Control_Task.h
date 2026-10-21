#ifndef __CONTROL_TASK_H
#define __CONTROL_TASK_H

#include "main.h"
#include "cmsis_os.h"
#include "MPU6050.h"
#include "queue.h"

BaseType_t MPU6050_Mailbox_Init(void);
BaseType_t MPU6050_Mailbox_Peek(MPU6050_Data_t *data, TickType_t timeout);
void MPU6050_Task(void *argument);
void Control_Task(void *argument);

#endif
