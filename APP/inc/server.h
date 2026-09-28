#ifndef __SERVER_H
#define __SERVER_H

#include "fm33lg0xx_fl.h"


#define IOT_DATA_LEN    	1040
#define SEND_DATA           0
#define START_TCP_MODBUS    1
#define STOP_TCP_MODBUS     2

typedef enum
{	 
	err_ok             =(0),
	err_net    	       =(1<<1),			/*网络信号ErrCode*/
	err_srv 	       =(1<<2),			/*连接服务器ErrCode*/
    err_signal         =(1<<3),
    err_link           =(1<<4),
    err_sim           =(1<<5),
} wireless_err_t;	

typedef struct 
{
    uint8_t  wireless_sim;
    int8_t   wireless_signal;
	uint8_t  upload_set;
    uint8_t  tcp_modbus_state;         //1:开modebus    2：关modebus                                                                         
    uint8_t  first_ntp;                                                                                         
    uint32_t err_status;
    float    lat;
    float    lng;
    uint8_t  ccid[24];
	uint8_t  imei[16];
}wireless_info_t;

void task_post_event_iot(uint32_t event);

void iot_en_deinit(void);

void gps_en_init(void);

void iot_4g_en_init(void);

void wifi_en_init(void);

void open_4g_en(void);
void close_4g_en(void);

void open_gps_en(void);
void close_gps_en(void);

void open_wifi_en(void);
void close_wifi_en(void);

uint16_t dev_info_to_buf(uint8_t *buf);

uint16_t newdata_to_buf(uint8_t *buf);
uint16_t tcp_udp_newdata_to_buf(uint8_t *buf);
uint16_t set_new_data_to_buf(void);

void wireless_set_timer(uint8_t *temp_buf);

void server_task_create(void *params, uint8_t priority);

int8_t config_get_signal(void);

#endif

