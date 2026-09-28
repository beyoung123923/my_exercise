#include "main.h"
#include "AT.h"
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
#include "string.h"
#include "stdio.h"
#include "stdlib.h"


/*******************************************************/
char TEST_AT(uint8_t usart_id,uint8_t ATT)   //检测AT命令
{ 
	while(ATT-- )
	{
		if(!uart_send_ack(usart_id, "AT\r\n", wireless_buf ,"OK", IOT_DATA_LEN ,300))
		{
			//device_info.net_err_stu &= ~Modul_err;
			return 0;
		}
	}
	trace_log("AT无响应，模块无应答\r\n");//网络模块异常
	//device_info.net_err_stu |= Modul_err;
	return 1;
}
/*******************************************************/
char SET_ATW(uint8_t usart_id)     //AT&W
{
	char re;
	re = uart_send_ack(usart_id, "AT&W\r\n", wireless_buf, "OK", IOT_DATA_LEN, 300);
	return re;
	
}
/*******************************************************
ate 0  关闭    1  开启
*******************************************************/
char SET_ATE(uint8_t usart_id,char ate)   //设置回显
{
	char re;
	uint8_t num=3;
	while(num--)
	{
		if(ate == 0)
		{
			re = uart_send_ack(usart_id, "ATE0\r\n", wireless_buf, "OK", IOT_DATA_LEN, 300);
			trace_log("关闭回显\r\n");
			if(re == TRUE)
				break;
		}
		else if(ate == 1)
		{
			re = uart_send_ack(usart_id, "ATE1\r\n", wireless_buf, "ATE1", IOT_DATA_LEN, 300);  //默认模式
			trace_log("开启回显\r\n");
			if(re == TRUE)
				break;
		}
	}
	return re;
}
/*******************************************************/
char TEST_ATI(uint8_t usart_id)     //读取模块信息
{
	if(uart_send_ack(usart_id, "ATI\r\n", wireless_buf, "OK", IOT_DATA_LEN, 1000))
	{ 
        trace_log("模块信息读取失败\r\n");
		return 1;
	}
	else
		return 0;
}
/*******************************************************
imei   字符串传递变量
lei    模块类型
*******************************************************/
void GET_IMEI(uint8_t usart_id,char *imei,char *lei)     //获取IMEI
{
	uint16_t rx_size;
	uint8_t wait_cnt = 3;//*temp_buf;
	char *p;
	while(wait_cnt--)
	{	
		if(strcmp(lei,"BC26")==NULL)
		{
			p=0;
			uart_transmit_str(usart_id, (uint8_t *)"AT+CGSN=1\r\n");
			rx_size = uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),3000);
			if(rx_size)
			{
				trace_log(wireless_buf);
				if(strstr((char *)wireless_buf, "+CGSN:"))
				{
					p=strstr(( char *)wireless_buf, "+CGSN:" );
		/* 					if(device_info.save.tab_module == BC95)    //少一个空格
						p--; */
					strncpy((char*)imei,(char*)(p+7),15);		
					imei[15] = 0;
					break;
				}
			}
		}
		else if(strcmp(lei,"EC20")==NULL || strcmp(lei,"EC200T")==NULL || strcmp(lei,"M26")==NULL)
		{
			uart_transmit_str(usart_id, (uint8_t *)"AT+CGSN\r\n");
			vTaskDelay(30);
			rx_size = uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),3000);
			if(rx_size)
			{
				trace_log(wireless_buf);
				if(strstr((char *)wireless_buf, "OK") && strstr((char *)wireless_buf, "CGSN") )
				{
					p = strstr((char *)wireless_buf, "CGSN");
					while(1)
					{
						if((*p >= 0x30) && (*p <= 0x39))
						  break;
						else
						  p++;
					}
					strncpy((char*)imei,p,15);	
					imei[15]=0;
					break;
				}
			}
		}
	}
//	uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),500);
}
/*******************************************************/
void GET_IMSI(uint8_t usart_id,char *imsi)     //获取IMSI
{
	uint16_t rx_size;
	uint8_t wait_cnt =4;
    char *temp_buf;
	    //
    trace_log("读取IMSI\r\n");
	while(wait_cnt--)
	{
        uart_transmit_str(usart_id, (uint8_t *)"AT+CIMI\r\n");
		vTaskDelay(30);
		rx_size = uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),5000);
		if(rx_size)
		{
			trace_log(wireless_buf);
			if(strstr((char *)wireless_buf, "CIMI") && strstr((char *)wireless_buf, "OK"))
			{
				temp_buf = strstr((char *)wireless_buf, "CIMI");				
				while(1)
                {
                    if((*temp_buf >= 0x30) && (*temp_buf <= 0x39))
                        break;
                    else
                        temp_buf++;
                }
                strncpy((char*)imsi,temp_buf,15);
				imsi[15]=0;
				break;
			}		
		}
        else
            trace_log("没有收到数据\r\n");
	}
//	uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),500);
}
/*******************************************************
iccid   字符串传递变量
lei    模块类型
*******************************************************/
void GET_ICCID(uint8_t usart_id,char *iccid,char *lei)    // 获取iccid
{
  uint16_t rx_size;
	uint8_t wait_cnt = 4;	
	char *p;	
	uint8_t iccid_size=0;

	if(strcmp(lei,"BC95")==NULL)
	{		
		while(wait_cnt--)
		{
			p=0;
            uart_transmit_str(usart_id, (uint8_t *)"AT+NCCID\r\n"); 
			vTaskDelay(30);
			rx_size = uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),5000);
			if(rx_size)
			{
				trace_log(wireless_buf);
				if(strstr((char *)wireless_buf, "+NCCID:"))
				{
					p= strstr(( char *)wireless_buf, "+NCCID:" );					
					strncpy(iccid,(char*)(p+7),20);		
					iccid[20] = 0;
					break;
				}
			}
		}
	}
	else
	{ 
		while(wait_cnt--)
		{	
			p=0;
            uart_transmit_str(usart_id, (uint8_t *)"AT+QCCID\r\n"); 
			vTaskDelay(30);
			rx_size = uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),5000);
			if(rx_size)
			{
				trace_log(wireless_buf);
				if(strcmp(lei,"EC20")==NULL || strcmp(lei,"EC200T")==NULL || strcmp(lei,"BC26")==NULL)
				{
					if(strstr((char *)wireless_buf, "+QCCID:"))
					{
						p = strstr(( char *)wireless_buf, "+QCCID:" );						
						strncpy(iccid,(char*)(p+8),20);		
						iccid[20] = 0;
						break;
					}
				}
				else if(strcmp(lei,"M26")==NULL)
				{			
					if(strstr((char *)wireless_buf, "OK")  && strstr((char *)wireless_buf, "QCCID"))
					{
						p = strstr((char *)wireless_buf, "QCCID");
						while(*p++ != 0x0A){};
                        strncpy(iccid,p,20);		
						iccid[20] = 0;
//						for(iccid_size = 0; p[iccid_size] != 0x0d; iccid_size++)
//						{
//							iccid[iccid_size] = p[iccid_size];
//						}
						iccid[iccid_size]=0;
						break;
					}
				}		
			}
		}
	}
//	uart_recv_data(usart_id, wireless_buf, sizeof(wireless_buf),500);
}
/*******************************************************/
void M26_GET_ICCID(uint8_t usart_id,char *iccid)    //2G获取iccid
{
	uint16_t rx_size;
	uint8_t wait_cnt = 4;
	char *p;	
	uint8_t iccid_size=0;
	uart_transmit_str(usart_id, (uint8_t *)"AT+QCCID\r\n");  
	while(wait_cnt--)
	{	
		p=0;
		rx_size = uart_recv_data(usart_id, wireless_buf, IOT_DATA_LEN,500);
		if(rx_size)
		{
			trace_log(wireless_buf);
			if(strstr((char *)wireless_buf, "OK")  && strstr((char *)wireless_buf, "QCCID"))
			{
				p = (char *)wireless_buf;
				while(*p++ != 0x0A);
				for(iccid_size = 0; p[iccid_size] != 0x0d; iccid_size++)
				{
					iccid[iccid_size] = p[iccid_size];
				}
				iccid[iccid_size]=0;
				break;
			}
		}
	}
}
/*******************************************************
cfun   默认设置1   启用全功能
*******************************************************/
char SET_CFUN(uint8_t usart_id,char cfun)   //设置启用性能
{
	char re;
	if(cfun == 0)
	{
		trace_log("关闭全功能\r\n");
		re = uart_send_ack(usart_id, "AT+CFUN=0\r\n", wireless_buf, "OK", IOT_DATA_LEN, 300);
	}
	else if(cfun == 1)
	{
		trace_log("开启全功能\r\n");
		re = uart_send_ack(usart_id, "AT+CFUN=1\r\n", wireless_buf, "OK", IOT_DATA_LEN, 5000);  //默认模式
	}
	else if(cfun == 4)
	{
		re = uart_send_ack(usart_id, "AT+CFUN=4\r\n", wireless_buf, "OK", IOT_DATA_LEN, 5000);  //默认模式
//		trace_log("开启...\r\n");
	}
	else if(cfun == 7)
	{
		re = uart_send_ack(usart_id, "AT+CFUN=7\r\n", wireless_buf, "OK", IOT_DATA_LEN, 5000);  //默认模式
//		trace_log("开启...\r\n");
	}
	return re;
}
/*******************************************************
*ipr  波特率
*******************************************************/
char SET_IPR(uint8_t usart_id,char *ipr)   //设置波特率   默认115200
{
//	uint8_t rx_size[20];
	sprintf((char *)transmit_cmd, "AT+IPR=%s\r\n",ipr);
	if(uart_send_ack(usart_id, transmit_cmd, wireless_buf, "OK", IOT_DATA_LEN, 300))
	{
		trace_log("设置波特率失败\r\n");
		return 1;
	}
	else
		return 0;
}
/*******************************************************/
char TEST_CPIN(uint8_t usart_id)     //检测SIM卡读取状态
{
	uint8_t re = 1;  //1
	uint8_t check_count = 10;
	uint8_t check_result;
	while(check_count-- )
	{
		sprintf((char *)transmit_cmd, "AT+CPIN?\r\n");
		uart_transmit_str(usart_id,transmit_cmd);
		check_result = check_recv3(usart_id, wireless_buf ,"+CPIN: READY","+CPIN: SIM PIN","+CPIN: SIM PUK", IOT_DATA_LEN ,500);
		if(check_result == 0)
		{
			re = 0;
			break;
		}
		else if(check_result == 1)
		{
			/*if(strcmp((char *)device_info.save.srv_apn, "") != 0)
			{				
				sprintf((char *)transmit_cmd, "AT+CPIN=%s\r\n",device_info.save.srv_sim_pin);
				if(!uart_send_ack(usart_id, transmit_cmd, wireless_buf, "OK", IOT_DATA_LEN, 350))
				{
					trace_log("设置PIN码成功\r\n");
				}
				else
				{
					trace_log("设置PIN码失败\r\n");
					re = 1;
					break;
				}
			}
			else
			{
				trace_log("请输入PIN码\r\n");
				re = 1;
				break;
			}*/
		}
		else if(check_result == 2)
		{
			/*if(strcmp((char *)device_info.save.srv_apn, "") != 0)
			{				
				sprintf((char *)transmit_cmd, "AT+CPIN=\"%s\",\"%s\"\r\n",device_info.save.srv_sim_puk,device_info.save.srv_sim_pin);
				if(!uart_send_ack(usart_id, transmit_cmd, wireless_buf, "OK", IOT_DATA_LEN, 350))
				{
					trace_log("设置PUK码成功\r\n");
				}
				else
				{
					trace_log("设置PUK码失败\r\n");
					re = 1;
					break;
				}
			}
			else
			{
				trace_log("请输入PUK码\r\n");
				re = 1;
				break;
			}*/
		}
		vTaskDelay(500);
	}
	if(re == 0)
    {
        trace_log("SIM卡正常\r\n");
        //device_info.net_err_stu &= ~SIM_RE_ERR;
    }
	else
	{
		//device_info.net_err_stu |= SIM_RE_ERR;
		trace_log("SIM卡异常\r\n");
	}
	return re;
}
/*******************************************************
wait_count  检测次数
*******************************************************/
char TEST_CGATT(uint8_t usart_id,uint8_t wait_count)    //检测网络附着状态
{	
	while(wait_count--)
	{ 
		if( !uart_send_ack(usart_id, "AT+CGATT?\r\n", wireless_buf, "OK", IOT_DATA_LEN, 350))
		{
//			trace_log(wireless_buf);
			if(strstr((char *)wireless_buf, "0"))    //BC95返回数据少一个空格  +CGATT:0    其他模块返回+CGATT: 0
			{
				trace_log( "网络附着中，请等待...\r\n");
			}
			else if(strstr((char *)wireless_buf, "1"))
			{
				trace_log( "网络附着成功\r\n");
				return 0;
			}
		}
		vTaskDelay(500);  
	}
	return 1;
}
/*******************************************************
wait_count  检测次数
*******************************************************/
char TEST_CREG(uint8_t usart_id,uint8_t wait_count)     //检测网络注册状态
{
//	uint16_t rx_size;
	uint8_t cregerr = 0;
	
	while(wait_count--)
	{ 
		if( !uart_send_ack(usart_id, "AT+CREG?\r\n", wireless_buf, "OK", IOT_DATA_LEN, 350))
		{
//			trace_log(wireless_buf);
			if(strstr((char *)wireless_buf, "+CREG: 0,2"))
			{
				trace_log( "CS业务网络注册中，请等待...\r\n");
			}
			else if(strstr((char *)wireless_buf, "+CREG:0,1"))
			{
				trace_log( "CS业务本地网络\r\n");
				return 0;
			}
			else if(strstr((char *)wireless_buf, "+CREG:0,5"))
			{
				trace_log( "CS业务漫游网络\r\n");
				return 0;
			}
			else if(strstr((char *)wireless_buf, "+CREG:0,3"))
			{			
				trace_log( "CS业务网络初始化失败,请检测SIM状态\r\n");//网络初始化失败,请检测SIM是否在有效期
				break;
			}
			else if(strstr((char *)wireless_buf, "+CREG:0,4"))
			{
				trace_log("未知错误\r\n");
				cregerr++;
			}
		}
		if(cregerr > 5)
				break;
		vTaskDelay(800);
	}
	return 1;
}
/*******************************************************
wait_count  检测次数
*******************************************************/
char TEST_CEREG(uint8_t usart_id,uint8_t wait_count)     //检测网络注册状态
{
	char re = 1,err;
	err = 0;
	while(wait_count--)
	{ 
		if( !uart_send_ack(usart_id, "AT+CEREG?\r\n", wireless_buf, "OK", IOT_DATA_LEN, 5000))
		{
			if(strstr((char *)wireless_buf, "0,2")) ////BC95返回数据少一个空格  +CEREG:0,2    其他模块返回+CEREG: 0,2
			{
				trace_log( "PS业务网络注册中，请等待...\r\n");
			}
//      else if(strstr((char *)wireless_buf, "5,2"))
//			{
//				trace_log( "PS业务网络注册中，请等待...\r\n");
//			}
			else if(strstr((char *)wireless_buf, "0,1"))
			{
				trace_log( "PS业务本地网络\r\n");
				wait_count = 0;
				re = 0;
			}
			else if(strstr((char *)wireless_buf, "0,5"))
			{
				trace_log( "PS业务漫游网络\r\n");
				wait_count = 0;
				re = 0;
			}
			else if(strstr((char *)wireless_buf, "0,3"))
			{			
				trace_log( "PS业务网络初始化失败,请检测SIM状态\r\n");//网络初始化失败,请检测SIM是否在有效期
				break;
			}
			else if(strstr((char *)wireless_buf, "0,4"))
			{
				trace_log("未知错误\r\n");
				err++;
                if(err>30)
                    break;
			}
		}
        else
            trace_log( "模块无应答\r\n");
		vTaskDelay(500);
	}
	if(re == 0)
		;//device_info.net_err_stu &= ~SIM_CN_ERR;
	else
		;//device_info.net_err_stu |= SIM_CN_ERR;
	
	return re;
}
/*******************************************************/
uint8_t GET_CSQ(uint8_t usart_id)     //检测信号强度
{
	char *buf_cache;
	uint8_t csq = 0;
    char signalnum=20;
	while(signalnum--)
	{		
		if( !uart_send_ack(usart_id, "AT+CSQ\r\n", wireless_buf, "+CSQ:", IOT_DATA_LEN, 5000))
		{
			buf_cache = strstr((const char *)wireless_buf, "+CSQ:" );
			buf_cache += 5;//6;
			csq = atoi(buf_cache);
			if(csq > 35)
			{
				csq = 0;
				vTaskDelay(300);
				continue;
			}
			else if(csq == 0)
			{
				trace_log("没有信号\r\n");
				vTaskDelay(300);
                continue;
			}
			else if(csq<=5)
			{
				trace_log("信号较弱，发送数据将不稳定\r\n");
				break;
			}
			else
				break;							
		}
	}	
	return csq;
}
/*******************************************************/
/*******************************************************/
uint16_t GET_CBC(uint8_t usart_id)     //检测模块供电电压
{
	char *buf_cache;
	uint16_t cbc;
	if( !uart_send_ack(usart_id, "AT+CBC\r\n", wireless_buf, "+CBC: ", IOT_DATA_LEN, 300))
	{
		buf_cache = strstr((const char *)wireless_buf, "+CBC:" );
		{
			buf_cache += 10;
			cbc = atoi(buf_cache);
		}
	}
	if(cbc==0)
	{
		if( !uart_send_ack(usart_id, "AT+CBC\r\n", wireless_buf, "+CBC: ", IOT_DATA_LEN, 300))
	  {
		buf_cache = strstr((const char *)wireless_buf, "+CBC:" );
		{
			buf_cache += 10;
			cbc = atoi(buf_cache);
		}
  	}
	}
//	nb.nb_cbc=cbc;
	
	return cbc;
}
/*******************************************************/
char SET_QPOWD(uint8_t usart_id)    //模块命令关机
{
	char re;
	//if(device_info.save.tab_module == A78)
    re = uart_send_ack(usart_id, "AT+CPOWD=1\r\n", wireless_buf, "POWER DOWN", IOT_DATA_LEN, 3000);
/*     else
    re = uart_send_ack(usart_id, "AT+QPOWD=1\r\n", wireless_buf, "POWERED DOWN", IOT_DATA_LEN, 3000); */

	return re;
}
/*******************************************************/
char SET_QRST(uint8_t usart_id)   //模块指令重启   BC26有
{
	if(!uart_send_ack(usart_id, "AT+QRST=1\r\n", wireless_buf ,"0000", IOT_DATA_LEN ,350))
	{	
		return 1;
	}
	else		
		return 0;
}
/*******************************************************/
char SET_NRB(uint8_t usart_id)   //模块指令重启   BC95有
{
	if(!uart_send_ack(usart_id, "AT+QRST=1\r\n", wireless_buf ,"0000", IOT_DATA_LEN ,350))
	{	
		return 1;
	}
	else		
		return 0;
}
/*******************************************************/

/* void TAB_RST(void)     //模块按键重启
{
	//拉低800ms开机 
	trace_log("模块复位重启\r\n");
	FL_GPIO_ResetOutputPin(pwrio_gpio, pwrio_gpio_id);    
	vTaskDelay(500); //500
	FL_GPIO_SetOutputPin(pwrio_gpio, pwrio_gpio_id); 
	vTaskDelay(800); ///800
	FL_GPIO_ResetOutputPin(pwrio_gpio, pwrio_gpio_id); 
	vTaskDelay(100);
} */
/*******************************************************/
/* void PCIE_PW_OPEN(void)    //打开PICE模块电源
{
	FL_GPIO_SetOutputPin(pwrpcie_gpio, pwrpcie_gpio_id); 	//模块上电
	trace_log("打开模块电源\r\n");
//	vTaskDelay(1000);   //1
} */
/*******************************************************/
/* void PCIE_PW_CLOSE(void)    //关闭PICE模块电源
{
	FL_GPIO_ResetOutputPin(pwrpcie_gpio, pwrpcie_gpio_id); 	//模块断电
	trace_log("\r\n关闭模块电源\r\n");
//	vTaskDelay(1000);
} */
/*******************************************************/
/* char SET_QBAND(char qband)    //设置运营商信息  NBIOT模块
{
	if(qband == 3)///
	{
		if(device_info.save.tab_module == BC95)
			strcpy((char *)transmit_cmd,"AT+NBAND=3,8\r\n");
		else
			strcpy((char *)transmit_cmd,"AT+QBAND=1,3\r\n");    //锁定中国联通频段
	}
	else if(qband == 5)
	{
		if(device_info.save.tab_module == BC95)
			strcpy((char *)transmit_cmd,"AT+NBAND=5\r\n");
		else
			strcpy((char *)transmit_cmd,"AT+QBAND=1,5\r\n");    //锁定中国电信频段
	}
	else if(qband == 8)
	{
		if(device_info.save.tab_module == BC95)
			strcpy((char *)transmit_cmd,"AT+NBAND=8\r\n");
		else
			strcpy((char *)transmit_cmd,"AT+QBAND=1,8\r\n");   //锁定中国移动频段
	}
	else
	{
		if(device_info.save.tab_module == BC95)
			strcpy((char *)transmit_cmd,"AT+NBAND=3,5,8\r\n");
		else
			strcpy((char *)transmit_cmd,"AT+QBAND=0\r\n");      //不锁定频段
	}
	
	if(!uart_send_ack(usart_id, transmit_cmd, wireless_buf ,"OK", IOT_DATA_LEN ,300))
	{	
		return 1;
	}
	else		
		return 0;
		
} */
/******************************************************************/
char SET_QRELLOCK(uint8_t usart_id)        //AT命令快速解锁   NBIOT模块
{
	if(!uart_send_ack(usart_id, "AT+QRELLOCK\r\n", wireless_buf ,"OK", IOT_DATA_LEN ,350))
	{	
		return 1;
	}
	else		
		return 0;
}
/******************************************************************/
/*uint8_t strstrlen(uint8_t *str1, uint8_t *str2, uint16_t len)  //字符串对比   
{
	uint16_t i = 0, j = 0, k = 0;
	for(; i < len; i++)
	{
		for(j = i; str1[j] == str2[k] && str2[k] != '\0';j++ ,k++ );
		
		if(j > 0 && str2[k] == '\0')
		{
			return 0;
		}
		k = 0;
	}
	return 1;
}*/
/******************************************************************
*uart  COM口
*recv_buf   数据存储数组
*str0     验证数组0   验证正确返回0
*str1     验证数组1   验证正确返回1
rd_size    接收数组长度空间
wait_tick    等待延时n ms
*****************************************************************/
uint8_t check_recv2(uint8_t uart, uint8_t *recv_buf ,const char *str0,const char *str1, uint16_t rd_size ,uint16_t wait_tick)
{
	
	uint16_t rx_size = 0;
	uint8_t ret =2;
	wait_tick /= 10;
	while(wait_tick--)
	{
		rx_size = uart_recv_data(uart, recv_buf ,rd_size, 10 );
		
		if(rx_size)
		{
		
		  trace_log((uint8_t *)recv_buf); ////////***打印串口1数据******///
			if(strstr((const char *)recv_buf, str0) != NULL)
			{
				ret = 0;
				break;
			}
			else if(strstr((const char *)recv_buf, str1) != NULL)
			{	
				ret = 1;
				break;
			}
		}
//		vTaskDelay(1);  //ms延时
	}
	return ret;
}	
/******************************************************************
*uart  COM口
*recv_buf   数据存储数组
*str0     验证数组0   验证正确返回0
*str1     验证数组1   验证正确返回1
*str2     验证数组2   验证正确返回2
rd_size    接收数组长度空间
wait_tick    等待延时n ms
******************************************************************/
uint8_t check_recv3(uint8_t uart, uint8_t *recv_buf ,const char *str0,const char *str1,const char *str2, uint16_t rd_size ,uint16_t wait_tick)
{
	
	uint16_t rx_size = 0;
	uint8_t ret =3;
		
	wait_tick /= 10;	
	while(wait_tick--)
	{
		rx_size = uart_recv_data(uart, recv_buf ,rd_size, 10 );
		
		if(rx_size)
		{
			trace_log((uint8_t *)recv_buf); ////////***打印串口1数据******///
			if(strstr((const char *)recv_buf, str0) != NULL)
			{
				ret = 0;
				break;
			}
			else if(strstr((const char *)recv_buf, str1) != NULL)
			{	
				ret = 1;
				break;
			}
			else if(strstr((const char *)recv_buf, str2) != NULL)
			{	
				ret = 2;
				break;
			}
		}
//		vTaskDelay(1);  //ms延时
	}
	return ret;
}


/*******************************************************/
uint8_t GET_CESQ(uint8_t usart_id)    //检测信号质量  BC26
{
	char *buf_cache;
	uint8_t cesq;
  char signalnum=10;
	
	while(signalnum--)
	{
		if( !uart_send_ack(usart_id, "AT+CESQ\r\n", wireless_buf, "+CESQ:", IOT_DATA_LEN, 350))
		{
			buf_cache = strstr((const char *)wireless_buf, "+CESQ:" );
			{
				buf_cache += 7;
				cesq = atoi(buf_cache);
				if(cesq > 70)
				{
					cesq = 0;
				}
			}
			if(cesq<=5)
			{
				trace_log("信号质量较差，发送数据将不稳定\r\n");
			  break;
			}
			else
				break;
		}
		vTaskDelay(300);
	}	
	return cesq;
}



/******************************************************************/


