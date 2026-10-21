#ifndef __KEY_H
#define __KEY_H

#include "main.h"


uint8_t Key_GetState(void);
void Key_Tick(void);
uint8_t Key_GetNum(void);

void Key_Task(void *argument);

#endif
