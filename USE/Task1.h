#ifndef __TASK1_H
#define __TASK1_H

#include "main.h"
#include "cmsis_os.h"

/* 声明 UI 任务函数 */
void UI_Task(void *argument);
void Key_Task(void *argument);

#endif
