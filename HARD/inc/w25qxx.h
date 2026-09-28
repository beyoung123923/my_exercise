#ifndef __W25QXX_H
#define __W25QXX_H	

#include "fm33lg0xx_fl.h"
#include "main.h"

extern SemaphoreHandle_t w25q_semaphore;	//W25Qxx访问互斥量，定义在 main.c

//#define sFLASH_SPI                SPI2
//#define sFLASH_SPI_CLK            RCC_APB1_PERIPH_SPI2
//#define sFLASH_SPI_SCK_PIN        FL_GPIO_PIN_13 /* PB13 */
//#define sFLASH_SPI_SCK_GPIO_PORT  GPIOB       /* GPIOB */
//#define sFLASH_SPI_SCK_GPIO_CLK   RCC_APB2_PERIPH_GPIOB
//#define sFLASH_SPI_MISO_PIN       FL_GPIO_PIN_14 /* PB14 */
//#define sFLASH_SPI_MISO_GPIO_PORT GPIOB      /* GPIOB */
//#define sFLASH_SPI_MISO_GPIO_CLK  RCC_APB2_PERIPH_GPIOB
//#define sFLASH_SPI_MOSI_PIN       FL_GPIO_PIN_15 /* PB15 */
//#define sFLASH_SPI_MOSI_GPIO_PORT GPIOB      /* GPIOB */
//#define sFLASH_SPI_MOSI_GPIO_CLK  RCC_APB2_PERIPH_GPIOB
//#define sFLASH_CS_PIN             FL_GPIO_PIN_12 /* PB12 */
//#define sFLASH_CS_GPIO_PORT       GPIOB      /* GPIOB */
//#define sFLASH_CS_GPIO_CLK        RCC_APB2_PERIPH_GPIOB

#define flash_nss_pin         FL_GPIO_PIN_7
#define flash_sck_pin         FL_GPIO_PIN_8
#define flash_miso_pin        FL_GPIO_PIN_9
#define flash_mosi_pin        FL_GPIO_PIN_10
#define flash_spi_port        GPIOC
#define flash_spi			  SPI2	


#define sFLASH_CS_LOW()           GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
#define sFLASH_CS_HIGH()          FL_GPIO_SetOutputPin(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)

/*寄存器表*/
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 


void Spi2Init(void);

void Spi2deInit(void);

uint32_t Spi2WriteAndRead(uint32_t data);

void Spi2WriteData(uint8_t data);

uint8_t Spi2ReadData(uint8_t data);

void Spi2Writebuf(uint8_t *data, uint16_t length);

void Spi2Readbuf(uint8_t *data, uint16_t length);

void w25qxx_init(void);

void w25qxx_deinit(void);

void w25qxx_write(uint8_t *wr_buf, uint32_t wr_addr, uint16_t wr_size);

void w25qxx_read(uint8_t *read_buf, uint32_t read_addr, uint16_t read_size);

void w25qxx_erase_sector(uint32_t dst_addr);

void w25qxx_erase_chip(void);

void w25qxx_power_down(void);

void w25qxx_power_wakeup(void);


#endif

