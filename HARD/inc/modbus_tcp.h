#ifndef __MODBUS_TCP_H__
#define __MODBUS_TCP_H__

#include "fm33lg0xx_fl.h"

//预留八通道
#define CH_TYPE  0x06

/*
	通道数据 [32 float]
	通道类型 [16byte]
	通道校准
	通道速率
	波形输出控制 三个端口
*/
typedef enum{
	REG_SENSOR_1 = 0,
    REG_SENSOR_2 = REG_SENSOR_1 + 2,
    REG_SENSOR_3 = REG_SENSOR_2 + 2,
    REG_SENSOR_4 = REG_SENSOR_3 + 2,
	ONLY_READ5 = REG_SENSOR_4 + 2,
	ONLY_READ6 = ONLY_READ5 + 2,
    ONLY_READ7 = ONLY_READ6 + 2,
    ONLY_READ8 = ONLY_READ7 + 2,
    ONLY_READ9 = ONLY_READ8 + 2,
    ONLY_READ10 = ONLY_READ9 + 2,
	REG_ADDR = ONLY_READ10 + 2,
	REG_MAX = REG_ADDR + 1,
}reg_addr_e;


void mbs_write_reg_float(float *value, uint16_t reg_addr);

void mbs_write_reg_int(uint32_t *value, uint16_t reg_addr);

void mbs_write_reg_16(uint16_t *value, uint16_t reg_addr);

uint16_t mbrtu_proc(uint8_t *rx_cmd, uint16_t cmd_size, uint8_t *ack_buf);

uint16_t mbs_tcp_poll(uint8_t *tcp_buf,uint16_t tcp_size, uint8_t *tx_buf);

void mbs_data_init(void);


#endif

