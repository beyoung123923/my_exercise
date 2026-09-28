#ifndef _SYS_SETTING_H
#define _SYS_SETTING_H

#include "fm33lg0xx_fl.h"


#define APP_UPDATE_FLAG                 (uint32_t)(1024*250)      						//硬件版本号  软件版本号	250K
#define MODEL_SN_ADDRESS                APP_UPDATE_FLAG+512   							//设备SN号地址			250.5K
#define DEVICE_INFO   					(uint32_t)(252 * 1024)    						//设备数据保存到			252K			

typedef enum
{
	CHINESE = 0,
	ENGLISH,
}SYS_LANGUAGE;                  

typedef enum
{
	UNIT_CENTIGRADE = 0,            //摄氏度
	UNIT_FAHRENHEIT,                //华摄氏度
}TEM;

typedef enum
{
	TCP_MODE = 0,            //TCP_MODE
	MQTT_MODE,                //MQTT
}NET_MODEL;

typedef enum
{
    NO = 0,
	AGPS,            		//AGPS
	GPS,                	//GPS
}GPS_MODE;

typedef struct 
{ 
	/* 秒 分 时 周 日 月 年*/
	uint8_t sec;
	uint8_t min;
	uint8_t hour;
	uint8_t day;
	uint8_t week;
	uint8_t month;
	uint8_t year;
	uint8_t nop_time;   			//对齐使用
}rtc_t;

typedef struct 
{
    uint8_t language;               //语言切换
    uint8_t trans_model;            //通讯模组				0:4G			    1:WIFI			
    uint8_t tem_unit;               //温度单位				0:摄氏度			1：华氏度
    uint8_t internal_en;            //内部传感器使能                  
	uint8_t external_en;			//外置传感器使能
    uint8_t internal_model;         //内部传感器型号         0:GXHT3x			2:GXHT40C(SHT4x兼容)
    uint8_t external_model;         //外置传感器型号         0 TP   			1 DS18B20		
	uint8_t dhcp_en;				//自动获取IP		    	0：OFF			1:ON
	uint8_t net_mode;				//网络模式				0:TCP_MODE			1:MQTT			
	uint8_t gps_mode;			    //GPS定位				0: OFF			1:AGPS           2:GPS
	uint8_t back_light_time;		//背光时间				0:20			1:40			2：60			3：常亮
	uint8_t alarm_en;				//报警使能				0：OFF			1:ON
	uint8_t auto_switch_show;		//自动显示				0：手动			 1:自动
    uint8_t up_type;                //上报模式              0：间隔上报      1：整点上报
    uint8_t dev_addr;        	    //设备地址
	uint8_t ota_en;					//OTA使能
	uint16_t collect_time;          //采样间隔              1-6500 秒
	uint8_t external_long_power;	//外置传感器长供电
    uint8_t modbus_en;				//modbus使能		
	uint8_t wifi_ssid[64];			//WIFI账号
    uint8_t wifi_pawd[64];			//WIFI密码
	uint8_t wifi_ip[20];			//WIFI ip
    uint8_t wifi_mask[20];			//子网掩码
    uint8_t wifi_gw[20];			//网关
    uint8_t wifi_mac[20];			//MAC
	uint8_t mqtt_client_id[64];		//mqtt client_id
    uint8_t mqtt_user_name[64];		//mqtt 用户名
    uint8_t mqtt_user_pawd[64];		//mqtt 密码
    uint8_t mqtt_post_topic[64];	//发布主题
    uint8_t mqtt_subs_topic[64];	//订阅主题
    uint8_t server_ip[40];			//服务器地址
    uint8_t server_port[8];			//端口
	uint8_t apn[24];				//APN
    uint8_t apn_user_name[24];		//APN账号
    uint8_t apn_user_pwd[24];		//APN密码
    uint8_t model_sn[16];           //设备SN编码       
	float up_data[4];				//数据上限				
	float down_data[4];				//数据下限
	float amend_k[4];				//数据斜率修正
	float amend_b[4];				//数据常量修正
	float data_hc[4];				//数据回差值
	float data_bd[4];				//数据波动
	uint32_t soft_date;				//软件版本时间
	uint16_t updata_time;           //上报时间              1-1440	
    uint8_t alarm_interval_en;      //报警上报间隔使能       2  min            
	uint8_t reserve;
    uint32_t reserve_num[49];       //预留空间
    uint32_t crc;	
}sys_settings_t;

uint8_t write_flash(uint32_t addr, uint16_t erase_page_num, uint32_t *data, uint16_t len);
void system_restore_default(void);

void system_write_setting(void);

void system_read_setting(void);

uint32_t Version_DATA(void);

uint8_t system_set_cloud_config(uint8_t *buf);
void system_format_cloud_config(char *buf);

//语言
uint8_t config_get_language(void);
void config_set_language(uint8_t data);

//通讯模组
uint8_t config_get_trans_model(void);
void config_set_trans_model(uint8_t data);

//温度单位
uint8_t config_get_tem_unit(void);
void config_set_tem_unit(uint8_t data);

//内部传感器开关
uint8_t config_get_internal_en(void);
void config_set_internal_en(uint8_t data);

//外部传感器开关
uint8_t config_get_external_en(void);
void config_set_external_en(uint8_t data);

//内部传感器型号
uint8_t config_get_internal_model(void);
void config_set_internal_model(uint8_t data);

//外置单总线型号
uint8_t config_get_external_model(void);
void config_set_external_model(uint8_t data);

//dncp
uint8_t config_get_dhcp_en(void);
void config_set_dhcp_en(uint8_t data);

//网络模式
uint8_t config_get_net_mode(void);
void config_set_net_mode(uint8_t data);

//GPS
uint8_t config_get_gps_mode(void);
void config_set_gps_mode(uint8_t data);

//背光时间
uint8_t config_get_back_light_time(void);
void config_set_back_light_time(uint8_t data);

//报警使能
uint8_t config_get_alarm_en(void);
void config_set_alarm_en(uint8_t data);

//自动显示
uint8_t config_get_auto_switch_show(void);
void config_set_auto_switch_show(uint8_t data);

//WIFI账号
uint8_t* config_get_wifi_ssid(void);
void config_set_wifi_ssid(uint8_t *data);

//WIFI密码
uint8_t* config_get_wifi_pawd(void);
void config_set_wifi_pawd(uint8_t *data);

//WIFI IP
uint8_t* config_get_wifi_ip(void);
void config_set_wifi_ip(uint8_t *data);

//WIFI子网掩码
uint8_t* config_get_wifi_mask(void);
void config_set_wifi_mask(uint8_t *data);

//WIFI网关
uint8_t* config_get_wifi_gw(void);
void config_set_wifi_gw(uint8_t *data);

//WIFI mac
uint8_t* config_get_wifi_mac(void);
void config_set_wifi_mac(uint8_t *data);

//MQTT CLIENT ID
uint8_t* config_get_mqtt_client_id(void);
void config_set_mqtt_client_id(uint8_t *data);

//MQTT user name
uint8_t* config_get_mqtt_user_name(void);
void config_set_mqtt_user_name(uint8_t *data);

//MQTT user pawd
uint8_t* config_get_mqtt_user_pawd(void);
void config_set_mqtt_user_pawd(uint8_t *data);

//MQTT发布主题
uint8_t* config_get_mqtt_post_topic(void);
void config_set_mqtt_post_topic(uint8_t *data);

//MQTT订阅主题
uint8_t* config_get_mqtt_subs_topic(void);
void config_set_mqtt_subs_topic(uint8_t *data);

//服务器IP
uint8_t* config_get_server_ip(void);
void config_set_server_ip(uint8_t *data);

//服务器端口
uint8_t* config_get_server_port(void);
void config_set_server_port(uint8_t *data);

//APN
uint8_t* config_get_apn(void);
void config_set_apn(uint8_t *data);

//APN账号
uint8_t* config_get_apn_name(void);
void config_set_apn_name(uint8_t *data);

//APN密码
uint8_t* config_get_apn_pawd(void);
void config_set_apn_pawd(uint8_t *data);

//SN号
uint8_t* config_get_sn(void);
void config_set_sn(uint8_t *data);

//数据上限
float config_get_up_data(uint8_t chose);
void config_set_up_data(float data,uint8_t chose);

//数据下限
float config_get_down_data(uint8_t chose);
void config_set_down_data(float data,uint8_t chose);

//数据斜率修正
float config_get_amend_k(uint8_t chose);
void config_set_amend_k(float data,uint8_t chose);

//数据常量修正
float config_get_amend_b(uint8_t chose);
void config_set_amend_b(float data,uint8_t chose);

//校准系数合法性校验(NaN/inf/超范围返回0)
uint8_t system_amend_k_legal(float data);
uint8_t system_amend_b_legal(float data);

//数据回差值
float config_get_data_hc(uint8_t chose);
void config_set_data_hc(float data,uint8_t chose);

//数据波动值
float config_get_data_bd(uint8_t chose);
void config_set_data_bd(float data,uint8_t chose);

//采样间隔时间
uint16_t config_get_collect_time(void);
void config_set_collect_time(uint16_t data);

//上报模式
uint8_t config_get_up_type(void);
void config_set_up_type(uint8_t data);

//上报间隔时间
uint16_t config_get_updata_time(void);
void config_set_updata_time(uint16_t data);

//软件版本
uint32_t config_get_soft_date(void);
void config_set_soft_date(uint32_t data);

//OTA开关
uint8_t config_get_ota_en(void);
void config_set_ota_en(uint8_t data);

//外置传感器长供电
uint8_t config_get_external_long_power(void);
void config_set_external_long_power(uint8_t data);

//设备地址
uint8_t config_get_dev_addr(void);
void config_set_dev_addr(uint8_t data);

//modbus使能
uint8_t config_get_modbus_en(void);
void config_set_modbus_en(uint8_t data);

//报警时上报间隔使能
uint8_t config_get_alarm_interval_en(void);
void config_set_alarm_interval_en(uint8_t data);

uint32_t system_get_net_err(void);
void system_set_net_err(uint32_t bit);
void system_clear_net_err(uint32_t bit);




#endif

