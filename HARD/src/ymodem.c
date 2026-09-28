#include "main.h"
#include "ymodem.h"

uint8_t ymodem_num;    //包号序号
uint16_t max_ymodem_num;


#define OTA_ADDR    FLASH_ADDR_END     //定义OTA数据存储的起始地址
/********************CRC********************************/
static const uint8_t s_CRCH[] = 
{
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,  
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,  
0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,  
0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,  
0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,  
0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,  
0x40  
};

/**********************CRC******************************/
static uint8_t s_CRCLo[] = 
{
0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,  
0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,  
0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,  
0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,  
0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,  
0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,  
0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,  
0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,  
0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,  
0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,  
0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,  
0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,  
0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,  
0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,  
0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,  
0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,  
0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,  
0x40
};
/*****************************************************/
/*Ymodem采用的是CRC16校验算法，校验值为2字节，传输时CRC高八位在前，低八位在后；
CRC计算数据为信息块数据，不包含帧头、包号、包号反码
*/
uint16_t ymodem_crc_16(uint8_t *p_buf, uint16_t length)   //
{
  uint8_t crc_h = 0XFF, crc_l = 0XFF;
  int index;
  
  if(length>10)
  {
    p_buf += 3;
    length -= 3;
  }
  else
    return FALSE;
  
  while(length--)
  {
    index = crc_l ^ *(p_buf++);  /*计算CRC*/
    crc_l = (uint8_t)(crc_h ^ s_CRCH[index]);
    crc_h = s_CRCLo[index];
  }
  return ((uint16_t)crc_h << 8 | crc_l);
}
/*****************************************************/
static void flash_erase(void)    //擦除数据存储区域
{
  uint8_t i;
  uint32_t addr;
  
  for(i=0;i<48;i++)
  {
    addr = OTA_ADDR+i*4096;
    if(addr>=OTA_ADDR && addr<0x400000)
      w25qxx_erase_sector(addr);
  }
}
/*****************************************************/
static uint8_t SOH_work(uint8_t *p_buf)
{
  uint8_t i,j ;
  uint8_t re = FALSE;
  uint32_t file_size;
  
  if(p_buf[1] == 0x00 && p_buf[2] == 0xFF)    //起始包
  {
    trace_log("收到起始包信息\r\n");
    ymodem_num = 0;    //数据包序号置0
    max_ymodem_num = 0;
    flash_erase();
    memset(transmit_cmd,0,sizeof(transmit_cmd));
    
    for(i=0;i<128;i++)     //filename
    {
      if(p_buf[i+3] != 0x00)
        transmit_cmd[i] = p_buf[i+3];
      else
        break;
    }
    trace_log("filename:");
    trace_log(transmit_cmd);
    trace_log("\r\n");
    
    
    memset(transmit_cmd,0,sizeof(transmit_cmd));
    j=0;
    i++;
    for(;i<128;i++)    //filezise
    {
      if(p_buf[i+3] != 0x00)
        transmit_cmd[j++] = p_buf[i+3];
      else
        break;
    }
    trace_log("filezise:0x");
    trace_log(transmit_cmd);
    trace_log("Byte\r\n");
    file_size = 0;
    for(i=0;i<j;i++)    //16进制转10进制
    {
      if(transmit_cmd[i] != 0x00)
      {
        file_size *= 16;
        if(transmit_cmd[i]>='0' && transmit_cmd[i] <='9')
          file_size += (transmit_cmd[i]-'0');
        else if(transmit_cmd[i]>='A' && transmit_cmd[i] <='F')
          file_size += (transmit_cmd[i]-'A'+10);
        else if(transmit_cmd[i]>='a' && transmit_cmd[i] <='f')
          file_size += (transmit_cmd[i]-'a'+10);        
      }
      else
        break;
    }
    sprintf((char *)transmit_cmd,"%d",file_size);
    trace_log("filezise:");
    trace_log(transmit_cmd);
    trace_log("Byte\r\n");
    if(file_size%1024 != 0)     //计算应该接收数据包个数
      max_ymodem_num = file_size/1024+1;
    else
      max_ymodem_num = file_size/1024;
    re = TRUE;
  }
  else
  {
    trace_log("收到结束包信息\r\n");
    re = FALSE;
    ymodem_num = 0;    //接收结束，数据包序号置0
  }
  
  return re;
}
/*****************************************************/
static uint8_t STX_work(uint8_t *p_buf)
{
  uint16_t i,wait_num = 0;
  uint8_t re = FALSE;
  uint32_t addr;
  uint8_t num,num_f;
  char buf[8];
  
  num = ymodem_num+1;
  num_f = ~num;
  sprintf(buf,"%d\r\n",num);
  trace_log("收到数据包:");
  trace_log(buf);
  if(p_buf[1] == num && p_buf[2] == num_f)    //数据包序号验证  
  {
    if(p_buf[1] <= max_ymodem_num)    //判断当前数据包序号小于filesize数据包个数
    {
      addr = OTA_ADDR + ymodem_num*1024;
      if(addr > 0x400000)
        return FALSE;
      ymodem_num++;
      re = TRUE;
      while(is_flash_finish() == 0)
      {
        if(wait_num++ >= 5000)
        {
          return FALSE;
        }
      }
      for(i=0;i<1024;i++)
        transmit_cmd[i] = p_buf[i+3];
      w25qxx_write(transmit_cmd,addr,1024);
      re = TRUE;
    }
    else
    {
      trace_log("当前包超出filesize范围\r\n");
      re = FALSE;
    }
  }
  else
  {
    trace_log("当前包序号不对\r\n");
    re = FALSE;
  }
  
  return re;
}
/*****************************************************/
/*****************************************************/
/*****************************************************/
uint8_t ymodem_get_data(uint8_t uart_id)
{
  uint8_t crc_h,crc_l,num;
  uint8_t re = FALSE;
  uint8_t set_n=0;     //0发送C   1发送ACK   2发送NAK   3发送ACK+C   4回复ACK并且退出
  uint16_t crc_da,recive_l;
  num = 5;
//  char bu[10];
  
  while(num--)
  {
//    vTaskDelay(200);    
    memset(wireless_buf,0,sizeof(wireless_buf));
    if(set_n == 0)
      uart_transmit_str(uart_id, (uint8_t *)"C");
    else if(set_n == 1)
    {
      trace_log("应答ACK\r\n");
      transmit_cmd[0] = YMODEM_ACK;    //应答平台  YMODEM_ACK
      uart_transmit_buf(uart_id, transmit_cmd,1);   
    }
    else if(set_n == 2)
    {
      trace_log("应答重传NACK\r\n");
      transmit_cmd[0] = YMODEM_NAK;      //应答平台  YMODEM_NAK
      uart_transmit_buf(uart_id, transmit_cmd,1);   
    }
    else if(set_n == 3)
    {
      trace_log("应答ACK+C\r\n");
      transmit_cmd[0] = YMODEM_ACK;    //应答平台  YMODEM_ACK+C
      uart_transmit_buf(uart_id, transmit_cmd,1);
      vTaskDelay(50);
      uart_transmit_str(uart_id, (uint8_t *)"C");
    }
    else if(set_n == 4)
    {
      trace_log("应答ACK并退出\r\n");
      transmit_cmd[0] = YMODEM_ACK;    //应答平台  YMODEM_ACK
      uart_transmit_buf(uart_id, transmit_cmd,1);
      break;
    }
    else
      continue;
    
    recive_l = uart_recv_data(uart_id, wireless_buf, sizeof(wireless_buf), 5000);
    if(recive_l)
    {
//      if(device_info.debug_en)   //接收数据打印不能打开，打印数据太多会造成系统崩溃错乱
//      {        
//        uart_transmit_buf(&COM5_USB, wireless_buf,recive_l);
//        trace_log("\r\n");
//      }
      if(recive_l>5)
      {
        crc_da = ymodem_crc_16(wireless_buf,recive_l-2);    //Ymodem协议的CRC16校验是高位在前，低位在后   与modbus协议相反
        crc_l = crc_da;
        crc_h = crc_da>>8;
        if(wireless_buf[recive_l-1] == crc_l && wireless_buf[recive_l-2] == crc_h)
        {
          trace_log("CRC校验通过\r\n");
          num = 5;
        }
        else
        {
          trace_log("CRC校验不通过\r\n");
          set_n = 2;
          continue;
        }
      }
      
      switch(wireless_buf[0])
      {
        case YMODEM_SOH: if(SOH_work(wireless_buf) == TRUE)   //收到起始帧，应答ACK+C
                         { 
//                           num = 3;
                           set_n = 3;
                         }
                         else    //收到结束帧，应答ACK
                         {
                           set_n = 1;
//                           num = 3;
                         }
                         break;
        case YMODEM_STX: if(STX_work(wireless_buf) == TRUE)  //接收正确   回复ACK，接收下一包
                         {
//                           num = 3;
                           set_n = 1;
                         }
                         else    //接收错误    回复NAK，重新获取当前数据包
                           set_n = 2;
                         break;
        case YMODEM_EOT: trace_log("文件传输结束\r\n");
                         set_n = 4;
                         num = 3;
                         if(ymodem_num == max_ymodem_num)
                         {
                           trace_log("文件传输包数正确\r\n");
                           re = TRUE;
                         }
                         else
                         {
                           trace_log("文件传输包数错误\r\n");
                           re = FALSE;
                         }
                         break;
        case YMODEM_CAN: trace_log("取消传输\r\n");re=FALSE;set_n = 4;num = 3;break;
      }
    }
  }
  
  if(re == TRUE)
  {
    
  }
  return re;
}


/*****************************************************/
uint8_t get_OTA_Version(uint8_t uart_id)    //获取平台固件版本信息
{
	uint8_t re = FALSE;
	uint8_t recive_l,num;
	char SN_buf[16];
	uint32_t da;
	char *getp;
	char *sn_start = NULL;  // 单个SN的起始指针
	char *sn_end = NULL;    // 单个SN的结束指针（逗号/双引号）
  
	num = 3;
	trace_log("\r\n获取云端版本信息\r\n");
	sprintf((char *)transmit_cmd,"{\"model\":\"TP500Vx.5_YY_%s\"}",config_get_sn());
	trace_log((uint8_t *)transmit_cmd); 
	trace_log("\r\n");
	while(num--)
	{    
		uart_transmit_str(uart_id, (uint8_t *)transmit_cmd);   //退出透传模式
		recive_l = uart_recv_data(uart_id, wireless_buf, sizeof(wireless_buf), 5000);
		if(recive_l)
		{
			trace_log(wireless_buf);
			//////////////////
			//{"last_v":"V20240427,ALL"}    ALL   表示全部升级
			//{"last_v":"V20240427,20231020001"}    20231020001  只升级对应SN号设备
			//////////////////
			getp = strstr((char *)wireless_buf,"{\"last_v\":");    //{"last_v":"V20240427,"}
			if(getp!=NULL)
			{        
				while(*getp++ != 'V');
				da = atoi(getp);
				getp = strstr((char *)wireless_buf,",");
				if(getp!=NULL)
				{
					sn_start = ++getp;  // sn_start指向第一个SN的开头
					while(1)
					{
						// 找当前SN的结束位置（逗号或双引号）
						sn_end = sn_start;
						while(*sn_end != ',' && *sn_end != '"' && sn_end < (char *)wireless_buf + sizeof(wireless_buf))
						{
							sn_end++;  // 向后遍历，直到遇到分隔符/结束符
						}

						// 4. 截取单个SN到SN_buf（最多15字符，避免溢出）
						memset(SN_buf, 0, 16);  // 清空缓冲区，复用
						uint16_t sn_len = sn_end - sn_start;
						if(sn_len > 15) sn_len = 15;  // 限制长度，适配16字节buf
						strncpy(SN_buf, sn_start, sn_len);

						// 5. 匹配当前SN（ALL或本机SN）
						if((strcmp(SN_buf,"ALL") == 0) || (strcmp(SN_buf,(char *)config_get_sn()) == 0))
						{    
							trace_log("\r\nOTA版本匹配成功\r\n");
							if(da>config_get_soft_date())     												//云端版本大于当前程序版本
							{
								re = TRUE;
								trace_log("\r\n云端版本高于当前版本，设备升级\r\n");
							}
							else
							{
								re = FALSE;   //
								trace_log("\r\n云端版本低于或等于当前版本,设备不升级\r\n");
							}
							break;  // 匹配成功，退出循环
						}

						// 6. 判断是否遍历完所有SN
						if(*sn_end == '"')  // 遇到双引号，无更多SN
						{
							trace_log("\r\nOTA版本匹配失败：未找到本机SN\r\n");
							break;
						}
						else if(*sn_end == ',')  // 遇到逗号，处理下一个SN
						{
							sn_start = sn_end + 1;  // sn_start指向下一个SN开头
						}
						else  // 异常情况（超出缓冲区）
						{
							trace_log("\r\nSN列表解析异常\r\n");
							break;
						}
					}
				}
				else
				{
					trace_log("\r\n版本号不对\r\n");
					return FALSE;
				}
				break;
			}
			
			getp = strstr((char *)wireless_buf,"{\"NO CARRIER\":");
			if(getp!=NULL)
			{
				trace_log("\r\n服务器断开，云端没有当前型号产品OTA固件\r\n");
				re = FALSE;
				break;
			}
			
			getp = strstr((char *)wireless_buf,"CLOSED");
			if(getp!=NULL)
			{
				trace_log("\r\n服务器断开\r\n");
				re = FALSE;
				break;				
			}
		}
		else
			re = FALSE;
	}
	return re;
}
/*****************************************************/

