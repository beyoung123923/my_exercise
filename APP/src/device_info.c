/* Includes ------------------------------------------------------------------*/
#include "device_info.h"
#include "usart.h"
#include "tool.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys_setting.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include "sd3078.h"
#include "hist_storage.h"
#include "server.h"
#include "sensor.h"
#include "flash.h"
#include "main.h"

static xTaskHandle device_config_handler;

/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
/* 变量 --------------------------------------------------------------*/

/**
 * @brief: 应答函数
 * @param  head:头   param:参数
 * @return: None
 */
void config_resp(void *head, void *param,uint32_t handler)
{
	uint8_t tx_size = 0;
	uint16_t crc;

	tx_size = strlen(head) + strlen(param) + 5;
	char tx_buf[tx_size];

	memset(tx_buf, 0, tx_size);

	tx_buf[0] = 0XFF;
	tx_buf[1] = 0X03;
	tx_buf[2] = tx_size - 5;

	strcat(tx_buf, head);
	strcat(tx_buf, param);

	crc = calc_crc16((uint8_t *)tx_buf, tx_size - 2);
	tx_buf[tx_size - 2] = crc >> 8;
	tx_buf[tx_size - 1] = crc;

    if(handler == 0)
	{
		uart_transmit_buf(USB_UART, (uint8_t *)tx_buf, tx_size);
	}
//    else
//	{
//		bt_snd_data((uint8_t *)tx_buf, tx_size, USER_IDX_READ_NOTIFY_VAL);
//	}
}

//通讯模组选择
static void system_ack_trans_model_chose(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_trans_model();

    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("通讯模块(4G/WIFI):", "4G" , handler);
        }
        else if(data == 1)                                //其他模块
        {
			config_resp("通讯模块(4G/WIFI):", "WIFI" , handler);
        }  
    }
    else
    {
        if(data == 0)
        {
            config_resp("Comm Module(4G/WIFI):", "4G" , handler);
        }
        else if(data == 1)                               //其他模块
        {
			config_resp("Comm Module(4G/WIFI):", "WIFI" , handler);
        }
    }
}
static void system_write_trans_model_chose(uint8_t *buf)
{
    if(strstr((char *)buf, "4G"))
	{
        config_set_trans_model(0);
    }
    else if(strstr((char *)buf, "WIFI"))                                          //其他模块
    {
		config_set_trans_model(1);
    }
}

//SN号
static void system_ack_sn(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_sn();
    if(language == CHINESE)
    {
        config_resp("机身编号(SN):", data , handler);
    }
    else
    {
        config_resp("Serial number:", data , handler);
    }
}

//语言设置
void system_write_language(uint8_t *buf)
{
	if(strstr((char *)buf, "中文") || strstr((char *)buf, "Chinese"))
	{
        config_set_language(CHINESE);	 
	}
	else if(strstr((char *)buf, "英文") || strstr((char *)buf, "English"))
	{
        config_set_language(ENGLISH);	
	}
}

//恢复出厂设置
void system_write_factory_reset(uint8_t *buf)
{
	system_restore_default();	
}

//上报模式
static void system_ack_up_type(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_up_type();
    if(language == CHINESE)
    {
		if(data == 0)
		{
			config_resp("上报模式(间隔/整点):", "间隔" , handler);
		}
		else if(data == 1)
		{
			config_resp("上报模式(间隔/整点):", "整点", handler);
		}	
    }
    else
    {
		if(data == 0)
		{
			config_resp("Upload mode(interval/o'clock):", "interval" , handler);
		}
		else if(data == 1)
		{
			config_resp("Upload mode(interval/o'clock):", "o'clock" , handler);
		}
    }
}
static void system_write_up_type(uint8_t *buf)
{
    if(strstr((char *)buf, "间隔")||strstr((char *)buf, "interval"))
    {
        config_set_up_type(0);
    }
    else if(strstr((char *)buf, "整点")||strstr((char *)buf, "o'clock"))
    {
        config_set_up_type(1);
    }
}

//上报间隔
static void system_ack_updata_time(uint8_t language, uint32_t handler)
{
    char data_t[16] = "";

    sprintf(data_t,"%d",config_get_updata_time());

    if(language == CHINESE)
    {
		config_resp("上报间隔[1~1440 分钟]:", data_t, handler);
    }
    else
    {
		config_resp("Upload time[1~1440 min]:", data_t , handler);
    }
}
static void system_write_updata_time(uint8_t *buf)
{
    int data;
    data = atoi((char *)buf);

    if(data > 1440)
    {
        data = 1440;
    }
    if(data < 1)
    {
        data = 1;
    }
    config_set_updata_time(data);  
}

//采样间隔
static void system_ack_collect_time(uint8_t language, uint32_t handler)
{
    char data_t[16] = "";
    uint16_t data;
    data = config_get_collect_time();
    sprintf(data_t, "%d", data);
    if(language == CHINESE)
    {
        config_resp("采样间隔[1~8640*10 秒]:", data_t , handler);
    }
    else
    {
        config_resp("Collect time[1~8640*10 sec]:", data_t , handler);
    }
}
static void system_write_collect_time(uint8_t *buf)
{
    int data;
    data = atoi((char *)buf);
    if(data > 8640)
    {
        data = 8640;
    }
    if(data < 1)
    {
        data = 1;
    }
	config_set_collect_time(data);
}

//亮屏时间
static void system_ack_back_light_time(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_back_light_time();

	 if(language == CHINESE)
    {
		if(data == 0)
		{
			config_resp("背光时间(20秒/40秒/60秒/常亮):","20秒", handler);
		}
		else if(data == 1)
		{
			config_resp("背光时间(20秒/40秒/60秒/常亮):","40秒", handler);
		}
		else if(data == 2)
		{
			config_resp("背光时间(20秒/40秒/60秒/常亮):","60秒", handler);
		}
		else if(data == 3)
		{
			config_resp("背光时间(20秒/40秒/60秒/常亮):","常亮", handler);
		}  
    }
    else
    {
		if(data == 0)
		{
			config_resp("Backlighting Time(20sec/40sec/60sec/Always On):","20sec", handler);
		}
		else if(data == 1)
		{
			config_resp("Backlighting Time(20sec/40sec/60sec/Always On):","40sec", handler);
		}
		else if(data == 2)
		{
			config_resp("Backlighting Time(20sec/40sec/60sec/Always On):","60sec", handler);
		}
		else if(data == 3)
		{
			config_resp("Backlighting Time(20sec/40sec/60sec/Always On)):","Always On", handler);
		} 
    }
}
static void system_write_back_light_time(uint8_t *buf)
{
    if(strstr((char *)buf, "20"))
    {
        config_set_back_light_time(0);
    }
    else if(strstr((char *)buf, "40"))
    {
        config_set_back_light_time(1);
    }
    else if(strstr((char *)buf, "60"))
    {
        config_set_back_light_time(2);
    }
    else
    {
        config_set_back_light_time(3);
    }
}

//自动手动显示
static void system_ack_auto_switch_show(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_auto_switch_show();

	 if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("主页切换方式(自动切换/手动切换):","手动切换", handler); 
        }
        else if(data == 1)
        {
            config_resp("主页切换方式(自动切换/手动切换):","自动切换", handler);
        }   
    }
    else
    {
        if(data == 0)
        {
            config_resp("Display Mode(Auto/Manual):","Manual", handler); 
        }
        else if(data == 1)
        {
            config_resp("Display Mode(Auto/Manual):","Auto", handler);
        } 
    }
}
static void system_write_auto_switch_show(uint8_t *buf)
{
    if((strstr((const char*)buf,"Auto"))||(strstr((const char*)buf,"自动切换")))
    {
        config_set_auto_switch_show(1);	
    }
    else if((strstr((const char*)buf,"Manual"))||(strstr((const char*)buf,"手动切换")))
    {
        config_set_auto_switch_show(0);
    }
}

//电池电量
/*static void system_ack_vbat_bfb(uint8_t language, uint32_t handler)
{
    char data_t[11] = "";
    float data;
    data = config_get_vbat_bfb();
    if(data > 100)
    {
        data = 100;
    }
    else if(data < 0)
    {
        data = 0;
    }
    sprintf(data_t, "%0.2f", data);
    if(language == CHINESE)
    {
        config_resp("电池电量[1~100%]:", data_t , handler);
    }
    else
    {
        config_resp("Battery level[1~100%]:", data_t , handler);
    }
}*/

//电池电压
static void system_ack_vbat_value(uint8_t language, uint32_t handler)
{
    char data_t[16] = "";
    float data;
    data = config_get_vbat_value();

    sprintf(data_t, "%.0f", data);
    if(language == CHINESE)
    {
        config_resp("电池电压[mV]:", data_t , handler);
    }
    else
    {
        config_resp("Battery voltage[mV]:", data_t , handler);
    }
}

void chr_find_insert(char *buf,char data)
{
	uint8_t buf_len;
	uint8_t j;
	uint8_t i;
	
	for(i = 0;i<62;i++)
	{
		if(buf[i] == data)
		{
			buf_len = strlen(buf);
			if(buf_len > 62)
			{
				buf_len = 62;
				buf[63] = '\0';
			}
			for(j = buf_len;j>i;j--)
			{
				buf[j] = buf[j-1];
			}	
			buf[i]= 0x5C;	
			buf[buf_len+1] = '\0';
			i++;
		}
	}
}


//WIFI 账号
static void system_ack_wifi_ssid(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_ssid();
    if(language == CHINESE)
    {
        config_resp("WIFI账号:", data , handler);
    }
    else
    {
        config_resp("WIFI SSID:", data , handler);
    }
}
static void system_write_wifi_ssid(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
		chr_find_insert((char *)buf,0x5C);	
		chr_find_insert((char *)buf,',');
		chr_find_insert((char *)buf,'"');
        config_set_wifi_ssid(buf);
    }
}

//WIFI 密码
static void system_ack_wifi_pawd(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_pawd();
    if(language == CHINESE)
    {
        config_resp("WIFI密码:", data , handler);
    }
    else
    {
        config_resp("WIFI password:", data , handler);
    }
}
static void system_write_wifi_pawd(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
		chr_find_insert((char *)buf,0x5C);	
		chr_find_insert((char *)buf,',');
		chr_find_insert((char *)buf,'"');
        config_set_wifi_pawd(buf);
    }
}

//dhcp en
static void system_ack_dhcp_en(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_dhcp_en();
    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("自动获取IP(ON/OFF):","OFF", handler);
        }
        else if(data == 1)
        {
            config_resp("自动获取IP(ON/OFF):","ON", handler);
        }
    }
    else
    {
        if(data == 0)
        {
            config_resp("DHCP(ON/OFF):","OFF", handler);
        }
        else if(data == 1)
        {
            config_resp("DHCP(ON/OFF):","ON", handler);
        }
    }
}
static void system_write_dhcp_en(uint8_t *buf)
{
    if(strstr((char *)buf, "ON"))
    {
        config_set_dhcp_en(1);
    }
    else if(strstr((char *)buf, "OFF"))
    {
        config_set_dhcp_en(0);
    }  
}

//WIFI IP
static void system_ack_wifi_ip(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_ip();
    config_resp("IP:", data , handler);
}
static void system_write_wifi_ip(uint8_t *buf)
{
    if(strlen((const char*)buf) < 20)
    {
        config_set_wifi_ip(buf);
    }
}

//WIFi 子网掩码
static void system_ack_wifi_mask(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_mask();
    if(language == CHINESE)
    {
        config_resp("子网掩码:", data , handler);
    }
    else
    {
        config_resp("Subnet mask:", data , handler);
    }
}
static void system_write_wifi_mask(uint8_t *buf)
{
    if(strlen((const char*)buf) < 20)
    {
        config_set_wifi_mask(buf);
    }
}

//WIFI网关
static void system_ack_wifi_gw(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_gw();
    if(language == CHINESE)
    {
        config_resp("网关地址:", data , handler);
    }
    else
    {
        config_resp("Gateway:", data , handler);
    }
}
static void system_write_wifi_gw(uint8_t *buf)
{
    if(strlen((const char*)buf) < 20)
    {
        config_set_wifi_gw(buf);
    }
}

//MAC
static void system_ack_wifi_mac(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_wifi_mac();
    config_resp("MAC:", data , handler);
}
static void system_write_wifi_mac(uint8_t *buf)
{
    if(strlen((const char*)buf) < 20)
    {
        config_set_wifi_mac(buf);
    }
}

//GPS
static void system_ack_gps_mode(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_gps_mode();
    if(data == 1)
    {
        config_resp("GPS(GPS/AGPS/OFF):","AGPS", handler);
    }
    else if (data == 2)
    {
        config_resp("GPS(GPS/AGPS/OFF):","GPS", handler);
    }
    else if (data == 0)
    {
        config_resp("GPS(GPS/AGPS/OFF):","OFF", handler);
    }
}
static void system_write_gps_mode(uint8_t *buf)
{
    if(strstr((char *)buf, "AGPS"))
    {
        config_set_gps_mode(1);
    }
    else if(strstr((char *)buf, "GPS"))
    {
        config_set_gps_mode(2);
    }
    else if(strstr((char *)buf, "OFF"))
    {
        config_set_gps_mode(0);
    }
}

//NET 
static void system_ack_net_mode(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_net_mode();
    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("工作模式(MQTT/TCP):","TCP", handler);
        }
        else if(data == 1)
        {
            config_resp("工作模式(MQTT/TCP):","MQTT", handler);
        }      
    }
    else 
    {
        if(data == 0)
        {
            config_resp("Work Mode(MQTT/TCP):","TCP", handler);
        }
        else if(data == 1)
        {
            config_resp("Work Mode(MQTT/TCP):","MQTT", handler);
        }
    }
}
static void system_write_net_mode(uint8_t *buf)
{
    if(strstr((char *)buf, "TCP"))
    {
        config_set_net_mode(0);
    }
    else if(strstr((char *)buf, "MQTT"))
    {
        config_set_net_mode(1);
    }
}

//mqtt client id
static void system_ack_mqtt_client_id(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_mqtt_client_id();
    config_resp("MQTT client id:", data , handler);
}
static void system_write_mqtt_client_id(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
        config_set_mqtt_client_id(buf);
    }
}

//MQTT 用户名
static void system_ack_mqtt_user_name(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_mqtt_user_name();
    if(language == CHINESE)
    {
        config_resp("MQTT用户名:", data , handler);
    }
    else
    {
        config_resp("MQTT user name:", data , handler);
    }
}
static void system_write_mqtt_user_name(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
        config_set_mqtt_user_name(buf);
    }
}

//MQTT 用户密码
static void system_ack_mqtt_user_pawd(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_mqtt_user_pawd();
    if(language == CHINESE)
    {
        config_resp("MQTT用户密码:", data , handler);
    }
    else
    {
        config_resp("MQTT user password:", data , handler);
    }
}
static void system_write_mqtt_user_pawd(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
        config_set_mqtt_user_pawd(buf);
    }
}

//发布主题
static void system_ack_mqtt_post_topic(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_mqtt_post_topic();
    if(language == CHINESE)
    { 
        config_resp("发布主题:", data , handler);
    }
    else
    {
        config_resp("Publish topic:",data , handler);
    }
}
static void system_write_mqtt_post_topic(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
        config_set_mqtt_post_topic(buf);
    }
}

//订阅主题
static void system_ack_mqtt_subs_topic(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_mqtt_subs_topic();
    if(language == CHINESE)
    { 
        config_resp("订阅主题:", data , handler);
    }
    else
    {
        config_resp("Subscribe topic:",data , handler);
    }
}
static void system_write_mqtt_subs_topic(uint8_t *buf)
{
    if(strlen((const char*)buf) < 64)
    {
        config_set_mqtt_subs_topic(buf);
    }
}

//服务器IP
static void system_ack_server_ip(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_server_ip();
    if(language == CHINESE)
    { 
        config_resp("服务器地址:", data , handler);
    }
    else
    {
        config_resp("Server address:",data , handler);
    }
}
static void system_write_server_ip(uint8_t *buf)
{
    if(strlen((const char*)buf) < 40)
    {
        config_set_server_ip(buf);
    }
}

//服务器端口
static void system_ack_server_port(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_server_port();
    if(language == CHINESE)
    { 
        config_resp("服务器端口:", data , handler);
    }
    else
    {
        config_resp("Server port:",data , handler);
    }
}
static void system_write_server_port(uint8_t *buf)
{
    if(strlen((const char*)buf) < 8)
    {
        config_set_server_port(buf);
    }
}

//APN
static void system_ack_apn(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_apn();
    config_resp("APN:", data , handler);
}
static void system_write_apn(uint8_t *buf)
{
    if(strlen((const char*)buf) < 24)
    {
        config_set_apn(buf);
    }
}

//APN账号
static void system_ack_apn_name(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_apn_name();
    config_resp("APN user:",data , handler); 
}
static void system_write_apn_name(uint8_t *buf)
{
    if(strlen((const char*)buf) < 24)
    {
        config_set_apn_name(buf);
    }
}

//APN密码
static void system_ack_apn_pawd(uint8_t language, uint32_t handler)
{
    uint8_t *data;
    data = config_get_apn_pawd();
    config_resp("APN pawd:", data , handler);
}
static void system_write_apn_pawd(uint8_t *buf)
{
    if(strlen((const char*)buf) < 24)
    {
        config_set_apn_pawd(buf);
    }
}

//内部传感器开关
static void system_ack_internal_en(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_internal_en();
    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("内部传感器(开启/关闭):", "关闭", handler); 
        }
        else if(data == 1)
        {
            config_resp("内部传感器(开启/关闭):", "开启", handler);
        }   
    }
    else
    {
        if(data == 0)
        {
            config_resp("Internal sensor(ON/OFF):", "OFF", handler); 
        }
        else if(data == 1)
        {
            config_resp("Internal sensor(ON/OFF):", "ON", handler);
        } 
    }
}
static void system_write_internal_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"OFF")))
    {
        config_set_internal_en(0);	
    }
    else if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"ON")))
    {
        config_set_internal_en(1);
    }	
}

//外部传感器开关
static void system_ack_external_en(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_external_en();

    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("外部传感器(开启/关闭):", "关闭", handler); 
        }
        else if(data == 1)
        {
            config_resp("外部传感器(开启/关闭):", "开启", handler);
        }   
    }
    else
    {
        if(data == 0)
        {
            config_resp("External sensor(ON/OFF):", "OFF", handler); 
        }
        else if(data == 1)
        {
            config_resp("External sensor(ON/OFF):", "ON", handler);
        } 
    }
}
static void system_write_external_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"OFF")))
    {
        config_set_external_en(0);	
    }
    else if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"ON")))
    {
        config_set_external_en(1);
    }	
}

//内部温湿度传感器类型
static void system_ack_internal_model (uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_internal_model();
    if(language == CHINESE)
    {
        if(data == 2)
        {
            config_resp("内部传感器类型(GXHT3x/SHT4x):", "SHT4x", handler);
        }
        else
        {
            config_resp("内部传感器类型(GXHT3x/SHT4x):", "GXHT3x", handler);
        }
    }
    else
    {
        if(data == 2)
        {
            config_resp("Internal sensor model(GXHT3x/SHT4x):", "SHT4x", handler);
        }
        else
        {
            config_resp("Internal sensor model(GXHT3x/SHT4x):", "GXHT3x", handler);
        }
    }
}
static void system_write_internal_model(uint8_t *buf)
{
    if(strstr((const char*)buf,"SHT4x"))
    {
        config_set_internal_model(2);
    }
    else if(strstr((const char*)buf,"GXHT3x"))
    {
        config_set_internal_model(0);
    }
}

//外部温湿度传感器类型
static void system_ack_external_model (uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_external_model();

    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("外部传感器类型(TP/DS18B20):", "TP", handler); 
        }  
		else if(data == 1)
        {
            config_resp("外部传感器类型(TP/DS18B20):", "DS18B20", handler); 
        }
    }
    else
    {
        if(data == 0)
        {
            config_resp("External sensor model(TP/DS18B20):", "TP", handler); 
        }
        else if(data == 1)
        {
            config_resp("External sensor model(TP/DS18B20):", "DS18B20", handler); 
        }
    }
}
static void system_write_external_model(uint8_t *buf)
{
    if(strstr((const char*)buf,"TP"))
    {
        config_set_external_model(0);	
    }	
    else if(strstr((const char*)buf,"DS18B20"))
    {
        config_set_external_model(1);	
    }
}

//外置传感器长供电使能
static void system_ack_external_long_power(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_external_long_power();
    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("外部常供电(开启/关闭):", "关闭", handler); 
        }
        else if(data == 1)
        {
            config_resp("外部常供电(开启/关闭):", "开启", handler); 
        }
    }
    else
    {
		if(data == 0)
		{
			config_resp("External continuous  power(Enable/Disable):","Disable", handler);
		}
		else if(data == 1) 
		{
			config_resp("External continuous power(Enable/Disable):","Enable", handler);
		}
    }
}
static void system_write_external_long_power(uint8_t *buf)
{
    if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"Enable")))
    {
        config_set_external_long_power(1);	
    }
    else if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"Disable")))
    {
        config_set_external_long_power(0);	
    }
}

//报警使能
static void system_ack_alarm_en(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_alarm_en();
    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("超限报警(关闭/开启):", "关闭", handler); 
        }
        else if(data == 1)
        {
            config_resp("超限报警(关闭/开启):", "开启", handler); 
        }
    }
    else
    {
		if(data == 0)
		{
			config_resp("Alarm(Disable/Enable):","Disable", handler);
		}
		else if(data == 1) 
		{
			config_resp("Alarm(Disable/Enable):","Enable", handler);
		}
    }
}
static void system_write_alarm_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"Enable")))
    {
        config_set_alarm_en(1);	
    }
    else if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"Disable")))
    {
        config_set_alarm_en(0);	
    }
}

//温度单位
static void system_ack_tem_unit(uint8_t language, uint32_t handler)
{
    uint8_t data;
    data = config_get_tem_unit();

    if(language == CHINESE)
    {
        if(data == 0)
        {
            config_resp("温度单位(摄氏度/华氏度):", "摄氏度", handler); 
        }
        else if(data == 1)
        {
            config_resp("温度单位(摄氏度/华氏度):", "华氏度", handler);
        }   
    }
    else
    {
        if(data == 0)
        {
            config_resp("Temperature unit(Centigrade/Fahrenheit):", "Centigrade", handler); 
        }
        else if(data == 1)
        {
            config_resp("Temperature unit(Centigrade/Fahrenheit):", "Fahrenheit", handler);
        } 
    }
}
static void system_write_tem_unit(uint8_t *buf)
{
    if(strstr((const char*)buf,"摄氏度")||strstr((const char*)buf,"Centigrade"))
    {
        config_set_tem_unit(0);	
    }
    else if(strstr((const char*)buf,"华氏度")||strstr((const char*)buf,"Fahrenheit"))
    {
        config_set_tem_unit(1);
    }	
}

//数据上限
static void system_ack_data_up(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_up_data(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d上限:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d range_H:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_data_up(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    config_set_up_data(data,chose);	
}

//数据下限
static void system_ack_data_down(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_down_data(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d下限:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d range_L:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_data_down(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    config_set_down_data(data,chose);	
}

//修正斜率
static void system_ack_amend_k(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_amend_k(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d斜率K值:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d value K:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_amend_k(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    if(system_amend_k_legal(data))
    {
        config_set_amend_k(data,chose);
    }
}

//修正常量
static void system_ack_amend_b(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_amend_b(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d修正B值:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d value B:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_amend_b(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    if(system_amend_b_legal(data))
    {
        config_set_amend_b(data,chose);
    }
}

//回差值
static void system_ack_data_hc(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_data_hc(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d报警回差:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d Alarm return difference:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_data_hc(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    config_set_data_hc(data,chose);	
}

//波动值
static void system_ack_data_bd(uint8_t chose, uint8_t language, uint32_t handler)
{
    char unit[10];
	char buf[30];
    float data;
	
    data = config_get_data_bd(chose);
    sprintf(unit,"%0.2f",data);
	
	if(language == CHINESE)
    {
        sprintf(buf,"通道%d波动阈值:", chose+1); 
    }
    else
    {
        sprintf(buf,"CH%d fluctuation threshold:", chose+1); 
    }
    config_resp(buf, unit, handler); 
}
static void system_write_data_bd(uint8_t chose, uint8_t *buf)
{
    float data;
    data = atof((char *)buf);
    config_set_data_bd(data,chose);	
}

//信号强度
static void system_ack_signal(uint8_t language, uint32_t handler)
{
    char data_t[5] = "";
    int32_t data;
    data = config_get_signal();
    sprintf(data_t, "%d", data);
    if(language == CHINESE)
    {
        config_resp("*信号值[dbm]:", data_t , handler);
    }
    else
    {
        config_resp("*Signal value[dbm]:", data_t , handler);
    }
}

//OTA开关
static void system_ack_ota_en(uint8_t language, uint32_t handler)
{
	if(config_get_ota_en())
	{
	    if(language == CHINESE)
		{
			config_resp("OTA(开启/关闭):", "开启" , handler);
		}
		else
		{
			config_resp("OTA(ON/OFF):", "ON" , handler);
		}
	}
	else
	{
	    if(language == CHINESE)
		{
			config_resp("OTA(开启/关闭):", "关闭" , handler);
		}
		else
		{
			config_resp("OTA(ON/OFF):", "OFF" , handler);
		}
	}
}
static void system_write_ota_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"ON")))
    {
        config_set_ota_en(1);	
    }
    else if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"OFF")))
    {
        config_set_ota_en(0);	
    }
}

//WIFI使能
static void system_ack_modbus_en(uint8_t language, uint32_t handler)
{
    if(language == CHINESE)
    {
        if(config_get_modbus_en())
        {
            config_resp("MODBUS(开启/关闭):", "开启" , handler);
        }
        else
        {
            config_resp("MODBUS(开启/关闭):", "关闭" , handler);
        }
    }
    else
    {
        if(config_get_modbus_en())
        {
            config_resp("MODBUS(ON/OFF):", "ON" , handler);
        }
        else
        {
            config_resp("MODBUS(ON/OFF):", "OFF" , handler);
        }
    }
}
static void system_write_modbus_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"ON")))
    {
        config_set_modbus_en(1);	
        wireless_info.tcp_modbus_state = 1;
    }
    else if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"OFF")))
    {
        config_set_modbus_en(0);	
        wireless_info.tcp_modbus_state = 2;
    }
}

//设备地址
static void system_ack_dev_addr(uint8_t language, uint32_t handler)
{
    char data_t[4] = "";
    uint8_t data;
    data = config_get_dev_addr();
    sprintf(data_t, "%d", data);
    if(language == CHINESE)
    {
        config_resp("设备地址:", data_t , handler);
    }
    else
    {
        config_resp("Device address:", data_t , handler);
    }
}
static void system_write_dev_addr(uint8_t *buf)
{
    uint32_t data;
    data = atof((char *)buf);
    if(data > 255)
    {
        data = 255;
    }
    else if(data < 1)
    {
        data = 1;	
    }
    config_set_dev_addr(data);	
}

//报警上报间隔
static void system_ack_alarm_interval_en(uint8_t language, uint32_t handler)
{
    if(language == CHINESE)
    {
        if(config_get_alarm_interval_en())
        {
            config_resp("报警上报间隔[2分钟](开启/关闭):", "开启" , handler);
        }
        else
        {
            config_resp("报警上报间隔[2分钟](开启/关闭):", "关闭" , handler);
        }
    }
    else
    {
        if(config_get_alarm_interval_en())
        {
            config_resp("Alarm Interval[2 min](ON/OFF):", "ON" , handler);
        }
        else
        {
            config_resp("Alarm Interval[2 min](ON/OFF):", "OFF" , handler);
        }
    }
}
static void system_write_alarm_interval_en(uint8_t *buf)
{
    if((strstr((const char*)buf,"开启"))||(strstr((const char*)buf,"ON")))
    {
        config_set_alarm_interval_en(1);	
    }
    else if((strstr((const char*)buf,"关闭"))||(strstr((const char*)buf,"OFF")))
    {
        config_set_alarm_interval_en(0);	
    }
}

static void device_get_internal(uint8_t get_id, uint32_t handler)
{
    uint8_t language;
    language = config_get_language();

    switch(get_id)
    {
        case CMD_DATA1_UP:         
        {	
			system_ack_data_up(0,language,handler);
        }break;
		
		case CMD_DATA2_UP:         
        {	
			system_ack_data_up(1,language,handler);
        }break;

        case CMD_DATA1_DOWN:         
        {	
			system_ack_data_down(0,language,handler);
        }break;
		
		case CMD_DATA2_DOWN:         
        {	
			system_ack_data_down(1,language,handler);
        }break;

        case CMD_AMEND_K1:         
        {	
			system_ack_amend_k(0,language,handler);
        }break;
		
		case CMD_AMEND_K2:         
        {	
			system_ack_amend_k(1,language,handler);
        }break;

        case CMD_AMEND_B1:         
        {	
			system_ack_amend_b(0,language,handler);
        }break;
		
		case CMD_AMEND_B2:         
        {	
			system_ack_amend_b(1,language,handler);
        }break;

        case CMD_DATA1_HC:         
        {	
			system_ack_data_hc(0,language,handler);
        }break;
		
		case CMD_DATA2_HC:         
        {	
			system_ack_data_hc(1,language,handler);
        }break;

        case CMD_DATA1_BD:         
        {	
			system_ack_data_bd(0,language,handler);
        }break;
		
		case CMD_DATA2_BD:         
        {	
			system_ack_data_bd(1,language,handler);
        }break;

        default : 
        {
			config_resp("", "",handler);  
        }break;
    }
}

static void device_set_internal(uint8_t *buf, uint32_t handler,uint8_t wr_id)
{
	uint8_t *temp_buf;

	temp_buf = &buf[7];

    switch(wr_id)
    {
        case CMD_DATA1_UP:         
        {	
			system_write_data_up(0,temp_buf);
        }break;
		
		case CMD_DATA2_UP:         
        {	
			system_write_data_up(1,temp_buf);
        }break;
			
		case CMD_DATA1_DOWN:         
        {	
			system_write_data_down(0,temp_buf);
        }break;
		
		case CMD_DATA2_DOWN:         
        {	
			system_write_data_down(1,temp_buf);
        }break;
		
		case CMD_AMEND_K1:         
        {	
			system_write_amend_k(0,temp_buf);
        }break;
		
		case CMD_AMEND_K2:         
        {	
			system_write_amend_k(1,temp_buf);
        }break;
			
		case CMD_AMEND_B1:         
        {	
			system_write_amend_b(0,temp_buf);
        }break;
		
		case CMD_AMEND_B2:         
        {	
			system_write_amend_b(1,temp_buf);
        }break;
		
		case CMD_DATA1_HC:         
        {	
			system_write_data_hc(0,temp_buf);
        }break;
		
		case CMD_DATA2_HC:         
        {	
			system_write_data_hc(1,temp_buf);
        }break;
		
		case CMD_DATA1_BD:         
        {	
			system_write_data_bd(0,temp_buf);
        }break;
		
		case CMD_DATA2_BD:         
        {	
			system_write_data_bd(1,temp_buf);
        }break;

        default : 
        {
			trace_enable(temp_buf[0] - 0X30);     
        }break;
    }
}

static void device_get_all_sensor(uint8_t get_id, uint32_t handler)
{
    uint8_t language;
    language = config_get_language();

    switch(get_id)
    {
        case CMD_ALL_DATA1_UP:         
        {	
			system_ack_data_up(0,language,handler);
        }break;
		
		case CMD_ALL_DATA2_UP:         
        {	
			system_ack_data_up(1,language,handler);
        }break;
		
        case CMD_ALL_DATA3_UP:         
        {	
			system_ack_data_up(2,language,handler);
        }break;
		
		case CMD_ALL_DATA4_UP:         
        {	
			system_ack_data_up(3,language,handler);
        }break;		

        case CMD_ALL_DATA1_DOWN:         
        {	
			system_ack_data_down(0,language,handler);
        }break;
		
		case CMD_ALL_DATA2_DOWN:         
        {	
			system_ack_data_down(1,language,handler);
        }break;
		
		case CMD_ALL_DATA3_DOWN:         
        {	
			system_ack_data_down(2,language,handler);
        }break;
		
		case CMD_ALL_DATA4_DOWN:         
        {	
			system_ack_data_down(3,language,handler);
        }break;

        case CMD_ALL_AMEND_K1:         
        {	
			system_ack_amend_k(0,language,handler);
        }break;
		
		case CMD_ALL_AMEND_K2:         
        {	
			system_ack_amend_k(1,language,handler);
        }break;
		
		case CMD_ALL_AMEND_K3:         
        {	
			system_ack_amend_k(2,language,handler);
        }break;
		
		case CMD_ALL_AMEND_K4:         
        {	
			system_ack_amend_k(3,language,handler);
        }break;

        case CMD_ALL_AMEND_B1:         
        {	
			system_ack_amend_b(0,language,handler);
        }break;
		
		case CMD_ALL_AMEND_B2:         
        {	
			system_ack_amend_b(1,language,handler);
        }break;

        case CMD_ALL_AMEND_B3:         
        {	
			system_ack_amend_b(2,language,handler);
        }break;
		
		case CMD_ALL_AMEND_B4:         
        {	
			system_ack_amend_b(3,language,handler);
        }break;
		
        case CMD_ALL_DATA1_HC:         
        {	
			system_ack_data_hc(0,language,handler);
        }break;
		
		case CMD_ALL_DATA2_HC:         
        {	
			system_ack_data_hc(1,language,handler);
        }break;
		
        case CMD_ALL_DATA3_HC:         
        {	
			system_ack_data_hc(2,language,handler);
        }break;
		
		case CMD_ALL_DATA4_HC:         
        {	
			system_ack_data_hc(3,language,handler);
        }break;
		
        case CMD_ALL_DATA1_BD:         
        {	
			system_ack_data_bd(0,language,handler);
        }break;
		
		case CMD_ALL_DATA2_BD:         
        {	
			system_ack_data_bd(1,language,handler);
        }break;

        case CMD_ALL_DATA3_BD:         
        {	
			system_ack_data_bd(2,language,handler);
        }break;
		
		case CMD_ALL_DATA4_BD:         
        {	
			system_ack_data_bd(3,language,handler);
        }break;
		
        default : 
        {
			config_resp("", "",handler);  
        }break;
    }
}

static void device_set_all_sensor(uint8_t *buf, uint32_t handler,uint8_t wr_id)
{
	uint8_t *temp_buf;

	temp_buf = &buf[7];

    switch(wr_id)
    {
        case CMD_ALL_DATA1_UP:         
        {	
			system_write_data_up(0,temp_buf);
        }break;
		
		case CMD_ALL_DATA2_UP:         
        {	
			system_write_data_up(1,temp_buf);
        }break;
		
        case CMD_ALL_DATA3_UP:         
        {	
			system_write_data_up(2,temp_buf);
        }break;
		
		case CMD_ALL_DATA4_UP:         
        {	
			system_write_data_up(3,temp_buf);
        }break;		
			
		case CMD_ALL_DATA1_DOWN:         
        {	
			system_write_data_down(0,temp_buf);
        }break;
		
		case CMD_ALL_DATA2_DOWN:         
        {	
			system_write_data_down(1,temp_buf);
        }break;
		
		case CMD_ALL_DATA3_DOWN:         
        {	
			system_write_data_down(2,temp_buf);
        }break;
		
		case CMD_ALL_DATA4_DOWN:         
        {	
			system_write_data_down(3,temp_buf);
        }break;		
		
		case CMD_ALL_AMEND_K1:         
        {	
			system_write_amend_k(0,temp_buf);
        }break;
		
		case CMD_ALL_AMEND_K2:         
        {	
			system_write_amend_k(1,temp_buf);
        }break;
		
		case CMD_ALL_AMEND_K3:         
        {	
			system_write_amend_k(2,temp_buf);
        }break;
		
		case CMD_ALL_AMEND_K4:         
        {	
			system_write_amend_k(3,temp_buf);
        }break;		
			
		case CMD_ALL_AMEND_B1:         
        {	
			system_write_amend_b(0,temp_buf);
        }break;
		
		case CMD_ALL_AMEND_B2:         
        {	
			system_write_amend_b(1,temp_buf);
        }break;

		case CMD_ALL_AMEND_B3:         
        {	
			system_write_amend_b(2,temp_buf);
        }break;
		
		case CMD_ALL_AMEND_B4:         
        {	
			system_write_amend_b(3,temp_buf);
        }break;
		
		case CMD_ALL_DATA1_HC:         
        {	
			system_write_data_hc(0,temp_buf);
        }break;
		
		case CMD_ALL_DATA2_HC:         
        {	
			system_write_data_hc(1,temp_buf);
        }break;
		
		case CMD_ALL_DATA3_HC:         
        {	
			system_write_data_hc(2,temp_buf);
        }break;
		
		case CMD_ALL_DATA4_HC:         
        {	
			system_write_data_hc(3,temp_buf);
        }break;		
		
		case CMD_ALL_DATA1_BD:         
        {	
			system_write_data_bd(0,temp_buf);
        }break;
		
		case CMD_ALL_DATA2_BD:         
        {	
			system_write_data_bd(1,temp_buf);
        }break;

		case CMD_ALL_DATA3_BD:         
        {	
			system_write_data_bd(2,temp_buf);
        }break;
		
		case CMD_ALL_DATA4_BD:         
        {	
			system_write_data_bd(3,temp_buf);
        }break;		
		
        default : 
        {
			trace_enable(temp_buf[0] - 0X30);     
        }break;
    }
}

/**
 * @brief: 获取参数
 * @param {type} None
 * @return: None
 */
static void device_get_param(uint8_t get_id,uint32_t handler)
{
    uint8_t language;
    language = config_get_language();
	switch(get_id)
	{
		case CMD_SAVE :
        {
            if(language == CHINESE)
            {
                config_resp("保存参数","",handler); 
            }
            else
            {
                config_resp("Save parameter","",handler);
            }
        }break;
        
		case CMD_RESART:         
        {
            if(language == CHINESE)
            {
                config_resp("重启设备","",handler); 
            }
            else
            {
                config_resp("Restart","",handler);
            }
        }break;

		case CMD_CLEAR_ALL:
        {
            if(language == CHINESE)
            {
                config_resp("清除历史数据","",handler); 
            }
            else
            {
                config_resp("Clear history","",handler);
            }
        }break;

        case CMD_SET_USE:
        {
            if(language == CHINESE)
            {
                config_resp("*设置参数说明:","带*配置为只读选项",handler); 
            }
            else
            {
                config_resp("Config User:","With * as read only",handler);
            }
        }break;

		case CMD_DEV_MODEL:         
        {
            if(language == CHINESE)
            {
                config_resp("*设备型号:","TP500",handler); 
            }
            else
            {
                config_resp("Device Model:","TP500",handler);
            }
        }break;

		case CMD_HARD_VER:         
        {
            if(language == CHINESE)
            {
                config_resp("*硬件版本:","TP500x.5_V03_ljy_260323",handler); 
            }
            else
            {
                config_resp("Hard Ver:","TP500x.5_V03_ljy_260323",handler);
            }
        }break;

        case CMD_SOFT_VER:         
        {
			char buf[50] = "TP500Vx.5_YY_";
			char date_s[10];
			uint32_t date = config_get_soft_date();
			sprintf(date_s,"%d",date);
			strcat(buf,date_s);

            if(language == CHINESE)
            {
                config_resp("*软件版本:",buf,handler); 
            }
            else
            {
                config_resp("Soft Ver:",buf,handler);
            }
        }break;

        case CMD_DEV_SN:         
        {
            system_ack_sn(language,handler);
        }break;

        case CMD_COMM_MODULE:         
        {
            system_ack_trans_model_chose(language,handler);
        }break;

        case CMD_BASE_SET:         
        {
            if(language == CHINESE)
            {
                config_resp("*设置参数说明:","基础参数设置",handler); 
            }
            else
            {
                config_resp("Config User:","Base Config",handler);
            }
        }break;

        case CMD_LANGUAGE:         
        {
            if(language == CHINESE)
            {
                config_resp("语言(英文/中文):","中文", handler);               
            }
            else
            {
                config_resp("Langauge(English/Chinese):","English", handler);
            }
        }break;
		
		case CMD_RESET_FACTOR:
		{
		    if(language == CHINESE)
            {
                config_resp("恢复出厂设置","", handler);               
            }
            else
            {
                config_resp("Factory reset","", handler);
            }
		}break;

        case CMD_DEV_ADDR:
        {
            system_ack_dev_addr(language,handler);
        }break;

       /*case CMD_BAT_RESET:         
        {
            if(language == CHINESE)
            {
                config_resp("复位电池","", handler);               
            }
            else
            {
                config_resp("Reset battery","", handler);
            }
        }break;*/

        case CMD_UP_TYPE:         
        {
            system_ack_up_type(language,handler);
        }break;   

        case CMD_UP_TIME:         
        {
            system_ack_updata_time(language,handler);
        }break;

        case CMD_COLLECT_TIME:         
        {
            system_ack_collect_time(language,handler);
        }break;

        case CMD_BACK_LIGHT_TIME:         
        {
            system_ack_back_light_time(language,handler);
        }break;

        case CMD_AUTO_SWITCH_SHOW:         
        {
            system_ack_auto_switch_show(language,handler);
        }break;
		
        case CMD_VBAT_VALUE:         
        {
            system_ack_vbat_value(language,handler);
        }break;
		
        case CMD_NET_SET:         
        {
            if(language == CHINESE)
            {
                config_resp("*设置参数说明:","网络参数设置",handler); 
            }
            else
            {
                config_resp("Config User:","NET Config",handler);
            }
        }break;
		
	    case CMD_CCID:         
        {
			config_resp("*ICCID:",wireless_info.ccid, handler); 
        }break;	
		
	    case CMD_IMEI:         
        {
			config_resp("*IMEI:",wireless_info.imei, handler); 
        }break;			

        case CMD_SIGNAL:
        {
            system_ack_signal(language,handler);
        }break;
		
		case CMD_OTA_EN:
        {
            system_ack_ota_en(language,handler);
        }break;
		
        case CMD_WIFI_SSID:         
        {
            system_ack_wifi_ssid(language,handler);
        }break;

        case CMD_WIFI_PAWD:         
        {
            system_ack_wifi_pawd(language,handler);
        }break;

        case CMD_DHCP_EN:         
        {
            system_ack_dhcp_en(language,handler);
        }break;

        case CMD_WIFI_IP:         
        {
            system_ack_wifi_ip(language,handler);
        }break;

        case CMD_WIFI_MASK:         
        {
            system_ack_wifi_mask(language,handler);
        }break;

        case CMD_WIFI_GW:         
        {
            system_ack_wifi_gw(language,handler);
        }break;

        case CMD_WIFI_MAC:         
        {
            system_ack_wifi_mac(language,handler);
        }break;

        case CMD_GPS_MODE:         
        {
            system_ack_gps_mode(language,handler);
        }break;

        case CMD_NET_MODE:         
        {
            system_ack_net_mode(language,handler);
        }break;

        case CMD_MODBUS_EN:         
        {
            system_ack_modbus_en(language,handler);
        }break;

        case CMD_MQTT_CLIENT_ID:         
        {
            system_ack_mqtt_client_id(language,handler);
        }break;

        case CMD_MQTT_USER_NAME:         
        {
            system_ack_mqtt_user_name(language,handler);
        }break;

        case CMD_MQTT_USER_PAWD:         
        {
            system_ack_mqtt_user_pawd(language,handler);
        }break;

        case CMD_MQTT_POST_TOPIC:         
        {
            system_ack_mqtt_post_topic(language,handler);
        }break;

        case CMD_MQTT_SUBS_TOPIC:         
        {
            system_ack_mqtt_subs_topic(language,handler);
        }break;

        case CMD_SERVER_IP:         
        {
            system_ack_server_ip(language,handler);
        }break;

        case CMD_SERVER_PORT:         
        {
            system_ack_server_port(language,handler);
        }break;

        case CMD_APN:         
        {
            system_ack_apn(language,handler);
        }break;

        case CMD_APN_USER_NAME:         
        {
            system_ack_apn_name(language,handler);
        }break;

        case CMD_APN_USER_PWD:         
        {
            system_ack_apn_pawd(language,handler);
        }break;

        case CMD_SENSOR_SET:         
        {
            if(language == CHINESE)
            {
                config_resp("*设置参数说明:","传感器参数设置",handler); 
            }
            else
            {
                config_resp("Config User:","Sensor Config",handler);
            }
        }break;
		
		case CMD_INTERNAL_EN:
		{
			system_ack_internal_en(language,handler);
		}break;

        case CMD_EXTERNAL_EN:         
        {
            system_ack_external_en(language,handler);
        }break;

        case CMD_INTERNAL_MODEL:         
        {
            system_ack_internal_model(language,handler);
        }break;

        case CMD_EXTERNAL_MODEL:         
        {
            system_ack_external_model(language,handler);
        }break;
		
		case CMD_EXTERNAL_LONG_POWER:         
        {
			system_ack_external_long_power(language,handler);
        }break;

        case CMD_ALARM_EN:         
        {
            system_ack_alarm_en(language,handler);
        }break;

         case CMD_ALARM_INTERVAL_EN:         
        {
            system_ack_alarm_interval_en(language,handler);
        }break;       

        case CMD_TEM_UNIT:         
        {
            system_ack_tem_unit(language,handler);
        }break;

        default : 
        {
            if(config_get_internal_en()&&(config_get_external_en() == 0))
            {
                device_get_internal(get_id - CMD_COM_END,handler);
            }
            else if(config_get_external_en())
            {
                device_get_all_sensor(get_id - CMD_COM_END,handler);
            }
            else 
            {
                config_resp("", "",handler);
            }
        }break;         
    }
}

/**
 * @brief: 参数设置
 * @param {type} None
 * @return: None
 */
static void device_set_param(uint8_t *buf, uint32_t handler)
{
	uint8_t *temp_buf, wr_id;

	temp_buf = &buf[7];
	wr_id = buf[3];

    switch(wr_id)
    {
        case CMD_SAVE :
        {
            device_set_updata_cloud(1);
            
            system_write_setting();
            if(wireless_info.tcp_modbus_state == 1)
            {
                wireless_info.tcp_modbus_state = 0;
                task_post_event_iot(START_TCP_MODBUS);	
            }
            else if(wireless_info.tcp_modbus_state == 2)
            {
                wireless_info.tcp_modbus_state = 0;
                task_post_event_iot(STOP_TCP_MODBUS);	
            }   
        }break;
        
		case CMD_RESART:         
        {
            device_get_param(buf[3],handler);
            vTaskDelay(1000);
			portDISABLE_INTERRUPTS();
            FL_RMU_SetSoftReset(RMU);
        }break;

		case CMD_CLEAR_ALL:
        {
            task_flash_post_event(HIST_CLEAR);
        }break;

        case CMD_SET_USE:
        {
            ;
        }break;

		case CMD_DEV_MODEL:         
        {
            ;
        }break;

		case CMD_HARD_VER:         
        {
            ;
        }break;

        case CMD_SOFT_VER:         
        {
            ;
        }break;

        case CMD_DEV_SN:         
        {
            ;
        }break;

        case CMD_COMM_MODULE:         
        {
            system_write_trans_model_chose(temp_buf);
        }break;

        case CMD_BASE_SET:         
        {
            ;
        }break;

        case CMD_LANGUAGE:         
        {
            system_write_language(temp_buf);
        }break;
		
		case CMD_RESET_FACTOR:
		{
			system_write_factory_reset(temp_buf);
		}break;

        case CMD_DEV_ADDR:         
        {
            system_write_dev_addr(temp_buf);
        }break;

        /*case CMD_BAT_RESET:         
        {
            system_write_bat_reset();
        }break;*/

        case CMD_UP_TYPE:         
        {
            system_write_up_type(temp_buf);
        }break;      

        case CMD_UP_TIME:         
        {
            system_write_updata_time(temp_buf);
        }break;

        case CMD_COLLECT_TIME:         
        {
            system_write_collect_time(temp_buf);
        }break;

		case CMD_BACK_LIGHT_TIME:
		{
			system_write_back_light_time(temp_buf);
		}break;

        case CMD_AUTO_SWITCH_SHOW:
		{
			system_write_auto_switch_show(temp_buf);
		}break;
		
       /* case CMD_VBAT_BFB:         
        {
            ;
        }break;*/

        case CMD_VBAT_VALUE:         
        {
            ;
        }break;

        case CMD_NET_SET:         
        {
            ;
        }break;
		
	    case CMD_CCID:         
        {
			;
        }break;	
		
	    case CMD_IMEI:         
        {
			;
        }break;		

        case CMD_SIGNAL:         
        {
            ;
        }break;
		
        case CMD_OTA_EN:         
        {    
            system_write_ota_en(temp_buf);
        }break;		
		
        case CMD_WIFI_SSID:         
        {    
            system_write_wifi_ssid(temp_buf);
        }break;

        case CMD_WIFI_PAWD:         
        {
            system_write_wifi_pawd(temp_buf);
        }break;

        case CMD_DHCP_EN:         
        {
            system_write_dhcp_en(temp_buf);
        }break;

        case CMD_WIFI_IP:         
        {
            if(config_get_dhcp_en() == 0)
            {
                system_write_wifi_ip(temp_buf);
            }
        }break;

        case CMD_WIFI_MASK:         
        {
            if(config_get_dhcp_en() == 0)
            {
                system_write_wifi_mask(temp_buf);
            }
        }break;

        case CMD_WIFI_GW:         
        {
            if(config_get_dhcp_en() == 0)
            {
                system_write_wifi_gw(temp_buf);
            } 
        }break;

        case CMD_WIFI_MAC:         
        {
			system_write_wifi_mac(temp_buf);         
        }break;

        case CMD_GPS_MODE:         
        {
            system_write_gps_mode(temp_buf);
        }break;

        case CMD_NET_MODE:         
        {
            system_write_net_mode(temp_buf);
        }break;

        case CMD_MODBUS_EN:         
        {
            if(config_get_net_mode() == TCP_MODE)
            {
                system_write_modbus_en(temp_buf);
            }
        }break;

        case CMD_MQTT_CLIENT_ID:         
        {
            system_write_mqtt_client_id(temp_buf);
        }break;

        case CMD_MQTT_USER_NAME:         
        {
            system_write_mqtt_user_name(temp_buf);
        }break;

        case CMD_MQTT_USER_PAWD:         
        {
            system_write_mqtt_user_pawd(temp_buf);
        }break;

        case CMD_MQTT_POST_TOPIC:         
        {
            system_write_mqtt_post_topic(temp_buf);
        }break;

        case CMD_MQTT_SUBS_TOPIC:         
        {
            system_write_mqtt_subs_topic(temp_buf);
        }break;

        case CMD_SERVER_IP:         
        {
            system_write_server_ip(temp_buf);
        }break;

        case CMD_SERVER_PORT:         
        {
            system_write_server_port(temp_buf);
        }break;

        case CMD_APN:         
        {
            system_write_apn(temp_buf);
        }break;

        case CMD_APN_USER_NAME:         
        {
            system_write_apn_name(temp_buf);
        }break;

        case CMD_APN_USER_PWD:         
        {
            system_write_apn_pawd(temp_buf);
        }break;

        case CMD_SENSOR_SET:         
        {
            ;
        }break;

        case CMD_INTERNAL_EN:         
        {
            system_write_internal_en(temp_buf);
        }break;
		
		 case CMD_EXTERNAL_EN:         
        {
            system_write_external_en(temp_buf);
        }break;

        case CMD_INTERNAL_MODEL:         
        {
            system_write_internal_model(temp_buf);
        }break;

        case CMD_EXTERNAL_MODEL:         
        {
            system_write_external_model(temp_buf);
        }break;
		
		case CMD_EXTERNAL_LONG_POWER:         
        {
            system_write_external_long_power(temp_buf);
        }break;

        case CMD_ALARM_EN:         
        {
            system_write_alarm_en(temp_buf);
        }break;

         case CMD_ALARM_INTERVAL_EN:         
        {
            system_write_alarm_interval_en(temp_buf);
        }break;           

        case CMD_TEM_UNIT:         
        {
            system_write_tem_unit(temp_buf);
        }break;
		
        default : 
        {
            if(config_get_internal_en()&&(config_get_external_en() == 0))
            {
                device_set_internal(buf,handler,wr_id - CMD_COM_END);
            }
            else if(config_get_external_en())
            {
                device_set_all_sensor(buf,handler,wr_id - CMD_COM_END);
            }
            else 
            {
                trace_enable(temp_buf[0] - 0X30);
            }
        }break;
    }   
    device_get_param(buf[3],handler);
}

/**
 * @brief: 设备配置处理
 * @param {type} None
 * @return: None
 */
void device_param(uint8_t *buf,uint32_t handler)
{
    if(buf[1] == 0x03)
    {
        device_get_param(buf[3],handler);
    }
    else if(buf[1] == 0x0f)
    {
        device_set_param(buf,handler);
    }
}

//电池电量
void device_nbvbat_set(uint32_t vbat)
{	
	sd3078_ram_set(user_ram1,vbat);
}
uint32_t device_nbvbat_get(void)
{
	return  sd3078_ram_get(user_ram1);
}

//电池复位次数
void device_restart_num_set(uint32_t restart_num)
{
	sd3078_ram_set(user_ram7,restart_num);
}
uint32_t device_restart_num_get(void)
{
	return  sd3078_ram_get(user_ram7);
}

//RTC日期
static void system_write_rtc_date(uint8_t *buf)
{
    rtc_t data;
	data.year  = buf[7];
	data.month = buf[8];
	data.day   = buf[9];
	data.hour  = buf[10];	
	data.min   = buf[11];
	data.sec   = buf[12];
	data.week  = buf[13];

    RTC_WriteDate(&data);
}

static void system_config_task(void *param)
{
    uint8_t *uart_buff;
    uint16_t rx_len = 0;
    char *cap;
    uint8_t *sn;
    uint8_t tx[30];
    uint32_t *sn_32 = NULL;
    uart_buff = pvPortMalloc(64);
    while(1)
    {
        rx_len = uart_recv_data(USB_UART, uart_buff, 64, 1000);
        if(rx_len)
        {
            if(uart_buff[0x02] == 0x4B)
            {
                device_param(uart_buff,0);
            }
            else if(uart_buff[2] == 0x4d)
            {
                config_resp("OK", "",0);
                system_write_rtc_date(uart_buff);
            }
            else if(uart_buff[0] == 0X7E && rx_len >= 15)   // 读取历史数据 
            {
                if(uart_buff[2] == 0X02)
                {
                    hist_set_data_read(uart_buff[10], uart_buff[11], uart_buff[12], uart_buff[13], uart_buff[14], uart_buff[15]);
                }
            }
            else if((uart_buff[3] == 0X2F  || uart_buff[3] == 0X29)    // 更新程序 
                    && calc_crc16(uart_buff, rx_len - 3) == (((uint16_t )uart_buff[rx_len - 2] << 8) | uart_buff[rx_len - 3]))
            {
				FL_FLASH_PageErase(FLASH, APP_UPDATE_FLAG);    //擦除页   
				FL_RMU_SetSoftReset(RMU);
            }
            else if(strstr((char *)uart_buff,"write"))
			{
				cap = strstr((char *)uart_buff,"sn");
                if(cap != NULL)
                {
                    cap += 5;
                    config_set_sn((uint8_t *)cap);
                    sn = config_get_sn();
                    sprintf((char *)tx,"{\"sn\":\"%s\"}\r\n",sn);    
                    uart_transmit_buf(USB_UART, (uint8_t *)tx, strlen((const char *)tx));  
                    sn_32 = (uint32_t *)sn;
                    flash_write(MODEL_SN_ADDRESS,sn_32,4,1);
                }
			}
        }
    }
}

void system_config_create_task(void *params, uint8_t priority)
{
    xTaskCreate(system_config_task, "config", 256, params, priority, &device_config_handler);
}


