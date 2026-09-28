#ifndef __OLED_H
#define __OLED_H

#include "fm33lg0xx_fl.h"
#include "fm33lg0xx_fl_gpio.h"
#include "fm33lg0xx.h"

void init_print_buf(void);

// DMA 发送相关声明
void SPI0_Init(void);
void SPI0_DMA_TX_Init(void);           				  // SPI1+DMA 初始化
void OLED_DMA_SendCmd(uint32_t *cmd, uint16_t len);    // DMA 发送单个命令
void OLED_DMA_SendData(uint32_t *data, uint16_t len);  // DMA 发送数据块

//
void OLED_Init(void);                  // 初始化
void oled_deinit(void);
void oled_restart(void);
void oled_display_on(void);
void oled_display_off(void);
void oled_refresh_gram(void);
void oled_display_clear(void);
void oled_display_point(uint8_t x, uint8_t y, uint8_t t);
void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot);
void oled_display_line(uint8_t s_x, uint8_t s_y, uint8_t e_x, uint8_t e_y, uint8_t mode);
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr , uint8_t size, uint8_t mode);
void oled_display_string(uint8_t x,uint8_t y, const uint8_t *string, uint8_t size, uint8_t moth);
void oled_display_bmp(uint8_t x, uint8_t y,uint8_t width, uint8_t height, uint8_t *ptr, uint8_t mode);
void GPIO_DeInit_OLED(void);

bool SpiWriteDataComplete(void);


#endif

