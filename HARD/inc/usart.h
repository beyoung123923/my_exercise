#ifndef __USART_H
#define __USART_H


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "fm33lg0xx_fl.h"
#include "main.h"

/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
#define IOT_4G_UART             0x00
#define USB_UART                0x01
#define WIFI_UART				0x02
#define GPS_UART				0x03
#define SENSOR_UART				0x04

#define IOT_BUF_SIZE         	1040
#define USB_BUF_SIZE            256
#define SENSOR_BUF_SIZE			64
/* 变量 --------------------------------------------------------------*/
/* 类型定义 ----------------------------------------------------------*/
typedef struct{
    UART_Type *uart;
	LPUART_Type *lp_uart;
    uint8_t *rx_buf;
    uint16_t rx_size;

    volatile uint16_t read_pos;
    volatile uint16_t write_pos;
    volatile uint16_t rx_count;
    volatile uint16_t rx_end;
}uart_t;

/* 函数声明 ----------------------------------------------------------*/
void usb_uart_init(void);

void iot_4g_uart_init(void);

void gps_uart_init(void);

void sensor_uart_init(void);

void wifi_uart_init(void);

void uart_transmit_buf(uint8_t uart_id, uint8_t *tx_buf, uint16_t tx_size);

void uart_transmit_str(uint8_t uart_id, uint8_t *tx_buf);

uint16_t uart_get_rx_cnt(uint8_t uart_id);

uint16_t uart_recv_data(uint8_t uart_id, uint8_t *recv_buf ,uint16_t recv_len, uint16_t wait_tick);

void uart_exti_sleep_mode(uint8_t uart_id);

void uart_enter_sleep_mode(uint8_t uart_id);

int8_t uart_recv_str_double(uint8_t uart_id, uint8_t *recv_buf ,const char *str1, const char *str2, uint16_t rd_size ,uint16_t wait_tick);

int8_t uart_recv_str_three(uint8_t uart_id, uint8_t *recv_buf ,const char *str1, const char *str2, const char *str3, uint16_t rd_size ,uint16_t wait_tick);

uint8_t uart_strstr(uint8_t uart_id, uint8_t *recv_buf ,const char *str, uint16_t rd_size ,uint16_t wait_tick);

uint8_t uart_send_ack(uint8_t uart_id, void *send_buf, uint8_t *recv_buf ,const char *str, uint16_t rd_size ,uint16_t wait_tick);

uint8_t uart_transmit_wait_ack(uint8_t uart_id, void *send_buf, uint16_t send_size,uint8_t *recv_buf ,uint16_t rd_size ,const char *str, uint16_t wait_tick);

void uart_clear_recv(uint8_t uart_id);

void trace_log(void *x);
void trace_enable(uint8_t enable);
uint8_t get_trace_log(void);

void uart_recv_irq_proc(uint8_t uart_id);
void uart_isr_rx_timeout(uint8_t uart_id);





#endif

