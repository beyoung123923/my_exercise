#include "string.h"
#include "FreeRTOS.h"
#include "task.h"
#include "sys_setting.h"
#include "display.h"
#include "semphr.h"
#include "math.h"
#include "stdio.h"
#include "myiic.h"
#include "ad.h"
#include "usart.h"
#include "sensor.h"
#include "touch_key.h"
#include "pwr_manage.h"
#include "main.h"
#include "modbus_tcp.h"
#include "delay.h"

static xTaskHandle sensor_handler;
th_sensor th_sensor_t;
th_sensor th_sensor_f;
static uint8_t oled_sensor_num[2] = {0 ,0};

const unsigned int GXHT30C_max_counts = 65535;				//GXHT30C最大转换次数
static uint8_t GXHT30C_COM[2] = {0x2C,0x06};
static uint8_t GXHT40C_COM = 0xFD;

void single_en_init(void)
{
 	FL_GPIO_InitTypeDef    GPIO_InitStruct;

    /*PC6:GPIOC */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);     
}

void single_pin_init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

	GPIO_InitStruct.pin = FL_GPIO_PIN_0 | FL_GPIO_PIN_1;    				//pcie 
	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	GPIO_InitStruct.analogSwitch = FL_DISABLE;
	FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void single_pin_deinit(void)
{
	if(config_get_external_long_power() == 0)
	{
		FL_GPIO_InitTypeDef GPIO_InitStruct = {0};

		GPIO_InitStruct.pin = FL_GPIO_PIN_6;    //pcie 
		GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin   = FL_DISABLE;
		GPIO_InitStruct.analogSwitch = FL_DISABLE;
		FL_GPIO_Init(GPIOC, &GPIO_InitStruct);	
		
		GPIO_InitStruct.pin = FL_GPIO_PIN_0 | FL_GPIO_PIN_1;    				//pcie 
		GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
		GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;	
		GPIO_InitStruct.pull = FL_DISABLE;
		GPIO_InitStruct.remapPin   = FL_DISABLE;
		FL_GPIO_Init(GPIOD, &GPIO_InitStruct);	
		
		FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_6);
		FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_0);
		FL_GPIO_ResetOutputPin(GPIOD, FL_GPIO_PIN_1);	
	}
}

void SN_OUT_MODE(uint16_t Pin)
{
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = Pin;    //pcie 
	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void SN_IN_MODE(uint16_t Pin)
{
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = Pin;    //pcie 
	GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

void SN_OUT_H(uint16_t Pin)
{
    FL_GPIO_SetOutputPin(GPIOD,Pin);
}

void SN_OUT_L(uint16_t Pin)
{
    FL_GPIO_ResetOutputPin(GPIOD,Pin);
}

uint32_t SN_READ(uint16_t Pin)
{
    return FL_GPIO_GetInputPin(GPIOD,Pin);
}

void single_en_open(void)
{
    FL_GPIO_SetOutputPin(GPIOC,FL_GPIO_PIN_6);
}

void single_en_close(void)
{
	if(config_get_external_long_power() == 0)
	{
		FL_GPIO_ResetOutputPin(GPIOC, FL_GPIO_PIN_6); 			//模块上电   初始化低电平输出   默认关闭
	}	
}

static uint8_t write_cmd(uint8_t model)
{
	I2CStart();
	I2CSendByte(INTERNAL_TH_ADDR);      
	if(I2CWaitAck()== FALSE)
	{
		I2CStop(); 
		return FALSE;
	}
	if(model == 2)
	{
		I2CSendByte(GXHT40C_COM);
		I2CWaitAck();
	}
	else
	{
		I2CSendByte(GXHT30C_COM[0]);
		I2CWaitAck();
		I2CSendByte(GXHT30C_COM[1]);
		I2CWaitAck();
	}
	I2CStop();
	return	TRUE;	
}

static uint8_t read_data(uint8_t *ad_data,uint8_t len)
{
	uint8_t i;
	I2CStart();
	I2CSendByte(INTERNAL_TH_ADDR+1);      
	if(I2CWaitAck()== FALSE)
	{
		I2CStop(); 
		return FALSE;
	}
	for(i = 0;i<len;i++)
	{
		ad_data[i]=I2CReceiveByte();
		if(i == (len-1))
		{
			I2CNoAck();
		}
		else
		{
			I2CAck();
		}
	}
	I2CStop(); 
	return	TRUE;
}

uint8_t get_GXHT30C(uint8_t *ad_data,uint8_t model)
{
	unsigned char count=0;
	if(write_cmd(model))
	{
		return 0;
	}
	while(1)
	{
		vTaskDelay(70);
		if(count >=5)
		{
			return 0;
		}
		if(!read_data(ad_data,6))
		{
			break;
		}
		count++;		
	}
	return 1;		
}


/*****************************************************************
功能：获取GXHT30C传感器数据
返回：0	获取数据失败	1 获取数据成功
*pHandle:I2C结构体类型 SI2C_HANDLE
******************************************************************/
uint8_t read_GXHT30C(th_sensor *th_sensor_t,uint8_t model)
{
	uint8_t ad_buf[10];
	uint8_t state;
	uint32_t GXHT30C_tem = 0;
	uint32_t GXHT30C_humidity = 0;
    float data0_k = config_get_amend_k(0);
    float data0_b = config_get_amend_b(0);
    float data1_k = config_get_amend_k(1);
    float data1_b = config_get_amend_b(1);
 
	memset(ad_buf,0,10);
	state = get_GXHT30C(ad_buf,model);
	if(!state)
	{
		return 0;
	}
    else
	{
		//获取温湿度有效值
		GXHT30C_tem = (((uint32_t)(ad_buf[0]&0x000000FF))<<8)|(uint32_t)ad_buf[1];
		GXHT30C_humidity = (((uint32_t)ad_buf[3]&0x000000FF)<<8)|((uint32_t)ad_buf[4]);
		
		//温湿度计算
		th_sensor_t->sensor_data[SENSOR_NUM_1] = ((float)GXHT30C_tem/GXHT30C_max_counts)*175-45;
		th_sensor_t->sensor_data[SENSOR_NUM_2]= (float)GXHT30C_humidity/GXHT30C_max_counts*100;
		th_sensor_t->sensor_type[SENSOR_NUM_1] = TYPE_TEMP;   
        th_sensor_t->sensor_type[SENSOR_NUM_2] = TYPE_HUMI;
  		th_sensor_t->sensor_state[SENSOR_NUM_1] = sensor_ok;
        th_sensor_t->sensor_state[SENSOR_NUM_2] = sensor_ok;

        //数据修正
        if(config_get_tem_unit())
        {
            th_sensor_t->sensor_data[SENSOR_NUM_1] = th_sensor_t->sensor_data[SENSOR_NUM_1] * 9.0f / 5.0f + 32.0f;
        }
        th_sensor_t->sensor_data[SENSOR_NUM_1] = th_sensor_t->sensor_data[SENSOR_NUM_1]*data0_k + data0_b;
        th_sensor_t->sensor_data[SENSOR_NUM_2] = th_sensor_t->sensor_data[SENSOR_NUM_2]*data1_k + data1_b;
        
		if(th_sensor_t->sensor_data[SENSOR_NUM_2] > 100)
		{
			th_sensor_t->sensor_data[SENSOR_NUM_2] = 100;
		}
		if(th_sensor_t->sensor_data[SENSOR_NUM_2] < 0)
		{
			th_sensor_t->sensor_data[SENSOR_NUM_2] = 0;
		}
		return 1;
	}
}

static uint8_t calc_cs(const void *cacl_buff, int16_t size)
{
    uint8_t sum;
    const uint8_t *buff;

    sum = 0;
    buff = (const uint8_t *)cacl_buff;

    while(size-- > 0)
    {
        sum += *buff++;
    }
    return sum;
}

static uint8_t calc_crc8_ds18b20(uint8_t *buf, uint16_t len)
{
    uint8_t crc = 0;

	while(len--)
	{
		crc ^= *buf++;
		for(uint8_t i = 0; i < 8; i++)
		{
			 if(crc & 0x01)
			 {
				crc = (crc>>1)^0x8C;
			 }else 
			 {
				crc >>= 1;
			 }
		}
	}
	return crc;
}

float BEBufToFloat(uint8_t *_pBuf)
{
	float Res = 0;
	uint8_t *Rec;
	
	Rec = (uint8_t*)&Res;
	*Rec = _pBuf[3];Rec++;
	*Rec = _pBuf[2];Rec++;
	*Rec = _pBuf[1];Rec++;
	*Rec = _pBuf[0];
	
	return (Res);
}

const double var_ttor_pt100[3] = {3.9083/1e3, -5.775/1e7, -4.183/1e12};	

#define R0     100.0
#define R1     1000.0
#define COEFF_A                 3.9083e-3
#define COEFF_B                 -5.775e-7
#define COEFF_A_A               (COEFF_A * COEFF_A)
#define COEFF_4B_R0             (4 * COEFF_B / R0)
#define COEFF_4B_R1             (4 * COEFF_B / R1)
#define COEFF_2B                (2 * COEFF_B)

const double cPPolyCoeff[7] ={
    -242.02, 2.2228, 0.0025859, -0.0000048260, -0.000000028183, 0.00000000015243, 1.0
};

const double cPPolyCoeff1[7] ={
    -241.96, 0.221615, 0.00002855, -0.00000000991681, 0.00000000000171, 1.0
};

#define POLY_CALC(ret_value, in_val, coeff_array) \
{ \
    double exp_value = 1.0; \
    const double* coeff = coeff_array; \
    ret_value = 0.0f; \
    while(*coeff != 1.0)\
    { \
        ret_value += *coeff * exp_value; \
        exp_value *= in_val; \
        coeff++; \
    }\
}

static void ds18b20_out_init(uint16_t Pin)
{
	FL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	GPIO_InitStruct.pin = Pin;    					 
	GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
	GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
	GPIO_InitStruct.pull = FL_DISABLE;
	GPIO_InitStruct.remapPin   = FL_DISABLE;
	FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

static void ds18b20_in_init(uint16_t Pin)
{
	FL_GPIO_EnablePinInput(GPIOD,Pin);
	FL_GPIO_SetPinMode(GPIOD, Pin, FL_GPIO_MODE_INPUT);
}

static uint8_t sensor_reset_ds18b20(uint16_t channle)
{
    uint32_t time_out;

	ds18b20_out_init(channle);
    SN_OUT_H(channle);
    SN_OUT_L(channle);
	sensor_delay_us(480);
    SN_OUT_H(channle);
    ds18b20_in_init(channle);
	sensor_delay_us(10);
    for(time_out = 0;time_out < 100; time_out++)
    {
        if(SN_READ(channle) == 0)
        {
           break; 
        }
        sensor_delay_us(1);
    }
    if(time_out >= 100)
    {
        return 1;
    }
    for(time_out = 0;time_out < 300; time_out++)
    {
        if(SN_READ(channle))
        {
            break;
        }
        sensor_delay_us(1);
    }
    if(time_out >= 300)
    {
        return 1;
    }

	ds18b20_out_init(channle);
	SN_OUT_H(channle);
	sensor_delay_us(50);
    return 0;
}

static void sensor_write_ds18b20(uint16_t channle, uint8_t byte)
{
	ds18b20_out_init(channle);
	SN_OUT_H(channle);
    for(uint8_t bit = 0; bit < 8; bit++)
    {		
        SN_OUT_L(channle);
        sensor_delay_us(1);
        if(byte & 0x01)
        {
            SN_OUT_H(channle);
        }
		else
		{
			SN_OUT_L(channle);
		}
        sensor_delay_us(50);
        SN_OUT_H(channle);
        sensor_delay_us(1);
        byte >>= 1;
    }   
}

static uint8_t sensor_read_ds18b20(uint16_t channle)
{
    uint8_t read_byte = 0;
    for(uint8_t bit = 0; bit < 8; bit++)
    {
		ds18b20_out_init(channle);
        read_byte >>= 1;
        SN_OUT_L(channle);
        ds18b20_in_init(channle);
		sensor_delay_us(1);
        if(SN_READ(channle))
        {
            read_byte |= 0x80;
        }
        sensor_delay_us(10);
    }
    return read_byte;
}

uint8_t sensor_get_ds18b20_value(uint16_t channle)
{
    uint8_t sensor_buf[9];
    char minus = 0;
    uint16_t raw_value;
    float calc_value;
    float data_k = 0;
    float data_b = 0;    
    uint8_t internal_en = config_get_internal_en();

    if(sensor_reset_ds18b20(channle))
    {
        if(sensor_reset_ds18b20(channle))
        {
            goto err_exit;
        }
    }
    sensor_write_ds18b20(channle, 0xcc);
    sensor_write_ds18b20(channle, 0x44);
	
    vTaskDelay(1);
	//sensor_delay_us(800);

    if(sensor_reset_ds18b20(channle))
    {
        if(sensor_reset_ds18b20(channle))
        {
            goto err_exit;
        }
    }
    sensor_write_ds18b20(channle, 0xcc);
    sensor_write_ds18b20(channle, 0xbe);

 //   sensor_delay_us(5);
    sensor_delay_us(1);
    portDISABLE_INTERRUPTS(); 
    for(uint8_t read_num = 0;read_num < 9; read_num++)
    {
        sensor_buf[read_num] = sensor_read_ds18b20(channle);
    }
    portENABLE_INTERRUPTS(); 
    if(calc_crc8_ds18b20(sensor_buf, 8) != sensor_buf[8])
    {
        goto err_exit;
    }
    if(sensor_buf[1] > 0x07)
    {
        sensor_buf[1] = ~sensor_buf[1];
        sensor_buf[0] = ~sensor_buf[0];
        minus = 1;
    }

    raw_value = (uint16_t)sensor_buf[1] << 8 | sensor_buf[0];

    calc_value = raw_value * 0.0625;
    if(minus)
    {
        calc_value = -(calc_value);
    }

    if(calc_value == 85.0f)
        return 1;
    if(config_get_tem_unit())
    {
        calc_value = calc_value * 9.0f / 5.0f + 32.0f;
    }
    if(channle == FL_GPIO_PIN_0)
    {
		data_k = config_get_amend_k(SENSOR_NUM_1+internal_en*2);
		data_b = config_get_amend_b(SENSOR_NUM_1+internal_en*2);
		calc_value = data_k*calc_value+data_b;
        th_sensor_t.sensor_state[SENSOR_NUM_1 + internal_en * 2]  = sensor_ok;
        th_sensor_t.sensor_data[SENSOR_NUM_1 + internal_en * 2] = calc_value;
    }
    else 
    {
		data_k = config_get_amend_k(SENSOR_NUM_2+internal_en*2);
		data_b = config_get_amend_b(SENSOR_NUM_2+internal_en*2);
		calc_value = data_k*calc_value+data_b;		
        th_sensor_t.sensor_state[SENSOR_NUM_2 + internal_en * 2]  = sensor_ok;
        th_sensor_t.sensor_data[SENSOR_NUM_2 + internal_en * 2] = calc_value;
    }
    return 0;

    err_exit:
    return 1;
}

static void sensor_record_th(uint16_t channle,uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    int16_t raw_value;
    float calc_value;
    float data_k = 0;
    float data_b = 0;

	if((channle == FL_GPIO_PIN_0) && (config_get_internal_en() == 0))
	{
		sn_id = 0;
	}
	else 
	{
		if(config_get_internal_en() == 1)
		{
			sn_id = 2;
		}
		else 
		{
			sn_id = oled_sensor_num[0];
		}
	}

    raw_value = ((int16_t)sensor_buf[2] << 8) | sensor_buf[3];
    calc_value = raw_value / 10.0f;
    data_k = config_get_amend_k(sn_id+1);
    data_b = config_get_amend_b(sn_id+1);
    calc_value = calc_value * data_k+data_b;
    if(calc_value >= 100)
    {
        calc_value = 100;
    }
    else if(calc_value < 0)
    {
        calc_value = 0;
    }
	th_sensor_t.sensor_data[sn_id+1] = calc_value;
    th_sensor_t.sensor_state[sn_id+1] = sensor_ok;
	th_sensor_t.sensor_type[sn_id+1] = TYPE_HUMI;

    raw_value = ((int16_t)sensor_buf[4] << 8) | sensor_buf[5] ;
    calc_value = raw_value / 10.0f;
    if(config_get_tem_unit())
    {
        calc_value = calc_value * 9.0f / 5.0f + 32.0f;
    }
    data_k = config_get_amend_k(sn_id);
    data_b = config_get_amend_b(sn_id);
    calc_value = calc_value * data_k+data_b;
	th_sensor_t.sensor_data[sn_id] = calc_value;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;
	th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
}

float pt100_ohm_2_temp(double ohm_value)
{
	double pt100_data = 0.0f;
	if(ohm_value > R0)
	{
		pt100_data = (-COEFF_A + sqrt(COEFF_A_A - COEFF_4B_R0 * (R0 - ohm_value))) / COEFF_2B;
	}
	else
	{
		POLY_CALC(pt100_data, ohm_value, &cPPolyCoeff[0]);
	}
	return (float)pt100_data;
}

float pt1000_ohm_2_temp(double ohm_value)
{
	double pt100_data;
	if(ohm_value > R1)
	{
		pt100_data = (-COEFF_A + sqrt(COEFF_A_A - COEFF_4B_R1 * (R1 - ohm_value))) / COEFF_2B;
	}
	else
	{
		POLY_CALC(pt100_data, ohm_value, &cPPolyCoeff1[0]);
	}
	return (float)pt100_data;
}

static void sensor_record_temp(uint16_t channle, uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    uint16_t raw_value;
    float calc_value;
	float data_k = 0;
    float data_b = 0;

	if((channle == FL_GPIO_PIN_1) && (config_get_internal_en() == 0)) 
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if((channle == FL_GPIO_PIN_0) && (config_get_internal_en()))
	{
		sn_id = 2;
	}
	else if((channle == FL_GPIO_PIN_1) && (config_get_internal_en()))
	{
		sn_id = 2 + oled_sensor_num[0];
	}

    th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;

    raw_value = sensor_buf[2] << 8 | sensor_buf[3];
    calc_value = pt100_ohm_2_temp(raw_value / 100.0f);

	
    if(config_get_tem_unit())
    {
        calc_value = calc_value * 9.0f / 5.0f + 32.0f;
    }
	data_k = config_get_amend_k(sn_id);
    data_b = config_get_amend_b(sn_id);
	calc_value = calc_value * data_k+data_b;
    th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_ir(uint16_t channle, uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    float raw_value;
	float data_k;
	float data_b;
		
    if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
    {
		sn_id = 2;
    }
    else if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
    {
        sn_id = oled_sensor_num[0];  //偏锟斤拷
    }
    else if(channle != FL_GPIO_PIN_0 && config_get_internal_en())
    {
        return ;
    }
    th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
    th_sensor_t.sensor_type[sn_id + 1] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id + 1] = sensor_ok;

    raw_value = ((uint16_t)sensor_buf[2] << 8 | sensor_buf[3]) * 0.02 - 273.15;
    if(config_get_tem_unit())
    {
        raw_value = raw_value * 9.0f / 5.0f + 32.0f;
    }
    if(raw_value != 0 && raw_value <= 280 && raw_value >= -20)
	{
		data_k = config_get_amend_k(sn_id);
		data_b = config_get_amend_b(sn_id);	
		raw_value = raw_value * data_k+data_b;
		th_sensor_t.sensor_data[sn_id] = raw_value;
	}
   
    raw_value = ((uint16_t)sensor_buf[4] << 8 | sensor_buf[5]) * 0.02 -273.15;
    if(config_get_tem_unit())
    {
        raw_value = raw_value * 9.0f / 5.0f + 32.0f;
    }
    if(raw_value != 0 && raw_value <= 280 && raw_value >= -20)
	{
		data_k = config_get_amend_k(sn_id + 1);
		data_b = config_get_amend_b(sn_id + 1);	
		raw_value = raw_value * data_k+data_b;
		th_sensor_t.sensor_data[sn_id + 1] = raw_value;
	}	
}

static void sensor_record_dp(uint16_t channle, uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    float raw_value;
    int16_t tmp_value ;
	float data_k;
	float data_b;

    th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
    th_sensor_t.sensor_type[sn_id + 1] = TYPE_HUMI;
    th_sensor_t.sensor_state[sn_id + 1] = sensor_ok;

    th_sensor_t.sensor_type[sn_id + 2] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id + 2] = sensor_ok;

    tmp_value = (int16_t)sensor_buf[2] << 8 | (int16_t)sensor_buf[3];
    raw_value = (float)tmp_value / 10.0f;
	
	data_k = config_get_amend_k(sn_id + 1);
	data_b = config_get_amend_b(sn_id + 1);	
	raw_value = raw_value * data_k+data_b;
	
    //raw_value += system_get_correct(sn_id + 1);

    if(raw_value >= 100)
    {
        raw_value = 100;
    }
    th_sensor_t.sensor_data[sn_id + 1] = raw_value;
    
    tmp_value = (int16_t)sensor_buf[4] << 8 | (int16_t)sensor_buf[5];

    raw_value = (float)tmp_value / 10.0f;
    if(config_get_tem_unit())
    {
        raw_value = raw_value * 9.0f / 5.0f + 32.0f;
    }
   // raw_value += system_get_correct(sn_id);
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	raw_value = raw_value * data_k+data_b;	

	th_sensor_t.sensor_data[sn_id] = raw_value;

    tmp_value = (int16_t)sensor_buf[6] << 8 | (int16_t)sensor_buf[7];
    raw_value = (float)tmp_value / 10.0f;
	
   // raw_value += system_get_correct(sn_id + 2);
	data_k = config_get_amend_k(sn_id + 2);
	data_b = config_get_amend_b(sn_id + 2);	
	raw_value = raw_value * data_k+data_b;		
	
    th_sensor_t.sensor_data[sn_id + 2] = raw_value;
}

static void sensor_record_pt1000(uint16_t channle, uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    float calc_value;
	float data_k;
	float data_b;

	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}
    th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;

    calc_value = BEBufToFloat(&sensor_buf[2]);

    if(config_get_tem_unit())
    {
        calc_value = calc_value * 9.0f / 5.0f + 32.0f;
    }
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;		
	
    //calc_value += system_get_correct(sn_id);
    th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_nh3(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_NH3;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;

	calc_value = ((int16_t)sensor_buf[2] << 8 | sensor_buf[3]);
	
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	
	
	//calc_value += system_get_correct(sn_id);
	th_sensor_t.sensor_data[sn_id] = calc_value;
}


static void sensro_record_co2(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;	
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}
	th_sensor_t.sensor_type[sn_id] = TYPE_CO2;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;

	calc_value = ((int16_t)sensor_buf[2] << 8 | sensor_buf[3]);
	
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	

	//calc_value += system_get_correct(sn_id);
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_therm(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}
	
	th_sensor_t.sensor_type[sn_id] = TYPE_TEMP;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;

	calc_value = BEBufToFloat(&sensor_buf[2]);
	
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;

	//calc_value += system_get_correct(sn_id);
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_three(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	float data_k;
	float data_b;
	
	th_sensor_t.sensor_type[SENSOR_NUM_1] = TYPE_TEMP;
	th_sensor_t.sensor_type[SENSOR_NUM_2] = TYPE_HUMI;
	th_sensor_t.sensor_type[SENSOR_NUM_3] = TYPE_AIR_PRESSURE;
	
	th_sensor_t.sensor_state[SENSOR_NUM_1] = sensor_ok;
	th_sensor_t.sensor_state[SENSOR_NUM_2] = sensor_ok;
	th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_ok;

	calc_value = (uint16_t)sensor_buf[2] << 8 | sensor_buf[3];
	calc_value /= 10.0f;

	//calc_value += system_get_correct(SENSOR_NUM_1);
	data_k = config_get_amend_k(SENSOR_NUM_1);
	data_b = config_get_amend_b(SENSOR_NUM_1);	
	calc_value = calc_value * data_k+data_b;
	
	th_sensor_t.sensor_data[SENSOR_NUM_1] = calc_value;
	
	calc_value = (uint16_t)sensor_buf[4] << 8 | sensor_buf[5];
	calc_value /= 10.0f;

	//calc_value += system_get_correct(SENSOR_NUM_2);
	data_k = config_get_amend_k(SENSOR_NUM_2);
	data_b = config_get_amend_b(SENSOR_NUM_2);	
	calc_value = calc_value * data_k+data_b;
    if(calc_value >= 100)
    {
        calc_value = 100;
    }
	th_sensor_t.sensor_data[SENSOR_NUM_2] = calc_value;
	
	calc_value = (uint16_t)sensor_buf[6] << 8 | sensor_buf[7];
	calc_value /= 100.0f;
	
	//calc_value += system_get_correct(SENSOR_NUM_3);
	data_k = config_get_amend_k(SENSOR_NUM_3);
	data_b = config_get_amend_b(SENSOR_NUM_3);	
	calc_value = calc_value * data_k+data_b;	
	
	th_sensor_t.sensor_data[SENSOR_NUM_3] = calc_value;
}

static void sensor_record_hcho(uint16_t channle, uint8_t *sensor_buf)
{
    uint8_t sn_id = 0;
    float calc_value;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}
    th_sensor_t.sensor_type[sn_id] = TYPE_HCHO;
    th_sensor_t.sensor_state[sn_id] = sensor_ok;

    calc_value = ((int16_t)sensor_buf[2] << 8 | sensor_buf[3]);
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	
	
    //calc_value += system_get_correct(sn_id);

    th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_co2_persen(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_PERSEN;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;

	calc_value = BEBufToFloat(&sensor_buf[2]);
	
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	
	//calc_value +=system_get_correct(sn_id);;
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_tvoc(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_TVOC;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
	calc_value = BEBufToFloat(&sensor_buf[2]);
	//calc_value +=system_get_correct(sn_id);;
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	
	
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_noise(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_NOISE;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
	calc_value = BEBufToFloat(&sensor_buf[2]);
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;	
	//calc_value +=system_get_correct(sn_id);;
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_nh3_1(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_NH3_1;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
	calc_value = ((int16_t)sensor_buf[2] << 8 | sensor_buf[3]);
	calc_value *= 0.1f;
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;		
	
	//calc_value += system_get_correct(sn_id);
	th_sensor_t.sensor_data[sn_id] = calc_value;
	//calc_value +=system_get_correct(sn_id);;
	//th_sensor_t.sensor_data[sn_id] = calc_value;
}

static void sensor_record_pm(uint16_t channle, uint8_t *sensor_buf)
{
	float calc_value;
	uint8_t sn_id = 0;
	float data_k;
	float data_b;
	
	if(channle == FL_GPIO_PIN_1 && config_get_internal_en() == 0)
	{
		sn_id = oled_sensor_num[0];  //偏锟斤拷
	}
	else if(channle == FL_GPIO_PIN_0 && config_get_internal_en())
	{
		sn_id = 2;
	}
	else if(channle == FL_GPIO_PIN_1 && config_get_internal_en())
	{
		sn_id = 2 + oled_sensor_num[0];
	}

	th_sensor_t.sensor_type[sn_id] = TYPE_PM;
	th_sensor_t.sensor_state[sn_id] = sensor_ok;
    
	calc_value = BEBufToFloat(&sensor_buf[2]);
	data_k = config_get_amend_k(sn_id);
	data_b = config_get_amend_b(sn_id);	
	calc_value = calc_value * data_k+data_b;		
	
	//calc_value +=system_get_correct(sn_id);;
	th_sensor_t.sensor_data[sn_id] = calc_value;
}

uint8_t sensor_get_customize(uint16_t channle)
{
	uint8_t result = 0;
    uint8_t sensor_buf[12];
    uint32_t low_time = 0;
    uint32_t high_time = 0;
    uint32_t wait_time = 0;
	uint8_t read_cnt = 0;
	uint8_t read_bit = 0;

	SN_OUT_MODE(channle);
    SN_OUT_L(channle);
    vTaskDelay(3);
    SN_OUT_H(channle);
	sensor_delay_us(30);
    SN_IN_MODE(channle);
	
	while(SN_READ(channle))
	{
		if(++wait_time>300)
		{
			return 0;
		}
	}
	wait_time=0;
	
	while(SN_READ(channle) ==0)
	{
		if(++wait_time>250)
		{
			return 0;
		}
	}
	wait_time=0;
	
	while(SN_READ(channle))
	{	
		if(++wait_time>250)
		{
			return 0;
		}
	}	
	portDISABLE_INTERRUPTS(); 
    for(read_cnt = 0; read_cnt < 12; read_cnt++)
    {
        sensor_buf[read_cnt] = 0x00;
        for(read_bit = 0; read_bit < 8; read_bit++)
        {
            sensor_buf[read_cnt] <<= 1;
            low_time = 0;
            high_time = 0;
             
            while(SN_READ(channle) == 0) /*wait low level*/
            {
                if(++low_time > 250) 
                {
					portENABLE_INTERRUPTS(); 
                    return 0;
                }
            }  
            while(SN_READ(channle))   /*wait high level*/
            {
                if(++high_time > 250) 
                {
					portENABLE_INTERRUPTS(); 
                    return 0;
                }
            }
            if(high_time > low_time)
            {
                sensor_buf[read_cnt] |= 1;
            }
        }
    } 
	portENABLE_INTERRUPTS(); 	
    //SN_OUT_MODE(channle);
    if(sensor_buf[0] == 0x55 && sensor_buf[11] == 0x0d &&
       calc_cs(sensor_buf, 10) == sensor_buf[10])
    {
        switch(sensor_buf[1])
        {
				case SENSOR_TH:  /* ?????????? */
					sensor_record_th(channle,sensor_buf);
					result = 2;
				break;

				case SENSOR_TEMP:   /* ???????? */
				{
					sensor_record_temp(channle, sensor_buf);
					result = 1;
				}break;
					
				case SENSOR_IR:    /* ?????????????? */
				{
					sensor_record_ir(channle, sensor_buf);
					result = 2;
				}break;
				
				case SENSOR_DP:
				{
					sensor_record_dp(channle, sensor_buf);
					result = 3;
				}break;
					
				case SENSOR_HCHO:    /* 		??????			*/
				{
					sensor_record_hcho(channle, sensor_buf);
					result = 1;
				}
				break;
				
				case SENSOR_PT1000:
				{
					sensor_record_pt1000(channle, sensor_buf);
					result = 1;
				}break;
				
				case SENSOR_NH3:
				{
					sensor_record_nh3(channle, sensor_buf);
					result = 1;
				}break;
				
				case SENSOR_CO2 :
				{
					sensro_record_co2(channle, sensor_buf);
					result = 1;
				}break;
				
				case SENSOR_THERM:
				{
					sensor_record_therm(channle, sensor_buf);
					result = 1;
				}break;
				
				case SENSOR_THREE:
				{
					sensor_record_three(channle, sensor_buf);
					result = 3;
				}break;
				
				case SENSOR_CO2_PERSEN:
				{
					sensor_record_co2_persen(channle, sensor_buf);
					result = 1;
				}break;

                case SENSOR_PM2_5:
                {
                    sensor_record_pm(channle, sensor_buf);
                    result = 1;
                };break;
				
				case SENSOR_TVOC:
				{
					sensor_record_tvoc(channle, sensor_buf);
					result = 1;
				}break;

                case SENSOR_NOISE:
                {
					sensor_record_noise(channle, sensor_buf);
					result = 1;
                }break;
                case SENSOR_NH3_1:
                {
					sensor_record_nh3_1(channle, sensor_buf);
					result = 1;
                }break;      

                default:
                    break;
        }
    }
    else 
    {
		portENABLE_INTERRUPTS(); 
        return 0;
    }	
	portENABLE_INTERRUPTS(); 
    return result;
}

//获取传感器报警状态
uint16_t sensor_get_alarm(void)
{
	return th_sensor_t.alarm_state;
}
//获取传感器温度
float sensor_get_sensor_data(uint8_t chose)
{
    return th_sensor_t.sensor_data[chose];
}

static uint8_t device_set_data_legitimacy(void)  //上下限设置参数合法性校验
{
    float up_data;
    uint8_t re=0;

    if(config_get_up_data(0) <= config_get_down_data(0))
    {
        up_data = config_get_down_data(0)+config_get_data_hc(0)+2;
        config_set_up_data(up_data,0);
        re = 1;
    }
    if(config_get_data_bd(0) == 0)    
    {
        up_data = config_get_up_data(0);
        config_set_data_bd(up_data,0);
        re = 1;
    }

    if(config_get_up_data(1) <= config_get_down_data(1))
    {
        up_data = config_get_down_data(1)+config_get_data_hc(1)+2;
        config_set_up_data(up_data,1);
        re = 1;
    }
    if(config_get_data_bd(1) == 0)    
    {
        up_data = config_get_up_data(1);
        config_set_data_bd(up_data,1);
        re = 1;
    }

    if(config_get_up_data(2) <= config_get_down_data(2))
    {
        up_data = config_get_down_data(2)+config_get_data_hc(2)+2;
        config_set_up_data(up_data,2);
        re = 1;
    }
    if(config_get_data_bd(2) == 0)    
    {
        up_data = config_get_up_data(2);
        config_set_data_bd(up_data,2);
        re = 1;
    }

    if(config_get_up_data(3) <= config_get_down_data(3))
    {
        up_data = config_get_down_data(3)+config_get_data_hc(3)+2;
        config_set_up_data(up_data,3);
        re = 1;
    }
    if(config_get_data_bd(3) == 0)    
    {
        up_data = config_get_up_data(3);
        config_set_data_bd(up_data,3);
        re = 1;
    }    
    return re;
}

static void sensor_check(void)
{
//	trace_log("报警判断\r\n");
    if(device_set_data_legitimacy())   //设置参数有变动，下次上报会进行一次数据同步
    {
        system_write_setting();
    }
    if(config_get_alarm_en())
    {
        //数据1	
        if(th_sensor_t.sensor_state[0] != sensor_err)                                   //传感器读取正常再判断限值报警标志位
        {
            if(th_sensor_t.sensor_data[0] > config_get_up_data(0))
            {
                trace_log("数据1超过上限\r\n");
                th_sensor_t.alarm_state  |= data1_alarm_up;
            }
            else if((th_sensor_t.sensor_data[0] + config_get_data_hc(0)) < config_get_up_data(0))
            {
                th_sensor_t.alarm_state &= ~data1_alarm_up;
            }

            if(th_sensor_t.sensor_data[0] < config_get_down_data(0))
            {
                trace_log("数据1超过下限\r\n");
                th_sensor_t.alarm_state  |= data1_alarm_down;
            }
            else if((th_sensor_t.sensor_data[0] + config_get_data_hc(0)) > config_get_down_data(0))
            {
                th_sensor_t.alarm_state &= ~data1_alarm_down;
            }
            
            if(fabsf(th_sensor_t.sensor_data[0] - th_sensor_f.sensor_data[0]) > config_get_data_bd(0))
            {
                trace_log("数据1波动\r\n");
                th_sensor_t.alarm_state  |= data1_alarm_bd;
            }
            else
            {
                th_sensor_t.alarm_state  &= ~data1_alarm_bd; 
            }
            th_sensor_f.sensor_data[0] = th_sensor_t.sensor_data[0];                //更新缓存的上一组数据
        }
        else
        {
            th_sensor_t.alarm_state &= ~data1_alarm_up;
            th_sensor_t.alarm_state &= ~data1_alarm_down;
            th_sensor_t.alarm_state  &= ~data1_alarm_bd; 
        }
        //数据2
        if(th_sensor_t.sensor_state[1] != sensor_err)                                   //传感器读取正常再判断限值报警标志位
        {
            if(th_sensor_t.sensor_data[1] > config_get_up_data(1))
            {
                trace_log("数据2超过上限\r\n");
                th_sensor_t.alarm_state  |= data2_alarm_up;
            }
            else if((th_sensor_t.sensor_data[1] + config_get_data_hc(1)) < config_get_up_data(1))
            {
                th_sensor_t.alarm_state &= ~data2_alarm_up;
            }

            if(th_sensor_t.sensor_data[1] < config_get_down_data(1))
            {
                trace_log("数据2超过下限\r\n");
                th_sensor_t.alarm_state  |= data2_alarm_down;
            }
            else if((th_sensor_t.sensor_data[1] + config_get_data_hc(1)) > config_get_down_data(1))
            {
                th_sensor_t.alarm_state &= ~data2_alarm_down;
            }
            
            if(fabsf(th_sensor_t.sensor_data[1] - th_sensor_f.sensor_data[1]) > config_get_data_bd(1))
            {
                trace_log("数据2波动\r\n");
                th_sensor_t.alarm_state  |= data2_alarm_bd;
            }
            else
            {
                th_sensor_t.alarm_state  &= ~data2_alarm_bd; 
            }
            th_sensor_f.sensor_data[1] = th_sensor_t.sensor_data[1];                //更新缓存的上一组数据      
        }
        else
        {
            th_sensor_t.alarm_state &= ~data2_alarm_up;
            th_sensor_t.alarm_state &= ~data2_alarm_down;
            th_sensor_t.alarm_state  &= ~data2_alarm_bd; 
        }
        
        //数据3
        if(th_sensor_t.sensor_state[2] != sensor_err)                                   //传感器读取正常再判断限值报警标志位
        {
            if(th_sensor_t.sensor_data[2] > config_get_up_data(2))
            {
                trace_log("数据3超过上限\r\n");
                th_sensor_t.alarm_state  |= data3_alarm_up;
            }
            else if((th_sensor_t.sensor_data[2] + config_get_data_hc(2)) < config_get_up_data(2))
            {
                th_sensor_t.alarm_state &= ~data3_alarm_up;
            }

            if(th_sensor_t.sensor_data[2] < config_get_down_data(2))
            {
                trace_log("数据3超过下限\r\n");
                th_sensor_t.alarm_state  |= data3_alarm_down;
            }
            else if((th_sensor_t.sensor_data[2] + config_get_data_hc(2)) > config_get_down_data(2))
            {
                th_sensor_t.alarm_state &= ~data3_alarm_down;
            }
            
            if(fabsf(th_sensor_t.sensor_data[2] - th_sensor_f.sensor_data[2]) > config_get_data_bd(2))
            {
                trace_log("数据3波动\r\n");
                th_sensor_t.alarm_state  |= data3_alarm_bd;
            }
            else
            {
                th_sensor_t.alarm_state  &= ~data3_alarm_bd; 
            }
            th_sensor_f.sensor_data[2] = th_sensor_t.sensor_data[2];                //更新缓存的上一组数据      
        }
        else
        {
            th_sensor_t.alarm_state &= ~data3_alarm_up;
            th_sensor_t.alarm_state &= ~data3_alarm_down;
            th_sensor_t.alarm_state  &= ~data3_alarm_bd; 
        }
        //数据4
        if(th_sensor_t.sensor_state[3] != sensor_err)                                   //传感器读取正常再判断限值报警标志位
        {
            if(th_sensor_t.sensor_data[3] > config_get_up_data(3))
            {
                trace_log("数据4超过上限\r\n");
                th_sensor_t.alarm_state  |= data4_alarm_up;
            }
            else if((th_sensor_t.sensor_data[3] + config_get_data_hc(3)) < config_get_up_data(3))
            {
                th_sensor_t.alarm_state &= ~data4_alarm_up;
            }

            if(th_sensor_t.sensor_data[3] < config_get_down_data(3))
            {
                trace_log("数据4超过下限\r\n");
                th_sensor_t.alarm_state  |= data4_alarm_down;
            }
            else if((th_sensor_t.sensor_data[3] + config_get_data_hc(3)) > config_get_down_data(3))
            {
                th_sensor_t.alarm_state &= ~data4_alarm_down;
            }
            
            if(fabsf(th_sensor_t.sensor_data[3] - th_sensor_f.sensor_data[3]) > config_get_data_bd(3))
            {
                trace_log("数据4波动\r\n");
                th_sensor_t.alarm_state  |= data4_alarm_bd;
            }
            else
            {
                th_sensor_t.alarm_state  &= ~data4_alarm_bd; 
            }
            th_sensor_f.sensor_data[3] = th_sensor_t.sensor_data[3];                //更新缓存的上一组数据      
        }
        else
        {
            th_sensor_t.alarm_state &= ~data4_alarm_up;
            th_sensor_t.alarm_state &= ~data4_alarm_down;
            th_sensor_t.alarm_state  &= ~data4_alarm_bd; 
        }
        
        if(th_sensor_t.alarm_state)
        {
            beep(1000);
        }
    }
}

float sensor_get_value(uint8_t id)
{
    return th_sensor_t.sensor_data[id];
}

uint8_t sensor_is_link(uint8_t id)
{
    return th_sensor_t.sensor_state[id];
}

uint8_t sensor_get_type(uint8_t id)
{
    return th_sensor_t.sensor_type[id];
}

void sensor_data_init(void)
{
	th_sensor_t.sensor_data[0] = 0;
	th_sensor_t.sensor_data[1] = 0;
	th_sensor_t.sensor_data[2] = 0;
	th_sensor_t.sensor_data[3] = 0;
	th_sensor_t.sensor_type[0] = 0;
	th_sensor_t.sensor_type[1] = 0;
	th_sensor_t.sensor_type[2] = 0;
	th_sensor_t.sensor_type[3] = 0;
	th_sensor_t.sensor_state[0] = 0;
	th_sensor_t.sensor_state[1] = 0;
	th_sensor_t.sensor_state[2] = 0;
	th_sensor_t.sensor_state[3] = 0;
}

static uint8_t internal_fail_cont = 0;
static uint8_t external_fail_cont[2] = {0};

//电池电压
float config_get_vbat_value(void)
{
	return th_sensor_t.bat_voltage;
}

void read_bat_vol(void)
{
    adc_init();
// 	trace_log("电压采集\r\n");  
	read_ad();
}


char put_buf[100];
//传感器数据处理
void sensor_event_proc(uint32_t event)           
{
    uint8_t count = 0;
    uint8_t internal_en = 0;
	uint8_t external_en = 0;
    uint8_t internal_model = 0;
	uint8_t external_model = 0;
    uint8_t sensor_num = 0;

	internal_en = config_get_internal_en();
	external_en = config_get_external_en();
    internal_model = config_get_internal_model();
	external_model = config_get_external_model();
	
    if((event & SENSOR_EVENT_GET_VALUE) == SENSOR_EVENT_GET_VALUE)
    {
        if(internal_en == 1)
        {
//          trace_log("内部传感器\r\n");  
            for(count = 0;count<3;count++)
            {                
                if(read_GXHT30C(&th_sensor_t,internal_model))
                {
                    break;
                }   
            }
			if(count >= 3)
			{
                if(internal_fail_cont >= 3)
                {
                    trace_log("内部传感器采集失败\r\n"); 
                    th_sensor_t.sensor_state[SENSOR_NUM_1] = sensor_err;
                    th_sensor_t.sensor_state[SENSOR_NUM_2] = sensor_err;
                    th_sensor_t.sensor_type[SENSOR_NUM_1] = 0;
                    th_sensor_t.sensor_type[SENSOR_NUM_2] = 0; 
                    th_sensor_t.sensor_data[SENSOR_NUM_1] = 0;
                    th_sensor_t.sensor_data[SENSOR_NUM_2] = 0; 
                    internal_fail_cont = 0;
                }
                else
                {
                    internal_fail_cont++;
                }
			}
            else
            {
                internal_fail_cont = 0;
            }
        }
		
        if(external_en == 1)                                            //外部单总线温湿度采集
        {  
//          trace_log("外部传感器\r\n"); 
            single_en_open();
            if(external_model == DS18B20)
            {
                vTaskDelay(2000);
                for(count = 0; count < 5; count++)
                {
                    if(sensor_get_ds18b20_value(FL_GPIO_PIN_0) == 0)
                    {
                        break;
                    }
                    vTaskDelay(500);
                }
                if(count >= 5)
                {
					trace_log("外部传感器1采集失败\r\n"); 
                    if(external_fail_cont[0] >= 3)
                    {
                        th_sensor_t.sensor_state[SENSOR_NUM_1 + internal_en * 2] = sensor_err;
                        th_sensor_t.sensor_type[SENSOR_NUM_1 + internal_en * 2] = 0;
                        th_sensor_t.sensor_data[SENSOR_NUM_1 + internal_en * 2] = 0;
                        external_fail_cont[0]  = 0;
                    }
                    else 
                    {
                        external_fail_cont[0]++;
                    }
                }
                else
                {
                    external_fail_cont[0]  = 0;
                    th_sensor_t.sensor_state[SENSOR_NUM_1 + internal_en * 2] = sensor_ok;
                    th_sensor_t.sensor_type[SENSOR_NUM_1 + internal_en * 2] = TYPE_TEMP;
                }
                
                for(count = 0; count < 5; count++)
                {
                    if(sensor_get_ds18b20_value(FL_GPIO_PIN_1) == 0)
                    {
                        break;
                    }
                    vTaskDelay(500);
                }
                if(count >= 5)
                {
                    trace_log("外部传感器2采集失败\r\n");  
                    if(external_fail_cont[1] >= 3)
                    {
                        th_sensor_t.sensor_state[SENSOR_NUM_2 + internal_en * 2] = sensor_err;
                        th_sensor_t.sensor_type[SENSOR_NUM_2 + internal_en * 2] = 0;
                        th_sensor_t.sensor_data[SENSOR_NUM_2 + internal_en * 2] = 0;
                        external_fail_cont[1] = 0;
                    }
                    else 
                    {
                        external_fail_cont[1]++;
                    }  
                }
                else
                {
                    external_fail_cont[1] = 0;
                    th_sensor_t.sensor_state[SENSOR_NUM_2 + internal_en * 2] = sensor_ok;
                    th_sensor_t.sensor_type[SENSOR_NUM_2 + internal_en * 2] = TYPE_TEMP;
                }
            }
            else if(external_model == CUSTOMIZE)
            {
                single_pin_init();
                vTaskDelay(2000);
                for(count = 0; count < 5; count++)
                {
                    sensor_num = sensor_get_customize(FL_GPIO_PIN_0);
                    if(sensor_num > 0)
                    {
//						trace_log("外部传感器采集成功\r\n"); 
                        oled_sensor_num[0] = sensor_num;
                        break;
                    }
                    vTaskDelay(500);
                }
                if(count >= 5)
                {
                    if(external_fail_cont[0] >= 3)
                    {
                        trace_log("外部传感器1采集失败\r\n"); 
                        if(oled_sensor_num[0] == 3)
                        {
                            th_sensor_t.sensor_state[SENSOR_NUM_1] = sensor_err;
                            th_sensor_t.sensor_data[SENSOR_NUM_1] = 0;
                            th_sensor_t.sensor_type[SENSOR_NUM_1] = 0;

                            th_sensor_t.sensor_state[SENSOR_NUM_2] = sensor_err;
                            th_sensor_t.sensor_data[SENSOR_NUM_2] = 0;
                            th_sensor_t.sensor_type[SENSOR_NUM_2] = 0;
                            
                            th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_err;
                            th_sensor_t.sensor_data[SENSOR_NUM_3] = 0;
                            th_sensor_t.sensor_type[SENSOR_NUM_3] = 0;
                        }
                        else
                        {
                            th_sensor_t.sensor_state[SENSOR_NUM_1 +  internal_en * 2] = sensor_err;
                            th_sensor_t.sensor_data[SENSOR_NUM_1 +  internal_en * 2] = 0;
                            th_sensor_t.sensor_state[SENSOR_NUM_2 +  internal_en * 2] = sensor_err;
                            th_sensor_t.sensor_data[SENSOR_NUM_2 +  internal_en * 2] = 0;
                            th_sensor_t.sensor_type[SENSOR_NUM_1 + internal_en * 2] = 0;
                            th_sensor_t.sensor_type[SENSOR_NUM_2 + internal_en * 2] = 0;  
                        }
                        external_fail_cont[0] = 0;
                    }
                    else
                    {
                        external_fail_cont[0]++;
                    }
                }
                else
                {
                    external_fail_cont[0] = 0;
                }
                if(((oled_sensor_num[0] == 2) && (internal_en == 0)) || (oled_sensor_num[0] <= 1))
                {
                    for(count = 0; count < 5; count++)
                    {
						sensor_num = sensor_get_customize(FL_GPIO_PIN_1);
                        if(sensor_num > 0 )
                        {
							//trace_log("外部传感器2采集成功\r\n"); 
                            oled_sensor_num[1] = sensor_num;
                            break;
                        }
                        vTaskDelay(500);
                    }
                    if(count >= 5)
                    {
                        if(external_fail_cont[1] >= 3)
                        {
                            trace_log("外部传感器2采集失败\r\n"); 
                            if(oled_sensor_num[1] >= 2)
                            {
                                if(oled_sensor_num[1] == 3)
                                {
                                    th_sensor_t.sensor_state[SENSOR_NUM_1] = sensor_err;
                                    th_sensor_t.sensor_data[SENSOR_NUM_1] = 0;
                                    th_sensor_t.sensor_state[SENSOR_NUM_2] = sensor_err;
                                    th_sensor_t.sensor_data[SENSOR_NUM_2] = 0;
                                    th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_err;
                                    th_sensor_t.sensor_data[SENSOR_NUM_3] = 0;
                                    th_sensor_t.sensor_type[SENSOR_NUM_1] = 0;
                                    th_sensor_t.sensor_type[SENSOR_NUM_2] = 0;
                                    th_sensor_t.sensor_type[SENSOR_NUM_3] = 0;
                                }							
                                else 
                                {
                                    if((oled_sensor_num[0]+internal_en * 2) < 3)
                                    {
                                        th_sensor_t.sensor_state[oled_sensor_num[0] + internal_en * 2] = sensor_err;
                                        th_sensor_t.sensor_state[oled_sensor_num[0] + 1 + internal_en * 2] = sensor_err;
                                        th_sensor_t.sensor_data[oled_sensor_num[0]+ internal_en * 2] = 0;
                                        th_sensor_t.sensor_data[oled_sensor_num[0] + 1 + internal_en * 2] = 0;
                                        th_sensor_t.sensor_type[oled_sensor_num[0]+ internal_en * 2] = 0;
                                        th_sensor_t.sensor_type[oled_sensor_num[0] + 1 + internal_en * 2] = 0;										
                                    }
                                    if((oled_sensor_num[0]+internal_en * 2) < 4)
                                    {
                                        th_sensor_t.sensor_state[oled_sensor_num[0] + internal_en * 2] = sensor_err;
                                        th_sensor_t.sensor_data[oled_sensor_num[0]+ internal_en * 2] = 0;
                                        th_sensor_t.sensor_type[oled_sensor_num[0]+ internal_en * 2] = 0;								
                                    }		
                                }
                            }
                            else 
                            {
                                th_sensor_t.sensor_state[oled_sensor_num[0] + internal_en * 2] = sensor_err;
                                th_sensor_t.sensor_data[oled_sensor_num[0] + internal_en * 2] = 0;
                                th_sensor_t.sensor_type[oled_sensor_num[0] + internal_en * 2] = 0;
                            }
                            external_fail_cont[1] = 0;
                        }
                        else
                        {
                            external_fail_cont[1]++;
                        }
                    }
                    else
                    {
                        external_fail_cont[1] = 0;
                    } 
                }
            }
            else
            {
                th_sensor_t.sensor_state[SENSOR_NUM_1 + internal_en * 2] = sensor_ok;
                th_sensor_t.sensor_data[SENSOR_NUM_1 + internal_en * 2] = FL_GPIO_GetInputPin(GPIOD,FL_GPIO_PIN_0);
                th_sensor_t.sensor_type[SENSOR_NUM_1 + internal_en * 2] = TYPE_IO;

                th_sensor_t.sensor_state[SENSOR_NUM_2 + internal_en * 2] = sensor_ok;
                th_sensor_t.sensor_data[SENSOR_NUM_2 + internal_en * 2] = FL_GPIO_GetInputPin(GPIOD,FL_GPIO_PIN_1);
                th_sensor_t.sensor_type[SENSOR_NUM_2 + internal_en * 2] = TYPE_IO;
            }   
        }
        sprintf(put_buf,"sensor = %f\t%f\t%f\t%f\n",th_sensor_t.sensor_data[0],th_sensor_t.sensor_data[1],th_sensor_t.sensor_data[2],th_sensor_t.sensor_data[3]);
        trace_log(put_buf);		
		if(config_get_internal_en() == 0)
		{
			if(config_get_external_en() == 0)
			{                
				th_sensor_t.sensor_state[SENSOR_NUM_1] = sensor_err;
                th_sensor_t.sensor_state[SENSOR_NUM_2] = sensor_err;
                th_sensor_t.sensor_type[SENSOR_NUM_1] = 0;
                th_sensor_t.sensor_type[SENSOR_NUM_2] = 0; 
                th_sensor_t.sensor_data[SENSOR_NUM_1] = 0;
                th_sensor_t.sensor_data[SENSOR_NUM_2] = 0; 	
				
				th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_err;
				th_sensor_t.sensor_data[SENSOR_NUM_3] = 0;
				th_sensor_t.sensor_type[SENSOR_NUM_3] = 0;					
				th_sensor_t.sensor_state[SENSOR_NUM_4] = sensor_err;
				th_sensor_t.sensor_data[SENSOR_NUM_4] = 0;
				th_sensor_t.sensor_type[SENSOR_NUM_4] = 0;
			}
			else
			{
				if(oled_sensor_num[0] + oled_sensor_num[1] < 3)
				{				
					th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_err;
					th_sensor_t.sensor_data[SENSOR_NUM_3] = 0;
					th_sensor_t.sensor_type[SENSOR_NUM_3] = 0;					
					th_sensor_t.sensor_state[SENSOR_NUM_4] = sensor_err;
					th_sensor_t.sensor_data[SENSOR_NUM_4] = 0;
					th_sensor_t.sensor_type[SENSOR_NUM_4] = 0;
				}	
				else if(oled_sensor_num[0] + oled_sensor_num[1] < 4)
				{
					th_sensor_t.sensor_state[SENSOR_NUM_4] = sensor_err;
					th_sensor_t.sensor_data[SENSOR_NUM_4] = 0;
					th_sensor_t.sensor_type[SENSOR_NUM_4] = 0;				
				}			
			}
		}
		else
		{
			if(config_get_external_en() == 0)
			{
				th_sensor_t.sensor_state[SENSOR_NUM_3] = sensor_err;
				th_sensor_t.sensor_data[SENSOR_NUM_3] = 0;
				th_sensor_t.sensor_type[SENSOR_NUM_3] = 0;					
				th_sensor_t.sensor_state[SENSOR_NUM_4] = sensor_err;
				th_sensor_t.sensor_data[SENSOR_NUM_4] = 0;
				th_sensor_t.sensor_type[SENSOR_NUM_4] = 0;
			}
		}
        /* NaN/无穷防御:非法修正系数等产生的垃圾数据不显示、不记录、不对外上报 */
        for(count = 0; count < 4; count++)
        {
            if(th_sensor_t.sensor_state[count] == sensor_ok)
            {
                if(!(th_sensor_t.sensor_data[count] == th_sensor_t.sensor_data[count]) ||
                   (th_sensor_t.sensor_data[count] > 100000.0f) || (th_sensor_t.sensor_data[count] < -100000.0f))
                {
                    th_sensor_t.sensor_data[count] = 0;
                    th_sensor_t.sensor_state[count] = sensor_err;
                }
            }
        }
        mbs_write_reg_float(&th_sensor_t.sensor_data[SENSOR_NUM_1], REG_SENSOR_1);
        mbs_write_reg_float(&th_sensor_t.sensor_data[SENSOR_NUM_2], REG_SENSOR_2);
        mbs_write_reg_float(&th_sensor_t.sensor_data[SENSOR_NUM_3], REG_SENSOR_3);
        mbs_write_reg_float(&th_sensor_t.sensor_data[SENSOR_NUM_4], REG_SENSOR_4);         
    }  
	single_en_close();
    sensor_check();
}

void task_post_event_sensor_isr(uint32_t event)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(sensor_handler, event, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void task_post_event_sensor(uint32_t event)
{
    xTaskNotify(sensor_handler, event, eSetBits);
}

void task_sensor(void *params)
{
    uint32_t event_bit = SENSOR_EVENT_GET_VALUE;
	memset(&th_sensor_t,0,sizeof(th_sensor_t));
    while(1)
    {	
		pwr_opppse_halt(PWR_SENSOR_BIT);
		trace_log("采样开始\n");
		IIC_init();
        single_en_init();
		read_bat_vol();
        sensor_event_proc(event_bit);
		trace_log("采样完成\n");
		pwr_arg_halt(PWR_SENSOR_BIT);
        xTaskNotifyWait(0x00, 0xffffffff, &event_bit, portMAX_DELAY);
    }
}

void sensor_task_create(void *params, uint8_t priority)
{
    xTaskCreate(task_sensor, "task_sensor", 256, params, priority, &sensor_handler); 
}
