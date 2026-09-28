#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "touch_key.h"
#include "sensor.h"
#include "oled.h"
#include "sd3078.h"
#include "sys_setting.h"
#include "w25qxx.h"
#include "pwr_manage.h"
#include "myiic.h"
#include "usart.h"
#include "hist_storage.h"
#include "display.h"
#include "ad.h"
#include "main.h"
#include "iwdg.h"
#include "delay.h"

uint8_t pwr_off = 0;
static xTaskHandle pwr_manage_handler = NULL;
TaskHandle_t task_sleep_handle;
uint32_t wait_min = 0;
static PWR_ARG_HALT st_pwr_arg_halt;
static uint32_t collect_wait_cnt = 0;
static uint16_t one_hour_cont = 0;
uint16_t one_day_cont = 0;
static uint16_t zero_min_state = 0;
static uint8_t alarm_state = 0;
static uint8_t alarm_bit = 0;
static uint8_t alarm_flag = 0;
rtc_t next_uptime;
rtc_t last_uptime;
rtc_t real_time;
static char pwr_buf[50];



/**
* @brief  初始化MCU基本功能.
*/
uint8_t sb_enable_halt = 0;

void GPIO_IRQHandler(void)
{
    if((FL_GPIO_GetTriggerEdge0(GPIO, FL_GPIO_EXTI_LINE_2) != 							//RTC
        FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE) &&
        FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2))
    {
        FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_2);		
		task_post_isr_event_pwr_manage(PWR_EVENT_ALARM);
    }
	if((FL_GPIO_GetTriggerEdge0(GPIO, FL_GPIO_EXTI_LINE_5) != 							//USB
	FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE) &&
	FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_5))
	{
		FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_5);
		task_post_isr_event_pwr_manage(PWR_EVENT_KEY);
	}
	if((FL_GPIO_GetTriggerEdge0(GPIO, FL_GPIO_EXTI_LINE_8) != 							//SET按键
	FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE) &&
	FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_8))
	{
		FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_8);
		task_post_isr_event_pwr_manage(PWR_EVENT_KEY);
	}
	if((FL_GPIO_GetTriggerEdge0(GPIO, FL_GPIO_EXTI_LINE_9) != 							//POWER按键
	FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE) &&
	FL_GPIO_IsActiveFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_9))
	{
		FL_GPIO_ClearFlag_EXTI(GPIO, FL_GPIO_EXTI_LINE_9);
		task_post_isr_event_pwr_manage(PWR_EVENT_KEY);
	}
}

void exti_irq_init(void)
{
    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, GPIO_IRQn);  
}
void rtc_exti_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_EXTI_InitTypeDef    EXTI_InitStruct;

    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    EXTI_InitStruct.filter = FL_DISABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP1;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_FALLING;	
	FL_EXTI_Init(FL_GPIO_EXTI_LINE_2, &EXTI_InitStruct);    
}

void usb_exti_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_EXTI_InitTypeDef    EXTI_InitStruct;

    /* PB7 EXTI5_Select3 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    EXTI_InitStruct.filter = FL_DISABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP3;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_RISING;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_5, &EXTI_InitStruct);  
}

void key_exti_init(void)
{
	chip_touch_pad_init();
	touch_exit_init();
}

void chip_init(void)
{
	I2C_init();
	beep_and_led_init();
	usb_uart_init();
	gps_en_init();
	MF_BSTIM16_Init2();
	MF_GPTIM0_TimerBase_Init();
    return;
}

/**
  * @brief  允许进入休眠模式
  * @param  PWR_BIT_Type e_id: 任务ID
  * @retval None.
  */
void pwr_arg_halt(PWR_BIT_Type e_id)
{
    if(PWR_BIT_INVALID_MIN < e_id && e_id < PWR_BIT_INVALID_MAX)
    {
        st_pwr_arg_halt &= ~((PWR_ARG_HALT)1 << (e_id - PWR_BIT_INVALID_MIN -1));  
    }
}

/**
  * @brief  取消进入休眠模式
  * @param  PWR_BIT_Type e_id: 任务ID
  * @retval None.
        */
void pwr_opppse_halt(PWR_BIT_Type e_id)
{
    if(PWR_BIT_INVALID_MIN < e_id&& e_id < PWR_BIT_INVALID_MAX)
    {
        st_pwr_arg_halt |= ((PWR_ARG_HALT)1 << (e_id - PWR_BIT_INVALID_MIN -1));
    }
}

void DeepSleep(void)
{
    FL_RMU_PDR_Enable(RMU);             // 打开PDR
    FL_RMU_BOR_Disable(RMU);            // 关闭BOR 2uA
	
	FL_ADC_Disable(ADC);         //关闭ADC
    FL_VREF_Disable(VREF);              // 关闭VREF1p2
    FL_CDIF_DisableCPUToVAO(CDIF);      // 关闭CPU到VAO的通道
	FL_PMU_SetRCHFWakeupFrequency(PMU,FL_PMU_RCHF_WAKEUP_FREQ_24MHZ);
    FL_PMU_SetLowPowerMode(PMU, FL_PMU_POWER_MODE_SLEEP_OR_DEEPSLEEP);
    __WFI();                            //休眠
    FL_VREF_Enable(VREF);               //使能VREF1p2
	
//	FL_PMU_SleepInitTypeDef LPM_InitStruct;
//    
//    FL_CMU_RCLF_Enable();               // 暂开启RCLF
//	
//    FL_RMU_PDR_Enable(RMU);  			// 打开PDR
//    FL_RMU_BOR_Disable(RMU);  			// 关闭BOR 2uA
//    FL_VREF_Disable(VREF);              // 关闭VREF1p2
//	FL_ADC_Disable(ADC);         		//关闭ADC
//	FL_CDIF_DisableCPUToVAO(CDIF);      // 关闭CPU到VAO的通道
//    
//    LPM_InitStruct.deepSleep 			= FL_PMU_SLEEP_MODE_DEEP;
//    LPM_InitStruct.powerMode 			= FL_PMU_POWER_MODE_SLEEP_OR_DEEPSLEEP;
//    LPM_InitStruct.wakeupFrequency 		= FL_PMU_RCHF_WAKEUP_FREQ_24MHZ;
//    LPM_InitStruct.wakeupDelay 			= FL_PMU_WAKEUP_DELAY_2US;
//	LPM_InitStruct.LDOLowPowerMode   	= FL_PMU_LDO_LPM_DISABLE;
//    LPM_InitStruct.coreVoltageScaling   = FL_DISABLE; 
//    FL_PMU_Sleep_Init(PMU, &LPM_InitStruct);
//		
//	__WFI();                            //休眠
//    FL_VREF_Enable(VREF);               //使能VREF1p2
}


void chip_off(void)
{
	iwdt_reload();
	Spi2Init();
	w25qxx_init(); 
	w25qxx_power_down();    				//关闭FLASH
	
	if(pwr_off == 1)
	{
		vTaskSuspendAll();
	}
	
	oled_deinit(); 
    beep_and_led_deinit();
    //adc_deinit();
    uart_enter_sleep_mode(WIFI_UART);
    uart_enter_sleep_mode(GPS_UART);
    uart_enter_sleep_mode(IOT_4G_UART);
	Spi2deInit();
	single_pin_deinit();
	IIC_deinit();
	I2C_deinit();
    iot_en_deinit();
    uart_enter_sleep_mode(USB_UART);
	touch_exit_init();
	sb_enable_halt = 1;
	DeepSleep();
	if(pwr_off == 1)
	{
		xTaskResumeAll();
	}
}

void pwr_exit_low_pwr(void)
{
	sb_enable_halt = 0;
	chip_init();
	touch_exit_deinit();	
}

void pwr_enable_low_pwr(void)
{
	//if((sb_enable_halt == 0)&&(0 == st_pwr_arg_halt))
    if((sb_enable_halt == 0)&&(0 == st_pwr_arg_halt) && (!FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7)))
    {
		trace_log("sleep\n");
		chip_off();
		pwr_exit_low_pwr();
		trace_log("run\n");
    }
    // else
    // {
    //     if(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7))
    //     {
    //         pwr_off = 0;
    //     }
    // }
}

static void oled_start(void)
{
    trace_log("PWR  OLED_SHOW\r\n");
	//OLED_Init();
    task_oled_post_event(NEXT_TOUCH);
}

/**
  * @brief  电源管理
  * @param  None
  * @retval None
  */
void pwr_sleep_proc(uint32_t event_bit)
{
    if(RTC_ReadDate(&real_time) == FALSE)
    {
        trace_log("时间读取失败\n");
    } 
	if(event_bit & PWR_EVENT_ALARM)
    {
//        trace_log("rtc中断\r\n"); 
		if((pwr_off == 0)||(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7)))	//USB在位时关机仅关屏，采样存储上报继续（方案A）
		{
			collect_wait_cnt++;
			one_hour_cont++;

            if(collect_wait_cnt >= config_get_collect_time())            //达到采样时间
			{
				if(is_flash_finish())
				{
	                //trace_log("采样\r\n");  
					collect_wait_cnt = 0;
					task_post_event_sensor(SENSOR_EVENT_GET_VALUE);				
				}
			}

            if(sensor_get_alarm())
            {
                alarm_state = 1;
            }
            else
            {
                alarm_state = 0;
            }

            if((config_get_alarm_en())&&(alarm_state != alarm_bit))
            {
                alarm_flag = 1;
                if(last_uptime.min != real_time.min)
                {
                    if(alarm_flag)
                    {
                        trace_log("报警上报与解除报警上报\r\n"); 
                        alarm_bit = alarm_state;
                        alarm_flag = 0;
                        next_updata_time(real_time);
                        task_flash_post_event(HIST_SAVE);
                    }
                }
            }

			if(config_get_up_type() == 0)
			{
				if((real_time.min == next_uptime.min)&&(real_time.hour == next_uptime.hour))
				{
					if(zero_min_state == 0)
					{
						zero_min_state++;
						trace_log("间隔上报\r\n");  
						next_updata_time(real_time);
						task_flash_post_event(HIST_SAVE);            //记录数据 -->记录完会主动上报						
					}
				}
				else
				{
					zero_min_state = 0;
				}
			}
			else
			{	
				if(real_time.min == 0)
				{ 
					if(zero_min_state == 0)
					{
						zero_min_state++;
						trace_log("整点上报\r\n");
                        last_uptime.min = real_time.min;
						task_flash_post_event(HIST_SAVE);                 //记录数据 -->记录完会主动上报		
					}        
				}
				else
				{
					zero_min_state = 0;
				}
			}

#if 0
			if(one_hour_cont >= 12)				//测试
#endif				
			if(one_hour_cont >= 360)
			{
				if(config_get_ota_en())
				{
					one_day_cont++;
					if(one_day_cont >= 30)
					{
						one_day_cont = 30;
					}
				}
				else
				{
					one_day_cont = 0;
				}
				one_hour_cont = 0;
			}
		}   
    }
	
	if(event_bit & PWR_EVENT_KEY)						//唤醒屏幕和开机
	{
		if(pwr_off == 0)
		{
			oled_start();
		}
		else
		{
			pwr_off = 0;
			FL_RMU_SetSoftReset(RMU);			
		}
	}	
}

/**
 * @description: 发送任务时间
 * @param  event: 时间
 * @return None
 */
void task_post_event_pwr_manage(uint32_t event)
{
    xTaskNotify(pwr_manage_handler, event, eSetBits);
}

/**
 * @description: 发送任务时间ISR
 * @param  event: 时间
 * @return None
 */
void task_post_isr_event_pwr_manage(uint32_t event)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	xTaskNotifyFromISR(task_sleep_handle, event, eSetBits, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void task_sleep_post_event(uint32_t bit)
{
    xTaskNotify(task_sleep_handle, bit, eSetBits);
}

uint32_t sleep_get_last_wait_cnt(void)
{
    return wait_min;
}

void sleep_set_last_wait_cnt(uint32_t cnt)
{
    wait_min = cnt;
}

uint8_t is_sensor_finish(void)
{
	if((st_pwr_arg_halt&0x02)== 0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

uint8_t is_flash_finish(void)
{
	if((st_pwr_arg_halt&0x08)== 0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

uint8_t is_server_finish(void)
{
	if((st_pwr_arg_halt&0x01)== 0)
	{
		return 1;
	}
	else 
	{
		return 0;
	}
}

void next_updata_time(rtc_t rtc_time)
{
    uint16_t updata_min = 0;
	uint8_t up_load_hour;
	uint8_t up_load_min;

    sprintf(pwr_buf,"当前时间:%d:%d\n",rtc_time.hour,rtc_time.min);	
	trace_log(pwr_buf);

    memcpy(&last_uptime,&real_time,sizeof(real_time));

    if((sensor_get_alarm())&&(config_get_updata_time()>2)&&(config_get_alarm_en())&&(config_get_alarm_interval_en()))
    {
        updata_min = 2;					
    }
    else
    {
        updata_min = config_get_updata_time();
    }
	
	up_load_hour = updata_min/60;
	up_load_min = updata_min%60;
	
	next_uptime.hour = rtc_time.hour + up_load_hour;	
	next_uptime.min = rtc_time.min + up_load_min;
	
	if(next_uptime.min >= 60)
	{
        next_uptime.hour = next_uptime.hour+next_uptime.min/60;
		next_uptime.min = next_uptime.min%60;
	}
	
	if(next_uptime.hour >= 24)
	{
		next_uptime.hour = next_uptime.hour%24;
	}	

	sprintf(pwr_buf,"next:%d:%d\n",next_uptime.hour,next_uptime.min);	
	trace_log(pwr_buf);
}

void next_updata_init(void)
{
    if(RTC_ReadDate(&real_time) == FALSE)
    {
        trace_log("时间读取失败\n");
    } 
	next_updata_time(real_time);
}

void task_sleep(void *params)
{
    uint32_t sleep_event = 0;
	iwdt_init();
	MF_EXTI_Common_Init();
	usb_exti_init();
    rtc_exti_init();
	exti_irq_init();
	sd3078_alarm();
	next_updata_init();

    while(1)
    {
		iwdt_reload();
        pwr_sleep_proc(sleep_event);
        pwr_enable_low_pwr();
        xTaskNotifyWait(0x00, 0xffffffff, &sleep_event, 1000);	
    }
}

void create_task_pmu(void *params, uint8_t priority)
{
    st_pwr_arg_halt = ((PWR_ARG_HALT)1 << PWR_MANAGE_NUM) - 1;
    xTaskCreate(task_sleep, "pwr_mang", 256, params, priority, &task_sleep_handle);
    return;
}

