/**
  ****************************************************************************************************
  * @file    main.c
  * @author  FMSH Application Team
  * @brief   Header file of FL Module
  ****************************************************************************************************
  * @attention
  *
  * Copyright (c) [2021] [Fudan Microelectronics]
  * THIS SOFTWARE is licensed under Mulan PSL v2.
  * can use this software according to the terms and conditions of the Mulan PSL v2.
  * You may obtain a copy of Mulan PSL v2 at:
  *          http://license.coscl.org.cn/MulanPSL2
  * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, 
  * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, 
  * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
  * See the Mulan PSL v2 for more details.
  *
  ****************************************************************************************************
  */

#include "main.h"
#include "fm33_assert.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "fm33lg0xx_fl.h"
#include "usart.h"
#include "string.h"
#include "device_info.h"
#include "sys_setting.h"
#include "display.h"
#include "touch_key.h"
#include "sensor.h"
#include "pwr_manage.h"
#include "hist_storage.h"
#include "Air780E.h"
#include "iwdg.h"
#include "flash.h"

/**    
* Chip Series: FM33LG0xx;
* FL Version: v2.3;
* Project Name: new_tp500;
* Project Version: v2.3.0.0;
* Project ID: 2018284493629599745;
* Create Date: 2026-02-02;
*/

SemaphoreHandle_t sd3078_semaphore;				//SD3078访问互斥量
SemaphoreHandle_t w25q_semaphore;				//W25Qxx(SPI2)访问互斥量


static void iap_done(void)
{
	uint32_t save_flag = 0;
	uint8_t vbuf_c = 0;
	uint32_t rbuf[2];

	flash_read(APP_UPDATE_FLAG,rbuf, 2);
	save_flag = Version_DATA();      					//计算当前程序版本号
	if(rbuf[1] == 0xffffffff || rbuf[1]<save_flag)      //程序版本号未初始化或者读到版本小于当前程序版本
	{
		rbuf[1] = save_flag;
		vbuf_c = 1;        								//软件版本有变更
	}
	if(rbuf[0] != 0x0500 || vbuf_c == 1) 				//硬件型号没写入或者软件版本有变更
	{
		rbuf[0] = 0x0500;
		flash_write(APP_UPDATE_FLAG, rbuf, 2,1);
	}
}
  
int main(void)
{
	SCB->VTOR = 0x00002000;								//APP ADDR
    /* Configure the system clock */
    MF_Clock_Init();
    
    sd3078_semaphore = xSemaphoreCreateMutex();
    if(sd3078_semaphore == NULL)              //互斥量创建失败则软复位重试
    {
        FL_RMU_SetSoftReset(RMU);
        while(1);
    }
    w25q_semaphore = xSemaphoreCreateMutex();
    if(w25q_semaphore == NULL)                //互斥量创建失败则软复位重试
    {
        FL_RMU_SetSoftReset(RMU);
        while(1);
    }

    /* Initialize all configured peripherals */
    MF_Config_Init();
	chip_init();
	iap_done();
	system_read_setting();
	system_config_create_task(NULL,1);						//1K
	oled_create_task(NULL,4);								//2K
	create_task_hist(NULL,7);								//2K
	create_task_pmu(NULL,3);								//1K
	button_create_task(NULL,2);								//0.5K
	server_task_create(NULL,6);								//2K
	sensor_task_create(NULL,5);								//1K
		
	vTaskStartScheduler();
    while(1)
    {     

    }
}


