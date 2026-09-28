#ifndef _PWR_MANAGE_H_
#define _PWR_MANAGE_H_

/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl.h"
#include "fm33lg0xx_fl_gpio.h"
#include "fm33lg0xx.h"

#include "sys_setting.h"
/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
/**
* @brief  电源管理设置数目
*/
//电池容量
#define ER_14505_2          4400

#define PWR_EVENT_ALARM   	0x01 << 0
#define PWR_EVENT_OFF     	0x01 << 1
#define PWR_EVENT_KEY	  	0x01 << 2
#define PWR_EVENT_SENSOR_ALARM 0x01 << 3

/**
* @brief  进程是否同意停止MCU。
*/
#if (PWR_MANAGE_NUM <= 8)
    typedef uint8_t PWR_ARG_HALT;
#elif (PWR_MANAGE_NUM <= 16)
    typedef uint16_t PWR_ARG_HALT;
#elif (PWR_MANAGE_NUM <= 32)
    typedef uint32_t PWR_ARG_HALT;
#else 
    #error "pwr-manage "
#endif
/* 变量 --------------------------------------------------------------*/
/* 类型定义 ----------------------------------------------------------*/
/**
* @brief  电源管理ID!
*/
typedef enum
{
    PWR_BIT_INVALID_MIN = (uint8_t )0,
    PWR_WIRELESS_BIT,
    PWR_SENSOR_BIT,
    PWR_OLED_BIT,
    PWR_HIST_BIT,
    PWR_BIT_INVALID_MAX
}PWR_BIT_Type;

#define PWR_MANAGE_NUM  4
//#define PWR_MANAGE_NUM  3
/* 函数声明 ----------------------------------------------------------*/
/* 函数代码 ----------------------------------------------------------*/

void chip_init(void);

void pwr_arg_halt(PWR_BIT_Type e_id);

void pwr_opppse_halt(PWR_BIT_Type e_id);

void pwr_enter_low_pwr(void);

void pwr_init(void);

void task_post_event_pwr_manage(uint32_t event);

void task_post_isr_event_pwr_manage(uint32_t event);

void pwr_task_create(void *params, uint8_t priority);

void pwr_sleep(void);

void wakeup(void);

void power_open(void);

void power_close(void);

void power_en_init(void);

void power_en_deinit(void);

rtc_t *chip_get_time(void);

void create_task_pmu(void *params, uint8_t priority);

void task_sleep_post_event(uint32_t bit);

void pwr_exit_low_pwr(void);

uint8_t is_sensor_finish(void);

uint8_t is_flash_finish(void);

uint8_t is_server_finish(void);

void next_updata_time(rtc_t rtc_time);

#endif
