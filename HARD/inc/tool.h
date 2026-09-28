#ifndef TOOL_H_
#define TOOL_H_

/* Includes ------------------------------------------------------------------*/
#include "fm33lg0xx_fl.h"
/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
/* 变量 --------------------------------------------------------------*/
/* 类型定义 ----------------------------------------------------------*/
/* 函数声明 ----------------------------------------------------------*/
void find_symbol_split_to_string(uint8_t *str_buf,uint8_t *data,char symbol_split,uint8_t index);

uint16_t calc_crc16(uint8_t *buf, uint16_t len);

uint8_t strstrlen(uint8_t *buf,uint8_t *check_buf, uint16_t size);

uint16_t be_buf_2_uint16(uint8_t *buf);

uint32_t be_buf_2_uint32(uint8_t *buf);

float be_buf_2_float(uint8_t *buf);

uint16_t str_to_hex16(uint8_t *buf, uint8_t * format, uint16_t us_length);

void hex16_to_str(uint8_t *s_inchar, uint16_t s_len, uint8_t  *a_outtxt);

uint16_t strstraddr(uint8_t *buf,uint8_t *check_buf, uint16_t size);

uint16_t buf_ab_2_uint16(uint8_t *buf);

uint16_t buf_ba_2_uint16(uint8_t *buf);

int16_t buf_ab_2_int16(uint8_t *buf);

int16_t buf_ba_2_int16(uint8_t *buf);

uint32_t buf_abcd_2_uint32(uint8_t *buf);

uint32_t buf_cdab_2_uint32(uint8_t *buf);

uint32_t buf_badc_2_uint32(uint8_t *buf);

uint32_t buf_dcba_2_uint32(uint8_t *buf);

int32_t buf_abcd_2_int32(uint8_t *buf);

int32_t buf_cdab_2_int32(uint8_t *buf);

int32_t buf_badc_2_int32(uint8_t *buf);

int32_t buf_dcba_2_int32(uint8_t *buf);

float buf_abcd_2_float(uint8_t *buf);

float buf_cdab_2_float(uint8_t *buf);

float buf_badc_2_float(uint8_t *buf);

float buf_dcba_2_float(uint8_t *buf);

#endif
