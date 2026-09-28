#include "delay.h"
#include "usart.h"

void MF_BSTIM16_Init2(void)
{
	FL_BSTIM16_InitTypeDef    TimerBase_InitStruct;
	
    TimerBase_InitStruct.prescaler = 23;
    TimerBase_InitStruct.autoReload = 0;  //40
    TimerBase_InitStruct.autoReloadState = FL_DISABLE;
    TimerBase_InitStruct.clockSource = FL_CMU_BSTIM16_CLK_SOURCE_APBCLK;
    FL_BSTIM16_Init(BSTIM16, &TimerBase_InitStruct);
}

void sensor_delay_us(uint16_t us)
{
	FL_BSTIM16_WriteAutoReload(BSTIM16,us);
	FL_BSTIM16_ClearFlag_Update(BSTIM16);
	FL_BSTIM16_Enable(BSTIM16);
	while(FL_BSTIM16_IsActiveFlag_Update(BSTIM16) == 0);
	FL_BSTIM16_Disable(BSTIM16);
}

void MF_GPTIM0_TimerBase_Init(void)
{
    FL_GPTIM_InitTypeDef    TimerBase_InitStruct;

    TimerBase_InitStruct.prescaler = 23;
    TimerBase_InitStruct.counterMode = FL_GPTIM_COUNTER_DIR_UP;
    TimerBase_InitStruct.autoReload = 0;
    TimerBase_InitStruct.clockDivision = FL_GPTIM_CLK_DIVISION_DIV1;
    TimerBase_InitStruct.autoReloadState = FL_DISABLE;

    FL_GPTIM_Init(GPTIM0, &TimerBase_InitStruct);    
}

void rtc_delay_us(uint32_t us)
{
	FL_GPTIM_WriteAutoReload(GPTIM0,us);
	FL_GPTIM_ClearFlag_Update(GPTIM0);
	FL_GPTIM_Enable(GPTIM0);
	while(FL_GPTIM_IsActiveFlag_Update(GPTIM0) == 0);
	FL_GPTIM_Disable(GPTIM0);
}
