#ifndef __L76K_H
#define __L76K_H


#include "fm33lg0xx_fl.h"

uint8_t L76K_get_GPS(void); 
void gps_task_create(void *params, uint8_t priority);


#endif

