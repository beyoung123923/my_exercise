
#ifndef _DEVICE_INFO_H_
#define _DEVICE_INFO_H_
/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl.h"
/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
#define BACK_CLOUD_DR       0x00
#define BACK_RECORD_DR      0x01
#define BACK_PEND_NUM_DR    0x02
#define BACK_PEND_ADDR_DR   0x03

#define net_ok           0x00
#define Modul_err        (1<<0)     //bit1 模块读取失败
#define JOIN_ERR         (1<<2)     //bit3 入网注册失败
#define data_report_err  (1<<3)     //bit4 数据上报失败

#define send_IT        (1 << 0)   //数据发送
#define set_IT         (1 << 1)   //模块参数设置
#define read_IT        (1 << 2)   //模块参数读取

#define EVENT_OK      (1)
#define EVENT_RUN     (0)

typedef struct 
{
    uint8_t lorawan_set_data_en;
    uint8_t event_up;
    uint8_t lorawan_net_status;    //lorawan 网络状态  0 开机/联网失败   1联网成功
	uint8_t sys_status;			   //	
	uint8_t lorawan_first;         //第一次启动标志
    uint8_t net_err_stu;
}event_state;

typedef enum
{
	CMD_DATA1_UP,
	CMD_DATA2_UP,
	CMD_DATA1_DOWN,
	CMD_DATA2_DOWN,
	CMD_AMEND_K1,
	CMD_AMEND_K2,
	CMD_AMEND_B1,
	CMD_AMEND_B2,
	CMD_DATA1_HC,
	CMD_DATA2_HC,
	CMD_DATA1_BD,
	CMD_DATA2_BD,
    CMD_INTERNAL_END,
}CMD_INTERNAL;

typedef enum
{
	CMD_ALL_DATA1_UP,
	CMD_ALL_DATA2_UP,
	CMD_ALL_DATA3_UP,
	CMD_ALL_DATA4_UP,
	CMD_ALL_DATA1_DOWN,
	CMD_ALL_DATA2_DOWN,
	CMD_ALL_DATA3_DOWN,
	CMD_ALL_DATA4_DOWN,
	CMD_ALL_AMEND_K1,
	CMD_ALL_AMEND_K2,
	CMD_ALL_AMEND_K3,
	CMD_ALL_AMEND_K4,
	CMD_ALL_AMEND_B1,
	CMD_ALL_AMEND_B2,
	CMD_ALL_AMEND_B3,
	CMD_ALL_AMEND_B4,
	CMD_ALL_DATA1_HC,
	CMD_ALL_DATA2_HC,
	CMD_ALL_DATA3_HC,
	CMD_ALL_DATA4_HC,
	CMD_ALL_DATA1_BD,
	CMD_ALL_DATA2_BD,
	CMD_ALL_DATA3_BD,
	CMD_ALL_DATA4_BD,
    CMD_ALL_SENSOR_END,
}CMD_ALL_SENSOR;

/* 变量 --------------------------------------------------------------*/
/* 类型定义 ----------------------------------------------------------*/
typedef enum{
	CMD_SAVE  = 0,
	CMD_RESART,
	CMD_CLEAR_ALL,
    CMD_SET_USE,
    CMD_DEV_MODEL,
    CMD_HARD_VER,
    CMD_SOFT_VER,
    CMD_DEV_SN,
	CMD_COMM_MODULE,
    CMD_BASE_SET,
    CMD_LANGUAGE,
	CMD_RESET_FACTOR,
    CMD_DEV_ADDR,
    CMD_UP_TYPE,
    CMD_UP_TIME,
    CMD_COLLECT_TIME,
    CMD_BACK_LIGHT_TIME,
    CMD_AUTO_SWITCH_SHOW,
    CMD_VBAT_VALUE,
    CMD_NET_SET,
	CMD_CCID,
	CMD_IMEI,
    CMD_SIGNAL,
	CMD_OTA_EN,
    CMD_WIFI_SSID,
    CMD_WIFI_PAWD,
    CMD_DHCP_EN,
    CMD_WIFI_IP,
    CMD_WIFI_MASK,
    CMD_WIFI_GW,
    CMD_WIFI_MAC,
    CMD_GPS_MODE,
    CMD_NET_MODE, 
    CMD_MODBUS_EN,    
    CMD_MQTT_CLIENT_ID,
	CMD_MQTT_USER_NAME,
    CMD_MQTT_USER_PAWD,
    CMD_MQTT_POST_TOPIC,
    CMD_MQTT_SUBS_TOPIC,
    CMD_SERVER_IP,
    CMD_SERVER_PORT,
    CMD_APN,
    CMD_APN_USER_NAME,
    CMD_APN_USER_PWD,
    CMD_SENSOR_SET,
	CMD_INTERNAL_EN,
	CMD_EXTERNAL_EN, 
    CMD_INTERNAL_MODEL,
    CMD_EXTERNAL_MODEL,
	CMD_EXTERNAL_LONG_POWER,
    CMD_ALARM_EN,
    CMD_ALARM_INTERVAL_EN,
    CMD_TEM_UNIT,
    CMD_COM_END,
}CMD_T;


/* 函数声明 ----------------------------------------------------------*/
void config_resp(void *head, void *param,uint32_t handler);

void system_write_setting(void);
void device_param(uint8_t *buf,uint32_t hangdler);

void system_config_create_task(void *params, uint8_t priority);

void device_nbvbat_set(uint32_t vbat);
uint32_t device_nbvbat_get(void);

void device_restart_num_set(uint32_t restart_num);
uint32_t device_restart_num_get(void);

#endif

