#ifndef __Air780E_H
#define __Air780E_H
#include "main.h"

//#define IOT_4G_UART_COM1_BUF_LEN   1040     //400   //gprs串口接收数组长度

void A78_PSM_task(uint32_t event);

void a78_task(void *params);

#endif

