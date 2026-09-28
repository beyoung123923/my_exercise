#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "Air780E.h"
#include "sensor.h"
#include "hist_storage.h"
#include "pwr_manage.h"
#include "sd3078.h"
#include "usart.h"
#include "tool.h"
#include "server.h"
#include "AT.h"
#include "string.h"
#include "stdio.h"
#include "sd3078.h"
#include "w25qxx.h"
#include "stdlib.h"
#include "sys_setting.h"
#include "l76k.h"
#include "ymodem.h"
#include "flash.h"



uint8_t Air780EP_first = 0;
//static xTaskHandle server_handler;

/*****************************************************************/
void Air780E_GET_ICCID(char *iccid)    // 获取iccid
{
	uint16_t rx_size;
	uint8_t wait_cnt = 4;	
	char *p;	
//	uint8_t iccid_size=0;
 
    while(wait_cnt--)
    {	
        p=0;
        uart_transmit_str(IOT_4G_UART, (uint8_t *)"AT+ICCID\r\n");
        vTaskDelay(30);		
        rx_size = uart_recv_data(IOT_4G_UART, wireless_buf, sizeof(wireless_buf),3000);
        if(rx_size)
        {
            trace_log(wireless_buf);
            if(strstr((char *)wireless_buf, "+ICCID:"))
            {
                p = strstr(( char *)wireless_buf, "+ICCID:" );						
                strncpy(iccid,(char*)(p+8),20);	
                iccid[20] = 0;
                break;
            }		
        }
    }
}
/*****************************************************************/
static uint8_t A78_TEST_IP(void)
{
	uint8_t num = 10;
	while(num--)
	{
		if(!uart_send_ack(IOT_4G_UART, "AT+CIFSR\r\n" , wireless_buf, ".", sizeof(wireless_buf), 300))
			return TRUE;
	}
	return FALSE;
}
/*****************************************************************/
static void A78_SET_CSTT(void)
{
	if(strcmp((char *)config_get_apn(), "") != 0)  //检测到已经设置APN
	{
		sprintf((char *)transmit_cmd, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", config_get_apn(),
			config_get_apn_name(),config_get_apn_pawd());   //set APN number
		uart_send_ack(IOT_4G_UART, transmit_cmd , wireless_buf, "OK", sizeof(wireless_buf), 10000);
	}
	else
	{
		uart_send_ack(IOT_4G_UART, "AT+CSTT\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
	}
}

static uint8_t get_connet_statu(void)
{
    uint8_t *p = NULL;
    int32_t num;
    uint16_t size = 0;
    char *getp;
    uint8_t cont = 5;
	int datalen;

    if(config_get_net_mode() == MQTT_MODE)
    {
        if(uart_send_ack(WIFI_UART,"AT+MQTTSTATU\r\n",wireless_buf,"+MQTTSTATU :",sizeof(wireless_buf),1000) == 0)
        {
            p = wireless_buf;
            while(*p++ != ':');
            num = atoi((char *)p);
            if(num == 1)
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
            //+IPD,1:0x00
            size = uart_recv_data(IOT_4G_UART, wireless_buf, IOT_DATA_LEN, 3000);
            if(size)
            {
                getp = strstr(( char *)wireless_buf, "+IPD");
                if(getp != NULL)
                {
                    if(get_trace_log())
                    { 
                        uart_transmit_buf(USB_UART, wireless_buf, size); 
                    }
                    getp += 5;
                    datalen=atoi((char *)getp);
                    if(datalen == 1)
                    {
                        while(*getp++ != ':');
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
/*****************************************************************/
static uint8_t A78_init(void)
{
    close_4g_en();
	//FL_GPIO_SetOutputPin(pwrio_gpio, pwrio_gpio_id);
	vTaskDelay(1200);
	//FL_GPIO_ResetOutputPin(pwrio_gpio, pwrio_gpio_id);
	open_4g_en();
	vTaskDelay(500);
	if(TEST_AT(IOT_4G_UART, 30))  //失败
	{
		return FALSE;
	}
	else     //读卡
	{
		TEST_ATI(IOT_4G_UART);
        A78_SET_CSTT();
		GET_IMEI(IOT_4G_UART,(char *)wireless_info.imei,"EC20");	
		if(TEST_CPIN(IOT_4G_UART))
		{	
			return FALSE;
		}
	}
	Air780E_GET_ICCID((char *)wireless_info.ccid);

	vTaskDelay(50);
	if(TEST_CEREG(IOT_4G_UART,30))    //AT+CEREG?   检测4G
    { 
        return FALSE;
    }
    uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"CLOSE\"\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
/* 	if(device_info.save.second_mode == ON)
	{
		uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"CLOSE\"\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
	}
	else	
		uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"PSM+\",0,1\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
	 */
	//GET_IMSI((char *)wireless_info.nb_imsi);
    wireless_info.wireless_signal = (uint8_t)GET_CSQ(IOT_4G_UART);
	uart_send_ack(IOT_4G_UART, "AT+CREG=3\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 300);
	uart_send_ack(IOT_4G_UART, "AT+CREG?\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
	
//	GET_CESQ();
	if(TEST_CGATT(IOT_4G_UART,30))
	{
		trace_log("网络附着失败\r\n");
		return FALSE;
	}
//	uart_send_ack(IOT_4G_UART, "AT+CIPSTATUS\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);
//	A78_SET_CSTT();
//	uart_send_ack(IOT_4G_UART, "AT+CIICR\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
//	A78_TEST_IP();
//	uart_send_ack(IOT_4G_UART, "AT+CIPSTATUS\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);
	return TRUE;
}
/*****************************************************************/
/*static void goout_PSM_mode(void)
{
  uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"CLOSE\"\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
  uart_send_ack(IOT_4G_UART, "", wireless_buf, "EXIT PSM+", sizeof(wireless_buf), 500);
}*/
/*****************************************************************/
/* static void goto_PSM_mode(void)
{
  //只有在DTR为高电平模式下，模块才能进入PSM，先设置DTR为高电平
  
//  uart_send_ack(IOT_4G_UART, "AT+ECSIMCFG=\"SimPresenceDetect\",0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+ECPCFG=\"logCtrl\",0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+ECPCFG=\"usbCtrl\",2\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+ECPCFG=\"usbSlpMask\",1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+CNETLIGHT=0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"PSM+\",0,1\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
//  uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"PSM+\"\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);

	uart_send_ack(IOT_4G_UART, "AT+POWERMODE=\"PSM+\"\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
    FL_GPIO_SetOutputPin(psmio_gpio, psmio_gpio_id);  
    trace_log("模块休眠\r\n");
    uart_send_ack(IOT_4G_UART, "", wireless_buf, "ENTER PSM+", sizeof(wireless_buf), 200);
//  TEST_CEREG(30);
} */
/*****************************************************************/
/*static uint8_t A78_GET_GPS(void)
{
	trace_log("GPS定位暂不支持\r\n");
	
	return FALSE;
}*/
/*****************************************************************/
static uint8_t A78_GET_GNSS(void)
{
	uint8_t *gps_buf;
	trace_log("基站定位\r\n");
//	uart_send_ack(IOT_4G_UART, "AT+GSMLOCFG=\"bs.openluat.com\",12411\r\n", wireless_buf, "OK", sizeof(wireless_buf), 500);
	uart_send_ack(IOT_4G_UART, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
	uart_send_ack(IOT_4G_UART, "AT+SAPBR=3,1,\"APN\",\"\"\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
	uart_send_ack(IOT_4G_UART, "AT+SAPBR=1,1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
	uart_send_ack(IOT_4G_UART, "AT+SAPBR=2,1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);

/*+CIPGSMLOC:
 0,034.7983328,114.3214505,2023/06/05,14:38:50
 OK	*/
	if(uart_send_ack(IOT_4G_UART, "AT+CIPGSMLOC=1,1\r\n", wireless_buf, "+CIPGSMLOC:", sizeof(wireless_buf), 30000) == 0)
	{
		gps_buf = (uint8_t *)strstr((char *)wireless_buf, "+CIPGSMLOC:");
        if(gps_buf == NULL)
        {
            return FALSE;
        }
		while(*gps_buf!='\0' && *gps_buf++ != ',');
		wireless_info.lat = atof((char *)gps_buf);
		while(*gps_buf!='\0' && *gps_buf++ != ',');
		wireless_info.lng = atof((char *)gps_buf);
		//device_info.gps_satues = 1;
		sprintf((char *)transmit_cmd,"GPS:%.4f,%.4f\r\n",wireless_info.lng,wireless_info.lat);
		trace_log(transmit_cmd);
	}
	uart_send_ack(IOT_4G_UART, "AT+SAPBR=0,1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
	return TRUE;
}
/*****************************************************************/
static void A78_GPS_work(void)

{
//	A78_GET_GNSS();
//	return;
  if(config_get_gps_mode()) 
  {
   // device_info.gps_en = 0;
    if(config_get_gps_mode() == GPS)
      L76K_get_GPS();
    else if(config_get_gps_mode() == AGPS)
      A78_GET_GNSS();
/*     else if(device_info.save.gps_mode == GPS_GNSS)
    {
      if(A78_GET_GPS() != TRUE)
        A78_GET_GNSS();
    } */
  }
  else
    return;
}
/*****************************************************************/
static uint8_t A78_TCP_UDP_MQTT_online_ack(void)
{	
	uint8_t wait_sec = 8;
	uint16_t size = 0;
	uint8_t reack=FALSE;
	while(wait_sec--)
	{
		size = uart_recv_data(IOT_4G_UART, wireless_buf, IOT_DATA_LEN, 3000);
		if(size)
		{
			trace_log((uint8_t *)wireless_buf); ////////***打印串口1数据******///
			if(strstr((char *)wireless_buf, "FAIL")||strstr((char *)wireless_buf, "ERROR"))
			{
				reack=FALSE;
				break;
			}
			else if(strstr((char *)wireless_buf, "CONNECT"))
			{
				reack=TRUE;
				break;
			}		
		}
	}
	return reack;
}
/*****************************************************************/
static void A78_TCP_UDP_close(void)
{
	uart_send_ack(IOT_4G_UART, "AT+CIPCLOSE\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);//断开TCP_UDP连接
}
/*****************************************************/
//mod_buf   TCP/UDP
/*<access_mode> 整型。Socket 服务的数据访问模式。
 0 普通模式
 1 透传模式*/
static uint8_t A78_TCP_UDP_connect(char *mode_buf,char *ip_buf,char *port_buf,uint8_t con_id,uint8_t access_mode)
{
  uint8_t res = FALSE;
  
	uart_send_ack(IOT_4G_UART, "AT+CIPSHUT\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);
	uart_send_ack(IOT_4G_UART, "AT+CIPSTATUS\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);
	uart_send_ack(IOT_4G_UART, "AT+CIPMUX=0\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);//设置TCP为单链接模式   缺省模式
	sprintf((char *)wireless_buf,"AT+CIPMODE=%d\r\n",access_mode);
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", sizeof(wireless_buf), 300);//设置TCP为普通传输模式	
	
	A78_SET_CSTT();
	uart_send_ack(IOT_4G_UART, "AT+CIICR\r\n" , wireless_buf, "OK", sizeof(wireless_buf), 1000);
	A78_TEST_IP();
	uart_send_ack(IOT_4G_UART, "AT+CIPSTATUS\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);
	
	sprintf((char *)wireless_buf,"AT+CIPSTART=\"%s\",\"%s\",%s\r\n",mode_buf,ip_buf,port_buf);
	uart_transmit_str(IOT_4G_UART,wireless_buf);
	res = A78_TCP_UDP_MQTT_online_ack();

	if(res == TRUE)
	{
		trace_log("TCP/UDP连接成功\r\n");
		return TRUE;
	}
	else
	{
		trace_log("TCP/UDP连接失败,重连\r\n");
		A78_TCP_UDP_close();
		sprintf((char *)wireless_buf,"AT+CIPSTART=\"%s\",\"%s\",%s\r\n",mode_buf,ip_buf,port_buf);
		uart_transmit_str(IOT_4G_UART,wireless_buf);
		res = A78_TCP_UDP_MQTT_online_ack();
		if(res == TRUE)
		{
			trace_log("TCP/UDP连接成功\r\n");
			return TRUE;
		}
		else
			trace_log("TCP/UDP连接失败\r\n");			
	}
	
	return res;
}
/*****************************************************************/
static uint8_t A78_TCP_UDP_send_data(uint8_t con_id,uint8_t *tx_buf,uint16_t tx_size)    //指令模式下发送数据
{ 
  if(tx_size == 0)
  {
    tx_size = strlen((char *)tx_buf);
    if(tx_size == 0)
      return FALSE;
  }
  
  sprintf((char *)wireless_buf, "AT+CIPSEND=%d\r\n",tx_size);	
  if(uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, ">", sizeof(wireless_buf),5000))
  {
    trace_log("发送收不到'>'\r\n");
    return FALSE;
  }
  else
  {
    uart_transmit_buf(IOT_4G_UART , tx_buf ,tx_size);
    if(!uart_send_ack(IOT_4G_UART, "", wireless_buf, "SEND OK", sizeof(wireless_buf),5000))
    {
      trace_log("发送成功\r\n");
      return TRUE;
    }
    else
    {
      trace_log("发送失败\r\n");
      return FALSE;
    }
  }
}
/***************************************************************************/
static uint8_t A78_out_touchuan(void)  //退出透传模式
{
  uint8_t num=3;
  uint8_t re=FALSE;
  
  vTaskDelay(1000);

  while(num--)
  {
    if(!uart_send_ack(IOT_4G_UART, "+++", transmit_cmd, "OK", sizeof(transmit_cmd),800))
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
/*****************************************************************/
static uint8_t A78_OTA_ymodem(void)
{
  uint8_t res = FALSE;
  
	trace_log("OTA升级服务检测\r\n");
  res = A78_TCP_UDP_connect((char *)"TCP",(char *)"112.74.98.39",(char *)"8089",0,1);    //设置模块在透传模式下传输数据
  if(res == FALSE)
  {
    trace_log("OTA升级服务器连接失败\r\n");
//    out_touchuan();
    A78_TCP_UDP_close();
    return res;
  }
  
  res = FALSE;
  if(get_OTA_Version(IOT_4G_UART) == TRUE)
  {
    if(ymodem_get_data(IOT_4G_UART)== TRUE)
      res = TRUE;
    else
      res = FALSE;
  }
  
  trace_log("退出OTA升级任务\r\n");
  if(uart_send_ack(IOT_4G_UART, "AT+CIPSTATUS\r\n", transmit_cmd, "TCP CLOSED", sizeof(transmit_cmd),800) == 0)
  {
	res = FALSE;
	return res;
  }  
  
  A78_out_touchuan();
  A78_TCP_UDP_close();
  
  return res;
}
/*****************************************************************/
static void A78_MQTT_CLOSE(void)
{
	sprintf((char *)wireless_buf, "AT+MDISCONNECT\r\n");	   //
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
	sprintf((char *)wireless_buf, "AT+MIPCLOSE\r\n");	   //
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
}
/*****************************************************************/
static uint8_t A78_MQTT_CONNECT_ack(void)
{	
	uint8_t wait_sec = 8;
	uint16_t size = 0;
	uint8_t reack=FALSE;
	while(wait_sec--)
	{
		size = uart_recv_data(IOT_4G_UART, wireless_buf, IOT_DATA_LEN, 3000);
		if(size)
		{
			trace_log((uint8_t *)wireless_buf); ////////***打印串口1数据******///
			if(strstr((char *)wireless_buf, "FAIL")||strstr((char *)wireless_buf, "ERROR"))
			{
				trace_log("链接异常\r\n");
				reack=FALSE;
				break;
			}
			else if(strstr((char *)wireless_buf, "CLOSED"))
			{
				trace_log("服务器主动断开\r\n");
				reack=FALSE;
				break;
			}
			else if(strstr((char *)wireless_buf, "CONNACK OK"))
			{
				reack=TRUE;
				break;
			}		
		}
	}
	return reack;
}
/*****************************************************************/
static uint8_t A78_MQTT_CONNECT_LINE(void)
{
	uint8_t res;
//	uart_send_ack(IOT_4G_UART, "AT+CIPSHUT\r\n", wireless_buf, "OK", IOT_DATA_LEN,300);
		
	sprintf((char *)wireless_buf, "AT+MQTTMSGSET=1\r\n");	   //配置MQTT接收模式为缓存模式
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
	
	//设置MQTT登录信息
/* 	if(device_info.LP_ZRDZ_TJZD == 1)
	{
		sprintf((char *)wireless_buf, "AT+MCONFIG=\"%.13s\",\"%s\",\"%s\"\r\n",device_info.save.srv_login,device_info.save.srv_UserName,device_info.save.srv_Password);
	}
	else */
		sprintf((char *)wireless_buf, "AT+MCONFIG=\"%s\",\"%s\",\"%s\"\r\n",config_get_mqtt_client_id(),config_get_mqtt_user_name(),config_get_mqtt_user_pawd());	   //设置MQTT参数
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
	
	//设置TCP连接信息
	sprintf((char *)wireless_buf, "AT+MIPSTART=\"%s\",\"%s\"\r\n",config_get_server_ip(),config_get_server_port());   //打开TCP链接
	uart_transmit_str(IOT_4G_UART,wireless_buf);
	res = A78_TCP_UDP_MQTT_online_ack();
	if(res == TRUE)
	{
		trace_log("服务器连接成功\r\n");
	}
	else
	{
		trace_log("服务器连接失败\r\n");
		return FALSE;
	}
	
	//设置MQTT保活时间
	sprintf((char *)wireless_buf, "AT+MCONNECT=1,50\r\n"); 
	uart_transmit_str(IOT_4G_UART,wireless_buf); 
	res = A78_MQTT_CONNECT_ack();	
	if(res == TRUE)
	{
		trace_log("MQTT登录成功\r\n");
	}
	else
	{
		trace_log("MQTT登录失败\r\n");
		return  FALSE;
	}
	
	
	sprintf((char *)wireless_buf, "AT+MSUB=\"%s\",1\r\n",config_get_mqtt_subs_topic());   //MQTT订阅
//	if(uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "SUBACK", IOT_DATA_LEN,2000) == FALSE)
	uart_transmit_str(IOT_4G_UART,wireless_buf);
	if(check_recv2(IOT_4G_UART, wireless_buf,"SUBACK", "ERROR",sizeof(wireless_buf),5000) != 0)
	{
		trace_log("MQTT订阅失败\r\n");
	}
	else
		trace_log("MQTT订阅成功\r\n");
	
	return TRUE;
}
/*****************************************************************/
/*static uint8_t A78_MQTT_TLINK_CONNECT_LINE(void)
{
	uint8_t res;
//	uart_send_ack(IOT_4G_UART, "AT+CIPSHUT\r\n", wireless_buf, "OK", IOT_DATA_LEN,300);
		
	sprintf((char *)wireless_buf, "AT+MQTTMSGSET=1\r\n");	   //配置MQTT接收模式为缓存模式
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
	
	//设置MQTT登录信息
	sprintf((char *)wireless_buf, "AT+MCONFIG=\"%s\",\"MQTT_MODE\",\"MQTTPW\"\r\n",config_get_mqtt_client_id());	   //设置MQTT参数
	uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "OK", IOT_DATA_LEN,300);
	
	//设置TCP连接信息
	sprintf((char *)wireless_buf, "AT+MIPSTART=\"1883.dtuip.com\",\"1883\"\r\n");   //打开TCP链接
	uart_transmit_str(IOT_4G_UART,wireless_buf);
	res = A78_TCP_UDP_MQTT_online_ack();
	if(res == TRUE)
	{
		trace_log("服务器连接成功\r\n");
	}
	else
	{
		trace_log("服务器连接失败\r\n");
		return FALSE;
	}
	
	//设置MQTT保活时间
	sprintf((char *)wireless_buf, "AT+MCONNECT=1,50\r\n"); 
	uart_transmit_str(IOT_4G_UART,wireless_buf); 
	res = A78_MQTT_CONNECT_ack();	
	if(res == TRUE)
	{
		trace_log("MQTT登录成功\r\n");
	}
	else
	{
		trace_log("MQTT登录失败\r\n");
		return  FALSE;
	}
	
	sprintf((char *)wireless_buf, "AT+MSUB=\"%s/+\",1\r\n",config_get_mqtt_subs_topic());   //MQTT订阅
//	if(uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, "SUBACK", IOT_DATA_LEN,2000) == FALSE
	uart_transmit_str(IOT_4G_UART,wireless_buf);
	if(check_recv2(IOT_4G_UART, wireless_buf,"SUBACK", "ERROR",sizeof(wireless_buf),5000) != 0)
	{
		trace_log("MQTT订阅失败\r\n");
	}
	else
		trace_log("MQTT订阅成功\r\n");
	
	return TRUE;
}*/
/*****************************************************************/
static uint8_t A78_MQTT_send_data(uint8_t *sub_topic,uint16_t len,uint8_t *buf)
{
	uint8_t retry = 3;
	uint8_t re = FALSE;

	while(retry--)    //重复3次
	{
		sprintf((char *)wireless_buf, "AT+MPUBEX=\"%s\",1,0,\"%d\"\r\n",sub_topic,len);
		if(uart_send_ack(IOT_4G_UART, wireless_buf, wireless_buf, ">", sizeof(wireless_buf),5000) == FALSE)  
		{
			trace_log("发送收不到'>'\r\n");
			re = FALSE;
			A78_MQTT_CLOSE();
			if(A78_MQTT_CONNECT_LINE() == TRUE)
				continue;
			else
				return FALSE;
		}
		else
		{
			uart_transmit_str(IOT_4G_UART,buf);
//			vTaskDelay(100);
			if(check_recv2(IOT_4G_UART, wireless_buf,"PUBACK", "ERROR",sizeof(wireless_buf),9000) != 0)
			{
				trace_log("数据发送失败\r\n");
				re = FALSE;
				A78_MQTT_CLOSE();
				if(A78_MQTT_CONNECT_LINE() == TRUE)
					continue;
				else
					return FALSE;
			}
			else
			{
				trace_log("数据发送成功\r\n");
                re = TRUE;
                break;
			}
		}
	}
	return re;
}
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*****************************************************************/
/*static uint8_t A78_MQTT_connect_tlink_server(void)
{
	uint8_t num = 3;
	uint8_t re= FALSE;

	trace_log("MQTT连接Tlink平台\r\n");
	while(num--)
	{
		if(A78_MQTT_TLINK_CONNECT_LINE()==FALSE)    //连接服务器 
		{
			trace_log("平台重连\r\n");
			A78_MQTT_CLOSE();	  //关闭socket服务
			vTaskDelay(500);
		}
		else
		{
			re = TRUE;
			break;
		}
	}
	
	if(re == TRUE)
	{trace_log("平台连接成功\r\n");}
	else
	{trace_log("平台连接失败\r\n");}
	return re;
}*/
/*****************************************************/
static uint8_t A78_connect_server(void)
{
	uint8_t num = 3;
	uint8_t re= FALSE;
	if(config_get_net_mode() == MQTT_MODE) 
	{
		trace_log("MQTT连接\r\n");
		while(num--)
		{
			if(A78_MQTT_CONNECT_LINE()==FALSE)    //连接服务器 
			{
				trace_log("平台重连\r\n");
				A78_MQTT_CLOSE();	               //关闭socket服务
				vTaskDelay(500);
			}
			else
			{
				re = TRUE;
				break;
			}
		}
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		trace_log("TCP连接\r\n");
		while(num--)
		{
			if(A78_TCP_UDP_connect("TCP",(char *)config_get_server_ip(),(char *)config_get_server_port(),0,0) == FALSE)
			{
				trace_log("平台连接失败\r\n");
				A78_TCP_UDP_close();
				re = FALSE;
				vTaskDelay(500);
			}
			else
			{
				re = TRUE;
				break;
			}
		}
		if(A78_TCP_UDP_send_data(0,config_get_mqtt_post_topic(),strlen((char *)config_get_mqtt_post_topic())) == FALSE)
			re = FALSE;
		else
			re = TRUE;
	}
	else
		re = FALSE;
	
	if(re == TRUE)
	{trace_log("平台连接成功\r\n");}
	else
	{trace_log("平台连接失败\r\n");}
	return re;
}
/*****************************************************************/
static uint8_t A78_Tink_getParam(void)    
{
	uint8_t re;
  uint16_t len = 0;

	uart_send_ack(IOT_4G_UART, "AT+CIPHEAD=1\r\n", transmit_cmd, "OK", sizeof(transmit_cmd),800);   //设置接收数据时，有数据头
	sprintf((char *)transmit_cmd, "getParam");
	len = strlen((char *)transmit_cmd);
	if(config_get_net_mode() == MQTT_MODE)
	{
		re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);
	}
	else
		re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
	
	return re; 
}
/*****************************************************************/
/*static uint8_t A78_modeldata_params(void)
{
  uint8_t re = FALSE;
	uint16_t len;
   //模块信息和设备版本号	
	//len = modeldata_params((char *)transmit_cmd);  //上报数据格式与NB模式非AEP一致
	if(config_get_net_mode() == MQTT_MODE)
	{
		re = A78_MQTT_send_data(device_info.save.srv_Topic,len,transmit_cmd);	
	}
	else if(config_get_net_mode() == TCP) 
		re = A78_TCP_UDP_send_data(0,transmit_cmd,len);	
	return re;
}*/
static uint8_t A78_send_dev_info(void)  //设备信息
{
 	uint8_t re = TRUE;
	uint16_t len;   
    
    dev_info_to_buf(transmit_cmd);
    len = strlen((char *)transmit_cmd);
    if(config_get_net_mode() == MQTT_MODE)
    {
        re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);		
    }
    else if(config_get_net_mode() == TCP_MODE) 
        re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
    
	//len = setting_params((char *)transmit_cmd);    //上报数据格式与NB模式非AEP一致
	return re;
}

/*****************************************************************/
static uint8_t A78_send_Status(void)   //参数同步
{
	uint8_t re = TRUE;
	uint16_t len;

    system_format_cloud_config((char *)transmit_cmd);
    len = strlen((char *)transmit_cmd);
    if(config_get_net_mode() == MQTT_MODE)
    {
        re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);		
    }
    else if(config_get_net_mode() == TCP_MODE) 
        re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
    
	//len = setting_params((char *)transmit_cmd);    //上报数据格式与NB模式非AEP一致
	return re;
}

static uint8_t wireless_get_tlink_time(void)
{
	uint8_t re;
	uint16_t len = 0;

	uart_send_ack(IOT_4G_UART, "AT+CIPHEAD=1\r\n", transmit_cmd, "OK", sizeof(transmit_cmd),800);   //设置接收数据时，有数据头
	sprintf((char *)transmit_cmd, "getTime");
	len = strlen((char *)transmit_cmd);
	if(config_get_net_mode() == MQTT_MODE)
	{
		re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);
	}
	else
		re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
	
	return re; 	
}

static uint16_t A78_MQTT_TCP_recv_data(uint8_t wait_tick)
{	
	uint16_t size=0;
	uint8_t *post = NULL;
	char *getp;
	uint16_t datalen=0;
	uint16_t re_state = 0;
	uint16_t p = 0;
	
	post = config_get_mqtt_post_topic();
	trace_log("检测远程配置\r\n");	
	if(config_get_net_mode() == MQTT_MODE)
	{
		/*      +MSUB: 0
AT+MQTTMSGGET

+MSUB: "WPU8Y57GX67044MW/123",10 byte,{"msg":""}

OK*/
		//+MSUB: "SS6XOGKPC65SC522/123",19 byte,2026-05-18 10:03:50
		//+MSUB: "SS6XOGKPC65SC522/123",573 byte,{"记录间隔(分)":"2","内部传感器":"1","外置传感器":"0","通道1斜率修正":"1.00","通道2斜率修正":"1.00","通道3斜率修正":"1.00","通道4斜率修正":"1.00","通道1常量修正":"0.00","通道2常量修正":"0.00","通道3常量修正":"0.00","通道4常量修正":"0.00","报警开关":"1","通道1上限":"70.00","通道2上限":"90.00","通道3上限":"70.00","通道4上限":"90.00","通道1下限":"-40.00","通道2下限":"0.00","通道3下限":"-40.00","通道4下限":"0.00","通道1回差":"2.00","通道2回差":"2.00","通道3回差":"2.00","通道4回差":"2.00","通道1波动":"100.00","通道2波动":"100.00","通道3波动":"100.00","通道4波动":"100.00"}
		while(wait_tick--)
		{
//			uart_transmit_str(IOT_4G_UART,(uint8_t *)"AT+MQTTMSGGET\r\n");
//			size = uart_recv_data(IOT_4G_UART, wireless_buf, sizeof(wireless_buf), 300);
//			if(size)
			if(uart_send_ack(IOT_4G_UART,"AT+MQTTMSGGET\r\n", wireless_buf, "+MSUB:", sizeof(wireless_buf), 3000) == TRUE)
            {
				if(get_trace_log())
				{
					uart_transmit_buf(USB_UART, wireless_buf, size);
				}
                getp =strstr(( char *)wireless_buf, "+MSUB:");
				if(getp != NULL)
				{
					if(strstr((char*)wireless_buf,(const char*)post))
					{
						while(*getp++ != ',');
						datalen=atoi((char *)getp);
						if(datalen>5)
						{
							size = strlen((char*)wireless_buf);
							while(*getp++ != ',');
							trace_log("收到下行指令\r\n");
							re_state = 1;
							break;
						}
					}
				}
            }
            else
            {
                trace_log("没有收到应答\r\n");
            }
            vTaskDelay(1000);
        }
	}
	else   //TCP  UDP  接收数据，没有缓存模式，收到就会直接输出
	{
		//+IPD,19:2026-05-18 09:58:57
		//+IPD,573:{"记录间隔(分)":"2","内部传感器":"1","外置传感器":"0","通道1斜率修正":"1.00","通道2斜率修正":"1.00","通道3斜率修正":"1.00","通道4斜率修正":"1.00","通道1常量修正":"0.00","通道2常量修正":"0.00","通道3常量修正":"0.00","通道4常量修正":"0.00","报警开关":"1","通道1上限":"70.00","通道2上限":"90.00","通道3上限":"70.00","通道4上限":"90.00","通道1下限":"-40.00","通道2下限":"0.00","通道3下限":"-40.00","通道4下限":"0.00","通道1回差":"2.00","通道2回差":"2.00","通道3回差":"2.00","通道4回差":"2.00","通道1波动":"100.00","通道2波动":"100.00","通道3波动":"100.00","通道4波动":"100.00"}
		while(wait_tick--)
		{		
			size = uart_recv_data(IOT_4G_UART, wireless_buf, IOT_DATA_LEN, 3000);
			if(size)
			{
				getp = strstr(( char *)wireless_buf, "+IPD");
				if(getp != NULL)
				{
					if(get_trace_log())
					{ 
						uart_transmit_buf(USB_UART, wireless_buf, size); 
					}
					getp += 5;
					datalen=atoi((char *)getp);
					if(datalen >=5)
					{
						while(*getp++ != ':');
						trace_log("收到下行指令\r\n");
						re_state = 1;
						break;
					}
				}										
			}
            vTaskDelay(1000);
		}
	}
	if(re_state)
	{
		if(wireless_info.upload_set == 1)
		{
			trace_log("同步时间\r\n");	
			p = strstraddr((uint8_t *)getp, (uint8_t*)"20",size-(getp-(char*)wireless_buf));
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
			if((strchr((char *)wireless_buf,'{'))&&(strrchr((char *)wireless_buf,'}')))	
			{
				if((strrchr((char *)wireless_buf,'}') - strchr((char *)wireless_buf,'{')) == (datalen-1))
				{
					while(p<(size-(getp-(char *)wireless_buf)))
					{
						if(*(getp+p) == '{')
						{
							break;
						}
						p++;
					}
					
					if(p>=size)
					{
						trace_log("同步数据错误\r\n");
					}
					else
					{
						if(system_set_cloud_config((uint8_t *)(getp+p)) == TRUE)
						{
							A78_send_Status();
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
	return size;
}

/*****************************************************************/
static uint8_t A78_send_new_Sensordata(void)   //实时数据上传
{
	uint8_t re = FALSE;	
	uint16_t len;

	len = set_new_data_to_buf();
	if(config_get_net_mode() == MQTT_MODE)
	{		
		re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);
	}
	else if(config_get_net_mode() == TCP_MODE)
	{
		re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
	}
	return re;
}
/*****************************************************************/
static uint8_t A78_send_his_data(uint32_t pend_num)
{
	uint8_t re = FALSE;
	uint16_t len;
    uint32_t addr = device_perv_addr_get();

	while(pend_num)
	{
		sprintf((char *)transmit_cmd,"%d\r\n",pend_num);
		trace_log("待补发历史总数:");
        trace_log(transmit_cmd);

		vTaskDelay(1200);
        w25qxx_read((uint8_t *)&hist_data2, addr ,sizeof(hist_storage_t));
		//hist_read_onebuf(&hist_data2, device_info.pend_addr);    //读取历史
		addr = hist_data2.next_addr;   //地址移位
		if(hist_data2.data_time.year==255 && hist_data2.data_time.month==255)
		{
			trace_log("数据读取有错误\r\n");
            pend_num = 0;

            device_pending_num_set(pend_num);
			//device_info.pend_num = 0;	 
			//device_pending_num_set();
			break;
		} 
		
		len = newdata_to_buf(transmit_cmd);
		if(config_get_net_mode() == MQTT_MODE)
		{
			//len = newdata_to_buf(transmit_cmd);
			re = A78_MQTT_send_data(config_get_mqtt_post_topic(),len,transmit_cmd);
		}
		else if(config_get_net_mode() == TCP_MODE)
		{
			//len = tcp_udp_newdata_to_buf(transmit_cmd);
			re = A78_TCP_UDP_send_data(0,transmit_cmd,len);
		}

        if(re == FALSE)
        {
            return re;
        }
        else
        {
            re = get_connet_statu();
            if(re == FALSE)
            {
                system_set_net_err(err_srv);
                return re;
            }
            device_perv_addr_set(addr);   //发送成功，保存断点日志
        }
		
		if(pend_num>=1)
		{
			pend_num = pend_num - 1;
			device_pending_num_set(pend_num);
		}
	}	
	return re;
}
/*****************************************************************/
/*static uint8_t A78_send_ZRDZ_his_data(void)
{
	uint8_t re = FALSE;
	uint16_t len;
	uint8_t topic_buf[50];

	if((device_info.sensor_data.model_status&flash_err) != flash_err && device_info.save.history_mode == ON)
	{
		device_info.pend_num=device_pending_num_get();
		device_info.pend_addr=device_perv_addr_get();
		sprintf((char *)transmit_cmd,"%d\r\n",device_info.pend_num);
		trace_log("待补发历史总数:");
		trace_log(transmit_cmd);
	}
	else
	{
		trace_log("flash存储错误或者关闭历史补发，不发历史数据\r\n");
		return re;
	}
	
	sprintf((char *)topic_buf,"/api/v1/%.13s/mqtt/dataHistory",device_info.save.srv_Topic);
	while(device_info.pend_num)
	{
		trace_log("历史补发\r\n");
		vTaskDelay(1000);
		hist_read_onebuf(&hist_data2, device_info.pend_addr);    //读取历史
		device_info.pend_addr = hist_data2.next;   //地址移位
		if(hist_data2.data_time.year==255 && hist_data2.data_time.month==255)
		{
			trace_log("数据读取有错误\r\n");
			device_info.pend_num = 0;	 
			device_pending_num_set();
			break;
		}
		
		sprintf((char *)transmit_cmd,"{\"param\":[{\"deviceKey\":\"%.13s-01\",\"key\":\"LY01\",\"value\":\"%.3f\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"},"   //进口压力
		"{\"deviceKey\":\"%.13s-01\",\"key\":\"LY02\",\"value\":\"%.3f\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"},"  //进口温度
		"{\"deviceKey\":\"%.13s-01\",\"key\":\"LY03\",\"value\":\"%.3f\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"},"  //电池电压
		"{\"deviceKey\":\"%.13s-01\",\"key\":\"LY04\",\"value\":\"%d\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"},"  //压力探头状态
		"{\"deviceKey\":\"%.13s-01\",\"key\":\"LY08\",\"value\":\"%.3f\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"},"  //出口压力
		"{\"deviceKey\":\"%.13s-01\",\"key\":\"LY11\",\"value\":\"%.3f\",\"ts\":\"20%02d-%02d-%02d %02d:%02d:%02d\"}"  //出口温度
		"]}",  
		device_info.save.srv_login,hist_data2.hist_sensor_data.data4,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec,
		device_info.save.srv_login,hist_data2.hist_sensor_data.data3,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec,
		device_info.save.srv_login,hist_data2.hist_vvlue/1000,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec,
		device_info.save.srv_login,hist_data2.hist_sensor_data.model_status&0x01,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec,
		device_info.save.srv_login,hist_data2.hist_sensor_data.data2,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec,
		device_info.save.srv_login,hist_data2.hist_sensor_data.data1,
		hist_data2.data_time.year,hist_data2.data_time.month,hist_data2.data_time.day,
		hist_data2.data_time.hour,hist_data2.data_time.min,hist_data2.data_time.sec
		);

		len = strlen((char *)transmit_cmd);
		re = A78_MQTT_send_data(topic_buf,len,transmit_cmd);
			
		if(re == FALSE)
		{
			return re;
		}
		else
		{
			device_perv_addr_set();   //发送成功，保存断点日志
		}
		
		if(device_info.pend_num>=1)
		{
			device_info.pend_num = device_info.pend_num - 1;
			device_pending_num_set();
		}
	}	
	return re;
}*/
/*****************************************************************/
static void A78_close_connect(void)
{
	if(config_get_net_mode() == MQTT_MODE)
		A78_MQTT_CLOSE();
	else
		A78_TCP_UDP_close();
}
/*****************************************************************/
/*+CCLK:"18/05/16,15:49:28+32"
 OK*/

#if 0
static void A78_time_clb_work(void)
{
	uint8_t wait_sec,size;
	char *ackp;
	uint8_t atod = 0;
//	device_info.time_calib_en = 1;
	if(device_info.time_calib_en == 1)
	{
		device_info.time_calib_en = 0;
		uart_send_ack(IOT_4G_UART, "AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
		uart_send_ack(IOT_4G_UART, "AT+SAPBR=3,1,\"APN\",\"\"\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
		uart_send_ack(IOT_4G_UART, "AT+SAPBR=1,1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
		uart_send_ack(IOT_4G_UART, "AT+CNTPCID=1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
		wait_sec = 3;
		while(wait_sec--)
		{
			uart_send_ack(IOT_4G_UART, "AT+CNTP\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
			size = uart_recv_data(IOT_4G_UART, wireless_buf, IOT_DATA_LEN, 500);
			if(size)
			{
				trace_log((uint8_t *)wireless_buf); ////////***打印串口1数据******///
				if(strstr((char *)wireless_buf, "+CNTP:") != NULL)
				{
					ackp = strstr((char *)wireless_buf, "+CNTP:");
					ackp += 7;
					size = atoi(ackp);
					if(size == 1)
					{
						trace_log("网络时间同步成功\r\n");
						break;
					}
					else
					{
						trace_log("校时异常\r\n");
						return;
					}
				}		
			}
		}

		wait_sec = 3;
		while(wait_sec--)
		{
			if(!uart_send_ack(IOT_4G_UART, "AT+CCLK?\r\n", wireless_buf, "OK", sizeof(wireless_buf),500)) 
			{
				if(strstr((char *)wireless_buf, "+CCLK:") != NULL)
				{
					trace_log("获取基站校时消息\r\n");
					ackp =strstr(( char *)wireless_buf, "+CCLK:");
					
					while(*ackp++ != '"');
					atod = atoi((char *)ackp);
					
					if(atod<25 || atod>45)   //判断年份不合法，停止更新时间
						continue;
					else
					 device_info.data_time.year = atod;
					
					while(*ackp++ != '/');
					atod = atoi((char *)ackp);
					if(atod != 0)
						device_info.data_time.month = atod;
					while(*ackp++ != '/');
					atod = atoi((char *)ackp);
					if(atod != 0)
						device_info.data_time.day = atod;
					while(*ackp++ != ',');
					device_info.data_time.hour = atoi((char *)ackp);
					while(*ackp++ != ':');
					device_info.data_time.min = atoi((char *)ackp);
					while(*ackp++ != ':');
					device_info.data_time.sec = atoi((char *)ackp);
								
					uint64_t timestamp = 0;
					timestamp = time_2_time_stamp(&device_info.data_time);
					stamp_2_time(timestamp, &device_info.data_time);
//					stamp_2_time(timestamp + 28800, &device_info.data_time);
					RTC_WriteDate(&device_info.data_time);
					break;
				}
			}
		}
		uart_send_ack(IOT_4G_UART, "AT+SAPBR=0,1\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
	}	
}
#endif

/*****************************************************************/
static void A78_clode(void)
{
	A78_close_connect();
	//SET_QPOWD(IOT_4G_UART);
	close_4g_en();
}
/*****************************************************************/
/*static void A78_RESTART(void)
{
	uart_send_ack(IOT_4G_UART, "AT+RESET\r\n", wireless_buf, "OK", sizeof(wireless_buf),300);
}*/
/*****************************************************************/
/*****************************************************************/


void A78_PSM_task(uint32_t event)
{
	uint8_t ota_en=FALSE;
	uint32_t rbuf[2];
    uint8_t config_state = 0;
	uint32_t pend_bak;

	//device_info.event_again = 0;
	if(event == SEND_DATA)	
	{
		//PCIE_PW_OPEN();           //模块上电
		//trace_log("MQTT PSM task\r\n");

		memset(transmit_cmd,0,sizeof(transmit_cmd));
		memset(wireless_buf,0,sizeof(wireless_buf));

		//nb.device_restart_num++;  //记录重启次数
		//device_restart_num_set(nb.device_restart_num);
			
		vTaskDelay(200);
		if(A78_init() == TRUE)
		{
			trace_log("初始化完成\r\n");
			system_clear_net_err(err_net);
		}
		else
		{
			trace_log("初始化失败\r\n");
			system_set_net_err(err_net);
			goto end_A78;
		}
			
		//A78_time_clb_work();    //校时任务
	
		//先检测OTA  
		if(((one_day_cont >= 24)||(Air780EP_first == 0))&&(config_get_ota_en()))
		{
			one_day_cont = 0;
			ota_en = A78_OTA_ymodem();  
			if(ota_en == TRUE)
			{
				trace_log("OTA下载成功，设备升级\r\n");
				rbuf[0] = 0X100;
				rbuf[1] = max_ymodem_num;
				flash_write(APP_UPDATE_FLAG, rbuf, 2,1);
				FL_RMU_SetSoftReset(RMU);
			}
		} 

		if(A78_connect_server() == FALSE)
		{
			system_set_net_err(err_srv);
			goto end_A78;
		}
		else
		{
			system_clear_net_err(err_srv);
		}

        if(wireless_get_tlink_time() == TRUE)
        {
            wireless_info.upload_set = 1;
            A78_MQTT_TCP_recv_data(5);    
        }    
        if(Air780EP_first == 0)
        {
            if(A78_send_dev_info() == TRUE)
            {
                if(A78_send_Status() == TRUE)
                {
                    Air780EP_first = 1;
                    device_set_updata_cloud(0);
                }
            }
        }
        
		A78_GPS_work();                          //GPS工作任务
		
		if(A78_send_new_Sensordata() == FALSE)   //先发送实时数据
        {
            goto end_A78;
        }
		else
        {
            if(get_connet_statu() == FALSE)
            {
                system_set_net_err(err_srv);
                goto end_A78;
            }
            pend_bak = device_pending_num_get();
            if(pend_bak)
            {
                if( A78_send_his_data(pend_bak) == FALSE)
                {
                    system_set_net_err(err_srv);
                    goto end_A78;                
                }
            }
        }

        config_state = device_get_updata_cloude();
		if(config_state == 1)
		{
			if(A78_send_Status() == TRUE)
			{
				device_set_updata_cloud(0);
			}
		}
		else 
		{
			if(A78_Tink_getParam() == TRUE)
			{
				wireless_info.upload_set = 2;			
				A78_MQTT_TCP_recv_data(5);
			}    
		}

		/*if(device_info.Params_sever != 2)
		{
			A78_Tink_getParam();
			A78_MQTT_TCP_recv_data(10);
		}
		device_info.Params_sever = 0;*/
		
	//	goto_PSM_mode();
		//FL_GPIO_SetOutputPin(psmio_gpio, psmio_gpio_id);   //使能PSM模式
		//A78_close_connect();
	//	goto_PSM_mode();
		//device_info.PSM_mode = 1;
		A78_clode();
		return;
	  //return TRUE;
			
		end_A78:	
		/*if(device_info.PSM_mode == 1)
		{
			device_info.PSM_mode = 0;
			A78_RESTART();      //合宙的4G模块，在需要关机重启的时候，如果使用断电重启，一定要复位串口位高阻态，否则会出现串口漏电模块低电压进入异常模式无法正常开机         重启可以使用指令AT+RESET重启      也可以避免这个问题
	//		A78_clode();		//模块关闭电源
	//		uart_enter_sleep_mode(NB_UART); 
			trace_log("模块重启\r\n");
	//		vTaskDelay(2000);
	//		Uartx_Init(UART1,115200);
			goto begn_A78;
		}*/
		//device_info.PSM_mode = 0;
		A78_clode();
		trace_log("发送失败，数据备份\r\n");	
		pend_bak = device_pending_num_get();
		if(pend_bak == 0)//数据备份做断点续传
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
 
}

void a78_task(void *params)
{		
    uint32_t event_bit = 0xFF;
	memset(transmit_cmd,0,sizeof(transmit_cmd));
    memset(wireless_buf,0,sizeof(wireless_buf));
	while(1)
	{
		pwr_opppse_halt(PWR_WIRELESS_BIT);
		trace_log("4G上报开启\n");
		iot_4g_uart_init();
		iot_4g_en_init();	
		vTaskDelay(1000);
        A78_PSM_task(event_bit);
        iot_en_deinit();
        uart_enter_sleep_mode(IOT_4G_UART);
		trace_log("4G上报结束\n");	
		pwr_arg_halt(PWR_WIRELESS_BIT);			
		xTaskNotifyWait(0x00, 0xffffffff, &event_bit, portMAX_DELAY);		
	}
}

