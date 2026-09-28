#ifndef _TOUCH_KEY_H_
#define _TOUCH_KEY_H_

#include <stdio.h>
#include "fm33lg0xx_fl.h"

#define TOUCH_CNT  2

typedef struct 
{
    uint8_t buf[4];
    uint8_t read_offset;
    uint8_t write_offset;
}touch_fifo_t;

typedef struct
{
    uint8_t (*is_key_down_fun)(void);
    uint8_t  press_cnt;
    uint16_t long_cnt;
    uint16_t long_time;
    uint8_t  status;
}touch_t;

typedef enum{
    KEY_NONE  = 0,
    MENU_DOWN,
    MENU_UP,
    MENU_LONG,
    POWER_DOWN,
    POWER_UP,
    POWER_LONG,
}KEY_ENUM_T;

#define KEY_FILTER_TIME 2
#define KEY_LONG_TIME   200

uint8_t touch_get(void);

void touch_exit_init(void);

void touch_clear(void);

void touch_scan(void);

void chip_touch_pad_init(void);

void task_touch_pad(void *param);

void beep(uint16_t ms);

void beep_and_led_init(void);

void beep_and_led_deinit(void);

void button_create_task(void *params, uint8_t priority);

void touch_exit_deinit(void);

#endif

