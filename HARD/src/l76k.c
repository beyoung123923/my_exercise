#include "l76k.h"
#include "usart.h"
#include "server.h"
#include "sys_setting.h"
#include "main.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"


uint8_t L76K_get_GPS(void)                                //GPS定位模块   通用  合宙   移远
{
        uint16_t re_len;
        uint8_t tim=100;
        //uint8_t i;
        //char gnss_time[6];
        char *p;
        float yushu;
                
        trace_log("获取定位信息\r\n");
		gps_uart_init();
		gps_en_init();
		vTaskDelay(500);
        open_gps_en();
        vTaskDelay(2000);
        
        uart_send_ack(GPS_UART, "$PCAS02,1000*2E\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);   //设置更新率为1秒        
        uart_send_ack(GPS_UART, "$PCAS03,0,0,0,0,1,0,0,0,0,0,0,0,0*1F\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);   //设置只输出RMC语句
        uart_send_ack(GPS_UART, "$PCAS04,3*1A\r\n", wireless_buf, "OK", sizeof(wireless_buf), 300);   //设置GPS+北斗双模
        //         RMC语句格式
        //  $CNRMC,035438.996,A,2236.75147,N,11350.37110,E,0.01,0.00,151124,,,A,V*01
        while(tim--)
        {
            re_len = uart_recv_data(GPS_UART,wireless_buf,sizeof(wireless_buf),500);
//                sprintf((char *)wireless_buf,"$GNRMC,035438.996,A,2236.75147,N,11350.37110,E,0.01,0.00,151124,,,A,V*01\r\n");
//                re_len = strlen((char *)wireless_buf);
            if(re_len)
            {
                trace_log(wireless_buf);

//                if(strstr((char *)wireless_buf,",A,"))
//                {
				if(strstr((char *)wireless_buf,"GNRMC"))
				{
					p = strstr((char *)wireless_buf,",A,");
					if(p != NULL)
                    {
                        //p += 5;
                        while(*p != '\0' && *p++ != ',');     //查找第1个“，”
						while(*p != '\0' && *p++ != ',');     //查找第2个“，”
//                        for(i=0;i<6;i++)
//                        {
//                            gnss_time[i] = *(p++);
//                        }
//                        gnss_data_time.hour = ((gnss_time[0]-0x30)*10 + (gnss_time[1]-0x30)+8);    //hour
//                        gnss_data_time.min = ((gnss_time[2]-0x30)*10 + (gnss_time[3]-0x30));     //min
//                        gnss_data_time.sec = ((gnss_time[4]-0x30)*10 + (gnss_time[5]-0x30));     //sec
//                        if(gnss_data_time.hour >=24)
//                                gnss_data_time.hour -=24;
                        //while(*p != '\0' && *p++ != ',');     //查找第2个“，”
                        //while(*p != '\0' && *p++ != ',');     //查找第3个“，”
						wireless_info.lat = atof(p);
                        //device_info.GPS_N = s_data.value;
                        yushu = fmodf(wireless_info.lat,100);
                        yushu = yushu/60;
                        wireless_info.lat = (int16_t)wireless_info.lat/100;
                        wireless_info.lat += yushu;
                        //wireless_info.lat = (uint32_t)(wireless_info.lat*1000000);						
						
						
                        //s_data.value = atof(p);
                       // device_info.GPS_N = s_data.value;
//                        yushu = fmodf(s_data.value,100);
//                        yushu = yushu/60;
//                        s_data.value = (int16_t)s_data.value/100;
//                        s_data.value += yushu;
//                        h_data.value = (uint32_t)(s_data.value*1000000);
//                        gnssdata[1] = h_data.data[3];      //纬度数据
//                        gnssdata[2] = h_data.data[2];
//                        gnssdata[3] = h_data.data[1];
//                        gnssdata[4] = h_data.data[0];
                                        
                        while(*p != '\0' && *p++ != ',');     //查找第4个“，”                
                        //gnssdata[0] = *(p++);      		  //纬度方向                        
                        while(*p != '\0' && *p++ != ',');     //查找第5个“，”

						wireless_info.lng = atof(p);
						yushu = fmodf(wireless_info.lng,100);
                        yushu = yushu/60;
                        wireless_info.lng = (int16_t)wireless_info.lng/100;
                        wireless_info.lng += yushu;
                        //wireless_info.lng = (uint32_t)(wireless_info.lng*1000000);
//                        s_data.value = atof(p);
//                        device_info.GPS_E = s_data.value;
//                        yushu = fmodf(s_data.value,100);
//                        yushu = yushu/60;
//                        s_data.value = (int16_t)s_data.value/100;
//                        s_data.value += yushu;
//                        h_data.value = (uint32_t)(s_data.value*1000000);
//                        gnssdata[6] = h_data.data[3];      //经度数据
//                        gnssdata[7] = h_data.data[2];
//                        gnssdata[8] = h_data.data[1];
//                        gnssdata[9] = h_data.data[0];
//                        while(*p != '\0' && *p++ != ',');     //查找第6个“，”
//                        gnssdata[5] = *(p++);      //经度方向
//                        while(*p != '\0' && *p++ != ',');     //查找第7个“，”
//                        while(*p != '\0' && *p++ != ',');     //查找第8个“，”
//                        while(*p != '\0' && *p++ != ',');     //查找第9个“，”
//                        for(i=0;i<6;i++)
//                        {
//                                gnss_time[i] = *(p++);
//                        }
//                        gnss_data_time.day = ((gnss_time[0]-0x30)*10 + (gnss_time[1]-0x30));    //day
//                        gnss_data_time.month = ((gnss_time[2]-0x30)*10 + (gnss_time[3]-0x30));     //month
//                        gnss_data_time.year = ((gnss_time[4]-0x30)*10 + (gnss_time[5]-0x30));     //year
                        trace_log("定位成功\r\n");
                        return 1;
                    }	
				
				}
//                }
            }
        }
        close_gps_en();
        uart_enter_sleep_mode(GPS_UART);
       // LL_GPIO_ResetOutputPin(pwrGPS_gpio, pwrGPS_gpio_id);         //模块上电       
        //uart_enter_sleep_mode(GPS_UART);
        return 0;
}

