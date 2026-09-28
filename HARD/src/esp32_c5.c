#include "esp32_c5.h"
#include "string.h"
#include "stdio.h"
#include "server.h"
#include "usart.h"
#include "AT.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "hist_storage.h"
#include "sys_setting.h"
#include "math.h"
#include "stdlib.h"
#include "w25qxx.h"
#include "tool.h"
#include "l76k.h"
#include "pwr_manage.h"
#include "ymodem.h"
#include "flash.h"
#include "modbus_tcp.h"

uint8_t esp32_c5_first = 0;

static void esp32c5_get_MAC(void)
{	
	char mac_buf[20];
	char *sap;
	uint8_t num = 5;
	memset(mac_buf,0,sizeof(mac_buf));
	while(num--)
	{
		if(!uart_send_ack(WIFI_UART, "AT+CIPSTAMAC?\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500))
		{
			sap = strstr((char *)wireless_buf,"+CIPSTAMAC:");
			if(sap != NULL)
			{
				strncpy(mac_buf,(char*)(sap+12),17);
				mac_buf[2] = '-';
				mac_buf[5] = '-';
				mac_buf[8] = '-';
				mac_buf[11] = '-';
				mac_buf[14] = '-';	
				config_set_wifi_mac((uint8_t*)mac_buf);	
				break;
			}
		}
		vTaskDelay(200);
	}
}
/***************************************************************************/
static int8_t esp32c5_get_csq(void)
{
	int8_t csq = 0;
	float da;
	char *sa;
	
	if(!uart_send_ack(WIFI_UART, "AT+CWJAP?\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500))   //读取WIFI参数,获取信号强度
	{
		sa = strstr((char *)wireless_buf,"+CWJAP:");
        if(sa != NULL)
        {
            while(*sa++ != ',');
            while(*sa++ != ',');
            while(*sa++ != ',');
            da = atof(sa);
            csq = (uint8_t)(da+85);
            if(csq>31)
            {
                csq = 31;
            }
        }
	}
	return csq;
}
/***************************************************************************/
static uint8_t esp32c5_set_dhcp_static(uint8_t cnt,uint8_t cwmode)
{
	while(cnt--)
    {
        if(config_get_dhcp_en())
        {
            sprintf((char*)transmit_cmd,"AT+CWDHCP=1,%d\r\n",cwmode);
            uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500);  
            return TRUE;
        }
        else 
        {
            sprintf((char*)transmit_cmd,"AT+CWDHCP=0,%d\r\n",cwmode);
            if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500))
            {
                if(cwmode == 1)
                {
                    sprintf((char*)transmit_cmd,"AT+CIPSTA=\"%s\",\"%s\",\"%s\"\r\n",config_get_wifi_ip(),config_get_wifi_gw(),config_get_wifi_mask());
                }
                else if(cwmode == 2)
                {
                    sprintf((char*)transmit_cmd,"AT+CIPAP=\"%s\",\"%s\",\"%s\"\r\n",config_get_wifi_ip(),config_get_wifi_gw(),config_get_wifi_mask());
                }
                if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500))
                {
                    return TRUE;
                }
            }
        }
    }
    return FALSE;
}
/***************************************************************************/
static uint8_t esp32c5_get_static(uint8_t cnt,uint8_t cwmode)
{
    char *p = NULL;
	uint8_t r_buf[30];
	uint8_t i = 0;
    while(cnt--)
    {
        if(cwmode == 1)
        {
            if(!uart_send_ack(WIFI_UART, "AT+CIPSTA?\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500)) 
            {
                break;
            }
        }
        else if(cwmode == 2)
        {
            if(!uart_send_ack(WIFI_UART, "AT+CIPAP?\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500)) 
            {
                break;
            }
        }
    }
    if(strstr((char*)wireless_buf,"OK"))
    {
        p = strstr((char*)wireless_buf,"ip");
        if(p!=NULL)
        {
            p = p + 4;
			memset(r_buf,0,sizeof(r_buf));
			i = 0;
			while(*p != '\"')
			{
				r_buf[i] = *p;
				i++;
				p++;
			}
            config_set_wifi_ip(r_buf);
        }
        p = strstr((char*)wireless_buf,"gateway");
        if(p!=NULL)
        {
            p = p + 9;
			memset(r_buf,0,sizeof(r_buf));
			i = 0;
			while(*p != '\"')
			{
				r_buf[i] = *p;
				i++;
				p++;
			}
            config_set_wifi_gw(r_buf);
        }
        p = strstr((char*)wireless_buf,"netmask");
        if(p!=NULL)
        {
            p = p + 9;
			memset(r_buf,0,sizeof(r_buf));
			i = 0;
			while(*p != '\"')
			{
				r_buf[i] = *p;
				i++;
				p++;
			}
            config_set_wifi_mask(r_buf);
        }
        return TRUE;
    }
    return FALSE;
}
/***************************************************************************/
static void close_AP(void)    //断开与AP的连接
{
	uart_send_ack(WIFI_UART, "AT+CWQAP\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
}
/***************************************************************************/
static uint8_t esp32c5_open(void)
{		 
	if(TEST_AT(WIFI_UART,30))  //失败
	{
	 	 return FALSE;
	}	
	uart_send_ack(WIFI_UART, "AT+GMR\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);         //查询版本信息
	close_AP();
	uart_send_ack(WIFI_UART, "AT+CWMODE?\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);     //查询WIFI模式
	uart_send_ack(WIFI_UART, "AT+CWMODE=1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);    //设置WIFI模式
    uart_send_ack(WIFI_UART, "AT+CWBANDMODE=3\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);    //设置双频模式
    uart_send_ack(WIFI_UART, "AT+CWAUTOCONN=0\r\n", wireless_buf, "OK", sizeof(wireless_buf),500);     //关闭自动连接
    esp32c5_set_dhcp_static(10,1);
	esp32c5_get_MAC();
//	0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
//  1: Station 模式
//	2: SoftAP 模式
//	3: SoftAP+Station 模式
	//sprintf((char *)transmit_cmd,"AT+CWJAP=\"%s\",\"%s\"\r\n",device_info.save.srv_apn_user_name,device_info.save.srv_apn_user_pass);
    sprintf((char *)transmit_cmd,"AT+CWJAP=\"%s\",\"%s\",,,0,,1\r\n",config_get_wifi_ssid(),config_get_wifi_pawd());
	uart_transmit_str(WIFI_UART, transmit_cmd);     //发送数据
	if(check_recv2(WIFI_UART, wireless_buf,"OK", "ERROR",sizeof(wireless_buf),10000))
	{
		close_AP();
		uart_transmit_str(WIFI_UART, transmit_cmd);
		if(check_recv2(WIFI_UART, wireless_buf,"OK", "ERROR",sizeof(wireless_buf),10000))
		{
			trace_log("WIFI连接失败\r\n");
			return FALSE;
		}
	}
	else
    {
		trace_log("WIFI连接成功\r\n");
    }
    esp32c5_get_static(10,1);
    wireless_info.wireless_signal = esp32c5_get_csq();
	//nb.nb_csq = esp32c5_get_csq();
	return TRUE;
}

static uint8_t get_connect_state(void)
{
    uint8_t *p = NULL;
    int32_t num;
    uint16_t size = 0;
    char *getp;
    uint8_t cont = 5;
	int datalen;

    if(config_get_net_mode() == MQTT_MODE)
    {
        if(uart_send_ack(WIFI_UART,"AT+MQTTCONN?\r\n",wireless_buf,"+MQTTCONN:",sizeof(wireless_buf),1000) == 0)
        {
            p = wireless_buf;
            while(*p++ != ',');
            num = atoi((char *)p);
            if(num > 3)
            {
				return TRUE;
            }
        }
        trace_log("MQTT连接已断开\n");  
    }
    else
    {
        while(cont--)
        {
            //+CIPRECVDATA:1,0x00				//TCP 平台应答
            uart_transmit_str(WIFI_UART,(uint8_t *) "AT+CIPRECVDATA=768\r\n");
            size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 3000);
            if(strstr(( char *)wireless_buf, "+IPD"))
            {
                size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 3000);
            }
            if(size)
            {
                if(get_trace_log())
                {
                    uart_transmit_buf(USB_UART, wireless_buf, size);
                }
                getp=strstr(( char *)wireless_buf, "+CIPRECVDATA:");  
                if(getp != NULL)
                {
                    getp += 13;
                    datalen=atoi((char *)getp);
                    if(datalen == 1)                                                               
                    {
                        while(*getp++ != ',');
                        if(*getp == 0x00)
                        {
                            trace_log("数据发送成功\r\n");
                            return TRUE;
                        }					
                    }
                }
            }
            vTaskDelay(1000);
        }
        trace_log("数据发送失败\r\n");
    }
	return FALSE;
}
/***************************************************************************/
static uint8_t esp32c5_MQTT_CONNECT(void)
{	
/* 	sprintf((char *)transmit_cmd,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
		device_info.save.srv_login,device_info.save.srv_UserName,device_info.save.srv_Password); */

   	sprintf((char *)transmit_cmd,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",
		config_get_mqtt_client_id(),config_get_mqtt_user_name(),config_get_mqtt_user_pawd());     
	uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500);  //发送MQTT参数设置
	
	//sprintf((char *)transmit_cmd,"AT+MQTTCONN=0,\"%s\",%s,0\r\n",device_info.save.srv_addr,device_info.save.srv_port);
    sprintf((char *)transmit_cmd,"AT+MQTTCONN=0,\"%s\",%s,0\r\n",config_get_server_ip(),config_get_server_port());
	uart_transmit_str(WIFI_UART,transmit_cmd);

	if(check_recv2(WIFI_UART, wireless_buf,"+MQTTCONNECTED:", "ERROR",sizeof(wireless_buf),10000) != 0)
//	if(uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "+MQTTCONNECTED:", sizeof(wireless_buf), 10000))  //发送MQTT连接指令
	{
		trace_log("MQTT服务器连接失败,重连\r\n");
		uart_transmit_str(WIFI_UART,transmit_cmd);
		if(check_recv2(WIFI_UART, wireless_buf,"+MQTTCONNECTED:", "ERROR",sizeof(wireless_buf),10000) != 0)
		{
			trace_log("MQTT服务器连接失败2次,退出\r\n");
			return FALSE;
		}
	}
	trace_log("MQTT服务器连接成功\r\n");
	//sprintf((char *)transmit_cmd,"AT+MQTTSUB=0,\"%s\",0\r\n",config_get_mqtt_subs_topic()); //MQTT订阅
	sprintf((char *)transmit_cmd,"AT+MQTTSUB=0,\"%s\",0\r\n",config_get_mqtt_subs_topic()); //MQTT订阅    
	uart_transmit_str(WIFI_UART,transmit_cmd);
//	if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 10000))
	if(check_recv2(WIFI_UART, wireless_buf,"OK", "ERROR",sizeof(wireless_buf),5000) != 0)
	{
		trace_log("MQTT订阅失败\r\n");
	}
	else
    {
        trace_log("MQTT订阅成功\r\n");
    }
	return TRUE;
}
/***************************************************************************/
static void esp32c5_MQTT_close(void)
{
	sprintf((char *)transmit_cmd,"AT+MQTTUNSUB=0,\"%s\"\r\n",config_get_mqtt_subs_topic()); //取消MQTT订阅
	uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500);
	uart_send_ack(WIFI_UART, "AT+MQTTCLEAN=0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);//断开MQTT连接
}
/***************************************************************************/
static void esp32c5_TCP_close(void)
{
	uart_send_ack(WIFI_UART, "AT+CIPCLOSE\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);//断开TCP_UDP连接
}
/***************************************************************************/
static void esp32c5_TCP_server_close(void)
{
    uart_send_ack(WIFI_UART, "AT+CIPSERVER=0,1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);    //关闭服务器断开所有连接
}

/***************************************************************************/
static uint8_t esp32c5_MQTT_pub_data(uint8_t *buf)
{	
    uint8_t re = 2;
	//改指令最大可发送包括AT在内总长度256字节
//	sprintf((char *)transmit_cmd,"AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n",config_get_mqtt_post_topic(),buf);
//	if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 300))
//		return TRUE;
//	else
//		return FALSE;
	while(re--)
	{
        //该指令最大可发送512字节数据
        sprintf((char *)wireless_buf,"AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n",config_get_mqtt_post_topic(),strlen((char *)buf));
        if(uart_send_ack(WIFI_UART, wireless_buf, wireless_buf, ">", sizeof(wireless_buf),5000))
		{  	
			trace_log("发送收不到'>'\r\n");
			continue;
		}
        else
        {
            trace_log(buf);
            trace_log("\r\n");
            if(!uart_send_ack(WIFI_UART, buf, wireless_buf, "+MQTTPUB:OK", sizeof(wireless_buf), 5000))
            {
                return TRUE;
            }
        }   
    }
	return FALSE;
}
/***************************************************************************/
static uint8_t esp32c5_TCP_senddata(uint8_t *send_buf,uint16_t buf_len)
{
	uint8_t re = 2;
	
	/*if(device_info.debug_en) 
    {
        uart_transmit_buf(USB_UART, send_buf,buf_len);
        trace_log("\r\n");
    }*/
	while(re--)
	{
		sprintf((char *)wireless_buf,"AT+CIPSEND=%d\r\n",buf_len);
		if(uart_send_ack(WIFI_UART, wireless_buf, wireless_buf, ">", sizeof(wireless_buf),5000))
		{  	
			trace_log("发送收不到'>'\r\n");
			continue;
		}
        else
        {
            trace_log(send_buf);
            trace_log("\r\n");
            if(!uart_transmit_wait_ack(WIFI_UART, send_buf,buf_len,wireless_buf,sizeof(wireless_buf), "SEND OK",5000))
            {
                return TRUE;
            }
        }
	}
	return FALSE;
}
/***************************************************************************/
/*static uint8_t esp32c5_send_modeldata(void)
{
	uint16_t send_len;

	send_len = modeldata_params((char *)transmit_cmd);	
	if(config_get_net_mode() == TCP_MODE) 
	{
		if(esp32c5_TCP_senddata(transmit_cmd,send_len) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;
        }
	}
	else if(config_get_net_mode() == MQTT_MODE) 
	{
		if(esp32c5_MQTT_pub_data(transmit_cmd) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;
        }
	}
	return FALSE;
}*/

static uint8_t esp32c5_send_dev_info(void)  //设备信息
{
	uint16_t len;   
    
    dev_info_to_buf(transmit_cmd);
    len = strlen((char *)transmit_cmd);
    if(config_get_net_mode() == MQTT_MODE)
    {
		if(esp32c5_MQTT_pub_data(transmit_cmd) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;
        }
    }
    else if(config_get_net_mode() == TCP_MODE) 
    {
		if(esp32c5_TCP_senddata(transmit_cmd,len) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;
        }
    }
	//len = setting_params((char *)transmit_cmd);    //上报数据格式与NB模式非AEP一致
	return FALSE;
}


/***************************************************************************/
static uint8_t esp32c5_send_param(void)
{
	uint16_t send_len;

	//send_len = setting_params((char *)transmit_cmd);				//编辑同步数据
    system_format_cloud_config((char *)transmit_cmd);
    send_len= strlen((char *)transmit_cmd);
	trace_log(transmit_cmd);
	trace_log("\r\n");
	if(config_get_net_mode() == TCP_MODE) 
	{
		if(esp32c5_TCP_senddata(transmit_cmd,send_len) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;
        }
	}
	else if(config_get_net_mode() == MQTT_MODE) 
	{
		if(esp32c5_MQTT_pub_data(transmit_cmd) == FALSE)
        {
			return FALSE;
        }
		else
        {
			return TRUE;       
        }
	}
	return FALSE;
}

/***************************************************************************/
static void esp32c5_recive_data(uint8_t wait_tick)
{
	uint16_t size=0;
//	uint8_t wait_tick = 10;
	char *getp;
	uint16_t datalen=0;
	uint16_t p = 0;
	uint8_t re_state = 0;

	trace_log("检测远程配置\r\n");
	
	while(wait_tick--)
	{		
		if(config_get_net_mode() == TCP_MODE)
		{
			//+CIPRECVDATA:20,2026-05-18 09:52:44				//时间
			//下发参数
			//+CIPRECVDATA:573,{"记录间隔(分)":"1","内部传感器":"1","外置传感器":"0","通道1斜率修正":"1.00","通道2斜率修正":"1.00","通道3斜率修正":"1.00","通道4斜率修正":"1.00","通道1常量修正":"0.00","通道2常量修正":"0.00","通道3常量修正":"0.00","通道4常量修正":"0.00","报警开关":"1","通道1上限":"70.00","通道2上限":"90.00","通道3上限":"70.00","通道4上限":"90.00","通道1下限":"-40.00","通道2下限":"0.00","通道3下限":"-40.00","通道4下限":"0.00","通道1回差":"2.00","通道2回差":"2.00","通道3回差":"2.00","通道4回差":"2.00","通道1波动":"100.00","通道2波动":"100.00","通道3波动":"100.00","通道4波动":"100.00"}
			uart_transmit_str(WIFI_UART,(uint8_t *) "AT+CIPRECVDATA=768\r\n");
			size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 3000);
			if(strstr(( char *)wireless_buf, "+IPD"))
			{
				size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 3000);
			}
			if(size)
			{
				if(get_trace_log())
				{
					uart_transmit_buf(USB_UART, wireless_buf, size);
				}
				getp =strstr(( char *)wireless_buf, "+CIPRECVDATA:");  
				if(getp != NULL)
				{
					getp += 13;
					datalen=atoi((char *)getp);
					if(datalen >=10)    //2024.7.19     						字节数由30改成最小10字节，也就是最少一个配置信息包{"AT":"3"}
					{
						trace_log("收到下发参数\r\n");
                        while(*getp++ != ',');
//                        while(*getp == 0)
//                        {
//                            getp++;
//                            zero_count++;
//                        }
						memset((char*)transmit_cmd,0,sizeof(transmit_cmd));
                        strncpy((char *)transmit_cmd,getp,datalen);
						re_state = 1;						
						break;
					}
				}
			}
			else
            {
				trace_log("接收数据错误\r\n");
            }
		}
		else if(config_get_net_mode() == MQTT_MODE)
		{
			
			//+MQTTSUBRECV:0,"7E0BDS7JPXT5Q99F/123",19,2026-05-18 09:36:46				//时间
			//同步参数
			//+MQTTSUBRECV:0,"7E0BDS7JPXT5Q99F/123",573,{"记录间隔(分)":"1","内部传感器":"1","外置传感器":"0","通道1斜率修正":"1.00","通道2斜率修正":"1.00","通道3斜率修正":"1.00","通道4斜率修正":"1.00","通道1常量修正":"0.00","通道2常量修正":"0.00","通道3常量修正":"0.00","通道4常量修正":"0.00","报警开关":"1","通道1上限":"70.00","通道2上限":"90.00","通道3上限":"70.00","通道4上限":"90.00","通道1下限":"-40.00","通道2下限":"0.00","通道3下限":"-40.00","通道4下限":"0.00","通道1回差":"2.00","通道2回差":"2.00","通道3回差":"2.00","通道4回差":"2.00","通道1波动":"100.00","通道2波动":"100.00","通道3波动":"100.00","通道4波动":"100.00"}
			size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 3000);
			if(size)
			{
				if(get_trace_log())
				{
					uart_transmit_buf(USB_UART, wireless_buf, size);
				}
				getp =strstr(( char *)wireless_buf, "+MQTTSUBRECV:");  
				if(getp != NULL)
				{
					while(*getp++ != ',');
					while(*getp++ != ',');
					datalen=atoi((char *)getp);
					if(datalen >=10)    										//2024.7.19     字节数由30改成最小10字节，也就是最少一个配置信息包{"AT":"3"}
					{
						trace_log("收到下发参数\r\n");
						while(*getp++ != ',');
                        memset((char*)transmit_cmd,0,sizeof(transmit_cmd));
                        strncpy((char *)transmit_cmd,getp,datalen);
						re_state = 1;
						break;
					}				
				}
			}	
			else
			{
				trace_log("接收数据错误\r\n");
			}
		}
        vTaskDelay(1000);
	}
	
	if(re_state)
	{	
		if(wireless_info.upload_set == 1)
		{
			p = strstraddr((uint8_t *)getp,(uint8_t *)"20",datalen);
			if(p != 0xFFFF)
			{
				wireless_set_timer((uint8_t *)(getp+p));	
			}
			else
			{
				trace_log("同步时间数据错误\r\n");
			}			
		}
		else if(wireless_info.upload_set == 2)
		{
			trace_log("同步参数\r\n");
			if((strchr((char *)transmit_cmd,'{'))&&(strrchr((char *)transmit_cmd,'}')))	
			{		
				if((strrchr((char *)transmit_cmd,'}') - strchr((char *)transmit_cmd,'{')) == (datalen-1))
				{
					while(p < datalen)
					{
						if(*(getp+p) == '{')
						{
							break;
						}
						p++;
					}
					if(p >= datalen)
					{
						trace_log("同步数据错误\r\n");
					}
					else
					{
						if(system_set_cloud_config((uint8_t *)(getp+p)) == TRUE)
						{
                            esp32c5_send_param();
						}
					}					
				}
				else
				{
					trace_log("同步数据长度错误\r\n");
				}
			}
			else
			{
				trace_log("同步数据格式错误\r\n");
			}
		}
		else
		{
			trace_log("没有参数变化\r\n");			
		}
	}	
	wireless_info.upload_set = 0;
}

/***************************************************************************/
//mod_buf   TCP_MODE/UDP
/*<access_mode> 整型。Socket 服务的数据访问模式。
 0 普通模式
 1 透传模式*/
static uint8_t esp32c5_TCP_CONNECT(char *mode_buf,char *ip_buf,char *port_buf,uint8_t con_id,uint8_t access_mode)
{
	uint8_t re=FALSE;
	sprintf((char *)transmit_cmd,"AT+CIPMODE=%d\r\n",access_mode);
	uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 500);//设置TCP为普通传输模式
	uart_send_ack(WIFI_UART, "AT+CIPMUX=0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);//设置TCP为单链接模式
	if(access_mode == 0 )
    {
		uart_send_ack(WIFI_UART, "AT+CIPRECVTYPE=1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);//设置TCP数据接收为缓存模式
    }
	sprintf((char *)transmit_cmd,"AT+CIPSTART=\"%s\",\"%s\",%s\r\n",mode_buf,ip_buf,port_buf);
	if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 10000)) 
	{
		trace_log("TCP_MODE/UDP连接成功\r\n");
		re = TRUE;
	}
	else
	{
		trace_log("TCP_MODE/UDP连接失败,重连\r\n");
		esp32c5_TCP_close();
		if(!uart_send_ack(WIFI_UART, transmit_cmd, wireless_buf, "OK", sizeof(wireless_buf), 10000))
		{
			trace_log("TCP_MODE/UDP连接成功\r\n");
			re = TRUE;
		}
		else
        {
			trace_log("TCP_MODE/UDP连接失败\r\n");
        }
	}
	
	if(re == TRUE)
	{
		if(access_mode == 1 )
		{
			if(uart_send_ack(WIFI_UART, "AT+CIPSEND\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500))//进入透传模式
			{
				trace_log("进入透传模式失败\r\n");
				re = FALSE;
			}
		}
	}
	
	return re;
}
/***************************************************************************/
static uint8_t esp32c5_connect_server(void)
{	
    if(config_get_net_mode() == MQTT_MODE)
	{
		if(esp32c5_MQTT_CONNECT() == FALSE)
			return FALSE;
	}
    else if(config_get_net_mode() == TCP_MODE)
	{
        if(esp32c5_TCP_CONNECT("TCP",(char *)config_get_server_ip(),(char *)config_get_server_port(),0,0) == TRUE)
		{
			trace_log("发送登录包\r\n");
			memset(wireless_buf,0,sizeof(wireless_buf));
	        sprintf((char *)transmit_cmd,"%s", config_get_mqtt_post_topic());
			trace_log(transmit_cmd);
			trace_log("\r\n");
			if(esp32c5_TCP_senddata(transmit_cmd,strlen((char *)transmit_cmd)) == FALSE)
            {
                return FALSE;
            }	
		}
		else
        {
			return FALSE;
        }
	}
	return TRUE;
}
/******************************************************************************/
static uint8_t esp32c5_tcp_modbus_server(void)
{
    uint8_t re=FALSE;
    if((config_get_net_mode() == TCP_MODE)&&(config_get_modbus_en() == 1))
    {                                            
	    uart_send_ack(WIFI_UART, "AT+CIPMODE=0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);                //设置TCP为普通传输模式
		uart_send_ack(WIFI_UART, "AT+CIPMUX=1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);                 //设置TCP为多链接模式
        uart_send_ack(WIFI_UART, "AT+CIPSERVERMAXCONN=1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);       //设置TCP服务器最大连接数
        uart_send_ack(WIFI_UART, "AT+CIPRECVTYPE=1,0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);          //设置TCP数据接收为缓存模式
        if(!uart_send_ack(WIFI_UART, "AT+CIPSERVER=1,502\r\n", wireless_buf, "OK", sizeof(wireless_buf), 10000))      //设置TCP为服务器模式
        {
            trace_log("设置TCP为服务器模式成功\r\n");
            re = TRUE;
        }
        else
        {
            trace_log("设置TCP为服务器模式失败\r\n");
        }
    }
    return re;
}
/***************************************************************************/
static uint8_t esp32c5_get_data(void)
{
	trace_log("getParam\r\n");
	if(config_get_net_mode() == MQTT_MODE)
	{
		if(esp32c5_MQTT_pub_data((uint8_t *)"getParam") == FALSE)  //先发获取指令
			return FALSE;
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		if(esp32c5_TCP_senddata((uint8_t *)"getParam",8) == FALSE)
			return FALSE;
	}
	return TRUE;
}
///***************************************************************************/
static uint8_t esp32c5_get_time(void)
{
	trace_log("getTime\r\n");
	if(config_get_net_mode() == MQTT_MODE)
	{
		if(esp32c5_MQTT_pub_data((uint8_t *)"getTime") == FALSE)  //先发获取指令
			return FALSE;
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		if(esp32c5_TCP_senddata((uint8_t *)"getTime",7) == FALSE)
			return FALSE;
	}
	return TRUE;
}

/***************************************************************************/
static uint8_t esp32c5_mqtt_tcp_send_new_data(void)
{	
	uint8_t re = FALSE;	
	uint16_t len;

	len = set_new_data_to_buf();
	trace_log(transmit_cmd);
	trace_log("\r\n");
	if(config_get_net_mode() == MQTT_MODE)
	{		
        re = esp32c5_MQTT_pub_data(transmit_cmd);    
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		re = esp32c5_TCP_senddata(transmit_cmd,len);
	}	
	return re;
}
/***************************************************************************/
/*static uint8_t esp32c5_send_heart(void)
{	
	uint8_t re = FALSE;	
	uint16_t len;

	sprintf((char *)transmit_cmd,"{\"heart\":50}");
	len = strlen((char *)transmit_cmd);
	if(config_get_net_mode() == MQTT_MODE)
	{		
		re = esp32c5_MQTT_pub_data(transmit_cmd);
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		re = esp32c5_TCP_senddata(transmit_cmd,len);
	}	
	return re;
}*/
/***************************************************************************/
static uint8_t esp32c5_send_his_data(uint32_t pend_num)
{
	uint8_t re = FALSE;
	uint16_t len;
    uint32_t addr = device_perv_addr_get();

	/*if((device_info.sensor_data.model_status&flash_err) != flash_err && device_info.save.history_mode == ON)
	{
		device_pending_num_get()=device_pending_num_get();
		device_perv_addr_get()=device_perv_addr_get();
		sprintf((char *)transmit_cmd,"%d\r\n",device_pending_num_get());
		trace_log("待补发历史总数:");
		trace_log(transmit_cmd);
	}
	else
	{
		trace_log("flash存储错误或者关闭历史补发，不发历史数据\r\n");
		return re;
	}*/

	while(pend_num)
	{
		trace_log("历史补发\r\n");
		vTaskDelay(1200);
        w25qxx_read((uint8_t *)&hist_data2, addr ,sizeof(hist_storage_t));
		//hist_read_onebuf(&hist_data2, device_perv_addr_get());    //读取历史
		addr = hist_data2.next_addr;   //地址移位
		if(hist_data2.data_time.year==255 && hist_data2.data_time.month == 255)
		{
			trace_log("数据读取有错误\r\n");
			pend_num = 0;	 
			device_pending_num_set(pend_num);
			break;
		}
		len = newdata_to_buf(transmit_cmd);
		trace_log(transmit_cmd);
		trace_log("\r\n");
		if(config_get_net_mode() == MQTT_MODE)
		{
			//len = MQTT_newdata_to_buf(transmit_cmd);
			re = esp32c5_MQTT_pub_data(transmit_cmd);
		}
		else if(config_get_net_mode() == TCP_MODE)
		{
			//len = NB_TCP_newdata_to_buf(transmit_cmd);
			re = esp32c5_TCP_senddata(transmit_cmd,len);
		}

        if(re == FALSE)
        {
            return re;
        }
        else
        {
            re = get_connect_state();
            if(re == FALSE)
            {
                system_set_net_err(err_srv);
                return re;
            }
		    device_perv_addr_set(addr);   //发送成功，保存断点日志
        }
		//if(device_pending_num_get()>=1)
        if(pend_num>=1)
		{
            pend_num = pend_num - 1;
			//device_pending_num_get() = device_pending_num_get() - 1;
			device_pending_num_set(pend_num);
		}
	}	
	return re;
}
/***************************************************************************/
static uint8_t esp32c5_out_touchuan(void)  //退出透传模式
{
  uint8_t num=3;
  uint8_t re=FALSE;
  
  vTaskDelay(50);
  while(num--)
  {
		uart_transmit_str(WIFI_UART,(uint8_t *)"+++");
		vTaskDelay(50);
    if(!uart_send_ack(WIFI_UART, "AT\r\n", transmit_cmd, "OK", sizeof(transmit_cmd),300))
    {
      trace_log("退出透传模式成功\r\n");
      re=TRUE;
      break; 
    } 
    else
      trace_log("退出透传模式失败\r\n");
  }
  
  return re;
}
/***************************************************************************/
static uint8_t esp32c5_OTA_ymodem(void)
{
	uint8_t res = FALSE;
	trace_log("OTA升级服务检测\r\n");
	res = esp32c5_TCP_CONNECT((char *)"TCP",(char *)"112.74.98.39",(char *)"8089",0,1);    //设置模块在透传模式下传输数据
	if(res == FALSE)
	{
		trace_log("OTA升级服务器连接失败\r\n");
		//    out_touchuan();
		esp32c5_TCP_close();
		return res;
	}

	res = FALSE;
	if(get_OTA_Version(WIFI_UART) == TRUE)
	{
		if(ymodem_get_data(WIFI_UART)== TRUE)
		  res = TRUE;
		else
		  res = FALSE;
	}
	trace_log("退出OTA升级任务\r\n");
	esp32c5_out_touchuan();
	esp32c5_TCP_close();

	return res;
}
/***************************************************************************/
static void close_connect(void)
{	
	if(config_get_net_mode() == MQTT_MODE)
	{		
		esp32c5_MQTT_close();
	}
	else 
	{
        if(config_get_modbus_en() == 0)
        {
            esp32c5_TCP_close();
        }
        else
        {
            esp32c5_TCP_server_close();
        }
	}
}
/***************************************************************************/
static void close_esp32c5(void)
{
	close_connect();
	close_AP();
	close_wifi_en();
}
/*************************************************************************/

/**************************************************************************/
void esp32c5_client_pro(void)
{
    uint8_t ota_en=FALSE;
    uint32_t rbuf[2];
    uint32_t pend_bak;

    trace_log("WIFI主动上报开启\n");

    //uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf),500);  //清空接收缓存区
    memset(transmit_cmd,0,sizeof(transmit_cmd));
    memset(wireless_buf,0,sizeof(wireless_buf));
    
    if(esp32c5_open() == FALSE)
    {		 
        trace_log("模块初始化失败\r\n");
        system_set_net_err(err_srv);
        goto end_task;	
    }
    else
    {
        system_clear_net_err(err_srv);
        trace_log("模块初始化完成\r\n");
    }
        
    //先检测OTA  		
    if(((one_day_cont >= 24)||(esp32_c5_first == 0))&&(config_get_ota_en()))
    {
        one_day_cont = 0;
        ota_en = esp32c5_OTA_ymodem();  
        if(ota_en == TRUE)
        {
            trace_log("OTA下载成功，设备升级\r\n");
            rbuf[0] = 0X100;
            rbuf[1] = max_ymodem_num;
            flash_write(APP_UPDATE_FLAG, rbuf, 2,1);
            FL_RMU_SetSoftReset(RMU);
        }
    } 
    
    if(esp32c5_connect_server() == FALSE)
    {
        system_set_net_err(err_srv);
        goto end_task;	
    }
    else
    {
        system_clear_net_err(err_srv);
    }

    if(esp32c5_get_time() == TRUE)
    {
        wireless_info.upload_set = 1;
        esp32c5_recive_data(5); 
    }

    if(esp32_c5_first == 0)
    {
        if(esp32c5_send_dev_info() == TRUE)
        {
            if(esp32c5_send_param() == TRUE)
            {
                if(get_connect_state() == FALSE)
                {
                    system_set_net_err(err_srv);
                    goto end_task; 
                }

                device_set_updata_cloud(0);
                esp32_c5_first = 1;     
            }
        }
        else
        {
            system_set_net_err(err_srv);
            goto end_task;
        }
    }

    if(config_get_gps_mode() == GPS)
    {
        L76K_get_GPS();
    }
    
    if(esp32c5_mqtt_tcp_send_new_data()==FALSE)
    {
        goto end_task;
    }
    else
    {
        if(get_connect_state() == FALSE)
        {
            system_set_net_err(err_srv);
            goto end_task;
        }
        pend_bak = device_pending_num_get();
        if(pend_bak)
        {
            if(esp32c5_send_his_data(pend_bak) == FALSE)
            {
                system_set_net_err(err_srv);
                goto end_task;                          //历史数据发送
            } 
        }  
    }

    if(device_get_updata_cloude() == 1)
    {
        if(esp32c5_send_param() == TRUE)
        {
            device_set_updata_cloud(0);  
        }
    }
    else 
    {
        if(esp32c5_get_data() == TRUE)
        {
            wireless_info.upload_set = 2;
            esp32c5_recive_data(5);
        }		
    }
    close_esp32c5();
    return;

    end_task:   							//失败
        trace_log("发送失败，数据备份\r\n");
        close_esp32c5();
        pend_bak = device_pending_num_get();
        if(	pend_bak == 0)	//数据备份做断点续传
        {	
            device_perv_addr_set(device_get_flashaddr_addr());
        }
        if(device_pending_num_set(pend_bak+1) == FALSE)
        {
            trace_log("断点保存失败\n");
        }
        if((pend_bak == 0) && (device_perv_addr_get() != device_get_flashaddr_addr()))
        {
            trace_log("断点地址保存失败\n");
        }
        wireless_info.wireless_signal = 0;
        return;
}


/***************************************************************************/
/* Send Modbus TCP response via AT (server mode, CIPMUX=1)                 */
/***************************************************************************/
static uint8_t modbus_tcp_send_response(uint8_t *resp, uint16_t resp_len, uint8_t link_id)
{
    uint8_t re_try = 2;
    while (re_try--)
    {
        sprintf((char *)wireless_buf, "AT+CIPSEND=%d,%d\r\n", link_id, resp_len);
        if (uart_send_ack(WIFI_UART, wireless_buf, wireless_buf, ">", sizeof(wireless_buf), 3000))
        {
            trace_log("Modbus: AT+CIPSEND wait '>' fail\r\n");
            continue;
        }
        if (!uart_transmit_wait_ack(WIFI_UART, resp,resp_len, wireless_buf,sizeof(wireless_buf), "SEND OK", 5000))
        {
            return TRUE;
        }
        else
        {
            trace_log("Modbus: send response fail\r\n");
        }
    }
    return FALSE;
}

/***************************************************************************/
/* TCP Modbus server polling: receive Modbus TCP frame, process, respond   */
/*                                                                         */
/* ESP32-C5 AT firmware data notification (CIPRECVTYPE=0 auto mode):       */
/*   +IPD,<link_id>,<len>:<data>                                          */
/*                                                                         */
/* MCU polls UART every ~50ms via this function.                           */
/* Extracts Modbus TCP ADU, calls modbus_tcp_process(), sends response.    */
/***************************************************************************/
static void esp32c5_tcp_modbus_server_pro(void)
{
    uint16_t size;
    char *getp;
    uint16_t datalen;
    uint8_t link_id = 0;
    uint16_t resp_len;

    size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 200);
    if (size == 0) return;

    if (get_trace_log())
    {
        uart_transmit_buf(USB_UART, wireless_buf, size);
    }

    /*
     * Primary: +IPD,<link_id>,<len>:<data>
     * Standard ESP-AT unsolicited data notification (CIPRECVTYPE=0).
     */
    getp = strstr((char *)wireless_buf, "+IPD,");
    if (getp != NULL)
    {
        getp += 5;                /* skip "+IPD," */
        link_id = atoi(getp);     /* parse link id */
        while (*getp != ',' && *getp != '\0') getp++;
        if (*getp == ',') getp++;
        datalen = atoi(getp);     /* parse data length */
        while (*getp != ':' && *getp != '\0') getp++;
        if (*getp == ':') getp++; /* skip ':' separator, now points to data */

        if (datalen >= 8 && datalen <= 260)
        {
            resp_len = mbs_tcp_poll((uint8_t *)getp, datalen, transmit_cmd);
            if(resp_len)
            {
                modbus_tcp_send_response(transmit_cmd, resp_len, link_id);
            }
        }
    }
}
/**********************************************************************************/
void esp32c5_server_pro(void)
{
    uint32_t event_bit = 0;
	uint32_t size;
    
    trace_log("TCP_MODBUS SERVER\n");

    if(esp32c5_open() == FALSE)
    {		 
        trace_log("模块初始化失败\r\n");
        system_set_net_err(err_srv);
    }
    else
    {
        system_clear_net_err(err_srv);
        trace_log("模块初始化完成\r\n");
		
		if (esp32c5_tcp_modbus_server() == TRUE)
		{
			/* Drain any stale data in UART buffer */
			size = uart_recv_data(WIFI_UART, wireless_buf, sizeof(wireless_buf), 200);
			
            if (get_trace_log())
            {
                uart_transmit_buf(USB_UART, wireless_buf, size);
            }

			while (1)
			{
				esp32c5_tcp_modbus_server_pro();

				if (xTaskNotifyWait(0x00, 0xffffffff, &event_bit, 10) == pdTRUE)
				{
					if (event_bit == STOP_TCP_MODBUS)
					{
						trace_log("TCP_MODBUS stop\r\n");
						uart_send_ack(WIFI_UART, "AT+CIPSERVER=0,1\r\n",wireless_buf, "OK", sizeof(wireless_buf), 500);              
						break;
					}
				}
			}
		}
		else
		{
			trace_log("TCP_MODBUS start fail\n");
		}
    }

    close_connect();
}

void esp32c5_pro(uint32_t event)
{	
	if(event == SEND_DATA)
	{
		esp32c5_client_pro();
	}
    else if(event == START_TCP_MODBUS)
    {
        esp32c5_server_pro();
    }
}

void esp32c5_task(void *params)
{		
    uint32_t event_bit = 0xFF;
	
	if((config_get_net_mode() == 0)&&(config_get_modbus_en() == 1))
    {
		event_bit = START_TCP_MODBUS;
    }

	while(1)
	{
		pwr_opppse_halt(PWR_WIRELESS_BIT);
		wifi_uart_init();
		wifi_en_init();
        open_wifi_en();
        vTaskDelay(1000);
        esp32c5_pro(event_bit);	
		iot_en_deinit();
        uart_enter_sleep_mode(WIFI_UART);
		trace_log("WIFI上报结束\n");
		pwr_arg_halt(PWR_WIRELESS_BIT);		
		xTaskNotifyWait(0x00, 0xffffffff, &event_bit, portMAX_DELAY);	
	}
}
