#ifndef _FLASH_H_
#define _FLASH_H_


#include "main.h"


void flash_write(uint32_t write_addr, uint32_t *write_buf, uint32_t write_size,uint8_t page);

void flash_read(uint32_t read_addr, uint32_t *read_buf, uint32_t read_size);

#endif
