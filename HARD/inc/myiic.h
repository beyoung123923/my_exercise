#ifndef _MYIIC_H
#define _MYIIC_H

#include "main.h"

//SENSOR使用的iic
#define gpio_input 		0
#define gpio_output 	1

#define IIC_SCL_H  		FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_11)  	//输出
#define IIC_SCL_L  		FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_11)  	//输出

#define IIC_SDA_H  		FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_12)   	//输出 
#define IIC_SDA_L  		FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_12)

#define SDA_read    	FL_GPIO_GetInputPin(GPIOA,FL_GPIO_PIN_12)


//void ADS1115_IIC1_init(void);
//void ADS1115_IIC1_deinit(void);
void I2Cdelay(void);
void IIC_init(void);
void IIC_deinit(void);
uint8_t I2CStart(void);
void I2CStop(void);
void I2CAck(void);
void I2CNoAck(void);
uint8_t I2CWaitAck(void);
void I2CSendByte(uint8_t SendByte);
uint8_t I2CReceiveByte(void);
uint8_t I2CWriteSerial(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps);
uint8_t I2CReadSerial(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps);
#endif

