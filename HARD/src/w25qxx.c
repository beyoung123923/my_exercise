/*
 * @Author: your name
 * @Date: 2020-08-07 11:38:13
 * @LastEditTime: 2020-08-24 10:23:19
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \FM32_NB温湿度\BSP\Src\w25q32.c
 */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "w25qxx.h"
//
/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
/* 变量 --------------------------------------------------------------*/


/* 类型定义 ----------------------------------------------------------*/
/* 函数声明 ----------------------------------------------------------*/
static void w25qxx_wait_busy(void);
static uint8_t w25qxx_rd_sr(void);
//static void w25qxx_wr_sr(uint8_t sr);
static void w25qxx_wr_enable(void);
//static void w25qxx_wr_disable(void);
static void w25qxx_write_page(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size);
static void w25qxx_write_nocheck(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size);
/* 函数代码 ----------------------------------------------------------*/

void Spi2Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_SPI_InitTypeDef    SPI2_InitStruct;
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_7 | FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
	FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
    
	SPI2_InitStruct.softControl = FL_ENABLE;
    SPI2_InitStruct.mode = FL_SPI_WORK_MODE_MASTER;
    SPI2_InitStruct.baudRate = FL_SPI_BAUDRATE_DIV2;
    SPI2_InitStruct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    SPI2_InitStruct.dataWidth = FL_SPI_DATA_WIDTH_8B;
    SPI2_InitStruct.clockPhase = FL_SPI_PHASE_EDGE1;
    SPI2_InitStruct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    SPI2_InitStruct.clockPolarity = FL_SPI_POLARITY_NORMAL;
	FL_SPI_Init(SPI2, &SPI2_InitStruct); 
	
    FL_SPI_ClearTXBuff(SPI2);
    FL_SPI_ClearRXBuff(SPI2);
}

void Spi2deInit(void)
{
	FL_GPIO_InitTypeDef gpioInitStruct = {0};
	FL_SPI_DeInit(SPI2);
    
    gpioInitStruct.pin =  FL_GPIO_PIN_7 | FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10;
//	gpioInitStruct.pin =  FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10;
    gpioInitStruct.mode = FL_GPIO_MODE_INPUT;
    gpioInitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpioInitStruct.pull = FL_DISABLE;
    gpioInitStruct.remapPin = FL_DISABLE;
	gpioInitStruct.analogSwitch = FL_DISABLE;
	FL_GPIO_Init(GPIOC, &gpioInitStruct);
	
	FL_GPIO_DisablePinInput(GPIOC, FL_GPIO_PIN_7 | FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10);	
//	FL_GPIO_DisablePinInput(GPIOC, FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10);		
	
	
//	gpioInitStruct.pin = FL_GPIO_PIN_7;
//    gpioInitStruct.mode = FL_GPIO_MODE_OUTPUT;
//    gpioInitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
//    gpioInitStruct.pull = FL_ENABLE;
//    gpioInitStruct.remapPin = FL_DISABLE;
//	gpioInitStruct.analogSwitch = FL_DISABLE;
//	FL_GPIO_Init(GPIOC, &gpioInitStruct);	
}
/*******************************************************************/
uint32_t Spi2WriteAndRead(uint32_t data)
{
//    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	uint16_t wait_num = 0;
	FL_SPI_WriteTXBuff(SPI2, data);
	while (!(FL_SPI_IsActiveFlag_TXBuffEmpty(SPI2)))
	{
		if(wait_num++ >= 1000)
		{
			Spi2Init();
			break;
		}
		vTaskDelay(1);
	}
	wait_num = 0;
	while (!(FL_SPI_IsActiveFlag_RXBuffFull(SPI2)))
	{
		if(wait_num++ >= 1000)
		{
			Spi2Init();
			break;
		}
		vTaskDelay(1);
	}
	data = FL_SPI_ReadRXBuff(SPI2);
	
//    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	return data;
}
/*******************************************************************/
void Spi2WriteData(uint8_t data)
{
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);   //片选
	FL_SPI_WriteTXBuff(SPI2, data); 
	while (!(FL_SPI_IsActiveFlag_TXBuffEmpty(SPI2)));
    while (!(FL_SPI_IsActiveFlag_RXBuffFull(SPI2)));
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}
/*******************************************************************/
uint8_t Spi2ReadData(uint8_t data)
{
	uint8_t spi_data;
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	FL_SPI_WriteTXBuff(SPI2, data);
	while (!(FL_SPI_IsActiveFlag_RXBuffFull(SPI2)));
	spi_data = FL_SPI_ReadRXBuff(SPI2);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	return spi_data;
}
/*******************************************************************/
void Spi2Writebuf(uint8_t *data, uint16_t length)
{
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
    while (length--)
    {
        FL_SPI_WriteTXBuff(SPI2, *data); data++;
        while (!(FL_SPI_IsActiveFlag_TXBuffEmpty(SPI2)));
    }
    while (!(FL_SPI_IsActiveFlag_RXBuffFull(SPI2)));
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}
/*******************************************************************/
void Spi2Readbuf(uint8_t *data, uint16_t length)
{
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
    while (length--)
    {
        FL_SPI_WriteTXBuff(SPI2, 0x55);
        while (!(FL_SPI_IsActiveFlag_RXBuffFull(SPI2)));
        *data = FL_SPI_ReadRXBuff(SPI2); data++;
    }
    FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}

/**
 * @description:  等待FLASH芯片操
 * @param {type} 
 * @return {type} 
 */
static void w25qxx_wait_busy(void)
{
	uint16_t wait_num = 0;

	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);		
	while((w25qxx_rd_sr() & 0X01) == 0X01)
	{
		if(wait_num++ >= 3000)
		{
			break;
		}
		vTaskDelay(10);
	}
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}

/**
 * @description:  读取W25QXX状态寄存器
 * @param {type} 
 * @return {type} 
 */
static uint8_t w25qxx_rd_sr(void)
{
	uint8_t sr = 0;
	Spi2WriteAndRead(W25X_ReadStatusReg);
	sr = Spi2WriteAndRead(0Xff);
	return sr;
}

/**
 * @description:  使能写入
 * @param {type} 
 * @return {type} 
 */
static void w25qxx_wr_enable(void)
{
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	Spi2WriteAndRead(W25X_WriteEnable);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}

/**
 * @description:  关闭写入操作
 * @param {type} 
 * @return {type} 
 */
//static void w25qxx_wr_disable(void)
//{
//	spi_rd_wr(W25X_WriteDisable);
//}

/**
 * @description:  写入FLASH页
 * @param  wr_buf:要写入的数组指针
 * @param  wr_addr 写入的地址
 * @param  wr_szie 写入的大小
 * @return None
 */
static void w25qxx_write_page(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size)
{
	w25qxx_wait_busy();
	
	w25qxx_wr_enable();

	w25qxx_wait_busy();
	portENTER_CRITICAL();
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	
	Spi2WriteAndRead(W25X_PageProgram);
	Spi2WriteAndRead((uint8_t)((wr_addr) >> 16));
	Spi2WriteAndRead((uint8_t)((wr_addr) >> 8));
	Spi2WriteAndRead((uint8_t)wr_addr);
	
	while(wr_size--)
	{
		Spi2WriteAndRead( *wr_buf++);
	}
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	portEXIT_CRITICAL();
	w25qxx_wait_busy();
}
/**
 * @description:  写入FLASH 最大长度不大于65535 需要预先检查flash
 * @param  wr_buf:要写入的数组指针
 * @param  wr_addr 写入的地址
 * @param  wr_szie 写入的大小
 * @return None
 */
static void w25qxx_write_nocheck(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size)
{
	uint16_t pagermain;
	pagermain = 256 - wr_addr % 256;  /*求出剩余空间*/
	if(wr_size <= pagermain)   
	{
		pagermain = wr_size;  /*要写入的小于剩余空间直接写入，如果大于先写入剩余空间*/
	}
	while(1)
	{
		w25qxx_write_page(wr_buf, wr_addr, pagermain);
		if(wr_size == pagermain)
		{
			break; /*写完逃掉*/
		}
		else 
		{
			wr_buf += pagermain;
			wr_addr += pagermain;
			wr_size -= pagermain;
			
			if(wr_size > 256)
			{
				pagermain = 256;
			}else 
			{
				pagermain = wr_size;
			}
		}
	};
}

/**
 * @description:  擦除芯片
 * @param {type} 
 * @return {type} 
 */
void w25qxx_erase_chip(void)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	w25qxx_wait_busy();	
	
	w25qxx_wr_enable();
	
	w25qxx_wait_busy();	
	
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	Spi2WriteAndRead(W25X_ChipErase);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	
	w25qxx_wait_busy();
	xSemaphoreGive(w25q_semaphore);
}

/**
 * @description:  擦除芯片
 * @param {type} 
 * @return {type} 
 */
void w25qxx_erase_sector(uint32_t dst_addr)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	w25qxx_wait_busy();	
	
	w25qxx_wr_enable();
	
	w25qxx_wait_busy();
	
	portENTER_CRITICAL();
	
	/*清楚页*/
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	Spi2WriteAndRead( W25X_SectorErase);
	Spi2WriteAndRead( (uint8_t)((dst_addr) >> 16));
	Spi2WriteAndRead( (uint8_t)((dst_addr) >> 8));
	Spi2WriteAndRead( (uint8_t)dst_addr);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	
	portEXIT_CRITICAL();

	w25qxx_wait_busy();
	xSemaphoreGive(w25q_semaphore);
}

/**
 * @description: FLASH芯片休眠
 * @param  None
 * @return None
 */
void w25qxx_power_down(void)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	Spi2WriteAndRead( W25X_PowerDown);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	xSemaphoreGive(w25q_semaphore);
}

/**
 * @description: FLASH芯片唤醒
 * @param  None
 * @return None
 */
static void w25qxx_power_wakeup_raw(void)
{
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	Spi2WriteAndRead(W25X_ReleasePowerDown);
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
}
/**
 * @description: FLASH唤醒（内部raw版本，供持锁路径调用）
 */
void w25qxx_power_wakeup(void)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	w25qxx_power_wakeup_raw();
	xSemaphoreGive(w25q_semaphore);
}
/**
 * @description:  FLASH读取
 * @param  read_buf: 读取保存的数据指针地址
 * @param  read_addr 读取的FLASH地址
 * @param  read_szie 要读取的大小
 * @return None
 */
void w25qxx_read(uint8_t *read_buf, uint32_t read_addr, uint16_t read_size)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	w25qxx_wait_busy();
	
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);
	
	Spi2WriteAndRead(W25X_ReadData);
	Spi2WriteAndRead((uint8_t)((read_addr) >> 16));
	Spi2WriteAndRead((uint8_t)((read_addr) >> 8));
	Spi2WriteAndRead((uint8_t)read_addr);
	while(read_size--)
	{
		*read_buf++ = Spi2WriteAndRead(0XFF);
	}
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	
	xSemaphoreGive(w25q_semaphore);
}

/**
 * @description: w25qxx写入
 * @param  wr_buf:要写入的数组指针
 * @param  wr_addr 写入的地址
 * @param  wr_szie 写入的大小
 * @return None
 */
void w25qxx_write(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	w25qxx_write_nocheck(wr_buf, wr_addr, wr_size); /*直接写入 因为都是FF*/
	xSemaphoreGive(w25q_semaphore);
}
/**
 * @description:  w25qxx进入休眠状态
 * @param  None
 * @return None
 */
void w25qxx_enter_sleep_mode(void)
{
	FL_GPIO_InitTypeDef io_config = {0}; 
	io_config.pin        = FL_GPIO_PIN_7 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10 | FL_GPIO_PIN_8;
	io_config.mode       = FL_GPIO_MODE_INPUT;
	io_config.pull       = FL_DISABLE;
	io_config.remapPin   = FL_DISABLE;
	
	FL_GPIO_Init(GPIOC, &io_config);
	FL_GPIO_DisablePinInput(GPIOC, FL_GPIO_PIN_7 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10 | FL_GPIO_PIN_8);
	
}

/**
 * @description:  w25qxx退出休眠状态
 * @param  None
 * @return None
 */
void w25qxx_exti_sleep_mode(void)
{
	FL_GPIO_InitTypeDef io_config = {0}; 
	io_config.pin = FL_GPIO_PIN_7 | FL_GPIO_PIN_9 | FL_GPIO_PIN_10 | FL_GPIO_PIN_8;
	io_config.mode = FL_GPIO_MODE_DIGITAL;
	io_config.outputType = FL_GPIO_OUTPUT_PUSHPULL;
	io_config.pull = FL_ENABLE;
	io_config.remapPin = FL_DISABLE;
	FL_GPIO_Init(GPIOC, &io_config);

	FL_GPIO_EnablePinInput(GPIOC, FL_GPIO_PIN_9 | FL_GPIO_PIN_10 | FL_GPIO_PIN_7 | FL_GPIO_PIN_8);
}

/**
 * @description: 初始化W25qXX
 * @param  None
 * @return None
 */
void w25qxx_init(void)
{
	xSemaphoreTake(w25q_semaphore,portMAX_DELAY);
	uint8_t err_cnt = 0;
  
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_LOW);

	while((w25qxx_rd_sr() & 0X01) == 0X01)
	{
		if(++err_cnt > 10)
		{
			break;
		}
		vTaskDelay(10);
	}
	FL_SPI_SetSSNPin(SPI2, FL_SPI_SSN_HIGH);
	w25qxx_power_wakeup_raw();
	xSemaphoreGive(w25q_semaphore);
}
