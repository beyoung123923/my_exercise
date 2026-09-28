#ifndef _YMODEM_H_
#define _YMODEM_H_

#include "main.h"

//y_modem
#define YMODEM_SOH                     (0x01)  /* start of 128-byte data packet */
#define YMODEM_STX                     (0x02)  /* start of 1024-byte data packet */
#define YMODEM_EOT                     (0x04)  /* end of transmission 文件传输结束命令*/
#define YMODEM_ACK                     (0x06)  /* acknowledge 接收正确应答命令*/
#define YMODEM_NAK                     (0x15)  /* negative acknowledge 重传当前数据包请求命令*/
#define YMODEM_CAN                     (0x18)  /* two of these in succession aborts transfer 取消传输命令，连续发送5个该命令，中止传输*/
#define YMODEM_C                       (0x43)  /* 'C' == 0x43, request 16-bit CRC */


uint8_t get_OTA_Version(uint8_t uart_id);
uint8_t ymodem_get_data(uint8_t uart_id);



#endif 


