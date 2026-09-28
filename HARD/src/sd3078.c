#include "main.h"
#include "SD3078.h"
#include "myiic.h"
#include "delay.h"



enum Freq{F_0Hz, F32KHz, F4096Hz, F1024Hz, F64Hz, F32Hz, F16Hz, F8Hz, \
					F4Hz, F2Hz, F1Hz, F1_2Hz, F1_4Hz, F1_8Hz, F1_16Hz, F_1s};
enum clk_Souce{S_4096Hz, S_1024Hz, S_1s, S_1min};


//typedef struct My_Time
//{
////	uint8_t year;
////	uint8_t month;
////	uint8_t day;
////	uint8_t hour;
////	uint8_t min;
////	uint8_t sec;
////	uint8_t week;
//	uint8_t buff[10];
//} MY_TIME,*time_p;

//MY_TIME sMyTime;

typedef	struct
{
	enum clk_Souce d_clk;
	uint8_t   IM;	//IM=1:周期性中断
	uint32_t  init_val;
} CountDown_Def;

int IntToBCD(int i) //十进制转BCD
{
	return (((i / 10) << 4) + ((i % 10) & 0x0f));
}
int BCDToInt(int bcd) //BCD转十进制
{
	return (0xff & (bcd >> 4)) * 10 + (0xf & bcd);
}
 
//比较两个字符串指定长度的内容是否相等
//参数:s1,s2要比较的两个字符串;len,比较长度
//返回值:1,相等;0,不相等
uint8_t my_strcmp(uint8_t*s1,uint8_t*s2,uint8_t len)
{
	uint8_t i;
	for(i=0;i<len;i++)if((*s1++)!=*s2++)return 0;
	return 1;	   
}

void I2C_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
	//SCL
    GPIO_InitStruct.pin = FL_GPIO_PIN_6 | FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	I2C_SCL_H;
	I2C_SDA_H;
}

void I2C_deinit(void)    //有外部上拉电阻
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
    GPIO_InitStruct.pin = FL_GPIO_PIN_6 | FL_GPIO_PIN_7;
//	GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	I2C_SCL_H;
	I2C_SDA_H;
}

static void config_gpio_SDA_mode(uint8_t mode)
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
	
	GPIO_Initure.pin = FL_GPIO_PIN_7;                       	//SDA  
	GPIO_Initure.outputType = FL_GPIO_OUTPUT_PUSHPULL;			//FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_Initure.pull=FL_DISABLE;          						//上拉	
	GPIO_Initure.remapPin   = FL_DISABLE;
	GPIO_Initure.analogSwitch= FL_DISABLE;	
	FL_GPIO_Init(GPIOA,&GPIO_Initure);
//	FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_PAD);//时钟打开
}

/*********************************************
 * 函数名：SD3078_I2CStart
 * 描  述：开启I2C总线
 * 输  入：无
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
uint8_t SD3078_I2CStart(void)
{
    I2C_SDA_H;
	rtc_delay_us(5);
    I2C_SCL_H;
    rtc_delay_us(5);
	config_gpio_SDA_mode(gpio_input);
    if(!SDA_get)
	{
		config_gpio_SDA_mode(gpio_output);
		return FALSE;	//SDA线为低电平则总线忙,退出
	}
	config_gpio_SDA_mode(gpio_output);
    I2C_SDA_L;
    rtc_delay_us(5);
    I2C_SCL_L;
    rtc_delay_us(5);
    return TRUE;
}

/*********************************************
 * 函数名：SD3078_I2CStop
 * 描  述：释放I2C总线
 * 输  入：无
 * 输  出：无
 ********************************************/
void SD3078_I2CStop(void)
{
    I2C_SCL_L;
    rtc_delay_us(5);
    I2C_SDA_L;
    rtc_delay_us(5);
    I2C_SCL_H;
    rtc_delay_us(5);
    I2C_SDA_H;
    rtc_delay_us(5);
}

/*********************************************
 * 函数名：SD3078_I2CAck
 * 描  述：发送ASK
 * 输  入：无
 * 输  出：无
 ********************************************/
void SD3078_I2CAck(void)
{
    I2C_SCL_L;
    rtc_delay_us(5);
    I2C_SDA_L;
    rtc_delay_us(5);
    I2C_SCL_H;
    rtc_delay_us(5);
    I2C_SCL_L;
    rtc_delay_us(5);
}

/*********************************************
 * 函数名：SD3078_I2CNoAck
 * 描  述：发送NOASK
 * 输  入：无
 * 输  出：无
 ********************************************/
void SD3078_I2CNoAck(void)
{
    I2C_SCL_L;
    rtc_delay_us(5);
    I2C_SDA_H;
    rtc_delay_us(5);
    I2C_SCL_H;
    rtc_delay_us(5);
    I2C_SCL_L;
    rtc_delay_us(5);
}

/*********************************************
 * 函数名：SD3078_I2CWaitAck
 * 描  述：读取ACK信号
 * 输  入：无
 * 输  出：TRUE=有ACK,FALSE=无ACK
 ********************************************/
uint8_t SD3078_I2CWaitAck(void)
{
    I2C_SCL_L;
    rtc_delay_us(5);
    I2C_SDA_H;	
    config_gpio_SDA_mode(gpio_input);	
    rtc_delay_us(5);
    I2C_SCL_H;
    rtc_delay_us(5);
    if(SDA_get)
    { 
		I2C_SCL_L;
		config_gpio_SDA_mode(gpio_output);
		return FALSE;
    }
	config_gpio_SDA_mode(gpio_output);
    I2C_SCL_L;
    return TRUE;
}

/*********************************************
 * 函数名：SD3078_I2CSendByte
 * 描  述：MCU发送一个字节
 * 输  入：无
 * 输  出：无
 ********************************************/
void SD3078_I2CSendByte(uint8_t SendByte) //数据从高位到低位
{
    uint8_t i=8;
	while(i--)
	{
		I2C_SCL_L;
		rtc_delay_us(5);
		if(SendByte&0x80)
			I2C_SDA_H;  
		else 
			I2C_SDA_L;   
		SendByte<<=1;
		rtc_delay_us(5);
		I2C_SCL_H;
		rtc_delay_us(5);
	}
    I2C_SCL_L;	
}

/*********************************************
 * 函数名：SD3078_I2CReceiveByte
 * 描  述：MCU读入一个字节
 * 输  入：无
 * 输  出：ReceiveByte
 ********************************************/
uint8_t SD3078_I2CReceiveByte(void)
{
    uint8_t i=8;
    uint8_t ReceiveByte=0;

    I2C_SDA_H;
    config_gpio_SDA_mode(gpio_input);	
    while(i--)
    {
      ReceiveByte<<=1; 
      I2C_SCL_L;
      rtc_delay_us(5);
      I2C_SCL_H;
      rtc_delay_us(5);	
      if(SDA_get)
      {
        ReceiveByte|=0x1;
      }
    }
	config_gpio_SDA_mode(gpio_output);
    I2C_SCL_L;
    return ReceiveByte;   
}


/*********************************************
 * 函数名：WriteRTC_Enable
 * 描  述：RTC写允许程序
 * 输  入：无
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
uint8_t WriteRTC_Enable(void)
{
//	portDISABLE_INTERRUPTS();
    if(FALSE == SD3078_I2CStart()) 
	{
		return FALSE;	
	}
    SD3078_I2CSendByte(SD3078_ADDR); 
    if(SD3078_I2CWaitAck()== FALSE)   
    {
		SD3078_I2CStop();
		return FALSE;
	}
    SD3078_I2CSendByte(SD3078_CTR2_ADDR);      
    SD3078_I2CWaitAck();	
    SD3078_I2CSendByte(0x80);//置WRTC1=1      
    SD3078_I2CWaitAck();
    SD3078_I2CStop(); 
										
    SD3078_I2CStart();
    SD3078_I2CSendByte(SD3078_ADDR);      
    SD3078_I2CWaitAck();   
    SD3078_I2CSendByte(SD3078_CTR1_ADDR);
    SD3078_I2CWaitAck();	
    SD3078_I2CSendByte(0x84);//置WRTC2,WRTC3=1      
    SD3078_I2CWaitAck();
    SD3078_I2CStop(); 
//		portENABLE_INTERRUPTS();
    return TRUE;
}

/*********************************************
 * 函数名：WriteRTC_Disable
 * 描  述：RTC写禁止程序
 * 输  入：无
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
uint8_t WriteRTC_Disable(void)
{
//	portDISABLE_INTERRUPTS();
    if(FALSE == SD3078_I2CStart())
	{
		return FALSE;
	}
    SD3078_I2CSendByte(SD3078_ADDR); 
    if(SD3078_I2CWaitAck() == FALSE)
    { 
		SD3078_I2CStop(); 
		return FALSE;
	}
    SD3078_I2CSendByte(SD3078_CTR1_ADDR);//设置写地址0FH      
    SD3078_I2CWaitAck();	
    SD3078_I2CSendByte(0x0) ;//置WRTC2,WRTC3=0      
    SD3078_I2CWaitAck();
    SD3078_I2CSendByte(0x0) ;//置WRTC1=0(10H地址)      
    SD3078_I2CWaitAck();
    SD3078_I2CStop(); 
//		portENABLE_INTERRUPTS();
    return TRUE; 
}

/*********************************************
 * 函数名：RTC_WriteDate
 * 描  述：写RTC实时数据寄存器
 * 输  入：时间结构体指针
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
static uint8_t RTC_WriteDate_raw(rtc_t *psRTC)	//写时间操作要求一次对实时时间寄存器(00H~06H)依次写入，
{                               //不可以单独对7个时间数据中的某一位进行写操作,否则可能会引起时间数据的错误进位. 
                                //要修改其中某一个数据 , 应一次性写入全部 7 个实时时钟数据.
	uint8_t time_buf[7];
	
//	taskENTER_CRITICAL();//portDISABLE_INTERRUPTS();
	time_buf[6]=IntToBCD(psRTC->year);
	time_buf[5]=IntToBCD(psRTC->month);
	time_buf[4]=IntToBCD(psRTC->day);
	time_buf[3]=IntToBCD(psRTC->week );
	
	time_buf[2]=IntToBCD(psRTC->hour);
	time_buf[1]=IntToBCD(psRTC->min);
	time_buf[0]=IntToBCD(psRTC->sec );

	WriteRTC_Enable();				//使能，开锁

	SD3078_I2CStart();
	SD3078_I2CSendByte(SD3078_ADDR); 
	if(SD3078_I2CWaitAck() == FALSE)
	{
		SD3078_I2CStop(); 
	    return FALSE;
	}
	SD3078_I2CSendByte(0);					 //设置写起始地址      
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[0]);		 //second     
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[1]);		 //minute      
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[2]|0x80);	 //hour ,同时设置小时寄存器最高位（0：为12小时制，1：为24小时制）
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[3]);		//week      
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[4]);		//day      
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[5]);		//month      
	SD3078_I2CWaitAck();	
	SD3078_I2CSendByte(time_buf[6]);		//year      
	SD3078_I2CWaitAck();	
	SD3078_I2CStop();

	WriteRTC_Disable();				  //关锁
		
//		taskEXIT_CRITICAL();//portENABLE_INTERRUPTS();
	return	TRUE;
}

uint8_t RTC_WriteDate(rtc_t *psRTC)
{
	uint8_t try_cnt;
	uint8_t re = FALSE;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)	//仅NAK重试，时间在走不做回读
	{
		xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
		re = RTC_WriteDate_raw(psRTC);
		xSemaphoreGive(sd3078_semaphore);
		if(re == TRUE)
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

/*********************************************
 * 函数名：RTC_ReadDate
 * 描  述：读RTC实时数据寄存器
 * 输  入：时间结构体指针
 * 输  出：TRUE:操作成功，FALSE:操作失败
 ********************************************/
static uint8_t RTC_ReadDate_raw(rtc_t *psRTC)
{
	uint8_t buff[10];
	
//	taskENTER_CRITICAL();//
//	portDISABLE_INTERRUPTS();
	SD3078_I2CStart();
	SD3078_I2CSendByte(SD3078_ADDR);      
	if(SD3078_I2CWaitAck()== FALSE)
	{
		SD3078_I2CStop(); 
		return FALSE;
	}
	SD3078_I2CSendByte(0);
	SD3078_I2CWaitAck();
	SD3078_I2CStart();	
	SD3078_I2CSendByte(SD3078_ADDR+1);
	SD3078_I2CWaitAck();
	psRTC->sec=SD3078_I2CReceiveByte();
	SD3078_I2CAck();
	psRTC->min=SD3078_I2CReceiveByte();
	SD3078_I2CAck();
	psRTC->hour=SD3078_I2CReceiveByte() & 0x7F;
	SD3078_I2CAck();
	psRTC->week=SD3078_I2CReceiveByte();
	SD3078_I2CAck();
	psRTC->day=SD3078_I2CReceiveByte();
	SD3078_I2CAck();
	psRTC->month=SD3078_I2CReceiveByte();
	SD3078_I2CAck();
	psRTC->year=SD3078_I2CReceiveByte();		
	SD3078_I2CNoAck();
	SD3078_I2CStop(); 

//	portENABLE_INTERRUPTS();

	buff[0]=BCDToInt(psRTC->sec);    //十进制
	buff[1]=BCDToInt(psRTC->min);
	buff[2]=BCDToInt(psRTC->hour);
	
	buff[3]=BCDToInt(psRTC->day);
	buff[4]=BCDToInt(psRTC->month);
	buff[5]=BCDToInt(psRTC->year);
	buff[6]=BCDToInt(psRTC->week);
			
	psRTC->sec=	buff[0];
	psRTC->min= buff[1];
	psRTC->hour= buff[2];
	psRTC->day= buff[3];
	psRTC->month= buff[4];
	psRTC->year= buff[5];	
	psRTC->week= buff[6];	
//	taskEXIT_CRITICAL();//
	return	TRUE;
}

uint8_t RTC_ReadDate(rtc_t *psRTC)
{
	uint8_t try_cnt;
	uint8_t re = FALSE;
	memset(psRTC, 0, sizeof(rtc_t));		//失败时输出确定的全零
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
		re = RTC_ReadDate_raw(psRTC);
		xSemaphoreGive(sd3078_semaphore);
		if((re == TRUE)
		&&(psRTC->month >= 1)&&(psRTC->month <= 12)
		&&(psRTC->hour < 24)&&(psRTC->min < 60)&&(psRTC->sec < 60))
		{
			return TRUE;	//有效时间才返回成功
		}
		memset(psRTC, 0, sizeof(rtc_t));
		vTaskDelay(20);
	}
	return FALSE;
}
/*********************************************
 * 函数名     ：I2CWriteSerial
 * 描  述     ：I2C在指定地址写一字节数据
 * Device_Addr：I2C设备地址
 * Address    ：内部地址
 * length     ：字节长度
 * ps         ：缓存区指针
 * 输出       ：TRUE 成功，FALSE 失败
 ********************************************/	
static uint8_t I2CWriteSerial_raw(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps)
{
	//portDISABLE_INTERRUPTS();
	if(DeviceAddress == SD3078_ADDR)  WriteRTC_Enable();

	SD3078_I2CStart();
	SD3078_I2CSendByte(DeviceAddress);   
	if(SD3078_I2CWaitAck()== FALSE)
	{
		SD3078_I2CStop(); 
		return FALSE;
	}
	SD3078_I2CSendByte(Address);			
	SD3078_I2CWaitAck();
	for(;(length--)>0;)
	{ 	
		SD3078_I2CSendByte(*(ps++));		
		SD3078_I2CAck();			
	}
	SD3078_I2CStop(); 
	
	if(DeviceAddress == SD3078_ADDR)  WriteRTC_Disable();
//	portENABLE_INTERRUPTS();
	return	TRUE;
}

uint8_t I2CWriteSerial(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps)
{
	uint8_t re;
	xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
	re = I2CWriteSerial_raw(DeviceAddress, Address, length, ps);
	xSemaphoreGive(sd3078_semaphore);
	return re;
}

/*********************************************
 * 函数名     ：I2CReadSerial
 * 描  述     ：I2C在指定地址读一字节数据
 * Device_Addr：I2C设备地址
 * Address    ：内部地址
 * length     ：字节长度
 * ps         ：缓存区指针
 * 输出       ：TRUE 成功，FALSE 失败
 ********************************************/	
static uint8_t I2CReadSerial_raw(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps)
{
//	taskENTER_CRITICAL();// 
//	portDISABLE_INTERRUPTS();
	SD3078_I2CStart();
	SD3078_I2CSendByte(DeviceAddress);      
	if(SD3078_I2CWaitAck()== FALSE)
	{
		SD3078_I2CStop(); 
		return FALSE;
	}
	SD3078_I2CSendByte(Address);
	SD3078_I2CWaitAck();
	SD3078_I2CStart();	
	SD3078_I2CSendByte(DeviceAddress+1);
	SD3078_I2CWaitAck();
	for(;--length>0;ps++)
	{
		*ps = SD3078_I2CReceiveByte();
		SD3078_I2CAck();
	}
	*ps = SD3078_I2CReceiveByte();	
	SD3078_I2CNoAck();
	SD3078_I2CStop(); 
//	portENABLE_INTERRUPTS();
//		taskEXIT_CRITICAL();
	return	TRUE;
}

uint8_t I2CReadSerial(uint8_t DeviceAddress, uint8_t Address, uint8_t length, uint8_t *ps)
{
	uint8_t re;
	xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
	re = I2CReadSerial_raw(DeviceAddress, Address, length, ps);
	xSemaphoreGive(sd3078_semaphore);
	return re;
}

/*********************************************
 * 函数名：Set_CountDown
 * 描  述：设置倒计时中断
 * 输  入：CountDown_Init 倒计时中断结构体指针 
 * 输  出：无
 ********************************************/
static void Set_CountDown_raw(CountDown_Def *CountDown_Init)					
{
	uint8_t buf[6];
	uint8_t tem=0xF0;
	buf[0] = (CountDown_Init->IM<<6)|0xB4;							 //10H
	buf[1] = CountDown_Init->d_clk<<4;									 //11H
	buf[2] = 0;																					 //12H
	buf[3] = CountDown_Init->init_val & 0x0000FF;				 //13H
	buf[4] = (CountDown_Init->init_val & 0x00FF00) >> 8; //14H
	buf[5] = (CountDown_Init->init_val & 0xFF0000) >> 16;//15H
	I2CWriteSerial_raw(SD3078_ADDR,SD3078_CTR2_ADDR,1,&tem);
	I2CWriteSerial_raw(SD3078_ADDR,SD3078_CTR2_ADDR,6,buf);
}

void Set_CountDown(CountDown_Def *CountDown_Init)
{
	xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
	Set_CountDown_raw(CountDown_Init);
	xSemaphoreGive(sd3078_semaphore);
}

/*********************************************
 * 函数名：Set_Alarm
 * 描  述：设置报警中断（闹钟功能）
 * Enable_config：使能设置  
 * psRTC：报警时间的时间结构体指针
 * 输  出：无
 ********************************************/
void Set_Alarm(uint8_t Enable_config, rtc_t *psRTC)					
{
	uint8_t buf[10];
	buf[0] = psRTC->sec;
	buf[1] = psRTC->min;
	buf[2] = psRTC->hour;
	buf[3] = 0;
	buf[4] = psRTC->day;
	buf[5] = psRTC->month;
	buf[6] = psRTC->year;
	buf[7] = Enable_config;
	buf[8] = 0xFF;
	buf[9] = 0x92;	
	I2CWriteSerial(SD3078_ADDR,Alarm_SC,10,buf);
}

/*********************************************
 * 函数名：SetFrq
 * 描  述：设置RTC频率中断，从INT脚输出频率方波
 * 输  入：频率值
 * 输  出：无
 ********************************************/
void SetFrq(enum Freq F_Out)					
{
	uint8_t buf[2];
	buf[0] = 0xA1;
	buf[1] = F_Out;
	I2CWriteSerial(SD3078_ADDR,SD3078_CTR2_ADDR,2,buf);
}

/*********************************************
 * 函数名：ClrINT
 * 描  述：禁止中断
 * int_EN：中断类型 INTDE、INTDE、INTDE
 * 输  出：无
 ********************************************/
static void ClrINT_raw(uint8_t int_EN)         
{
	uint8_t buf;
	buf = 0x80 & (~int_EN);
	I2CWriteSerial_raw(SD3078_ADDR,SD3078_CTR2_ADDR,1,&buf);
}

void ClrINT(uint8_t int_EN)
{
	xSemaphoreTake(sd3078_semaphore,portMAX_DELAY);
	ClrINT_raw(int_EN);
	xSemaphoreGive(sd3078_semaphore);
}
/****************************************************************************************/
void sd3078_alarm(void)
{	
	CountDown_Def cdInit;
//	
// #if (INT_TYPE == FREQUENCY)
///**************** 频率中断设置演示 ****************/
//	SetFrq(F2Hz);//输出2Hz频率方波
//#elif (INT_TYPE == ALARM)
///**************** 报警中断设置演示 ****************/
//	Set_Alarm(sec_ALM|min_ALM|hor_ALM, &Alarm_init);//报警使能：时、分、秒
//#elif (INT_TYPE == COUNTDOWN)	
/*************** 倒计时中断设置演示***************/
	cdInit.IM = 1;				//设置为周期性中断
	cdInit.d_clk = S_1s;		//倒计时中断源选择1s
//	if(device_info.save.second_mode == ON)
//		cdInit.init_val = 1;	//倒计时初值设置为1
//	else
//		cdInit.init_val = 10;	//倒计时初值设置为10
	cdInit.init_val = 10;
	Set_CountDown(&cdInit);
//#elif (INT_TYPE == DISABLE)	
///****************** 禁止中断演示 *****************/
//	ClrINT(INTDE|INTAE|INTFE);
//#endif

}
/****************************************************************************************/
void sd3078_dealarm(void)
{
	//portDISABLE_INTERRUPTS();
 	ClrINT(INTDE|INTAE|INTFE);//倒计时、报警、频率
	//portENABLE_INTERRUPTS();
}
	
/****************************************************************************************/
//void sd3078_alarm_Callback(void)
//{	
//	iwdt_reload();  //喂狗
//	device_info.sensor_timer++;				//采集间隔
//	device_info.sensor2_timer++;
//	device_info.wait_min++;	  				//上报计时时间点累加
//	device_info.vabt_timer++;    //电量更新时间点累加
//	device_info.time_read++;    //时钟更新计数器
//	device_info.save_min++;     //数据保存计数器
//  
//  if(device_info.save.shock_status == ON)
//    device_info.shock_delay_time_num++;    //震动报警频率计数器
//	
//	if(device_info.ble_connect_status == 1)
//		device_info.ble_connect_time++;
//	
//	if(device_info.save.zhengdian == ON)
//	{
//		device_info.zhengdian_min++;
//	}
//	
////	if(ble_print<10)
////		ble_print++;
//}
/****************************************************************************************/

uint8_t sd3078_ram_set(uint8_t Address, uint32_t setbuf)
{
	uint8_t i;
	uint8_t ram_setbuf[4];//转换为字节
	
	for(i=0;i<4;i++)
	{	
		ram_setbuf[i]=(setbuf>>((3-i)*8))& 0xFF;       //高  低 
	}
	i=I2CWriteSerial(SD3078_ADDR,Address,4,ram_setbuf);
/*	for(k=0;k<3;k++)
  {	
		i=I2CWriteSerial(SD3078_ADDR,Address,4,ram_setbuf);
		I2CReadSerial(SD3078_ADDR, Address, 4, ram_setbuf2);
		for(j=0;j<4;j++)
		{	if(ram_setbuf[j] != ram_setbuf2[j])
			{ delay_vtask(1);	
				break;
			}
		}
		if(j==4)
		{	break;
		}	
	}
	if(k==3)
	{		Trace_log("----------SD3078写失败----------\r\n");
	}*/
	return i;
}

/****************************************************************************************/
uint32_t sd3078_ram_get(uint8_t Address)
{
	uint8_t ram_getbuf[4] = {0xFF,0xFF,0xFF,0xFF};//4
	uint32_t getbuf=0;
	
	//转换为字节
	if(I2CReadSerial(SD3078_ADDR, Address, 4, ram_getbuf) == FALSE)
	{
		return 0xFFFFFFFF;//失败返回无效值(与RTC掉电读出的擦除态一致)，调用方按范围检查兜底
	}
	getbuf=(uint32_t)(ram_getbuf[0]<<24)|(uint32_t)(ram_getbuf[1]<<16)|(uint32_t)(ram_getbuf[2]<<8)|ram_getbuf[3];
	
	return  getbuf;
}

//断点续传地址
uint32_t device_perv_addr_get(void)
{
	uint32_t num = sd3078_ram_get(user_ram2);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
        trace_log("续传地址读取失败\n");
		return 0;
	}
	return num;
}
uint8_t device_perv_addr_set(uint32_t pend_addr)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram2,pend_addr) == TRUE)
		&&(sd3078_ram_get(user_ram2) == pend_addr))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

//断点续传数
uint32_t device_pending_num_get(void)
{
	uint32_t num = sd3078_ram_get(user_ram3);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
        trace_log("续传数读取失败\n");
		return 0;
	}
	return num;
}
uint8_t device_pending_num_set(uint32_t num)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram3,num) == TRUE)
		&&(sd3078_ram_get(user_ram3) == num))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

//flash地址
uint32_t device_get_flashaddr_addr(void)
{
	uint32_t num = sd3078_ram_get(user_ram8);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
        trace_log("flash地址读取失败\n");
		return 0;
	}
	return num;
}
uint8_t device_set_flashaddr_addr(uint32_t flag_addr)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram8,flag_addr) == TRUE)
		&&(sd3078_ram_get(user_ram8) == flag_addr))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

//历史数据记录个数
uint32_t device_get_record_num(void)
{
	uint32_t num = sd3078_ram_get(user_ram5);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
        trace_log("历史数据个数读取失败\n");
		return 0;
	}
	return num;
}
uint8_t device_set_record_num(uint32_t num)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram5,num) == TRUE)
		&&(sd3078_ram_get(user_ram5) == num))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

//模块重启次数
uint32_t device_get_restar_num(void)
{
	uint32_t num = sd3078_ram_get(user_ram7);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
		return 0;
	}
	return num;
}
uint8_t device_set_restart_num(uint32_t num)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram7,num) == TRUE)
		&&(sd3078_ram_get(user_ram7) == num))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

//配置参数
uint32_t device_get_updata_cloude(void)
{
	uint32_t num = sd3078_ram_get(user_ram6);
	if(num == 0xFFFFFFFF)	//读取失败或RTC RAM无效，返回0
	{
		return 0;
	}
	return num;
}
uint8_t device_set_updata_cloud(uint32_t num)
{
	uint8_t try_cnt;
	for(try_cnt = 0; try_cnt < 3; try_cnt++)
	{
		if((sd3078_ram_set(user_ram6,num) == TRUE)
		&&(sd3078_ram_get(user_ram6) == num))
		{
			return TRUE;
		}
		vTaskDelay(20);
	}
	return FALSE;
}

/*********************************************END OF FILE**********************/


