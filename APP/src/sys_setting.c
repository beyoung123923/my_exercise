#include "sys_setting.h"
#include "string.h"
#include "tool.h"
#include "stdlib.h"
#include "sensor.h"
#include "stdio.h"
#include "string.h"
#include "usart.h"
#include "main.h"
#include "flash.h"

sys_settings_t sys_setting;
uint32_t net_err_bit;

void system_write_setting(void)
{
	uint8_t write_count = 0;
	uint16_t crc = 0;
	crc = calc_crc16((uint8_t *)&sys_setting, sizeof(sys_settings_t) - 4);
	if(sys_setting.crc != crc)
	{
		sys_setting.crc = crc;

		for(write_count = 0; write_count<3; write_count++)
		{
			flash_write(DEVICE_INFO, (uint32_t *)&sys_setting, sizeof(sys_settings_t)/4,0);	
		}
	}
}

void system_read_setting(void)
{
    uint8_t set_state = 0;
	uint16_t crc = 0;
	uint32_t rbuf[2];
	
	flash_read(DEVICE_INFO,(uint32_t *)&sys_setting,sizeof(sys_settings_t)/4);

	crc = calc_crc16((uint8_t *)&sys_setting, sizeof(sys_settings_t) - 4);
	if(sys_setting.crc != crc)
	{
		system_restore_default();	
		system_write_setting();

		device_pending_num_set(0);
		device_perv_addr_set(0);
		if((device_pending_num_set(0) == FALSE)||(device_perv_addr_set(0) == FALSE))
		{
			trace_log("清除断点信息失败\n");
		}
	}
	else
	{
		flash_read(APP_UPDATE_FLAG,rbuf, 2);
        if(sys_setting.dev_addr < 1)
        {
            sys_setting.dev_addr = 0x01;
            set_state = 1;
        }

        if(sys_setting.updata_time < 1)
        {
            sys_setting.updata_time = 1;
            set_state = 1;
        }

        if(sys_setting.collect_time < 1)
        {
            sys_setting.collect_time = 1;
            set_state = 1;
        }

        /* 消毒:NaN等非法校准系数可能已被合法写入闪存(CRC校验通过),开机强制复位 */
        for(uint8_t amend_i = 0; amend_i < 4; amend_i++)
        {
            if(!system_amend_k_legal(sys_setting.amend_k[amend_i]))
            {
                sys_setting.amend_k[amend_i] = 1;
                set_state = 1;
            }
            if(!system_amend_b_legal(sys_setting.amend_b[amend_i]))
            {
                sys_setting.amend_b[amend_i] = 0;
                set_state = 1;
            }
        }
        if(set_state == 1)
        {
            system_write_setting();
        }

		if(rbuf[1] != 0xffffffff)
		{
			sys_setting.soft_date = rbuf[1];
		}
		
		flash_read(MODEL_SN_ADDRESS,(uint32_t *)&sys_setting.model_sn,4);
		if((sys_setting.model_sn[0] < 0x30) || (sys_setting.model_sn[0] > 0x39))
		{
			strcpy((char *)&sys_setting.model_sn, "1234567890123");
			flash_write(MODEL_SN_ADDRESS, (uint32_t *)sys_setting.model_sn, 4,1);	
		}
	}
}

void system_format_cloud_config(char *buf)
{
    if(sys_setting.language == CHINESE)
    {
		sprintf(buf,"{\"params\":{\"记录间隔(分)\":\"%d\","		
                            "\"内部传感器\":\"%d\","
                            "\"外置传感器\":\"%d\","
                            "\"通道1斜率\":\"%.2f\","
                            "\"通道2斜率\":\"%.2f\","
                            "\"通道3斜率\":\"%.2f\","
                            "\"通道4斜率\":\"%.2f\","
                            "\"通道1修正\":\"%.2f\","
                            "\"通道2修正\":\"%.2f\","
                            "\"通道3修正\":\"%.2f\","
                            "\"通道4修正\":\"%.2f\","                           
                            "\"报警开关\":\"%d\","
                            "\"通道1上限\":\"%.2f\","
                            "\"通道2上限\":\"%.2f\","
                            "\"通道3上限\":\"%.2f\","
                            "\"通道4上限\":\"%.2f\","
                            "\"通道1下限\":\"%.2f\","
                            "\"通道2下限\":\"%.2f\","
                            "\"通道3下限\":\"%.2f\","
                            "\"通道4下限\":\"%.2f\","
                            "\"通道1回差\":\"%.2f\","
                            "\"通道2回差\":\"%.2f\","
                            "\"通道3回差\":\"%.2f\","
                            "\"通道4回差\":\"%.2f\","
                            "\"通道1波动\":\"%.2f\","
                            "\"通道2波动\":\"%.2f\","
                            "\"通道3波动\":\"%.2f\","
                            "\"通道4波动\":\"%.2f\"}}",
                            sys_setting.updata_time,
                            sys_setting.internal_en,
                            sys_setting.external_en,
                            sys_setting.amend_k[SENSOR_NUM_1],
                            sys_setting.amend_k[SENSOR_NUM_2],
                            sys_setting.amend_k[SENSOR_NUM_3],
                            sys_setting.amend_k[SENSOR_NUM_4],
                            sys_setting.amend_b[SENSOR_NUM_1],
                            sys_setting.amend_b[SENSOR_NUM_2],
                            sys_setting.amend_b[SENSOR_NUM_3],
                            sys_setting.amend_b[SENSOR_NUM_4],
                            sys_setting.alarm_en,
                            sys_setting.up_data[SENSOR_NUM_1],
                            sys_setting.up_data[SENSOR_NUM_2],
                            sys_setting.up_data[SENSOR_NUM_3],
                            sys_setting.up_data[SENSOR_NUM_4],
                            sys_setting.down_data[SENSOR_NUM_1],
                            sys_setting.down_data[SENSOR_NUM_2],
                            sys_setting.down_data[SENSOR_NUM_3],
                            sys_setting.down_data[SENSOR_NUM_4],
                            sys_setting.data_hc[SENSOR_NUM_1],
                            sys_setting.data_hc[SENSOR_NUM_2],
                            sys_setting.data_hc[SENSOR_NUM_3],
                            sys_setting.data_hc[SENSOR_NUM_4],
                            sys_setting.data_bd[SENSOR_NUM_1],
                            sys_setting.data_bd[SENSOR_NUM_2],
                            sys_setting.data_bd[SENSOR_NUM_3],
                            sys_setting.data_bd[SENSOR_NUM_4]                          
                            );
    }
    else 
    {
        sprintf(buf,"{\"params\":{\"interval(min)\":\"%d\","	
                            "\"internal\":\"%d\","
                            "\"external\":\"%d\","
                            "\"CH1correct_k\":\"%.2f\","
                            "\"CH2correct_k\":\"%.2f\","
                            "\"CH3correct_k\":\"%.2f\","
                            "\"CH4correct_k\":\"%.2f\","
                            "\"CH1correct_b\":\"%.2f\","
                            "\"CH2correct_b\":\"%.2f\","
                            "\"CH3correct_b\":\"%.2f\","
                            "\"CH4correct_b\":\"%.2f\","
                            "\"ALM\":\"%d\","
                            "\"CH1Upper\":\"%.2f\","
                            "\"CH2Upper\":\"%.2f\","
                            "\"CH3Upper\":\"%.2f\","
                            "\"CH4Upper\":\"%.2f\","
                            "\"CH1Lower\":\"%.2f\","
                            "\"CH2Lower\":\"%.2f\","
                            "\"CH3Lower\":\"%.2f\","
                            "\"CH4Lower\":\"%.2f\","
                            "\"CH1RD\":\"%.2f\","
                            "\"CH2RD\":\"%.2f\","
                            "\"CH3RD\":\"%.2f\","
                            "\"CH4RD\":\"%.2f\","
                            "\"CH1Undulate\":\"%.2f\","
                            "\"CH2Undulate\":\"%.2f\","
                            "\"CH3Undulate\":\"%.2f\","
                            "\"CH4Undulate\":\"%.2f\"}}",
                            sys_setting.updata_time,
                            sys_setting.internal_en,
                            sys_setting.external_en,
                            sys_setting.amend_k[SENSOR_NUM_1],
                            sys_setting.amend_k[SENSOR_NUM_2],
                            sys_setting.amend_k[SENSOR_NUM_3],
                            sys_setting.amend_k[SENSOR_NUM_4],
                            sys_setting.amend_b[SENSOR_NUM_1],
                            sys_setting.amend_b[SENSOR_NUM_2],
                            sys_setting.amend_b[SENSOR_NUM_3],
                            sys_setting.amend_b[SENSOR_NUM_4],
                            sys_setting.alarm_en,
                            sys_setting.up_data[SENSOR_NUM_1],
                            sys_setting.up_data[SENSOR_NUM_2],
                            sys_setting.up_data[SENSOR_NUM_3],
                            sys_setting.up_data[SENSOR_NUM_4],
                            sys_setting.down_data[SENSOR_NUM_1],
                            sys_setting.down_data[SENSOR_NUM_2],
                            sys_setting.down_data[SENSOR_NUM_3],
                            sys_setting.down_data[SENSOR_NUM_4],
                            sys_setting.data_hc[SENSOR_NUM_1],
                            sys_setting.data_hc[SENSOR_NUM_2],
                            sys_setting.data_hc[SENSOR_NUM_3],
                            sys_setting.data_hc[SENSOR_NUM_4],
                            sys_setting.data_bd[SENSOR_NUM_1],
                            sys_setting.data_bd[SENSOR_NUM_2],
                            sys_setting.data_bd[SENSOR_NUM_3],
                            sys_setting.data_bd[SENSOR_NUM_4]
                            );
    }
	//trace_log( );
    //TRACE_LOG("WIFI", "%s", buf);
}

uint8_t system_set_cloud_config(uint8_t *buf)
{
    char *param;
    int int_value = 0;
	uint8_t index = 0;
    uint8_t change = 0;
    float   f_value = 0;

    if(strlen((char *)buf) == 0)
    {
        return FALSE;
    }
   // TRACE_LOG("mqtt", "%s", buf);
    param = strstr((char *)buf, "记录间隔");
    if(param == NULL)
        param = strstr((char *)buf, "interval");
    if(param != NULL)
    {
        while(*param++ != '"');
        while(*param++ != '"');
        int_value = atoi(param);

        if(int_value != sys_setting.updata_time)
        {
            if((int_value <= 1440)&&(int_value > 0))
            {
                change = 1;		
                sys_setting.updata_time = int_value;
            }
        }			
    }
    param = strstr((char *)buf, "内部传感器");
    if(param == NULL)
        param = strstr((char *)buf, "internal");
    if(param != NULL)
    {
        while(*param++ != '"');
        while(*param++ != '"');
        int_value = atoi(param);
        if(sys_setting.internal_en != int_value)
        {
            if((int_value == 1)||(int_value == 0))
            {
                change = 1;
                sys_setting.internal_en = int_value;
            }
        }
    }
    param = strstr((char *)buf, "外置传感器");
    if(param == NULL)
        param = strstr((char *)buf, "external");
    if(param != NULL)
    {
        while(*param++ != '"');
        while(*param++ != '"');
        int_value = atoi(param);
        if(sys_setting.external_en != int_value)
        {
            if((int_value == 1)||(int_value == 0))
            {
                change = 1;
                sys_setting.external_en = int_value;
            }
        }
    }
    param = strstr((char *)buf, "报警开关");
    if(param == NULL)
        param = strstr((char *)buf, "ALM");
    if(param != NULL)
    {
        while(*param++ != '"');
        while(*param++ != '"');
        int_value = atoi(param);
        if(sys_setting.alarm_en  != int_value)
        {
            if((int_value == 1)||(int_value == 0))
            {
                change = 1;
                sys_setting.alarm_en = int_value;
            }
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "斜率");
        if(param == NULL)
            param = strstr((char *)buf, "k");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(system_amend_k_legal(f_value) && (f_value != sys_setting.amend_k[index]))
            {
                sys_setting.amend_k[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "修正");
        if(param == NULL)
            param = strstr((char *)buf, "correct");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(system_amend_b_legal(f_value) && (f_value != sys_setting.amend_b[index]))
            {
                sys_setting.amend_b[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "上限");
        if(param == NULL)
            param = strstr((char *)buf, "Upper");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(f_value != sys_setting.up_data[index])
            {
                sys_setting.up_data[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "下限");
        if(param == NULL)
            param = strstr((char *)buf, "Lower");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(f_value != sys_setting.down_data[index])
            {
                sys_setting.down_data[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "回差");
        if(param == NULL)
            param = strstr((char *)buf, "RD");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(f_value != sys_setting.data_hc[index])
            {
                sys_setting.data_hc[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }
    param = (char *)buf;
    for(uint8_t i = 0; i < 4; i++)
    {
        param = strstr((char *)param, "波动");
        if(param == NULL)
            param = strstr((char *)buf, "Undulate");
        if(param != NULL)
        {
            --param;
            index = atoi(param) - 1;
            if(index > SENSOR_NUM_4)
            {
                break;
            }
            while(*param++ != '"');
            while(*param++ != '"');
            f_value = atof(param);
            if(f_value != sys_setting.data_bd[index])
            {
                sys_setting.data_bd[index] = f_value;
                change = 1;
            }
        }
        else 
        {
            break;
        }
    }    
    if(change == 1)
    {
        system_write_setting();
		trace_log("同步数据完成\r\n");
		return TRUE;
    }
	else
	{
		trace_log("同步数据无变化\r\n");
		return FALSE;
	}
}

const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug","Sep", "Oct", "Nov", "Dec"};

uint32_t Version_DATA(void)
{
  uint32_t data=0;
  char temp[] = __DATE__;     //Jan 12 2024
  uint8_t i;
  uint8_t month,day;
  uint16_t year;
  
  year = atoi(temp+7);
  *(temp+6) = 0;
  day = atoi(temp+4);
  *(temp+3) = 0;
  for(i=0;i<12;i++)
  {
    if(!strcmp(temp,months[i]))
    {
      month = i+1;
      break;
    }
  }
  data = year*10000+month*100+day;     //20240112
  
  return data;
}

void system_restore_default(void)
{
	uint32_t rbuf[2];
	memset(&sys_setting, 0, sizeof(sys_settings_t));
    sys_setting.language = 0;
	sys_setting.trans_model = 1;
    sys_setting.tem_unit = 0;
	sys_setting.internal_en = 1;
	sys_setting.external_en = 0;
	sys_setting.internal_model = 0;
	sys_setting.external_model = 0;
    sys_setting.dhcp_en = 1;
    sys_setting.net_mode = 1;
    sys_setting.gps_mode = 0;
	sys_setting.back_light_time = 2;
	sys_setting.alarm_en = 0;
    sys_setting.auto_switch_show = 0;
    sys_setting.ota_en = 1;
    sys_setting.dev_addr = 0x01;
    sys_setting.alarm_interval_en = 1;

    strcpy((char *)&sys_setting.wifi_ssid, "HUAWEI-0180");
    strcpy((char *)&sys_setting.wifi_pawd, "TP29558358");
	strcpy((char *)&sys_setting.mqtt_client_id, "MQTT_TEST");
    strcpy((char *)&sys_setting.mqtt_user_name, "MQTT");
    strcpy((char *)&sys_setting.mqtt_user_pawd, "MQTTPW");
    strcpy((char *)&sys_setting.mqtt_post_topic, "P10P1TTTV89DI60U");
    strcpy((char *)&sys_setting.mqtt_subs_topic, "P10P1TTTV89DI60U/+");

    strcpy((char *)&sys_setting.server_ip, "1883.dtuip.com");
    strcpy((char *)&sys_setting.server_port, "1883");

    sys_setting.collect_time = 6;              
    sys_setting.updata_time = 5;
    sys_setting.up_data[0] = 70;
    sys_setting.up_data[1] = 100;
    sys_setting.up_data[2] = 70;
    sys_setting.up_data[3] = 100;
    sys_setting.down_data[0] = -40;
    sys_setting.down_data[1] = 0;
    sys_setting.down_data[2] = -40;
    sys_setting.down_data[3] = 0;  
    sys_setting.amend_k[0] = 1;
    sys_setting.amend_k[1] = 1;
    sys_setting.amend_k[2] = 1;
    sys_setting.amend_k[3] = 1;
    sys_setting.amend_b[0] = 0;
    sys_setting.amend_b[1] = 0;
    sys_setting.amend_b[2] = 0;
    sys_setting.amend_b[3] = 0;
    sys_setting.data_hc[0] = 2;
    sys_setting.data_hc[1] = 2;
    sys_setting.data_hc[2] = 2;
    sys_setting.data_hc[3] = 2;
    sys_setting.data_bd[0] = 100;
    sys_setting.data_bd[1] = 100;
    sys_setting.data_bd[2] = 100;
    sys_setting.data_bd[3] = 100;

	flash_read(APP_UPDATE_FLAG,rbuf, 2);
	if(rbuf[1] != 0xffffffff)
	{
		sys_setting.soft_date = rbuf[1];
	}

	flash_read(MODEL_SN_ADDRESS,(uint32_t *)sys_setting.model_sn,4);
	if((sys_setting.model_sn[0] < 0x30) || (sys_setting.model_sn[0] > 0x39))
	{
		strcpy((char *)&sys_setting.model_sn, "1234567890123");
		flash_write(MODEL_SN_ADDRESS, (uint32_t *)sys_setting.model_sn, 4,1);	
	}
}

//语言
uint8_t config_get_language(void)
{
	return sys_setting.language;
}
void config_set_language(uint8_t data)
{
	sys_setting.language = data;
}

//通讯模组
uint8_t config_get_trans_model(void)
{
	return sys_setting.trans_model;
}
void config_set_trans_model(uint8_t data)
{
	sys_setting.trans_model = data;
}

//温度单位
uint8_t config_get_tem_unit(void)
{
	return sys_setting.tem_unit;
}
void config_set_tem_unit(uint8_t data)
{
	sys_setting.tem_unit = data;
}

//内部传感器开关
uint8_t config_get_internal_en(void)
{
	return sys_setting.internal_en;
}
void config_set_internal_en(uint8_t data)
{
	sys_setting.internal_en = data;
}

//外部传感器开关
uint8_t config_get_external_en(void)
{
	return sys_setting.external_en;
}
void config_set_external_en(uint8_t data)
{
	sys_setting.external_en = data;
}

//内部传感器型号
uint8_t config_get_internal_model(void)
{
	return sys_setting.internal_model;
}
void config_set_internal_model(uint8_t data)
{
	sys_setting.internal_model = data;
}

//外置单总线型号
uint8_t config_get_external_model(void)
{
	return sys_setting.external_model;
}
void config_set_external_model(uint8_t data)
{
	sys_setting.external_model = data;
}

//dhcp en 
uint8_t config_get_dhcp_en(void)
{
	return sys_setting.dhcp_en;
}
void config_set_dhcp_en(uint8_t data)
{
	sys_setting.dhcp_en = data;
}

//网络模式
uint8_t config_get_net_mode(void)
{
	return sys_setting.net_mode;
}
void config_set_net_mode(uint8_t data)
{
	sys_setting.net_mode = data;
}

//GPS
uint8_t config_get_gps_mode(void)
{
	return sys_setting.gps_mode;
}
void config_set_gps_mode(uint8_t data)
{
	sys_setting.gps_mode = data;
}

//背光时间
uint8_t config_get_back_light_time(void)
{
	return sys_setting.back_light_time;
}
void config_set_back_light_time(uint8_t data)
{
	sys_setting.back_light_time = data;
}

//报警使能
uint8_t config_get_alarm_en(void)
{
	return sys_setting.alarm_en;
}
void config_set_alarm_en(uint8_t data)
{
	sys_setting.alarm_en = data;
}

//自动显示 
uint8_t config_get_auto_switch_show(void)
{
	return sys_setting.auto_switch_show;
}
void config_set_auto_switch_show(uint8_t data)
{
	sys_setting.auto_switch_show = data;
}

//WIFI账号
uint8_t* config_get_wifi_ssid(void)
{
	return sys_setting.wifi_ssid;
}
void config_set_wifi_ssid(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_ssid, (char *)data);
}

//WIFI密码
uint8_t* config_get_wifi_pawd(void)
{
	return sys_setting.wifi_pawd;
}
void config_set_wifi_pawd(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_pawd, (char *)data);
}

//WIFI ip
uint8_t* config_get_wifi_ip(void)
{
	return sys_setting.wifi_ip;
}
void config_set_wifi_ip(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_ip, (char *)data);
}

//子网掩码
uint8_t* config_get_wifi_mask(void)
{
	return sys_setting.wifi_mask;
}
void config_set_wifi_mask(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_mask, (char *)data);
}

//网关
uint8_t* config_get_wifi_gw(void)
{
	return sys_setting.wifi_gw;
}
void config_set_wifi_gw(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_gw, (char *)data);
}

//mac
uint8_t* config_get_wifi_mac(void)
{
	return sys_setting.wifi_mac;
}
void config_set_wifi_mac(uint8_t* data)
{
    strcpy((char *)&sys_setting.wifi_mac, (char *)data);
}

//MQTT ID
uint8_t* config_get_mqtt_client_id(void)
{
	return sys_setting.mqtt_client_id;
}
void config_set_mqtt_client_id(uint8_t* data)
{
    strcpy((char *)&sys_setting.mqtt_client_id, (char *)data);
}

//MQTT USER NAME
uint8_t* config_get_mqtt_user_name(void)
{
	return sys_setting.mqtt_user_name;
}
void config_set_mqtt_user_name(uint8_t* data)
{
    strcpy((char *)&sys_setting.mqtt_user_name, (char *)data);
}

//MQTT USER PAWD
uint8_t* config_get_mqtt_user_pawd(void)
{
	return sys_setting.mqtt_user_pawd;
}
void config_set_mqtt_user_pawd(uint8_t* data)
{
    strcpy((char *)&sys_setting.mqtt_user_pawd, (char *)data);
}

//MQTT发布主题
uint8_t* config_get_mqtt_post_topic(void)
{
	return sys_setting.mqtt_post_topic;
}
void config_set_mqtt_post_topic(uint8_t* data)
{
    strcpy((char *)&sys_setting.mqtt_post_topic, (char *)data);
}

//MQTT订阅主题
uint8_t* config_get_mqtt_subs_topic(void)
{
	return sys_setting.mqtt_subs_topic;
}
void config_set_mqtt_subs_topic(uint8_t* data)
{
    strcpy((char *)&sys_setting.mqtt_subs_topic, (char *)data);
}

//服务器IP
uint8_t* config_get_server_ip(void)
{
	return sys_setting.server_ip;
}
void config_set_server_ip(uint8_t* data)
{
    strcpy((char *)&sys_setting.server_ip, (char *)data);
}

//服务器端口
uint8_t* config_get_server_port(void)
{
	return sys_setting.server_port;
}
void config_set_server_port(uint8_t* data)
{
    strcpy((char *)&sys_setting.server_port, (char *)data);
}

//APN
uint8_t* config_get_apn(void)
{
	return sys_setting.apn;
}
void config_set_apn(uint8_t* data)
{
    strcpy((char *)&sys_setting.apn, (char *)data);
}

//APN 账号
uint8_t* config_get_apn_name(void)
{
	return sys_setting.apn_user_name;
}
void config_set_apn_name(uint8_t* data)
{
    strcpy((char *)&sys_setting.apn_user_name, (char *)data);
}

//APN密码
uint8_t* config_get_apn_pawd(void)
{
	return sys_setting.apn_user_pwd;
}
void config_set_apn_pawd(uint8_t* data)
{
    strcpy((char *)&sys_setting.apn_user_pwd, (char *)data);
}

//SN号
uint8_t* config_get_sn(void)
{
	return sys_setting.model_sn;
}
void config_set_sn(uint8_t *sn)
{
	uint8_t i = 0;
	for(i=0;i<16;i++)
    {
        if((sn != NULL)&&(sn[i] != '"'))
        {
			//printf("sn=%c\n",sn[i]);
            sys_setting.model_sn[i] = sn[i];
        } 
		else
		{
			sys_setting.model_sn[i] = '\0';
			break;
		}                 
    }
}

//采样间隔时间
uint16_t config_get_collect_time(void)
{
	return sys_setting.collect_time;
}
void config_set_collect_time(uint16_t data)
{
	sys_setting.collect_time = data;
}

//上报模式
uint8_t config_get_up_type(void)
{
    return sys_setting.up_type;
}
void config_set_up_type(uint8_t data)
{
    sys_setting.up_type = data;
}

//上报间隔时间
uint16_t config_get_updata_time(void)
{
	return sys_setting.updata_time;
}
void config_set_updata_time(uint16_t data)
{
	sys_setting.updata_time = data;
}

//数据上限
float config_get_up_data(uint8_t chose)
{
	return sys_setting.up_data[chose];
}
void config_set_up_data(float data,uint8_t chose)
{
	sys_setting.up_data[chose]= data;
}

//数据下限
float config_get_down_data(uint8_t chose)
{
	return sys_setting.down_data[chose];
}
void config_set_down_data(float data,uint8_t chose)
{
	sys_setting.down_data[chose]= data;
}

//数据斜率修正
float config_get_amend_k(uint8_t chose)
{
	return sys_setting.amend_k[chose];
}
void config_set_amend_k(float data,uint8_t chose)
{
	sys_setting.amend_k[chose]= data;
}

//数据常量修正
float config_get_amend_b(uint8_t chose)
{
	return sys_setting.amend_b[chose];
}
void config_set_amend_b(float data,uint8_t chose)
{
	sys_setting.amend_b[chose]= data;
}

/* 校准系数合法性校验:NaN/inf及超范围值一律视为非法(data==data为可移植的NaN判定)
   B为偏移修正,量纲随通道类型:温湿度±20足够,CO2(ppm)标定偏移可达数百,取±1000覆盖全部类型 */
uint8_t system_amend_k_legal(float data)
{
	return (data == data) && (data > 0.1f) && (data < 10.0f);
}

uint8_t system_amend_b_legal(float data)
{
	return (data == data) && (data > -1000.0f) && (data < 1000.0f);
}

//数据回差值
float config_get_data_hc(uint8_t chose)
{
	return sys_setting.data_hc[chose];
}
void config_set_data_hc(float data,uint8_t chose)
{
	sys_setting.data_hc[chose]= data;
}

//数据波动值
float config_get_data_bd(uint8_t chose)
{
	return sys_setting.data_bd[chose];
}
void config_set_data_bd(float data,uint8_t chose)
{
	sys_setting.data_bd[chose]= data;
}
	
//软件版本日期
uint32_t config_get_soft_date(void)
{
	return sys_setting.soft_date;
}
void config_set_soft_date(uint32_t data)
{
	sys_setting.soft_date = data;
}

//OTA开关
uint8_t config_get_ota_en(void)
{
	return sys_setting.ota_en;
}
void config_set_ota_en(uint8_t data)
{
	sys_setting.ota_en = data;
}

//外置传感器长供电
uint8_t config_get_external_long_power(void)
{
	return sys_setting.external_long_power;
}
void config_set_external_long_power(uint8_t data)
{
	sys_setting.external_long_power = data;
}

//设备地址
uint8_t config_get_dev_addr(void)
{
	return sys_setting.dev_addr;
}
void config_set_dev_addr(uint8_t data)
{
	sys_setting.dev_addr = data;
}

//modbus使能
uint8_t config_get_modbus_en(void)
{
	return sys_setting.modbus_en;
}
void config_set_modbus_en(uint8_t data)
{
	sys_setting.modbus_en = data;
}

//报警时上报间隔使能
uint8_t config_get_alarm_interval_en(void)
{
    return sys_setting.alarm_interval_en;
}
void config_set_alarm_interval_en(uint8_t data)
{
    sys_setting.alarm_interval_en = data;
}

uint32_t system_get_net_err(void)
{
    return net_err_bit;
}

void system_set_net_err(uint32_t bit)
{
    net_err_bit |= bit;
}

void system_clear_net_err(uint32_t bit)
{
    net_err_bit &= ~bit;
}


















