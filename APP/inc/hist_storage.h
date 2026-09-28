#ifndef HIST_STORAGE_H_
#define HIST_STORAGE_H_

#include <stdint.h>
#include "sd3078.h"

#define FLASH_SECTOR_SIZE   (uint32_t)4096
#define FLASH_SECTOR_NUM    (uint32_t)976
#define FLASH_ADDR_END		(uint32_t)0x3D0000

#define HIST_SAVE   (1 << 0)
#define HIST_READ   (1 << 1)
#define HIST_CLEAR  (1 << 2)

typedef struct 
{
    rtc_t data_time;
    float sensor_value[4];
    float battery_voltage;
    uint16_t sensor_alarm_state;
    int16_t rssi;
    uint32_t prev_addr;
    uint32_t next_addr;
    uint32_t crc;
}hist_storage_t;

#define MAX_RECORD_NUM      (uint32_t)90856
#define SECTOR_RECORD_NUM 	(uint32_t)93	

typedef struct 
{
    uint32_t save_addr;
    uint8_t  start_year;
    uint8_t  start_mon;
    uint8_t  start_day;
    uint8_t  end_year;
    uint8_t  end_mon;
    uint8_t  end_day;
}hist_run_t;

uint32_t hist_init(void);

uint8_t hist_save(uint32_t *addr);

uint8_t hist_detect(uint32_t addr);

void hist_read(hist_storage_t *read_hist, uint32_t addr);

void hist_read_date(uint8_t s_year, uint8_t s_mon, uint8_t s_day, uint8_t e_year, uint8_t e_mon, uint8_t e_day);

uint32_t hist_date_addr_get(uint8_t year, uint8_t mon, uint8_t day);

void hist_callback(void);

void task_flash_post_event(uint32_t event);

void task_flash(void *pvrams);

void hist_set_data_read(uint8_t s_year, uint8_t s_mon, uint8_t s_day, uint8_t e_year, uint8_t e_mon, uint8_t e_day);

void create_task_hist(void *params, uint8_t priority);
#endif

