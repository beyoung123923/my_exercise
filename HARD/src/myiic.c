#include "myiic.h"
#include "main.h"
#include "delay.h"

void IIC_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
	//SCL
    GPIO_InitStruct.pin = FL_GPIO_PIN_11 | FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	IIC_SCL_H;
	IIC_SDA_H;
}

void IIC_deinit(void)    //有外部上拉电阻
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
    GPIO_InitStruct.pin = FL_GPIO_PIN_11 | FL_GPIO_PIN_12;
//	GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	IIC_SCL_H;
	IIC_SDA_H;
}

static void gpio_set_SDA_mode(uint8_t mode)
{
	FL_GPIO_InitTypeDef GPIO_Initure = {0};
	if(mode == gpio_input)
	{
		GPIO_Initure.mode=FL_GPIO_MODE_INPUT;
	}
	else if(mode == gpio_output)
	{
		GPIO_Initure.mode=FL_GPIO_MODE_OUTPUT; 		
	}
	
	GPIO_Initure.pin = FL_GPIO_PIN_12;                       		//SDA  
	GPIO_Initure.outputType = FL_GPIO_OUTPUT_PUSHPULL;			//FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_Initure.pull=FL_DISABLE;          //上拉	
	GPIO_Initure.remapPin   = FL_DISABLE;
	GPIO_Initure.analogSwitch= FL_DISABLE;	
	FL_GPIO_Init(GPIOA,&GPIO_Initure);
//	FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_PAD);//时钟打开
}

/*********************************************
 * 函数名：I2CStart
 * 描  述：开启I2C总线
 * 输  入：无
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
uint8_t I2CStart(void)
{
    IIC_SDA_H;
	sensor_delay_us(5);
    IIC_SCL_H;;
    sensor_delay_us(5);
	gpio_set_SDA_mode(gpio_input);
    if(!SDA_read)
	{
		gpio_set_SDA_mode(gpio_output);
		return FALSE;	//SDA线为低电平则总线忙,退出
	}
	gpio_set_SDA_mode(gpio_output);
    IIC_SDA_L;
    sensor_delay_us(5);
    IIC_SCL_L;
    sensor_delay_us(5);
    return TRUE;
}

/*********************************************
 * 函数名：I2CStop
 * 描  述：释放I2C总线
 * 输  入：无
 * 输  出：无
 ********************************************/
void I2CStop(void)
{
    IIC_SCL_L;
    sensor_delay_us(5);
    IIC_SDA_L;
    sensor_delay_us(5);
    IIC_SCL_H;;
    sensor_delay_us(5);
    IIC_SDA_H	;
    sensor_delay_us(5);
}

/*********************************************
 * 函数名：I2CAck
 * 描  述：发送ASK
 * 输  入：无
 * 输  出：无
 ********************************************/
void I2CAck(void)
{
    IIC_SCL_L;
    sensor_delay_us(5);
    IIC_SDA_L;
    sensor_delay_us(5);
    IIC_SCL_H;;
    sensor_delay_us(5);
    IIC_SCL_L;
    sensor_delay_us(5);
}

/*********************************************
 * 函数名：I2CNoAck
 * 描  述：发送NOASK
 * 输  入：无
 * 输  出：无
 ********************************************/
void I2CNoAck(void)
{
    IIC_SCL_L;
    sensor_delay_us(5);
    IIC_SDA_H;
    sensor_delay_us(5);
    IIC_SCL_H;;
    sensor_delay_us(5);
    IIC_SCL_L;
    sensor_delay_us(5);
}

/*********************************************
 * 函数名：I2CWaitAck
 * 描  述：读取ACK信号
 * 输  入：无
 * 输  出：TRUE=有ACK,FALSE=无ACK
 ********************************************/
uint8_t I2CWaitAck(void)
{
    IIC_SCL_L;
    sensor_delay_us(5);
    IIC_SDA_H;	
    gpio_set_SDA_mode(gpio_input);	
    sensor_delay_us(5);
    IIC_SCL_H;
    sensor_delay_us(5);
    if(SDA_read)
    { 
		IIC_SCL_L;
		gpio_set_SDA_mode(gpio_output);
		return FALSE;
    }
	gpio_set_SDA_mode(gpio_output);
    IIC_SCL_L;
    return TRUE;
}

/*********************************************
 * 函数名：I2CSendByte
 * 描  述：MCU发送一个字节
 * 输  入：无
 * 输  出：无
 ********************************************/
void I2CSendByte(uint8_t SendByte) //数据从高位到低位
{
    uint8_t i=8;
	while(i--)
	{
		IIC_SCL_L;
		sensor_delay_us(5);
		if(SendByte&0x80)
			IIC_SDA_H;  
		else 
			IIC_SDA_L;   
		SendByte<<=1;
		sensor_delay_us(5);
		IIC_SCL_H;;
		sensor_delay_us(5);
	}
    IIC_SCL_L;	
}

/*********************************************
 * 函数名：I2CReceiveByte
 * 描  述：MCU读入一个字节
 * 输  入：无
 * 输  出：ReceiveByte
 ********************************************/
uint8_t I2CReceiveByte(void)
{
    uint8_t i=8;
    uint8_t ReceiveByte=0;

    IIC_SDA_H	;
    gpio_set_SDA_mode(gpio_input);	
    while(i--)
    {
      ReceiveByte<<=1; 
      IIC_SCL_L;
      sensor_delay_us(5);
      IIC_SCL_H;;
      sensor_delay_us(5);	
      if(SDA_read)
      {
        ReceiveByte|=0x1;
      }
    }
	gpio_set_SDA_mode(gpio_output);
    IIC_SCL_L;
    return ReceiveByte;   
}



