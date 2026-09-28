#ifndef __AT_H
#define __AT_H

#ifdef __cplusplus
extern "C" {
#endif

//#include "usart2.h"
#include "main.h"
#include "usart.h"



char TEST_AT(uint8_t usart_id,uint8_t ATT);   //检测AT命令
char SET_ATW(uint8_t usart_id);     //AT&W
char SET_ATE(uint8_t usart_id,char ate);   //设置回显
char SET_CFUN(uint8_t usart_id,char cfun);   //设置启用性能
char TEST_ATI(uint8_t usart_id);     //读取模块信号
char SET_IPR(uint8_t usart_id,char *ipr);   //设置波特率

char TEST_CGMR(uint8_t usart_id);      //查询版本号信息

void GET_IMEI(uint8_t usart_id,char *imei,char *lei);     //获取IMEI
void GET_IMSI(uint8_t usart_id,char *imsi);     //获取IMSI
void GET_ICCID(uint8_t usart_id,char *iccid,char *lei);    //获取iccid

char TEST_CPIN(uint8_t usart_id);     //检测SIM卡读取状态
char TEST_CGATT(uint8_t usart_id,uint8_t wait_count);    //检测网络附着状态
char TEST_CREG(uint8_t usart_id,uint8_t wait_count);     //检测网络注册状态1
char TEST_CEREG(uint8_t usart_id,uint8_t wait_count);     //检测网络注册状态2
uint8_t GET_CSQ(uint8_t usart_id);     //检测信号强度
uint8_t GET_CESQ(uint8_t usart_id);    //检测信号质量   BC26
uint16_t GET_CBC(uint8_t usart_id);    //检测模块供电电压
char SET_QPOWD(uint8_t usart_id);    //模块关机
char SET_QRST(uint8_t usart_id);   //模块重启   BC26有
char SET_NRB(uint8_t usart_id);   //模块指令重启   BC95有
char SET_QBAND(uint8_t usart_id,char qband);    //设置运营商信息  NBIOT模块
char SET_QRELLOCK(uint8_t usart_id);        //AT命令快速解锁   NBIOT模块

void TAB_RST(void);     //模块按键重启
	
//uint8_t strstrlen(uint8_t *str1, uint8_t *str2, uint16_t len);  //字符串对比 
uint8_t check_recv2(uint8_t uart, uint8_t *recv_buf ,const char *str,const char *str2, uint16_t rd_size ,uint16_t wait_tick);
uint8_t check_recv3(uint8_t uart, uint8_t *recv_buf ,const char *str0,const char *str1,const char *str2, uint16_t rd_size ,uint16_t wait_tick);


#ifdef __cplusplus
}
#endif
#endif /*__ AT_H */
