#include "server.h"
#include "FreeRTOS.h"
#include "task.h"
#include "usart.h"
#include "string.h"
#include "stdio.h"
#include "l76k.h"
#include "sys_setting.h"
#include "AT.h"
#include "sd3078.h"
#include "pwr_manage.h"
#include "Air780E.h"
#include "hist_storage.h"
#include "w25qxx.h"
#include "stdlib.h"
#include "esp32_c5.h"
#include "tool.h"
#include "main.h"
#include "math.h"

hist_storage_t hist_data2;
static xTaskHandle server_handler;
extern rtc_t next_uptime;
extern rtc_t last_uptime;

wireless_info_t wireless_info;

uint8_t wireless_buf[IOT_DATA_LEN]={0};   //接收数组
uint8_t transmit_cmd[IOT_DATA_LEN]={0};  //发送数组1


void iot_en_deinit(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};	
    /**/
	GPIO_InitStruct.pin = FL_GPIO_PIN_1 | FL_GPIO_PIN_5;  
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin   = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);	
	
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_1 | FL_GPIO_PIN_5); 

    GPIO_InitStruct.pin =  FL_GPIO_PIN_5;  
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
    FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_5); 
}

void iot_4g_en_init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_5);
}

void gps_en_init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_5);	
}

void wifi_en_init(void)
{
    FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_1);	
}

void open_4g_en(void)
{
    FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_5);    
}
void close_4g_en(void)
{
    FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_5);    
}

void open_gps_en(void)
{
    FL_GPIO_SetOutputPin(GPIOB,FL_GPIO_PIN_5);    
}
void close_gps_en(void)
{
    FL_GPIO_ResetOutputPin(GPIOB,FL_GPIO_PIN_5);    
}

void open_wifi_en(void)
{
    FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_1);    
}
void close_wifi_en(void)
{
	FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_1);   		
}

int8_t config_get_signal(void)
{
	return wireless_info.wireless_signal;
}

void task_post_event_iot(uint32_t event)
{
  	xTaskNotify(server_handler, event, eSetBits);
}

uint16_t dev_info_to_buf(uint8_t *buf)
{
	uint16_t len;

    sprintf((char *)buf, "{\"dev_info\":""{\"p\":\"tp500\",\"ver\":\"YY_%d\",\"hard\":\"ljy_260323\",\"sn\":\"%s\"}}",
    
    config_get_soft_date(),
    config_get_sn());

	len = strlen((char *)buf);	
	return len;
}

uint16_t newdata_to_buf(uint8_t *buf)
{
	uint16_t len;
	
	if(config_get_gps_mode()==0)    
	{
		sprintf((char *)buf, "{\"times\":\"20%d-%02d-%02d %02d:%02d:%02d\","
			"\"sensorDatas\":[{\"flag\":\"1\",\"value\":%.2f},{\"flag\":\"2\",\"value\":%.2f},{\"flag\":\"3\",\"value\":%.2f},"
		"{\"flag\":\"4\",\"value\":%.2f},{\"flag\":\"5\",\"value\":%.2f},{\"flag\":\"6\",\"switcher\":\"%d\"},{\"flag\":\"7\",\"value\":%d},{\"flag\":\"ICCID\",\"str\":\"%s\"}]}",
		
        hist_data2.data_time.year,
		hist_data2.data_time.month,
		hist_data2.data_time.day,
		hist_data2.data_time.hour,
		hist_data2.data_time.min,
		hist_data2.data_time.sec,

		hist_data2.sensor_value[SENSOR_NUM_1],
		hist_data2.sensor_value[SENSOR_NUM_2], 
		hist_data2.sensor_value[SENSOR_NUM_3],
		hist_data2.sensor_value[SENSOR_NUM_4],
        hist_data2.battery_voltage,
        FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7),
		hist_data2.rssi,
		wireless_info.ccid);
	}
	else
	{
		sprintf((char *)buf,"{\"times\":\"20%d-%02d-%02d %02d:%02d:%02d\","
		"\"sensorDatas\":[{\"flag\":\"1\",\"value\":%.2f},{\"flag\":\"2\",\"value\":%.2f},{\"flag\":\"3\",\"value\":%.2f},"
			"{\"flag\":\"4\",\"value\":%.2f},{\"flag\":\"5\",\"value\":%.2f},{\"flag\":\"6\",\"switcher\":\"%d\"},{\"flag\":\"7\",\"value\":%d},{\"flag\":\"8\",\"lat\":%f,\"lng\":%f},{\"flag\":\"ICCID\",\"str\":\"%s\"}]}", 
        hist_data2.data_time.year,
		hist_data2.data_time.month,
		hist_data2.data_time.day,
		hist_data2.data_time.hour,
		hist_data2.data_time.min,
		hist_data2.data_time.sec,

		hist_data2.sensor_value[SENSOR_NUM_1],
		hist_data2.sensor_value[SENSOR_NUM_2], 
		hist_data2.sensor_value[SENSOR_NUM_3],
		hist_data2.sensor_value[SENSOR_NUM_4],
        hist_data2.battery_voltage,
        FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7),
		hist_data2.rssi,
		wireless_info.lat,
		wireless_info.lng,
		wireless_info.ccid);
	}
	
	len = strlen((char *)buf);	
	return len;
}

uint16_t tcp_udp_newdata_to_buf(uint8_t *buf)
{
	uint16_t len;
	sprintf((char *)buf,"{\"dev_newdata\":");
	if(config_get_gps_mode()==0)    
	{
		sprintf((char *)wireless_buf, "{\"times\":\"20%d-%02d-%02d %02d:%02d:%02d\","
			"\"value1\":%.2f,\"value2\":%.2f,\"value3\":%.2f,\"value4\":%.2f,"
			"\"voltage\":%.2f,\"switcher\":%d,\"signal\":%d,\"ICCID\":%s",
		
        hist_data2.data_time.year,
		hist_data2.data_time.month,
		hist_data2.data_time.day,
		hist_data2.data_time.hour,
		hist_data2.data_time.min,
		hist_data2.data_time.sec,

		hist_data2.sensor_value[SENSOR_NUM_1],
		hist_data2.sensor_value[SENSOR_NUM_2], 
		hist_data2.sensor_value[SENSOR_NUM_3],
		hist_data2.sensor_value[SENSOR_NUM_4],
        hist_data2.battery_voltage,
        FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7),
		hist_data2.rssi,
		wireless_info.ccid);
	}
	else
	{
		sprintf((char *)wireless_buf, "{\"times\":\"20%d-%02d-%02d %02d:%02d:%02d\","
			"\"value1\":%.2f,\"value2\":%.2f,\"value3\":%.2f,\"value4\":%.2f,"
			"\"voltage\":%.2f,\"switcher\":%d,\"signal\":%d,\"lat\":%f,\"lng\":%f,\"ICCID\":%s", 
        hist_data2.data_time.year,
		hist_data2.data_time.month,
		hist_data2.data_time.day,
		hist_data2.data_time.hour,
		hist_data2.data_time.min,
		hist_data2.data_time.sec,

		hist_data2.sensor_value[SENSOR_NUM_1],
		hist_data2.sensor_value[SENSOR_NUM_2], 
		hist_data2.sensor_value[SENSOR_NUM_3],
		hist_data2.sensor_value[SENSOR_NUM_4],
        hist_data2.battery_voltage,
        FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7),
		hist_data2.rssi,
		wireless_info.lat,
		wireless_info.lng,
		wireless_info.ccid);
	}
	strcat((char *)buf,(char *)wireless_buf);
	strcat((char *)buf,"}}");
	len = strlen((char *)buf);	
	return len;
}

uint16_t set_new_data_to_buf(void)   //设置新数据的发送数组
{
	uint8_t shuju_ok=0;
	uint32_t adds = 0;
	uint16_t send_len = 0;

    adds = device_get_flashaddr_addr();                                             //读取最近一次历史数据，保持上报数据与存储数据一致性
    w25qxx_read((uint8_t *)&hist_data2, adds ,sizeof(hist_storage_t));

        if((hist_data2.data_time.year == 255)||(hist_data2.data_time.month == 255))
        {	
            trace_log("没有存储数据 发送实时数据\r\n");
            shuju_ok = 1;
        }

	if(shuju_ok == 1)     //历史故障发送实时数据或者秒级模式
	{	
		hist_data2.data_time.week  = real_time.week;
		hist_data2.data_time.day   = real_time.day;
		hist_data2.data_time.hour  = real_time.hour;
		hist_data2.data_time.min   = real_time.min;
		hist_data2.data_time.month = real_time.month;
		hist_data2.data_time.sec   = real_time.sec;
		hist_data2.data_time.year  = real_time.year;
		
		hist_data2.sensor_value[SENSOR_NUM_1] = th_sensor_t.sensor_data[SENSOR_NUM_1];
		hist_data2.sensor_value[SENSOR_NUM_2] = th_sensor_t.sensor_data[SENSOR_NUM_2];
		hist_data2.sensor_value[SENSOR_NUM_3] = th_sensor_t.sensor_data[SENSOR_NUM_3];
		hist_data2.sensor_value[SENSOR_NUM_4]= th_sensor_t.sensor_data[SENSOR_NUM_4];
		hist_data2.sensor_alarm_state = th_sensor_t.alarm_state;
		//hist_data2.battery_bfb = config_get_vbat_bfb();
		hist_data2.battery_voltage = config_get_vbat_value();
		hist_data2.rssi = wireless_info.wireless_signal;	
	}
	else
	{
		hist_data2.rssi = wireless_info.wireless_signal;   //这里需要更新发最新的信号强度，因为历史存储的时候模块信号并未更新
		//hist_data2.restart_num = nb.device_restart_num;
	}
	
	send_len = newdata_to_buf(transmit_cmd);
//	trace_log(transmit_cmd);
	return send_len;	
}


void wireless_set_timer(uint8_t *temp_buf)
{	
    int last_time;
	int rtc_time;
    int diff;
    uint8_t rtc_state;
	rtc_t time;
	if(strstr((char *)temp_buf, "20") != NULL)
	{
		time.year = atoi((char *)temp_buf) - 2000;
		if(strstr((char *)temp_buf, "-") == NULL)
		{
			trace_log("同步时间失败\r\n");
			return;
		}
		while(*temp_buf++ != '-')
		{
		}
		time.month  = atoi((char *)temp_buf);
		if(strstr((char *)temp_buf, "-") == NULL)
		{
			trace_log("同步时间失败\r\n");
			return;
		}
		while(*temp_buf++ != '-')
		{
		}
		time.day  = atoi((char *)temp_buf);
		if(strstr((char *)temp_buf, " ") == NULL)
		{
			trace_log("同步时间失败\r\n");
			return;
		}
		while(*temp_buf++ != ' ')
		{
		}
		time.hour  = atoi((char *)temp_buf);
		if(strstr((char *)temp_buf, ":") == NULL)
		{
			trace_log("同步时间失败\r\n");
			return;
		}
		while(*temp_buf++ != ':')
		{
		}
		time.min = atoi((char *)temp_buf);
		if(strstr((char *)temp_buf, ":") == NULL)
		{
			trace_log("同步时间失败\r\n");
			return;
		}
		while(*temp_buf++ != ':')
		{
		}
		time.sec = atoi((char *)temp_buf);
		
        rtc_state = RTC_WriteDate(&time);

		if(rtc_state == TRUE)
		{
            if(wireless_info.first_ntp == 0)
			{
				next_updata_time(time);
				wireless_info.first_ntp = 1;
			}
            else
            {
				last_time = last_uptime.hour*60 + last_uptime.min;   
                rtc_time  = time.hour*60 + time.min;                  // 00:00 → 0
				
                diff = rtc_time - last_time;                          // 0 - 1430 = -1430
                if (diff < -720)
                {
                    diff += 1440;                       			 // -1430+1440 = +10
                }
				else if(diff > 720)
				{
					diff -=1440;
				}
                if(abs(diff) > config_get_updata_time())              // abs(10) > 1  → TRUE
                {
                   next_updata_time(time);
                }
            }
			trace_log("同步时间成功\r\n");
		}
		else
		{
			trace_log("同步时间失败\r\n");
		}	
	}	
	else
	{
		trace_log("同步时间格式错误\r\n");
	}
}

void server_task_create(void *params, uint8_t priority)
{  
    if(config_get_trans_model() == 0)
    {
        xTaskCreate(a78_task, "a78_task", 512, params, priority, &server_handler);
    } 
    else 
    {
        xTaskCreate(esp32c5_task, "esp32c5_task", 512, params, priority, &server_handler);
    }
}

