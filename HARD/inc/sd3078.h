#ifndef __SD3078_H
#define __SD3078_H

#include "fm33lg0xx_fl.h"
#include "sys_setting.h"

//SD3078使用的iic
#define I2C_SCL_H  		FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_6)  	//输出
#define I2C_SCL_L  		FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_6)  	//输出

#define I2C_SDA_H  		FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_7)   	//输出 
#define I2C_SDA_L  		FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_7)

#define SDA_get    		FL_GPIO_GetInputPin(GPIOA,FL_GPIO_PIN_7)


#define	SD3078_ADDR					0x64		//SD3078设备地址

#define		Alarm_SC				0x07
#define		Alarm_MN				0x08
#define		Alarm_HR				0x09
#define		Alarm_WK				0x0A
#define		Alarm_DY				0x0B
#define		Alarm_MO				0x0C
#define		Alarm_YR				0x0D
#define		Alarm_EN				0x0E

#define SD3078_SECOND_ADDR			0x00		//秒
#define SD3078_MINUTE_ADDR			0x01		//分
#define SD3078_HOUR_ADDR			0x02		//时
#define SD3078_DAY_ADDR				0x04		//天
#define SD3078_MONTH_ADDR			0x05		//月
#define SD3078_YEAR_ADDR			0x06		//年

#define SD3078_CTR1_ADDR			0x0F		//控制器1
#define SD3078_CTR2_ADDR			0x10		//控制器2
#define SD3078_CTR3_ADDR			0x11		//控制器3
#define SD3078_TIME_LOW_ADDR		0x13		//定时器数据低位
#define SD3078_TIME_MID_ADDR		0x14		//定时器数据中位
#define SD3078_TIME_HIG_ADDR		0x15		//定时器数据高位


#define SD3078_CTR2_COM				0x74		//启动周期性倒计时
#define SD3078_CTR3_COM				0x30		//倒计时频率1S

/********************** 中断使能宏定义 **********************/
#define		INTDE						0x04		//倒计时中断
#define		INTAE						0x02		//报警中断
#define		INTFE						0x01		//频率中断

/*********************Users RAM    2CH----71H  ******************************/
#define   user_ram1       0x2c    //电池电量存储
#define   user_ram2       0x30    //断点续传地址
#define   user_ram3       0x34    //断点续传个数
#define   user_ram4       0x38    //看门狗复位标志
#define   user_ram5       0x3c    //记录历史数据个数
#define   user_ram6       0x40    //参数配置
#define   user_ram7       0x44    //重启次数存储
#define   user_ram8       0x48    //历史记录存储地址


void I2C_init(void);
void I2C_deinit(void);

uint8_t RTC_WriteDate(rtc_t	*psRTC);
uint8_t RTC_ReadDate(rtc_t	*psRTC);
void sd3078_alarm(void);
void sd3078_dealarm(void);
uint8_t sd3078_ram_set(uint8_t Address, uint32_t setbuf);
uint32_t sd3078_ram_get(uint8_t Address);

//断点续传地址
uint8_t device_perv_addr_set(uint32_t pend_addr);
uint32_t device_perv_addr_get(void);

//断点续传数
uint8_t device_pending_num_set(uint32_t num);
uint32_t device_pending_num_get(void);

//flash地址
uint8_t device_set_flashaddr_addr(uint32_t flag_addr);
uint32_t device_get_flashaddr_addr(void);

//历史数据记录个数
uint8_t device_set_record_num(uint32_t num);
uint32_t device_get_record_num(void);

//模块重启次数
uint8_t device_set_restart_num(uint32_t num);
uint32_t device_get_restar_num(void);

//配置参数
uint8_t device_set_updata_cloud(uint32_t num);
uint32_t device_get_updata_cloude(void);

#endif
