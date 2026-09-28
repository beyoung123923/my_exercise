#ifndef __DELAY_H
#define __DELAY_H

#include "fm33lg0xx_fl.h"
#include "fm33lg0xx.h"

void MF_BSTIM16_Init2(void);
void sensor_delay_us(uint16_t us);
void MF_GPTIM0_TimerBase_Init(void);
void rtc_delay_us(uint32_t us);

#endif

