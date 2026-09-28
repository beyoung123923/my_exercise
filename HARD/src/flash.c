#include "main.h"
#include "flash.h"

void flash_write(uint32_t write_addr, uint32_t *write_buf, uint32_t write_size,uint8_t page)
{
//	uint16_t addrx = 0;
	uint32_t endaddr = 0;

	endaddr = write_addr + write_size * 4;
	if((write_size * 4)<512 && page == 1)
	{
		FL_FLASH_PageErase(FLASH, write_addr);    //²Á³ýÒ³   
	}
	else
		FL_FLASH_SectorErase(FLASH, write_addr);     //²Á³ýÉÈÇø
	while(write_addr < endaddr)
	{
		if(FL_FLASH_Program_Word(FLASH, write_addr, *write_buf) != FL_PASS)
		{
			break;
		}
		write_addr += 4;
		write_buf ++;
	}
}

void flash_read(uint32_t read_addr, uint32_t *read_buf, uint32_t read_size)
{
	uint32_t read_pos = 0;
	while(read_pos < read_size)
	{
		read_buf[read_pos] = *(volatile uint32_t *)read_addr;
		read_pos++;
		read_addr += 4;
	}
}
