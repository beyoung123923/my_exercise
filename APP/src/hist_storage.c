#include <stdio.h>
#include "hist_storage.h"
#include "sys_setting.h"
#include "w25qxx.h"
#include "sensor.h"
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "pwr_manage.h"
#include "tool.h"
#include "usart.h"
#include "touch_key.h"
#include "sd3078.h"
#include "server.h"
#include "iwdg.h"
#include "main.h"

char histroy_buf[150];

uint8_t sensor_code_cn[13][13]  = {"    ","温度", "CO2", "湿度", "气压", "HCHO", "NH3 ", "CO2", "TVOC", "NOISE", "TYPE_NH3_1", "TYPE_PM"};
uint8_t sensor_code_en[13][13] = {"    ","TEMP", "CO2", "HUMI", "PRESSURE", "HCHO", "NH3 ", "CO2", "TVOC", "NOISE", "TYPE_NH3_1", "TYPE_PM"};
uint8_t device_state_cn[4][13] = {"电压","报警状态","信号强度"};
uint8_t device_state_en[4][13] = {"BAT_V","ALARM_S","SIGNAL"};
hist_storage_t hist_data;
hist_run_t     hist_run;
TaskHandle_t   task_hist_handle;

static uint32_t hist_get_date_addr(uint8_t year, uint8_t mon, uint8_t day);

/**
  * @brief  历史数据保存函数
  * @param  写入地址
  * @retval None
  */
uint8_t hist_save(uint32_t *addr)
{
	uint32_t current_addr = 0;
    uint32_t record_num;
	
	current_addr = *addr;

	if((current_addr % FLASH_SECTOR_SIZE + sizeof(hist_storage_t)) / sizeof(hist_storage_t) == (FLASH_SECTOR_SIZE / sizeof(hist_storage_t)))   /*判断是否扇区最后一个*/
	{
		if((current_addr / FLASH_SECTOR_SIZE + 1)!= FLASH_SECTOR_NUM)
		{
			//TRACE_LOG(TAG, "\r\nPAGE_SWITHC\r\n");
			hist_data.next_addr = (current_addr / FLASH_SECTOR_SIZE + 1) * FLASH_SECTOR_SIZE;                /*next_addr 指向下一个扇区起始地址*/
			trace_log("跳下一个扇区存储\n");
		}
		else
		{
			hist_data.next_addr = 0;
			trace_log("存满\n");
		}
	}
	else
	{
		hist_data.next_addr = current_addr + sizeof(hist_data);
	}
	if(current_addr % 4096 == 0)  /* 扇区首地址 */
	{
		w25qxx_erase_sector(current_addr);
	}
	
	sprintf(histroy_buf,"%d-%d-%d %d:%d:%d\n",real_time.year,real_time.month,real_time.day,real_time.hour,real_time.min,real_time.sec);
	trace_log(histroy_buf);

	hist_data.data_time.sec = real_time.sec;
	hist_data.data_time.min = real_time.min;
	hist_data.data_time.hour = real_time.hour;
	hist_data.data_time.week = real_time.week;
	hist_data.data_time.day = real_time.day;
	hist_data.data_time.month = real_time.month;
	hist_data.data_time.year = real_time.year;

	hist_data.prev_addr = device_get_flashaddr_addr();

	for(uint8_t index = 0; index < 4; index++)
	{
		hist_data.sensor_value[index] = sensor_get_value(index);
	}

    hist_data.battery_voltage = config_get_vbat_value();
    hist_data.sensor_alarm_state = sensor_get_alarm();
    hist_data.rssi = config_get_signal();

	hist_data.crc = (uint32_t)calc_crc16((uint8_t *)&hist_data, sizeof(hist_storage_t) - 4);

	w25qxx_write((uint8_t *)&hist_data, current_addr, sizeof(hist_storage_t));
	
	w25qxx_read((uint8_t *)&hist_data, current_addr, sizeof(hist_storage_t));
	if(hist_data.crc != (uint32_t)calc_crc16((uint8_t *)&hist_data, sizeof(hist_storage_t) - 4))
	{
		trace_log("数据保存失败\n");
		return 1;
	}
	
    if(device_set_flashaddr_addr(current_addr) == FALSE)
    {
        trace_log("历史地址保存失败\n");
        return 1;
    }

    if(device_get_record_num() >= MAX_RECORD_NUM)
    {
        record_num = MAX_RECORD_NUM - SECTOR_RECORD_NUM;
    }
    else
    {
        record_num = device_get_record_num() + 1;
    }
    if(device_set_record_num(record_num) == FALSE)
    {
        trace_log("记录数保存失败\n");
        return 1;
    }

	if(hist_detect(current_addr))
	{
        sprintf(histroy_buf,"当前地址:%d\r\n",current_addr);
        trace_log(histroy_buf);
		/*printf("current_addr:%d", current_addr);
		printf("CRC ERROR");*/
		return 1;
	}
	*addr = hist_data.next_addr;  /* 偏移到当前地址  */
	return 0;
}


/**
  * @brief  检测最后一包数据是否出错，存在掉电保护功能
  * @param  读取地址
  * @retval 1：掉电导致数据错误 0：数据完整
  */

uint8_t hist_detect(uint32_t addr)
{
	w25qxx_read((uint8_t *)&hist_data, addr, sizeof(hist_storage_t));

	if(hist_data.crc != calc_crc16((uint8_t *)&hist_data, sizeof(hist_storage_t) - 4))   /* 校验CRC是否正确 */
	{
		//TRACE_LOG(TAG, "crc err");
		return 1;
	}
	return 0;
}

/**
  * @brief  历史记录功能初始化
  * @param  None
  * @retval 当前写入数据地址
  */
uint32_t hist_init(void)
{
	uint32_t addr;
	vTaskDelay(5000);
	addr = device_get_flashaddr_addr();
	
	if(addr >= FLASH_ADDR_END)     //0x400000     留出0x30000 192K空间做OTA数据升级使用
	{ 
		trace_log("FLASH已满，从头开始");
		addr = 0;

		device_pending_num_set(0);
		device_perv_addr_set(0);
		device_set_flashaddr_addr(0);
		device_set_record_num(0);
		if((device_pending_num_set(0) == FALSE)||(device_perv_addr_set(0) == FALSE)
		||(device_set_flashaddr_addr(0) == FALSE)||(device_set_record_num(0) == FALSE))
		{
			trace_log("RTC RAM复位失败\n");
		}
	}
	
	if(hist_detect(addr))                       /*校验错误 */
	{
		hist_save(&addr);
	}
	else 
	{
		w25qxx_read((uint8_t *)&hist_data, addr, sizeof(hist_storage_t));
		addr = hist_data.next_addr;
	}
	return addr;
}


/**
  * @brief  读取历史数据
  * @param  读取存放结构体
	* @param  读取地址
  * @retval None
  */
void hist_read(hist_storage_t *read_hist, uint32_t addr)
{
	w25qxx_read((uint8_t *)read_hist, addr ,sizeof(hist_storage_t));
}


/**
  * @brief  获得给出时间的最接近数据所在flash的地址
  * @param  年月日
  * @retval flash地址
  */

static uint32_t hist_get_date_addr(uint8_t year, uint8_t mon, uint8_t day)
{
	hist_storage_t min_hist;  			/*当前数组里面最小的*/
	uint32_t sec_count = 0;
	uint8_t  mon_min = 12;
	int abs = 0;
	int read_addr = 0;
	uint32_t current_addr = 0;

	for(sec_count = 0;sec_count <  FLASH_SECTOR_NUM; sec_count ++)   /* 枚举每个扇区的首地址 求出最符合的扇区 */
	{
		hist_read(&hist_data, sec_count * 4096);
		
		if(hist_data.next_addr > FLASH_ADDR_END)  /* 无数据或者数据异常清空(数据异常基本不会出现) */
		{
			continue;
		}

		if(hist_data.data_time.year ==  year &&	   	/* 处于同一个月的时候跳出*/
			 hist_data.data_time.month == mon ) 
		{
			memcpy(&min_hist, &hist_data, sizeof(hist_storage_t));
			if(day > hist_data.data_time.day && sec_count + 1 < FLASH_SECTOR_NUM)
			{
				hist_read(&hist_data, (sec_count + 1) * 4096);
				if(hist_data.data_time.month != mon)
				{
					break;
				}
			}
			else 
			{
				break;
			}
		}
		
		if(hist_data.data_time.year == year && 
			 hist_data.data_time.month < mon)
		{
			/*条件1: 当数据小于起始月份但是大于当前最小值的时候取值 当月份相差小于的时候取值*/
			if(mon - hist_data.data_time.month <= mon_min)
			{
				current_addr = sec_count * FLASH_SECTOR_SIZE;
				mon_min = mon - hist_data.data_time.month;
				memcpy(&min_hist, &hist_data, sizeof(hist_storage_t));
			}
		}
		else if(hist_data.data_time.year == year &&
			      hist_data.data_time.month > mon )
		{
			/*条件2：当数据月份是大于 最小日期且小于最小值的月份时成立 或者是首次进入复制*/
			if(hist_data.data_time.month - mon < mon_min)
			{
				current_addr = sec_count * FLASH_SECTOR_SIZE;
				mon_min = hist_data.data_time.month - mon ;
				memcpy(&min_hist, &hist_data, sizeof(hist_storage_t));
			}
		}
	}
	
	if(current_addr == 0)        /*当符合日期的时候 则把扇区首地址当成最小值*/
	{
		if(min_hist.next_addr == 0)  /* 找不到该年的数据 */
		{
			return 1;
		}
	}

	while(min_hist.data_time.month != mon)  /* 求出月份 */
	{
		if(min_hist.data_time.month > mon)  /*大于的话向前 返回值应该为后面一个数据的地址*/
		{
			abs = min_hist.data_time.month - mon;
			read_addr = min_hist.prev_addr;
			
			w25qxx_read((uint8_t *)&min_hist, read_addr, sizeof(hist_storage_t));  
		
			if((min_hist.data_time.month < mon) || (min_hist.data_time.month - mon) > abs  || min_hist.data_time.year != year)
			{
				return current_addr;				/* 当月份绝对值变大，年份不一致 返回上一个地址 */
			}	
			if(read_addr == min_hist.prev_addr)
			{
				return read_addr;
			}
			current_addr = read_addr;
		}
		else if(min_hist.data_time.month < mon)
		{
			abs = mon - min_hist.data_time.month;
			read_addr = min_hist.next_addr;
			w25qxx_read((uint8_t *)&min_hist, min_hist.next_addr, sizeof(hist_storage_t));  
			
			
			if(min_hist.data_time.month > mon || (mon - min_hist.data_time.month) > abs || min_hist.data_time.year != year) /* 由小变大，选择当前的日期 */  
			{
				if(min_hist.next_addr > FLASH_ADDR_END)
				{
					return 1;
				}
				return read_addr; 
			}
		}
	}

	while(min_hist.data_time.day != day) /*[相等月份的]求出日期的指针 */
	{
		if(min_hist.data_time.day > day)
		{
			abs = min_hist.data_time.day - day;
			
			read_addr = min_hist.prev_addr;
			w25qxx_read((uint8_t *)&min_hist, min_hist.prev_addr, sizeof(hist_storage_t));   /* 向前偏移 */
			if(
				 year != min_hist.data_time.year    ||
			     mon != min_hist.data_time.month    ||
				 min_hist.data_time.day < day       || 
				 (min_hist.data_time.day - day) > abs)  								/*当年月月份变了，或者相差变大了*/
			{
				return current_addr;                                           /* 前一个才是最佳解 */
			}
			if(read_addr == min_hist.prev_addr)
			{
				return read_addr;
			}
			current_addr = read_addr;
		}
		else if(min_hist.data_time.day < day)
		{
			abs = day - min_hist.data_time.day;
			read_addr = min_hist.next_addr;
			w25qxx_read((uint8_t *)&min_hist, min_hist.next_addr, sizeof(hist_storage_t));   /* 向前偏移 */
			
			if( 
				year != min_hist.data_time.year ||
			  mon != min_hist.data_time.month ||
				min_hist.data_time.day > day || 
				(day - min_hist.data_time.day ) > abs )  /*当前值？*/
			{
				if(min_hist.next_addr > FLASH_ADDR_END)
				{
					return 1;
				}
				return read_addr;
			}
		}
	}
	while(min_hist.data_time.day == day && min_hist.data_time.month == mon)
	{
		read_addr = min_hist.prev_addr;
		w25qxx_read((uint8_t *)&min_hist, min_hist.prev_addr, sizeof(hist_storage_t));
		
		if(min_hist.prev_addr >= FLASH_ADDR_END)
		{
			break;
		}
		current_addr = min_hist.next_addr;
		
		if(read_addr == min_hist.prev_addr)
		{
			return read_addr;
		}
	}
	return current_addr;
}

/**
  * @brief  读取数据，按起始年月日和结束年月日来进行
  * @param  起始年月日-->结束年月日
  * @retval None
  */
void hist_read_date(uint8_t s_year, uint8_t s_mon, uint8_t s_day, uint8_t e_year, uint8_t e_mon, uint8_t e_day)
{
	uint32_t start_addr = 1;
	uint32_t read_count = 0;
	hist_storage_t   min_hist;  			/*当前数组里面最小的*/
	uint32_t curren_day;

	uint32_t from_day = s_year * 365 + s_mon * 30 + s_day;
	uint32_t to_day = e_year * 365 + e_mon * 30 + e_day;
	
	uint8_t test = 0;
	
	while(s_year <= e_year)
	{
		start_addr = hist_get_date_addr(s_year, s_mon, s_day);
		if(start_addr != 1)
		{
			break;
		}
		else 
		{
			s_year += 1;s_mon = 1;s_day = 1;
		}
	}
	
	if(config_get_language() == CHINESE)
	{	
		sprintf(histroy_buf,"记录时间           %s    %s    %s   %s        %s        %s    %s\n", 
				sensor_code_cn[sensor_get_type(SENSOR_NUM_1)],
				sensor_code_cn[sensor_get_type(SENSOR_NUM_2)],
				sensor_code_cn[sensor_get_type(SENSOR_NUM_3)],
				sensor_code_cn[sensor_get_type(SENSOR_NUM_4)],
                device_state_cn[0],
                device_state_cn[1],
                device_state_cn[2]);
	}
	else 
	{
		sprintf(histroy_buf,"Time               %s    %s    %s   %s    %s    %s    %s\n", 
				sensor_code_en[sensor_get_type(SENSOR_NUM_1)],
				sensor_code_en[sensor_get_type(SENSOR_NUM_2)],
				sensor_code_en[sensor_get_type(SENSOR_NUM_3)],
				sensor_code_en[sensor_get_type(SENSOR_NUM_4)],
                device_state_en[0],
                device_state_en[1],
                device_state_en[2]);
	}
	uart_transmit_str(USB_UART,(uint8_t *)histroy_buf);
	if(start_addr == 1)
	{
		return ;
	}
	
	while(1)
	{
		iwdt_reload();
		w25qxx_read((uint8_t *)&min_hist, start_addr, sizeof(hist_storage_t));
		if((++read_count >= (4096 / sizeof(hist_storage_t) * 1024)) || (min_hist.next_addr == 0XFFFFFFFF))
		{
			break;  /*读了一圈 马上退出*/
		}
		
		curren_day = min_hist.data_time.year * 365 + min_hist.data_time.month * 30 + min_hist.data_time.day;
		
		if((curren_day >= from_day) && (curren_day <= to_day))
		{
			sprintf(histroy_buf, "%d-%02d-%02d %02d:%02d%10.2f%10.2f%10.2f%10.2f%12.2f%10d%10d\n",
																min_hist.data_time.year,
																min_hist.data_time.month,
																min_hist.data_time.day,
																min_hist.data_time.hour,
																min_hist.data_time.min,
																min_hist.sensor_value[SENSOR_NUM_1],
																min_hist.sensor_value[SENSOR_NUM_2],
																min_hist.sensor_value[SENSOR_NUM_3],
																min_hist.sensor_value[SENSOR_NUM_4],
                                                                min_hist.battery_voltage,
                                                                min_hist.sensor_alarm_state,
                                                                min_hist.rssi);

			uart_transmit_str(USB_UART, (uint8_t *)histroy_buf);
			vTaskDelay(5);
			start_addr = min_hist.next_addr;
            test = 0;
		}
		else 
		{
			if(test++ > 5)
			{
				break;
			}
		}
		start_addr = min_hist.next_addr;		
	}
	uart_transmit_str(USB_UART,(uint8_t *)"读取完毕\r\n");
}

void hist_set_data_read(uint8_t s_year, uint8_t s_mon, uint8_t s_day, uint8_t e_year, uint8_t e_mon, uint8_t e_day)
{
	hist_run.start_year = s_year;
	hist_run.start_mon = s_mon;
	hist_run.start_day = s_day;

	hist_run.end_year = e_year;
	hist_run.end_mon = e_mon;
	hist_run.end_day = e_day;
	task_flash_post_event(HIST_READ);
}

void hist_clear()
{
	w25qxx_erase_chip();
	device_pending_num_set(0);
	device_set_flashaddr_addr(0);
    device_perv_addr_set(0);
    device_set_record_num(0);
	if((device_pending_num_set(0) == FALSE)||(device_set_flashaddr_addr(0) == FALSE)
	||(device_perv_addr_set(0) == FALSE)||(device_set_record_num(0) == FALSE))
	{
		trace_log("RTC RAM复位失败\n");
	}

	hist_run.save_addr = 0;
    
	beep(100);
	vTaskDelay(100);
	beep(100);
	vTaskDelay(100);
	beep(100);
	vTaskDelay(100);
}

void task_flash_post_event(uint32_t event)
{
	xTaskNotify(task_hist_handle, event, eSetBits);
}

void create_task_hist(void *params, uint8_t priority)
{
	xTaskCreate(task_flash, "hist_flash", 512, params, priority, &task_hist_handle);
}

void task_flash(void *pvrams)
{
	uint32_t flash_event = HIST_SAVE;
	uint8_t flash_err_cont = 0;
	Spi2Init();
	vTaskDelay(100);
    w25qxx_init();
	while(is_sensor_finish() == 0)
	{
		vTaskDelay(500);
	}    
	hist_run.save_addr = hist_init();
	while(1)
	{
		pwr_opppse_halt(PWR_HIST_BIT);
		trace_log("存储功能开始\n");
		Spi2Init();
		vTaskDelay(100);
		w25qxx_init();
		if((HIST_SAVE & flash_event) == HIST_SAVE)
		{
			if(hist_save(&hist_run.save_addr))
			{
				flash_err_cont++;
				if(flash_err_cont >= 10)
				{
					trace_log("存储故障，清除历史\n");
					hist_clear();
					vTaskDelay(200);
				}
				else
				{
					task_flash_post_event(HIST_SAVE);
				}						
			}
            else
            {	
                if(config_get_modbus_en() == 0)
                {
				    task_post_event_iot(SEND_DATA);	
                }
            }
		}
		else if((HIST_READ & flash_event) == HIST_READ)
		{
			hist_read_date(hist_run.start_year, hist_run.start_mon, hist_run.start_day,
						   hist_run.end_year, hist_run.end_mon, hist_run.end_day);
		}
		if((HIST_CLEAR & flash_event) == HIST_CLEAR)
		{
			hist_clear();
			vTaskDelay(200);
		}
		trace_log("存储功能结束\n");
		pwr_arg_halt(PWR_HIST_BIT);
		xTaskNotifyWait(0x00, 0xffffffff, &flash_event, portMAX_DELAY);
	}
}

