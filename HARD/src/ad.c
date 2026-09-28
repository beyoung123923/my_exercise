#include "ad.h"
#include "sys_setting.h"
#include "sensor.h"
#include "main.h"
#include "usart.h"
#include "stdio.h"
#include "pwr_manage.h"

static void MF_ADC_Common_Init(void)
{
    FL_ADC_CommonInitTypeDef    Common_InitStruct;

    Common_InitStruct.clockSource = FL_CMU_ADC_CLK_SOURCE_RCHF;                         /*配置ADC工作时钟源*/
    Common_InitStruct.clockPrescaler = FL_ADC_CLK_PSC_DIV8;                             /*配置ADC工作时钟分配*/
    Common_InitStruct.referenceSource = FL_ADC_REF_SOURCE_VDDA;                         /*配置ADC参考源*/
    Common_InitStruct.bitWidth = FL_ADC_BIT_WIDTH_12B;                                  /*配置ADC输出位宽*/
	FL_ADC_CommonInit(&Common_InitStruct);
}

static void MF_ADC_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct = {0};

    FL_ADC_InitTypeDef    ADC_InitStruct;

    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
		
    ADC_InitStruct.conversionMode = FL_ADC_CONV_MODE_SINGLE;                            /*配置ADC转换模式*/
    ADC_InitStruct.autoMode = FL_ADC_SINGLE_CONV_MODE_AUTO;                             /*配置ADC转换流程，仅对单次转换有效*/
    ADC_InitStruct.waitMode = FL_ENABLE;                                                /*配置ADC等待模式*/
    ADC_InitStruct.overrunMode = FL_ENABLE;                                             /*配置ADC_Overrun模式*/
    ADC_InitStruct.scanDirection = FL_ADC_SEQ_SCAN_DIR_FORWARD;                         /*配置ADC扫描顺序*/
    ADC_InitStruct.externalTrigConv = FL_ADC_TRIGGER_EDGE_NONE;                         /*配置非软件触发使能及极性*/
    ADC_InitStruct.triggerSource = FL_DISABLE;                                          /*配置ADC非软件触发源*/
    ADC_InitStruct.fastChannelTime = FL_ADC_FAST_CH_SAMPLING_TIME_2_ADCCLK;             /*配置ADC快速通道采样时间*/
    ADC_InitStruct.lowChannelTime = FL_ADC_SLOW_CH_SAMPLING_TIME_512_ADCCLK;            /*配置ADC慢速通道采样时间*/
    ADC_InitStruct.oversamplingMode = FL_DISABLE;                                       /*配置ADC过采样模式*/
    ADC_InitStruct.overSampingMultiplier = FL_ADC_OVERSAMPLING_MUL_128X;                 /*配置ADC过采样率*/
    ADC_InitStruct.oversamplingShift = FL_ADC_OVERSAMPLING_SHIFT_8B;                    /*配置ADC过采样结果移位*/

    (void)FL_ADC_Init(ADC, &ADC_InitStruct);
}

void adc_init(void)
{
	MF_ADC_Common_Init();                                                               /*ADC基础配置*/
    MF_ADC_Init();                                                                      /*ADC初始化配置*/
}

/*void adc_deinit(void)
{
	ADC_Enable(ADC1, DISABLE);
    ADC_DeInit(ADC1);
	ADC_EnableTempSensorVrefint(DISABLE);
}*/

static uint8_t GetVREF1P2Sample_POLL(uint32_t *ADCRdresult)
{
    uint32_t counter = 0;
    uint32_t EOC_Flag;
    uint8_t State;
    FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV8);                    //配置ADC工作时钟分频
    FL_VREF_EnableVREFBuffer(VREF);                                 //使能VREF BUFFER,
    //为了考虑功耗BUFFER可关闭，从使能到ADC可以正常采样需要100us的建立时间，本例程配置慢速通道较长的采样时间，来保证建立时间满足。
    FL_ADC_DisableSequencerChannel(ADC, FL_ADC_ALL_CHANNEL);        //清空打开的通道
    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);    //通道选择

    FL_ADC_ClearFlag_EndOfConversion(ADC);                          //清标志
    FL_ADC_Enable(ADC);                                             //启动ADC
    FL_ADC_EnableSWConversion(ADC);                                 //开始转换

    do
    {
        EOC_Flag = FL_ADC_IsActiveFlag_EndOfConversion(ADC);
        counter++;
    } while((counter != 0xFFFFFFFFU) && (EOC_Flag == 0U));           //等待转换完成

    if(EOC_Flag == 0x01U)
    {
        FL_ADC_ClearFlag_EndOfConversion(ADC);                        //清标志
        *ADCRdresult = FL_ADC_ReadConversionData(ADC);                 //获取采样值

        FL_ADC_Disable(ADC);                                          //关闭ADC
        FL_ADC_DisableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2); //通道关闭
        FL_VREF_DisableVREFBuffer(VREF);                              //关闭VREF BUFFER
        State = 0;
    }
    else
    {
        State = 1 ;
    }
    return State;
}

static uint8_t GetSingleChannelSample_POLL(uint32_t channel, uint32_t *ADCRdresult)
{
    uint32_t counter = 0;
    uint32_t EOC_Flag;
    uint8_t State;

    FL_CMU_SetADCPrescaler(FL_CMU_ADC_PSC_DIV1);                  //配置ADC工作时钟分频
    FL_ADC_DisableSequencerChannel(ADC, FL_ADC_ALL_CHANNEL);      //清空打开的通道
    FL_ADC_EnableSequencerChannel(ADC, channel);                  //通道选择

    FL_ADC_ClearFlag_EndOfConversion(ADC);                        //清标志
    FL_ADC_Enable(ADC);                                           //启动ADC
    FL_ADC_EnableSWConversion(ADC);                               //开始转换

    do
    {
        EOC_Flag = FL_ADC_IsActiveFlag_EndOfConversion(ADC);
        counter++;
    } while((counter != 0xFFFFFFFFU) && (EOC_Flag == 0U));           //等待转换完成

    if(EOC_Flag == 0x01U)
    {
        FL_ADC_ClearFlag_EndOfConversion(ADC);                        //清标志
        *ADCRdresult = FL_ADC_ReadConversionData(ADC);                 //获取采样值

        FL_ADC_Disable(ADC);                                          //关闭ADC
        FL_ADC_DisableSequencerChannel(ADC, channel);                 //通道关闭
        State = 0;
    }
    else
    {
        State = 1 ;
    }
    return State;
}

uint32_t GetSingleChannelVoltage_POLL(uint32_t channel)
{
    uint32_t Get122VSample = 0, GetChannelVoltage = 0, GetVSample = 0;
    uint8_t  Get122VSample_State, GetVSample_State;

    Get122VSample_State = GetVREF1P2Sample_POLL(&Get122VSample);
    GetVSample_State = GetSingleChannelSample_POLL(channel, &GetVSample);

    if((Get122VSample != 0) && (Get122VSample_State == 0)  && (GetVSample_State == 0))
    {
        GetChannelVoltage = (uint32_t)(((uint64_t)GetVSample * 3000 * (ADC_VREF)) / ((uint64_t)Get122VSample * 4095));  //计算通道电压
    }

    return GetChannelVoltage;
}

/*void get_adc_vbat_value(void)
{    	
	uint32_t v_data = 0;
	uint8_t num=3;
	char ADC_VALUE[10];	
//电池电压获取	
	while(num--)
	{
		v_data += GetSingleChannelVoltage_POLL(ADC_VBAT_chanl);
		vTaskDelay(3);
	}
	th_sensor_t.bat_voltage = v_data/3/560*2160;
	
	if(FL_GPIO_GetInputPin(usbpower_gpio,usbpower_gpio_id))    //因为输入阻抗太大，电压检测偏高0.2v
	{
		th_sensor_t.bat_voltage -= 200;
	}
	
	sprintf((char *)ADC_VALUE,"%.0f",th_sensor_t.bat_voltage);
	Trace_log("电池电压：");
	Trace_log(ADC_VALUE);
	Trace_log("mV\r\n");	
}*/

//电压检测
/*static void vbat_check(void)
{
	float data=100;
	//一次性电池版本

    if((bat_data_t.voltage < 3350) && (bat_data_t.voltage > 3200))
    {
        data = (bat_data_t.voltage-3200)/150*20;
    }
    else if(bat_data_t.voltage<=3200)
    {
        data = 0;	
		trace_log("no power\r\n"); 
    }
    
    if(data<=20)
    {
		trace_log("low power\r\n");
        bat_data_t.low_power_num++;
        if(bat_data_t.low_power_num>5)                                     //连续6轮电池电压检测都处于低20%电平状态,更新电池值
        {
            bat_data_t.low_power_num = 0;
            config_set_vbat_bfb(data);
            //bat_data_t.vbat_bfb = data;
        }
        if(data<=5)                                                         //电池低于5%，关机判断值++
        {
            bat_data_t.ll_power_num++;
            if(bat_data_t.ll_power_num>3)                                  //连续4轮电池电压检测都处于低电平状态，进入低电量关机
			{
                //bat_data_t.MCU_CLOSE = 2;
				task_post_event_pwr_manage(PWR_EVENT_OFF);			
			}
        }
        else
            bat_data_t.ll_power_num =0;
    }
    else
    {
		//bat_data_t.vbat_bfb = data;
        bat_data_t.ll_power_num = 0;
        bat_data_t.low_power_num = 0;
    }	
}*/

void read_ad(void)
{
	uint32_t v_data = 0;
	uint8_t num=3;
	//电池电压获取	
	while(num--)
	{
		v_data += GetSingleChannelVoltage_POLL(FL_ADC_EXTERNAL_CH0);
		vTaskDelay(3);
	}
	th_sensor_t.bat_voltage = (float)v_data*2160.0f/3.0f/560.0f;
	
	if(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7))    //因为输入阻抗太大，电压检测偏高0.2v
	{
		th_sensor_t.bat_voltage -= 200;
	}
}

