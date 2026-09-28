#ifndef __SENSOR_H
#define __SENSOR_H

#include "fm33lg0xx_fl.h"
#include "fm33lg0xx.h"

#define SENSOR_EVENT_GET_VALUE    0x01 << 0
#define SENSOR_EVENT_DS_ALARM	  0x01 << 1

#define SDA0_READ			FL_GPIO_GetInputPin(GPIOD,FL_GPIO_PIN_0)
#define SDA0_OUT_H			FL_GPIO_SetOutputPin(GPIOD,FL_GPIO_PIN_0)
#define SDA0_OUT_L			FL_GPIO_ResetOutputPin(GPIOD,FL_GPIO_PIN_0)

#define INTERNAL_TH_ADDR	0x88
//#define	GXHT30C_ADDR	

#define CUSTOMIZE    0x00
#define DS18B20      0x01
//#define DI_          0x02

#define Single_PIN		0

//#define SN2_OUT_MODE()  GPIOx_MODE_SET(SN2,Single_PIN,GPIO_MODE_OUTPUT_PP)    //SN2输出
//#define SN2_IN_MODE()   GPIOx_MODE_SET(SN2,Single_PIN,GPIO_MODE_INPUT)        //SN2输入

/*#define SN2_OUT_H()     FL_GPIO_SetOutputPin(SN2,SN2_Pin)	                  		//SN2输出高
#define SN2_OUT_L()     GPIO_ResetBits(SN2,SN2_Pin)                   		//SN2输出低

#define SN2_IN()        (((SN2->PID) & (1<<Single_PIN))?1:0)                  //SN2输入*/

#define SENSOR_NUM_1   0x00
#define SENSOR_NUM_2   0x01
#define SENSOR_NUM_3   0x02
#define SENSOR_NUM_4   0x03

#define SENSOR_TH      0x01
#define SENSOR_TEMP    0x02
#define SENSOR_IR      0x03
#define SENSOR_DP      0x04
#define SENSOR_PT1000  0x06

#define SENSOR_HCHO    0x05
#define SENSOR_NH3     0x08

#define SENSOR_CO2     0x0a
#define SENSOR_THERM   0x0b
#define SENSOR_THREE   0x0d
#define SENSOR_CO2_PERSEN  0x0e
#define SENSOR_PM2_5   0x10
#define SENSOR_TVOC    0x11
#define SENSOR_NOISE   0x12
#define SENSOR_NH3_1   0x09

#define data_ok          0x0000
#define data1_alarm_up          (1<<0)     //bit0:数据1超上限
#define data1_alarm_down        (1<<1)     //bit1:数据1超下限
#define data1_alarm_bd          (1<<2)     //bit2:数据1波动报警
#define data2_alarm_up          (1<<4)     //bit4:数据2超上限
#define data2_alarm_down        (1<<5)     //bit5:数据2超下限
#define data2_alarm_bd          (1<<6)     //bit6:数据2波动报警
#define data3_alarm_up          (1<<8)     //bit8:数据3超上限
#define data3_alarm_down        (1<<9)     //bit9:数据3超下限
#define data3_alarm_bd          (1<<10)     //bit10:数据3波动报警
#define data4_alarm_up          (1<<12)     //bit12:数据4超上限
#define data4_alarm_down        (1<<13)     //bit13:数据4超下限
#define data4_alarm_bd          (1<<14)     //bit14:数据4波动报警


#define sensor_err      0x00
#define sensor_ok       0x01


enum DISPLAY_TEYP
{
	TYPE_TEMP =  0x01,
	TYPE_PERSEN,
	TYPE_HUMI,
	TYPE_AIR_PRESSURE,
	TYPE_HCHO,
	TYPE_NH3,
	TYPE_CO2,
	TYPE_TVOC,
	TYPE_NOISE,
	TYPE_NH3_1,
	TYPE_PM,
	TYPE_IO,
};

typedef struct 
{
	uint8_t ad[7];
	float temperature;
	float humidity;
}gxht30c;


//温湿度传感器
typedef struct 
{
	float sensor_data[4];
	uint8_t sensor_type[4];
    uint8_t sensor_state[4];
    float bat_voltage;
	uint16_t alarm_state;
}th_sensor;


void single_pin_deinit(void);
void sensor_task_create(void *params, uint8_t priority);
void task_post_event_sensor_isr(uint32_t event);
void task_post_event_sensor(uint32_t event);
uint16_t sensor_get_alarm(void);
float sensor_get_sensor_data(uint8_t chose);
void single_pin_deinit(void);
void is_bluetooth_connect(uint8_t data);

float sensor_get_value(uint8_t id);

uint8_t sensor_is_link(uint8_t id);

uint8_t sensor_get_type(uint8_t id);

void single_pin_init(void);

float config_get_vbat_value(void);

#endif

