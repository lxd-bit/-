// Encode.h
#ifndef __ENCODER_H
#define __ENCODER_H

#include "main.h"
#include "tim.h"   // 用于访问 htim3, htim4

void Encoder_Init(void);
int16_t Encoder_Get(uint8_t n);

#endif
