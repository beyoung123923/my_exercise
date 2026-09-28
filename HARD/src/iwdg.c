#include "iwdg.h"
#include "usart.h"



//看门狗初始化
void iwdt_init(void)
{
	trace_log("看门狗初始化\r\n");
    FL_IWDT_InitTypeDef    IWDT_InitStruct;

    IWDT_InitStruct.iwdtWindows = 0;
    IWDT_InitStruct.overflowPeriod = FL_IWDT_PERIOD_16000MS;

    FL_IWDT_Init(IWDT, &IWDT_InitStruct);
}

//看门狗重装载
void iwdt_reload(void)
{
	FL_IWDT_ReloadCounter(IWDT);
}

