#include "display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "sys_setting.h"
#include "string.h"
#include "stdio.h"
#include "oled_bmp.h"
#include "oled.h"
#include "sensor.h"
#include "pwr_manage.h"
#include "sd3078.h"
#include "usart.h"
#include "main.h"
#include "pwr_manage.h"

#define BATTERY_X      120
#define BATTERY_Y      0
#define BATTERY_WIDTH  8
#define BATTERY_HEIGHT 12

static show_info_t show_info;
static TimerHandle_t oled_timer[2] = {NULL, NULL};
static uint8_t oled_buff[64];

TaskHandle_t task_oled_handle;

//static void oled_display_spp(void);

/* 主菜单 -----------------------------------------------------------*/
menu_t menu_main = 
{
	0,2,7,0,0,
	{
		(uint8_t *)item_language_cn,
		(uint8_t *)item_alarm_settings_cn,
		(uint8_t *)item_fault_message_cn,
		(uint8_t *)item_sys_settings_cn,
		(uint8_t *)item_clear_record_cn,
		(uint8_t *)item_about_cn,
		(uint8_t *)item_home_page_cn,
	},
	{
		(uint8_t *)item_language_en,
		(uint8_t *)item_alarm_settings_en,
		(uint8_t *)item_fault_message_en,
		(uint8_t *)item_sys_settings_en,
		(uint8_t *)item_clear_record_en,
		(uint8_t *)item_about_en,
		(uint8_t *)item_home_page_en,
	},
	{
		sizeof(item_language_cn),
		sizeof(item_alarm_settings_cn),
		sizeof(item_fault_message_cn),
		sizeof(item_sys_settings_cn),
		sizeof(item_clear_record_cn),
		sizeof(item_about_cn),
		sizeof(item_home_page_cn),
	},
	{
		sizeof(item_language_en),
		sizeof(item_alarm_settings_en),
		sizeof(item_fault_message_en),
		sizeof(item_sys_settings_en),
		sizeof(item_clear_record_en),
		sizeof(item_about_en),
		sizeof(item_home_page_en),
	},
	(uint8_t *)main_menu_cn,
	(uint8_t *)main_menu_en,
	sizeof(main_menu_cn),
	sizeof(main_menu_en),
	NULL,
	NULL,
	NULL
};

/* 报警菜单 -----------------------------------------------------------*/
menu_t alarm_menu = 
{
	0, 1, 2, 0, 1,
	{
		(uint8_t *)item_disable_cn,
		(uint8_t *)item_enable_cn,
	},
	{
		(uint8_t *)item_disable_en,
		(uint8_t *)item_enable_en,
	},
	{
		sizeof(item_disable_cn),
		sizeof(item_enable_cn),
	},
	{
		sizeof(item_disable_en),
		sizeof(item_enable_en),
	},
	(uint8_t *)item_alarm_settings_cn,
	(uint8_t *)item_alarm_settings_en,
	sizeof(item_alarm_settings_cn),
	sizeof(item_alarm_settings_en),
	NULL,
	NULL,
	NULL
};

/* 系统设置菜单 -----------------------------------------------------------*/
menu_t sys_option_menu = 
{
	0, 2, 10, 0, 0,
	{
		(uint8_t *)item_backlight_time_cn,
		(uint8_t *)item_up_time_cn,
		(uint8_t *)item_internal_sensor_cn,
		(uint8_t *)item_external_sensor_cn,
		(uint8_t *)item_external_continuous_power_cn,	
		(uint8_t *)item_sensor_type_cn,
		(uint8_t *)item_temp_unit_option_cn,
		(uint8_t *)item_display_mode_cn,
        (uint8_t *)item_modbus_en,
		(uint8_t *)item_return_cn,
	},
	{
		(uint8_t *)item_backlight_time_en,
		(uint8_t *)item_up_time_en,
		(uint8_t *)item_internal_sensor_en,
		(uint8_t *)item_external_sensor_en,
		(uint8_t *)item_external_continuous_power_en,			
		(uint8_t *)item_sensor_type_en,
		(uint8_t *)item_temp_unit_option_en,
		(uint8_t *)item_display_mode_en,
        (uint8_t *)item_modbus_en,
		(uint8_t *)item_return_en,
	},
	{
		sizeof(item_backlight_time_cn),
		sizeof(item_up_time_cn),
		sizeof(item_internal_sensor_cn),	
		sizeof(item_external_sensor_cn),
		sizeof(item_external_continuous_power_cn),
		sizeof(item_sensor_type_cn),
		sizeof(item_temp_unit_option_cn),
		sizeof(item_display_mode_cn),
        sizeof(item_modbus_en),
		sizeof(item_return_cn),
	},
	{
		sizeof(item_backlight_time_en),
		sizeof(item_up_time_en),
		sizeof(item_internal_sensor_en),	
		sizeof(item_external_sensor_en),
		sizeof(item_external_continuous_power_en),		
		sizeof(item_sensor_type_en),
		sizeof(item_temp_unit_option_en),
		sizeof(item_display_mode_en),
        sizeof(item_modbus_en),
		sizeof(item_return_en),
	},
	(uint8_t *)item_sys_settings_cn,
	(uint8_t *)item_sys_settings_en,
	sizeof(item_sys_settings_cn),
	sizeof(item_sys_settings_en),
	NULL,
	NULL,
	NULL
};

/* 背光设置菜单 -----------------------------------------------------------*/
menu_t back_light_menu = 
{
	0,2,4,0,1,
	{
		(uint8_t *)item_backlight_cn[0],
		(uint8_t *)item_backlight_cn[1],
		(uint8_t *)item_backlight_cn[2],
		(uint8_t *)item_alway_on_cn,
	},
	{
		(uint8_t *)item_backlight_en[0],
		(uint8_t *)item_backlight_en[1],
		(uint8_t *)item_backlight_en[2],
		(uint8_t *)item_alway_on_en,
	},
	{
		sizeof(item_backlight_cn[0]),
		sizeof(item_backlight_cn[1]),
		sizeof(item_backlight_cn[2]),
		sizeof(item_alway_on_cn),
	},
	{
		sizeof(item_backlight_en[0]),
		sizeof(item_backlight_en[1]),
		sizeof(item_backlight_en[2]),
		sizeof(item_alway_on_en),
	},
	(uint8_t *)item_backlight_time_cn,
	(uint8_t *)item_backlight_time_en,
	sizeof(item_backlight_time_cn),
	sizeof(item_backlight_time_en),
	NULL,
	NULL,
	NULL
};

/* 温度单位 -----------------------------------------------------------*/
menu_t temp_unit_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_temp_unit_cn[0],
		(uint8_t *)item_temp_unit_cn[1],
	},
	{
		(uint8_t *)item_temp_unit_celsius_en,
		(uint8_t *)item_temp_unit_fahrenheit_en,
	},
	{
		sizeof(item_temp_unit_cn[0]),
		sizeof(item_temp_unit_cn[1]),
	},
	{
		sizeof(item_temp_unit_celsius_en),
		sizeof(item_temp_unit_fahrenheit_en),
	},
	(uint8_t *)item_temp_unit_option_cn,
	(uint8_t *)item_temp_unit_option_en,
	sizeof(item_temp_unit_option_cn),
	sizeof(item_temp_unit_option_en),
	NULL,
	NULL,
	NULL
};

//内部传感器
menu_t internal_sensor_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_disable_cn,
		(uint8_t *)item_enable_cn,
	},
	{
		(uint8_t *)item_disable_en,
		(uint8_t *)item_enable_en,
	},
	{
		sizeof(item_disable_cn),
		sizeof(item_enable_cn),
	},
	{
		sizeof(item_disable_en),		
		sizeof(item_enable_en),
	},
	(uint8_t *)item_internal_sensor_cn,
	(uint8_t *)item_internal_sensor_en,
	sizeof(item_internal_sensor_cn),
	sizeof(item_internal_sensor_en),
	NULL,
	NULL,
	NULL
};

//外部传感器
menu_t external_sensor_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_disable_cn,
		(uint8_t *)item_enable_cn,
	},
	{
		(uint8_t *)item_disable_en,
		(uint8_t *)item_enable_en,
	},
	{
		sizeof(item_disable_cn),
		sizeof(item_enable_cn),
	},
	{
		sizeof(item_disable_en),		
		sizeof(item_enable_en),
	},
	(uint8_t *)item_external_sensor_cn,
	(uint8_t *)item_external_sensor_en,
	sizeof(item_external_sensor_cn),
	sizeof(item_external_sensor_en),
	NULL,
	NULL,
	NULL
};

//外部常供电
menu_t external_continuous_power_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_disable_cn,
		(uint8_t *)item_enable_cn,
	},
	{
		(uint8_t *)item_disable_en,
		(uint8_t *)item_enable_en,
	},
	{
		sizeof(item_disable_cn),
		sizeof(item_enable_cn),
	},
	{
		sizeof(item_disable_en),		
		sizeof(item_enable_en),
	},
	(uint8_t *)item_external_continuous_power_cn,
	(uint8_t *)item_external_continuous_power_en,
	sizeof(item_external_continuous_power_cn),
	sizeof(item_external_continuous_power_en),
	NULL,
	NULL,
	NULL
};

/* 传感器类型 */
menu_t sensor_type_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_sensor_tp_cn,
		(uint8_t *)item_sensor_ds_cn,
	},
	{
		(uint8_t *)item_sensor_tp_en,
		(uint8_t *)item_sensor_ds_en,
	},
	{
		sizeof(item_sensor_tp_cn),
		sizeof(item_sensor_ds_cn),
	},
	{
		sizeof(item_sensor_tp_en),
		sizeof(item_sensor_ds_en),
	},
	(uint8_t *)item_sensor_type_cn,
	(uint8_t *)item_sensor_type_en,
	sizeof(item_sensor_type_cn),
	sizeof(item_sensor_type_en),
	NULL,
	NULL,
	NULL
};

//语言菜单
menu_t language_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_chinese_cn,		
		(uint8_t *)item_english_cn,
	},
	{
		(uint8_t *)item_chinese_en,
		(uint8_t *)item_english_en,
	},
	{
		sizeof(item_chinese_cn),		
		sizeof(item_english_cn),

	},
	{
		sizeof(item_chinese_en),		
		sizeof(item_english_en),
	},
	(uint8_t *)item_language_cn,
	(uint8_t *)item_language_en,
	sizeof(item_language_cn),
	sizeof(item_language_en),
	NULL,
	NULL,
	NULL
};
/* 启动轮转显示 -----------------------------------------------------------*/
menu_t display_mode_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_manual_cn,
		(uint8_t *)item_auto_mode_cn,
	},
	{
		(uint8_t *)item_manual_en,
		(uint8_t *)item_auto_mode_en,
	},
	{
		sizeof(item_manual_cn),
		sizeof(item_auto_mode_cn),
	},
	{
		sizeof(item_manual_en),
		sizeof(item_auto_mode_en),
	},
	(uint8_t *)item_display_mode_cn,
	(uint8_t *)item_display_mode_en,
	sizeof(item_display_mode_cn),
	sizeof(item_display_mode_en),
	NULL,
	NULL,
	NULL
};

/* 上报时间菜单 -----------------------------------------------------------*/
menu_t upload_interval_menu = 
{
	0,2,5,0,1,
	{
		(uint8_t *)item_up_time_min_cn[0],
		(uint8_t *)item_up_time_min_cn[1],
		(uint8_t *)item_up_time_min_cn[2],
		(uint8_t *)item_up_time_min_cn[3],
		(uint8_t *)item_up_time_min_cn[4],
	},
	{
		(uint8_t *)item_up_time_min_en[0],
		(uint8_t *)item_up_time_min_en[1],
		(uint8_t *)item_up_time_min_en[2],
		(uint8_t *)item_up_time_min_en[3],
		(uint8_t *)item_up_time_min_en[4],
	},
	{
		sizeof(item_up_time_min_cn[0]),
		sizeof(item_up_time_min_cn[1]),
		sizeof(item_up_time_min_cn[2]),
		sizeof(item_up_time_min_cn[3]),
		sizeof(item_up_time_min_cn[4]),
	},
	{
		sizeof(item_up_time_min_en[0]),
		sizeof(item_up_time_min_en[1]),
		sizeof(item_up_time_min_en[2]),
		sizeof(item_up_time_min_en[3]),
		sizeof(item_up_time_min_en[4]),
	},
	(uint8_t *)item_up_time_cn,
	(uint8_t *)item_up_time_en,
	sizeof(item_up_time_cn),
	sizeof(item_up_time_en),
	NULL,
	NULL,
	NULL
};

//MODBUS开关
menu_t modbus_en_menu = 
{
	0, 1, 2, 0 ,1,
	{
		(uint8_t *)item_disable_cn,
		(uint8_t *)item_enable_cn,
	},
	{
		(uint8_t *)item_disable_en,
		(uint8_t *)item_enable_en,
	},
	{
		sizeof(item_disable_cn),
		sizeof(item_enable_cn),
	},
	{
		sizeof(item_disable_en),		
		sizeof(item_enable_en),
	},
	(uint8_t *)item_modbus_en,
	(uint8_t *)item_modbus_en,
	sizeof(item_modbus_en),
	sizeof(item_modbus_en),
	NULL,
	NULL,
	NULL
};

/* 清除历史数据菜单 */
menu_t clear_read_menu = 
{
	0,1,2,0,0,
	{
		(uint8_t *)item_clear_record_cn,
		(uint8_t *)item_return_cn,
	},
	{
		(uint8_t *)item_clear_record_en,
		(uint8_t *)item_return_en,
	},
	{
		sizeof(item_clear_record_cn),
		sizeof(item_return_cn),
	},
	{
		sizeof(item_clear_record_en),
		sizeof(item_return_en),
	},
	(uint8_t *)item_clear_record_cn,
	(uint8_t *)item_clear_record_en,
	sizeof(item_clear_record_cn),
	sizeof(item_clear_record_en),
	NULL,
	NULL,
	NULL
};

menu_t *current_menu;

void oled_ui_battery(void)
{
    uint32_t battery_voltage;

	battery_voltage = config_get_vbat_value();

    if(battery_voltage < 3600)
    {
       oled_display_bmp(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT, (uint8_t *)icon_electricity_25, 0); 
    }
	else if(battery_voltage < 3700)
	{
		oled_display_bmp(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT, (uint8_t *)icon_electricity_50, 0);
	}
	else if(battery_voltage < 3800)
	{
		oled_display_bmp(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT, (uint8_t *)icon_electricity_75, 0);
	}
	else
	{
		oled_display_bmp(BATTERY_X, BATTERY_Y, BATTERY_WIDTH, BATTERY_HEIGHT, (uint8_t *)icon_electricity_100, 0);
	}

	//充电标志
    if(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7))
	{
		oled_display_bmp(112, 0, 8 ,12, (uint8_t *)icon_charging, 0);
	}
}


void oled_display_time(void)
{
	sprintf((char *)oled_buff,"%02d-%02d-%02d     %02d:%02d:%02d",
			real_time.year,
			real_time.month,
			real_time.day,
			real_time.hour, 
			real_time.min,
			real_time.sec);
	oled_display_string(2, 52, oled_buff, 12, 1); 
}

void oled_ui_gprs(void)
{
	uint8_t icon_index = 128 - 9 - 12;
	if(config_get_trans_model() == 0)
	{
		if(config_get_signal() == 0)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_none, 0);
		}
		else if(config_get_signal() < 12)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_weak, 0);
		}
		else if(config_get_signal() < 20)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_medium, 0);
		}
		else
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_good, 0);
		}
		if(config_get_signal() != 0)
		{
			oled_display_string(12, 0 , (uint8_t *)"LTE", 12, 1);
		}
	}
	else 
	{	
		if(config_get_signal() == 0)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_none, 0);
		}
		else if(config_get_signal() < 12)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_weak, 0);
		}
		else if(config_get_signal() < 20)
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_medium, 0);
		}
		else
		{
			oled_display_bmp(0, 0, 8, 12, (uint8_t *)icon_signal_good, 0);
		}
		if(config_get_signal() != 0)
		{
			oled_display_string(12, 0 , (uint8_t *)"wifi", 12, 1);
		}
	}

	if(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7))
	{
		icon_index -= 8;
	}
	
	if(config_get_gps_mode()&&(wireless_info.lat != 0 && wireless_info.lng != 0))
//	if(wireless_info.lat != 0 && wireless_info.lng != 0)
	{
		oled_display_bmp(icon_index, 0, 13, 12, (uint8_t *)icon_gps, 0);
		icon_index -= 12;
	}
}

void oled_display_ui(void)
{
	oled_ui_gprs();
	oled_ui_battery();
}

static void oled_display_item(menu_t *p_menu)
{
	uint8_t i = 0;
	int x,y;
	x = 0;
	y = 13;
	
	oled_display_clear();
	oled_display_line(0, 12, 128, 12, 1);
	oled_display_line(0, 51, 128, 51, 1);
	
	if(config_get_language() == CHINESE)
	{
		oled_display_bmp(0, 52, sizeof(button_next_cn) /2, 12, (uint8_t *)button_next_cn, 0);
        if(p_menu == &modbus_en_menu)
        {
          oled_display_string(64 - strlen((char *)p_menu->en_menu_bar) * 3, 0, p_menu->en_menu_bar, 12, 1);  
        }
        else
        {
            oled_display_bmp(64 - p_menu->bar_width_cn / 4, 0 ,p_menu->bar_width_cn / 2, 12, p_menu->cn_menu_bar, 0);
        }
	}
	else
	{
		oled_display_string(0, 52, button_next_en, 12, 1);
		oled_display_string(64 - strlen((char *)p_menu->en_menu_bar) * 3, 0, p_menu->en_menu_bar, 12, 1);
	}
	
	if(p_menu->is_button != 0)
	{
		x += 12;
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(128 - sizeof(button_confirm_and_return_cn) / 2, 52, sizeof(button_confirm_and_return_cn) /2, 12, (uint8_t *)button_confirm_and_return_cn, 0);
		}
		else 
		{
			oled_display_string(128 - strlen((char *)button_confirm_en) * 6, 52, button_confirm_en, 12, 1);
		}
	}
	else 
	{
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(128 - sizeof(button_confirm_cn)/2, 52, sizeof(button_confirm_cn) / 2, 12, (uint8_t *)button_confirm_cn, 0);
		}
		else 
		{
			oled_display_string(128 - strlen((char *)button_confirm_en) * 6, 52, button_confirm_en, 12, 1);
		}
	}

	/* 三组列表 */
	for(i = p_menu->range_from; i <= p_menu->range_to; i++)
	{
		if(config_get_language() == CHINESE)
		{
            if(i == 8)
            {
               oled_display_string(x, y, p_menu->item_code_en[i], 12, i == p_menu->select_item ? 0 : 1); 
            }
            else
            {
                oled_display_bmp(x, y, p_menu->item_width_cn[i] / 2, 
                12, p_menu->item_code_cn[i], i == p_menu->select_item ?1 : 0);
            }
		}
		else 
		{
			oled_display_string(x, y, p_menu->item_code_en[i], 12, i == p_menu->select_item ? 0 : 1);
		}
		if(p_menu->is_button != 0)
		{
			if(p_menu->is_button -1 == i)
			{
				oled_display_bmp(0, y, 12, 12, (uint8_t *)icon_reward, i == p_menu->select_item ?1 : 0);
			}
			else 
			{
				oled_display_bmp(0, y, 12, 12, (uint8_t *)icon_no_select, i == p_menu->select_item ?1 : 0);
			}
		}
		if(i == p_menu->select_item)
		{
			if(config_get_language() == CHINESE)
			{
                if(i == 8)
                {
                    oled_fill(x + p_menu->item_width_en[i] * 6 - 6, y, 128, y + 11, 1);
                }
                else
                {
				    oled_fill(x + p_menu->item_width_cn[i] / 2, y, 128, y + 11, 1);
                }
			}
			else 
			{
				oled_fill(x + p_menu->item_width_en[i] * 6 - 6, y, 128, y + 11, 1);
			}
		}
		y += 13;
	}
	oled_refresh_gram();
}

static void show_ok(void)
{
	oled_display_clear();
	oled_display_bmp(0, 12, 128, 64, (uint8_t *)icon_ok, 0);
	oled_refresh_gram();
	vTaskDelay(300);
}

void alarm_closed_func(void)
{
	config_set_alarm_en(0);
	alarm_menu.select_item = 0;
	alarm_menu.is_button = 1;
	current_menu = current_menu->parent;
}

void alarm_open_func(void)
{
	config_set_alarm_en(1);
	alarm_menu.select_item = 0;
	alarm_menu.is_button = 2;
	current_menu = current_menu->parent;
}

void dialog_back_light(void)
{
	config_set_back_light_time(current_menu->select_item);
	current_menu->is_button     = current_menu->select_item + 1;
	current_menu->select_item   = 0;
	current_menu->range_from    = 0;
	current_menu->range_to      = 2;
	current_menu = current_menu->parent;
}

void dialog_temp_unit(void)
{
	config_set_tem_unit(current_menu->select_item);
	current_menu->is_button    = current_menu->select_item + 1;
	current_menu->select_item  = 0;
	current_menu->range_from   = 0;
	current_menu->range_to     = 1;
	current_menu = current_menu->parent;
}

void dialog_display_mode(void)
{
	config_set_auto_switch_show(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_internal_sensor(void)
{
	config_set_internal_en(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_external_sensor(void)
{
	config_set_external_en(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_external_long_power(void)
{
	config_set_external_long_power(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_sensor_type(void)
{
	config_set_external_model(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_modbus_en(void)
{
    if(config_get_net_mode() == 0)
    {
        config_set_modbus_en(current_menu->select_item);
    }
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_language(void)
{
	config_set_language(current_menu->select_item);
	current_menu->select_item = 0;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_upload_time(void)
{
    uint16_t load_time = 0;
    switch (current_menu->select_item)
    {
        case 0:
            load_time = 1;
            break;
        case 1:
            load_time = 2;
            break;
        case 2:
            load_time = 5;
            break;
        case 3:
            load_time = 30;
            break;
        case 4:
            load_time = 60;
            break;                                    
        default:
            break;
    }
	config_set_updata_time(load_time);
	current_menu->select_item = 0;
	current_menu->range_from  = 0;
	current_menu->range_to    = 2;
	current_menu->is_button   = 1;
	current_menu = current_menu->parent;
}

void dialog_back(void)
{
	current_menu->select_item = 0;
	current_menu->range_from  = 0;
	if(current_menu->item_cnt >= 3)
	{
		current_menu->range_to = 2;
	}
	else 
	{
		current_menu->range_to = 1;
	}
	current_menu = current_menu->parent;
}


void low_power_dialog(void)
{
	//static int fire = 0;
    uint16_t wait_sec = 6;
	/*if(fire == 1)
	{
		return;
	}*/
    while(!(FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7)))
    {
		oled_display_clear();	
		sprintf((char *)oled_buff, "%01dS", wait_sec);
		
		/* menu bar */
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(0, 18 ,sizeof(low_pow_waring_cn)/ 2, 12, (uint8_t *)low_pow_waring_cn, 0);
			oled_display_bmp(0, 30 ,sizeof(batt_waring_cn)/ 2, 12, (uint8_t *)batt_waring_cn, 0);
		}
		else
		{
			oled_display_string(0, 18, (uint8_t *)low_pow_waring_en, 12, 1);
		}

		oled_display_string(110, 52, oled_buff, 12, 1);
		oled_refresh_gram();

		if(--wait_sec == 0)
		{
			//fire = 1;
			//trace_log("auto:", "off it");
//			pwr_arg_halt(PWR_OLED_BIT);
			trace_log("auto power down\r\n");
			pwr_off = 1;
//			task_sleep_post_event(PWR_EVENT_OFF);
			break;	
		}
		vTaskDelay(1000);
    }
}

void net_err_dialog(void)
{
    uint32_t event_bit = 0;
    if(system_get_net_err() == 0x00)
    {
        return;
    }
    else 
    {
        while(1)
        {
			oled_display_clear();	
			oled_display_line(0, 12, 128, 12, 1);
			oled_display_line(0, 51, 128, 51, 1);

            if((event_bit & CONFIRM_TOUCH) == CONFIRM_TOUCH)
            {
                system_clear_net_err(0xffffffff);
            }
            if(system_get_net_err() == 0x00)
            {
                break;
            }
			
            if((event_bit & EXIT_SHOW) == EXIT_SHOW)
            {
				trace_log("oled off\r\n");
				//trace_log("OLED", "OFF");
                task_oled_post_event(EXIT_SHOW);
                break;
            }

			if(config_get_language() == CHINESE)
			{
				oled_display_bmp(64 - sizeof(item_fault_message_cn)/4, 0 ,sizeof(item_fault_message_cn) / 2, 12, (uint8_t *)item_fault_message_cn, 0);

				if((system_get_net_err() & err_net) == err_net)
				{
					oled_display_bmp(0, 26 ,sizeof(wireless_net_err_cn)/ 2, 12, (uint8_t *)wireless_net_err_cn, 0);
				}
				else if(system_get_net_err() == err_srv)
				{
					oled_display_bmp(0, 26 ,sizeof(wireless_conn_err_cn)/ 2, 12, (uint8_t *)wireless_conn_err_cn, 0);
				}

				oled_display_bmp(128 - sizeof(item_return_cn)/2, 52, sizeof(item_return_cn) /2, 12, (uint8_t *)item_return_cn, 0);
			}
			else
			{
				oled_display_string(64 - strlen((char *)item_fault_message_en) * 3, 0 ,(uint8_t *)item_fault_message_en, 12, 1);

				if((system_get_net_err() & err_net) == err_net)
				{
					oled_display_string(0, 26, (uint8_t *)wireless_net_err_en, 12, 1);
				}
				else if(system_get_net_err() == err_srv)
				{
					oled_display_string(0, 26, (uint8_t *)wireless_conn_err_en, 12, 1);
				}

				oled_display_string(128 - strlen((char *)item_return_en) * 6, 52, (uint8_t *)item_return_en, 12, 1);
			}
			oled_refresh_gram();
			xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, 1000);
        }
    }
}

void dialog_clear_record(void)
{
   task_flash_post_event(HIST_CLEAR);
   show_ok();
   current_menu = current_menu->parent;
}

void dialog_power_down(void)
{
    uint32_t event_bit = 0;
    uint8_t wait_sec = 6;
    uint8_t temp_buf[8];
    oled_display_clear();
    while(1)
    {
        if((event_bit & CONFIRM_TOUCH) == CONFIRM_TOUCH)
		{
//			pwr_arg_halt(PWR_OLED_BIT);
			trace_log("oled off\r\n");
			pwr_off = 1;
//			task_sleep_post_event(PWR_EVENT_OFF);
			break;			
		}
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(24, 24, sizeof(power_down_cn)/ 2, 16, (uint8_t *)power_down_cn, 0);
			oled_display_bmp(128 - sizeof(button_confirm_cn)/2, 52, sizeof(button_confirm_cn) /2, 12, (uint8_t *)button_confirm_cn, 0);
		}
		else 
		{
			oled_display_string(24, 24, (uint8_t *)power_down_en, 12, 1);
			oled_display_string(128 - strlen((char *)button_confirm_en) * 6, 52, button_confirm_en, 12, 1);
		}
		sprintf((char *)temp_buf, "%01dS", wait_sec);
		oled_display_string(0, 52, temp_buf, 12, 1);
		if(wait_sec-- == 0)
		{
			break;
		}
		oled_refresh_gram();
		xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, 1000);
	}
}

void dialog_about(void)
{
	//uint8_t sn[16];
	uint32_t event_bit = 0;
	while(1)
	{
		if((event_bit & CONFIRM_TOUCH) == CONFIRM_TOUCH)
        {
            break;
        }
        if((event_bit & MENU_TOUCH) == MENU_TOUCH)
        {
            task_oled_post_event(MENU_TOUCH);
            break;
        }
        
		oled_display_clear();	
		
		oled_display_line(0, 12, 128, 12, 1);
		
		oled_display_string(5, 14, (uint8_t *)"Edge Meter", 12, 1);
		oled_display_string(5, 26, (uint8_t *)"SN:", 12, 1);
        if(strlen((char*)config_get_mqtt_post_topic()) < 10)
        {
            oled_display_string(23, 26, config_get_mqtt_post_topic(), 12, 1);
        }
        else
        {
            oled_display_string(23, 26, config_get_sn(), 12, 1);
        }
		oled_display_string(5, 38,  (uint8_t *)"Ver:4.5.0", 12, 1);
		
        /* option bar */
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(64 - sizeof(item_about_cn) / 4, 0 ,sizeof(item_about_cn) / 2, 12, (uint8_t *)item_about_cn, 0);
			oled_display_bmp(128 - sizeof(item_return_cn) / 2, 52, sizeof(item_return_cn) /2, 12, (uint8_t *)item_return_cn, 0);
		}
		else 
		{
			oled_display_string(64 - strlen((char *)item_about_en) * 3, 0, (uint8_t *)item_about_en, 12, 1);
			oled_display_string(128 - strlen((char *)item_return_en) * 6, 52, (uint8_t *)item_return_en, 12, 1);
		}
		oled_refresh_gram();
		xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, portMAX_DELAY);
    }
}

void err_dialog(void)
{
	uint32_t event_bit = 0;
	uint8_t show_y = 14;
	while(1)
	{
		if((event_bit & CONFIRM_TOUCH) == CONFIRM_TOUCH)
		{
			break;
		}
		if((event_bit & MENU_TOUCH) == MENU_TOUCH)
		{
			task_oled_post_event(MENU_TOUCH);
			break;
		}
		oled_display_clear();	
		oled_display_line(0, 12, 128, 12, 1);

		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(64 - sizeof(item_fault_message_cn)/4, 0 ,sizeof(item_fault_message_cn)/ 2, 12, (uint8_t *)item_fault_message_cn, 0);

			if(config_get_signal() < 12)
			{
				oled_display_bmp(5, show_y, sizeof(signal_fail_cn)/ 2, 12, (uint8_t *)signal_fail_cn, 0);
				show_y += 12;
			}
			oled_display_bmp(128 - sizeof(item_return_cn)/2, 52, sizeof(item_return_cn) /2, 12, (uint8_t *)item_return_cn, 0);
		}
		else 
		{
			oled_display_string(64 - strlen((char *)item_fault_message_en) * 3, 0, (uint8_t *)item_fault_message_en, 12, 1);
			
			if(config_get_signal() < 12)
			{
				oled_display_string(5, show_y, (uint8_t *)signal_fail_en, 12, 1);
				show_y += 12;
			}
			oled_display_string(128 - strlen((char *)item_return_en) * 6, 52, (uint8_t *)item_return_en, 12, 1);
		}

		oled_refresh_gram();
		xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, portMAX_DELAY);
    }
}

void dialog_exit_menu(void)
{
	menu_main.range_from = 0;
	menu_main.range_to = 2;
	menu_main.select_item = 0;
    
    task_oled_post_event(MENU_TOUCH);
}

void oled_display_value()
{
	float value[2]      = {0, 0};
	uint8_t type_x      = 0;
	uint8_t icon_x[2]   = {38, 80};
	uint8_t show_type   = 1;	
    uint8_t sen_type[2] = {TYPE_TEMP, TYPE_TEMP};
	uint8_t sensor_link[2] = {sensor_err, sensor_err};

	type_x = 10;

	if(config_get_signal() != 0)
	{
		type_x += 26;
	}

	if(config_get_internal_en() == 1)
	{
		
		if(show_info.show_page == 0)  /* 内部温湿度 */
		{
			show_type = 0;
			value[0] = sensor_get_value(SENSOR_NUM_1);
			value[1] = sensor_get_value(SENSOR_NUM_2);
			sen_type[0] = sensor_get_type(SENSOR_NUM_1);
			sen_type[1] = sensor_get_type(SENSOR_NUM_2);
			sensor_link[0] = sensor_is_link(SENSOR_NUM_1);
			sensor_link[1] = sensor_is_link(SENSOR_NUM_2);
		}
		else if(show_info.show_page == 1)  /* 外部传感器 */
		{
			if(config_get_external_en() == 0)
			{
				show_type = 0;
				value[0] = sensor_get_value(SENSOR_NUM_1);
				value[1] = sensor_get_value(SENSOR_NUM_2);
				sen_type[0] = sensor_get_type(SENSOR_NUM_1);
				sen_type[1] = sensor_get_type(SENSOR_NUM_2);
				sensor_link[0] = sensor_is_link(SENSOR_NUM_1);
				sensor_link[1] = sensor_is_link(SENSOR_NUM_2);
			}
			else
			{
				value[0] = sensor_get_value(SENSOR_NUM_3);
				sen_type[0] = sensor_get_type(SENSOR_NUM_3);
				sensor_link[0] = sensor_is_link(SENSOR_NUM_3);
				if(sen_type[0] < TYPE_HCHO)  /* double显示 */
				{
					show_type = 0;
					value[1] = sensor_get_value(SENSOR_NUM_4);
					sen_type[1] = sensor_get_type(SENSOR_NUM_4);
					sensor_link[1] = sensor_is_link(SENSOR_NUM_4);
				}			
			}
		}
		else  /* 单个显示气体 */
		{	
			value[0] = sensor_get_value(SENSOR_NUM_4);
			sen_type[0] = sensor_get_type(SENSOR_NUM_4);
			sensor_link[0] = sensor_is_link(SENSOR_NUM_4);
		}
	}
	else
	{
		if(show_info.show_page == 0)
		{
			value[0] = sensor_get_value(SENSOR_NUM_1);
			sen_type[0] = sensor_get_type(SENSOR_NUM_1);
			sensor_link[0] = sensor_is_link(SENSOR_NUM_1);
			if(sen_type[0] < TYPE_HCHO)  /* double显示 */
			{
				show_type = 0;
				value[1] = sensor_get_value(SENSOR_NUM_2);
				sen_type[1] = sensor_get_type(SENSOR_NUM_2);
				sensor_link[1] = sensor_is_link(SENSOR_NUM_2);
			}	
		}
		else if(show_info.show_page == 1)
		{
			if(sensor_is_link(SENSOR_NUM_1) < TYPE_HCHO)  /* 1页显示了两组 */
			{
				value[0] = sensor_get_value(SENSOR_NUM_3);
				sen_type[0] = sensor_get_type(SENSOR_NUM_3);
				sensor_link[0] = sensor_is_link(SENSOR_NUM_3);
				if(sen_type[0] < TYPE_HCHO)  /* double显示 */
				{
					show_type = 0;
					value[1] = sensor_get_value(SENSOR_NUM_4);
					sen_type[1] = sensor_get_type(SENSOR_NUM_4);
					sensor_link[1] = sensor_is_link(SENSOR_NUM_4);
				}
			}
			else
			{
				value[0] = sensor_get_value(SENSOR_NUM_2);
				sen_type[0] = sensor_get_type(SENSOR_NUM_2);
				sensor_link[0] = sensor_is_link(SENSOR_NUM_2);
			}
		}
	}

    if(config_get_internal_en() == 0)
    {
		if(config_get_language() == CHINESE)
		{
			oled_display_bmp(type_x, 0, 12, 12, (uint8_t *)icon_in_ex_cn[1], 0);
		}
		else
		{
			oled_display_string(type_x, 0, (uint8_t *)icon_in_ex_en[1], 12, 1);
		}
		if(show_info.show_page == 0)
		{
			oled_show_char(type_x + 12, 0, '1', 12, 1);
		}
		else 
		{
			oled_show_char(type_x + 12, 0, '2', 12, 1);
		}
	}
    else
    {
		if(config_get_language() == CHINESE)
		{
			if(config_get_external_en() == 1)
			{
				oled_display_bmp(type_x, 0, 12, 12, (uint8_t *)icon_in_ex_cn[show_info.show_page], 0);
			}
			else
			{
				oled_display_bmp(type_x, 0, 12, 12, (uint8_t *)icon_in_ex_cn[0], 0);
			}	
		}
		else 
		{
			if(config_get_external_en() == 1)
			{
				oled_display_string(type_x, 0, (uint8_t *)icon_in_ex_en[show_info.show_page], 12, 1);
			}
			else
			{
				oled_display_string(type_x, 0, (uint8_t *)icon_in_ex_en[0], 12, 1);
			}						
		}
    }

	/* NaN/超范围数据不显示:固定坐标布局下大数会叠字越界,统一按未连接处理显示"---" */
	if((!(value[0] == value[0])) || (sen_type[0] < TYPE_HCHO && (value[0] > 999.0f || value[0] < -999.0f)))
	{
		sensor_link[0] = sensor_err;
	}
	if((!(value[1] == value[1])) || (sen_type[1] < TYPE_HCHO && (value[1] > 999.0f || value[1] < -999.0f)))
	{
		sensor_link[1] = sensor_err;
	}

	if(show_type == 0)
	{
		if(sensor_link[0] != sensor_err)
		{
			if(value[0] < 0 && value[0] > -1)
			{
				sprintf((char *)oled_buff, "-0.");
			}
			else 
			{
				sprintf((char *)oled_buff, "%d.", (int)value[0]);
			}
			
			if(value[0] >= 100 || value[0] <= -10)
			{
				icon_x[0] += 12;
			}
			if(value[0] < 10 && value[0] >= 0) /* 显示为个位数的时候 */
			{
				oled_display_string(16, 22, oled_buff, 24, 1);
			}
			else 
				oled_display_string(4, 22, oled_buff, 24, 1);
			
			
			value[0] = (int)(value[0] * 100) % 100;
			if(value[0] < 0)
			{
				value[0] *= -1;
			}
			sprintf((char *)oled_buff, "%02d", (int)value[0]);
			oled_display_string(icon_x[0], 32, oled_buff, 12, 1);
		}
		else
		{
			sprintf((char *)oled_buff,"---");
			oled_display_string(4, 22, oled_buff, 24, 1);
		}
		
		if(sensor_link[1] != sensor_err)
		{
			if(value[1] >= 100 || value[1] <= -10)
			{
				icon_x[1] -= 12;
			}
			else if(value[1] < 10 && value[1] >= 0)
			{
				icon_x[1] += 12;
			}
			if(value[1] < 0 && value[1] > -1)
			{
				sprintf((char *)oled_buff, "-0.");
			}
			else 
			{
				sprintf((char *)oled_buff, "%d.", (int)value[1]);
			}
			oled_display_string(icon_x[1], 22, oled_buff, 24, 1);
			
			value[1] = (int)(value[1] * 100) %100;
			if(value[1] < 0)
			{
				value[1] *= -1;
			}
			sprintf((char *)oled_buff,"%02d", (int)value[1]);
			oled_display_string(116, 32, oled_buff, 12, 1);
		}
		else
		{
			sprintf((char *)oled_buff,"---");
			oled_display_string(icon_x[1], 22, oled_buff, 24, 1);
		}

		if(sen_type[0] == TYPE_TEMP)  /* 第一个图标 */
		{
			oled_display_bmp(icon_x[0] + 4, 22, 8, 12, (uint8_t *)&icon_temperature[config_get_tem_unit()], 1);
		}
		else if(sen_type[0] == TYPE_HUMI || sen_type[0] == TYPE_PERSEN )  /*CO2/HUMI*/
		{
			oled_display_string(icon_x[0] + 4, 22, (uint8_t *)"%", 12, 1);
		}
		else if(sen_type[0] == TYPE_AIR_PRESSURE) //大气压kpa
		{
			oled_display_string(icon_x[0], 22, (uint8_t *)"Kpa", 12, 1);
		}
		if(sen_type[1] == TYPE_HUMI || sen_type[0] == TYPE_PERSEN)
		{
			oled_display_string(120 ,22, (uint8_t *)"%", 12, 1);
		}
		else if(sen_type[1] == TYPE_TEMP)
		{
			oled_display_bmp(120, 22, 8, 12, (uint8_t *)&icon_temperature[config_get_tem_unit()], 1);
		}
		else if(sen_type[1] == TYPE_AIR_PRESSURE) //大气压kpa
		{
			oled_display_string(128 - 18, 22, (uint8_t *)"Kpa", 12, 1);
		}	
	}
	else
	{
		if(sensor_link[0] == sensor_ok)
		{
			//甲醛的是ppb 氨气的是ppm 二氧化碳的也是ppm
			if(sen_type[0] == TYPE_HCHO)
			{
				oled_display_string(0, 22, (uint8_t *)"HCHO:", 24, 1);
				sprintf((char *)oled_buff, "%04d", (uint32_t)value[0]);
				oled_display_string(128 - 18 - 50, 24, oled_buff, 24, 1);
				oled_display_string(128 - 18, 32, (uint8_t *)"ppb", 12, 1);
			}
			else if(sen_type[0] == TYPE_NH3)
			{
				oled_display_string(4, 22, (uint8_t *)"NH", 24, 1);
				oled_display_string(34, 22, (uint8_t *)":", 24, 1);
				oled_display_string(30, 32, (uint8_t *)"3", 12, 1);
				sprintf((char *)oled_buff, "%3.1f", value[0]);
				oled_display_string(128 - 20 - 36 - 36, 24 ,oled_buff, 24, 1);
				oled_display_string(128 - 18, 32, (uint8_t *)"ppm", 12, 1);
			}
			else if(sen_type[0] == TYPE_TVOC)
			{
				//oled_display_string(0, 22, (uint8_t *)"TVOC:", 24);
				sprintf((char *)oled_buff, "%0.2f", value[0]);
				oled_display_string(90 - strlen((char *)oled_buff) * 12, 24, oled_buff, 24, 1);
				oled_display_string(128 - 30, 20, (uint8_t *)"TVOC", 12, 1);
				oled_display_string(128 - 30, 32, (uint8_t *)"mg/m3", 12, 1);
			}
			else if(sen_type[0] == TYPE_NOISE)
			{
				sprintf((char *)oled_buff, "%0.1f", value[0]);
				oled_display_string(90 - strlen((char *)oled_buff) * 12, 24, oled_buff, 24, 1);
				oled_display_string(128 - 30, 20, (uint8_t *)"NOISE", 12, 1);
				oled_display_string(128 - 30, 32, (uint8_t *)"dBA", 12, 1);
			}
			else if(sen_type[0] == TYPE_NH3_1)
			{
				sprintf((char *)oled_buff, "%0.1f", value[0]);
				oled_display_string(90 - strlen((char *)oled_buff) * 12, 24, oled_buff, 24, 1);
				oled_display_string(128 - 30, 20, (uint8_t *)"NH3", 12, 1);
				oled_display_string(128 - 18, 32, (uint8_t *)"ppm", 12, 1);
			}
			else if(sen_type[0] == TYPE_PM)
			{
				sprintf((char *)oled_buff, "%0.2f", value[0]);
				oled_display_string(90 - strlen((char *)oled_buff) * 12, 24, oled_buff, 24, 1);
				oled_display_string(128 - 30, 20, (uint8_t *)"PM2.5", 12, 1);
				oled_display_string(128 - 30, 32, (uint8_t *)"ug/m3", 12, 1);
			}
			else
			{

				oled_display_string(4, 22, (uint8_t *)"CO", 24, 1);
				oled_display_string(34, 22, (uint8_t *)":", 24, 1);
				oled_display_string(30, 32, (uint8_t *)"2", 12, 1);

				sprintf((char *)oled_buff, "%04d", (uint32_t)value[0]);
				oled_display_string(128 - 20 - 48, 24, oled_buff, 24, 1);
				oled_display_string(128 - 18, 32, (uint8_t *)"ppm", 12, 1);
			}
		}
		else
		{
			oled_display_string(46, 22, (uint8_t *)"---", 24, 1);
		}
	}
    oled_refresh_gram();
}

void oled_display_main(void)
{
    oled_display_clear();
    oled_display_ui();

    if((show_info.sec % 3) == 0)
    {
		if(is_flash_finish())
		{
			task_post_event_sensor(SENSOR_EVENT_GET_VALUE);			
		}
    }
    if(show_info.sec <= 3)
    {
        if(config_get_auto_switch_show())
        {
            show_info.show_page = PAGE_1;
        }
		oled_display_time();		
    }
    else 
    {
        if(config_get_auto_switch_show())
        {
            show_info.show_page = PAGE_2;
        }
        if(show_info.sec >= 6)
        {
            show_info.sec = 0;
        }
		//sprintf((char *)oled_buff,"LOG:%06d", 2000);

		sprintf((char *)oled_buff,"LOG:%06d", device_get_record_num());
		oled_display_string(2, 52, oled_buff, 12, 1);		
		
		//sprintf((char *)oled_buff,"%06d", system_get_prt_num());
		sprintf((char *)oled_buff,"%06d", 0);
		oled_display_string(92, 52, oled_buff, 12, 1);
    }
    oled_display_value();
}

void oled_main_menu_init(void)
{
	menu_main.sub_menu = pvPortMalloc(sizeof(struct menu) * menu_main.item_cnt);
	memset(menu_main.sub_menu, 0, sizeof(struct menu) * menu_main.item_cnt);
	menu_main.sub_menu[0] = &language_menu;
	menu_main.sub_menu[1] = &alarm_menu;
	menu_main.sub_menu[3] = &sys_option_menu;
	menu_main.sub_menu[4] = &clear_read_menu;

	menu_main.func = pvPortMalloc(sizeof(void *) * menu_main.item_cnt);
	memset(menu_main.func, 0, sizeof(void *) * menu_main.item_cnt);
	menu_main.func[2] = &err_dialog;
	menu_main.func[5] = &dialog_about;  
	menu_main.func[6] = &dialog_exit_menu;
	
	current_menu = &menu_main;
	
	//清除历史数据
	{
		clear_read_menu.parent = &menu_main;
		clear_read_menu.func = pvPortMalloc(sizeof(void *) * clear_read_menu.item_cnt);
		memset(clear_read_menu.func, 0, sizeof(void *) * clear_read_menu.item_cnt);
		clear_read_menu.func[0] = &dialog_clear_record;
		clear_read_menu.func[1] = &dialog_back;
	}

	/* 报警设置menu初始化 */
	{
		alarm_menu.parent = &menu_main;
		alarm_menu.func = pvPortMalloc(sizeof(void *) * alarm_menu.item_cnt);
		memset(alarm_menu.func, 0, sizeof(void *) * alarm_menu.item_cnt);
		alarm_menu.func[0] = &alarm_closed_func;
		alarm_menu.func[1] = &alarm_open_func;
	}
	
	/* 系统设置menu初始化 */
	{
		sys_option_menu.parent   = &menu_main;
		sys_option_menu.sub_menu = pvPortMalloc(sizeof(struct menu) * sys_option_menu.item_cnt);
		memset(sys_option_menu.sub_menu, 0, sizeof(void *) * sys_option_menu.item_cnt);
		sys_option_menu.sub_menu[0] = &back_light_menu;
		sys_option_menu.sub_menu[1] = &upload_interval_menu;
		sys_option_menu.sub_menu[2] = &internal_sensor_menu;
		sys_option_menu.sub_menu[3] = &external_sensor_menu;
		sys_option_menu.sub_menu[4] = &external_continuous_power_menu;	
		sys_option_menu.sub_menu[5] = &sensor_type_menu;
		sys_option_menu.sub_menu[6] = &temp_unit_menu;
		sys_option_menu.sub_menu[7] = &display_mode_menu;
        sys_option_menu.sub_menu[8] = &modbus_en_menu;
		
		sys_option_menu.func = pvPortMalloc(sizeof(void *) * sys_option_menu.item_cnt);
		memset(sys_option_menu.func, 0, sizeof(void *) * sys_option_menu.item_cnt);
		sys_option_menu.func[9] = &dialog_back;
	}
	
	/* 背光设置 */
	{
		back_light_menu.parent  = &sys_option_menu;
		back_light_menu.func    = pvPortMalloc(sizeof(void *) * back_light_menu.item_cnt);
		memset(back_light_menu.func, 0, sizeof(void *) * back_light_menu.item_cnt);
		back_light_menu.func[0] = &dialog_back_light;
		back_light_menu.func[1] = &dialog_back_light;
		back_light_menu.func[2] = &dialog_back_light;
		back_light_menu.func[3] = &dialog_back_light;
	}
	{
		upload_interval_menu.parent = &sys_option_menu;
		upload_interval_menu.func = pvPortMalloc(sizeof(void *) * upload_interval_menu.item_cnt);
		memset(upload_interval_menu.func, 0, sizeof(void *) * upload_interval_menu.item_cnt);
		upload_interval_menu.func[0] = &dialog_upload_time;
		upload_interval_menu.func[1] = &dialog_upload_time;
		upload_interval_menu.func[2] = &dialog_upload_time;
		upload_interval_menu.func[3] = &dialog_upload_time;
		upload_interval_menu.func[4] = &dialog_upload_time;
	}
	{
		temp_unit_menu.parent = &sys_option_menu;
		temp_unit_menu.func = pvPortMalloc(sizeof(void *) * temp_unit_menu.item_cnt);
		memset(temp_unit_menu.func, 0, sizeof(void *) * temp_unit_menu.item_cnt);
		temp_unit_menu.func[0] = &dialog_temp_unit;
		temp_unit_menu.func[1] = &dialog_temp_unit;
	}
	{
		display_mode_menu.parent = &sys_option_menu;
		display_mode_menu.func = pvPortMalloc(sizeof(void *) * display_mode_menu.item_cnt);
		memset(display_mode_menu.func, 0, sizeof(void *) * display_mode_menu.item_cnt);
		display_mode_menu.func[0] = &dialog_display_mode;
		display_mode_menu.func[1] = &dialog_display_mode;
	}
	{
		internal_sensor_menu.parent = &sys_option_menu;
		internal_sensor_menu.func = pvPortMalloc(sizeof(void *) * internal_sensor_menu.item_cnt);
		memset(internal_sensor_menu.func, 0, sizeof(void *) * internal_sensor_menu.item_cnt);
		internal_sensor_menu.func[0] = &dialog_internal_sensor;
		internal_sensor_menu.func[1] = &dialog_internal_sensor;
	}
	{
		external_sensor_menu.parent = &sys_option_menu;
		external_sensor_menu.func = pvPortMalloc(sizeof(void *) * external_sensor_menu.item_cnt);
		memset(external_sensor_menu.func, 0, sizeof(void *) * external_sensor_menu.item_cnt);
		external_sensor_menu.func[0] = &dialog_external_sensor;
		external_sensor_menu.func[1] = &dialog_external_sensor;
	}
	{
		external_continuous_power_menu.parent = &sys_option_menu;
		external_continuous_power_menu.func = pvPortMalloc(sizeof(void *) * external_continuous_power_menu.item_cnt);
		memset(external_continuous_power_menu.func, 0, sizeof(void *) * external_continuous_power_menu.item_cnt);
		external_continuous_power_menu.func[0] = &dialog_external_long_power;
		external_continuous_power_menu.func[1] = &dialog_external_long_power;
	}	
    {
		modbus_en_menu.parent = &menu_main;
		modbus_en_menu.func = pvPortMalloc(sizeof(void *) * modbus_en_menu.item_cnt);
		memset(modbus_en_menu.func, 0, sizeof(void *) * modbus_en_menu.item_cnt);
		modbus_en_menu.func[0] = &dialog_modbus_en;
		modbus_en_menu.func[1] = &dialog_modbus_en;
	}
	{
		sensor_type_menu.parent = &sys_option_menu;
		sensor_type_menu.func = pvPortMalloc(sizeof(void *) * sensor_type_menu.item_cnt);
		memset(sensor_type_menu.func, 0, sizeof(void *) * sensor_type_menu.item_cnt);
		sensor_type_menu.func[0] = &dialog_sensor_type;
		sensor_type_menu.func[1] = &dialog_sensor_type;
	}
	{
		language_menu.parent = &menu_main;
		language_menu.func = pvPortMalloc(sizeof(void *) * language_menu.item_cnt);
		memset(language_menu.func, 0, sizeof(void *) * language_menu.item_cnt);
		language_menu.func[0] = &dialog_language;
		language_menu.func[1] = &dialog_language;
	}
}

void oled_button_init(void)
{
	alarm_menu.is_button = config_get_alarm_en() +1;
	back_light_menu.is_button= config_get_back_light_time() + 1;
	temp_unit_menu.is_button= config_get_tem_unit() + 1;
	display_mode_menu.is_button= config_get_auto_switch_show() + 1;
	language_menu.is_button = config_get_language() + 1;
	internal_sensor_menu.is_button = config_get_internal_en() + 1;
	external_sensor_menu.is_button = config_get_external_en() + 1;	
	external_continuous_power_menu.is_button = config_get_external_long_power() + 1;
	sensor_type_menu.is_button = config_get_external_model() + 1;
	//upload_interval_menu.is_button = config_get_updata_time()+1;
	if(config_get_updata_time() == 1)
	{
		upload_interval_menu.is_button = 0+1;
	}
	else if(config_get_updata_time() == 2)
	{
		upload_interval_menu.is_button = 1+1;
	}
	else if(config_get_updata_time() == 5)
	{
		upload_interval_menu.is_button = 2+1;
	}
	else if(config_get_updata_time() == 30)
	{
		upload_interval_menu.is_button = 3+1;
	}
	else if(config_get_updata_time() == 60)
	{
		upload_interval_menu.is_button = 4+1;
	}
	else
	{
		upload_interval_menu.is_button = 0+1;
	}
	
    modbus_en_menu.is_button = config_get_modbus_en() + 1;
}

void oled_display_menu(void)
{
    uint32_t event_bit = 0;

    xTimerReset(oled_timer[0], 500);
    oled_button_init();

    while(1)
    {
        if((event_bit & NEXT_TOUCH) == NEXT_TOUCH)
        {
            if(current_menu->select_item == current_menu->item_cnt - 1)
			{
				current_menu->select_item = 0;
				current_menu->range_from = 0;
				current_menu->range_to = 2;
				
				if(current_menu->range_to >= current_menu->item_cnt)
				{
					current_menu->range_to = current_menu->item_cnt -1;
				}
            }
            else
            {
                if(++current_menu->select_item > current_menu->range_to)
				{
					current_menu->range_to = current_menu->select_item;
					current_menu->range_from++;
				}
            }
        }
		else if((event_bit & CONFIRM_TOUCH) == CONFIRM_TOUCH)
		{
			if(current_menu->sub_menu != NULL && current_menu->sub_menu[current_menu->select_item] != NULL)
			{
                current_menu = current_menu->sub_menu[current_menu->select_item];
			}
			else if(current_menu->func != NULL && current_menu->func[current_menu->select_item] != NULL)
			{
                if(current_menu->is_button != 0)
                {
                    show_ok();
                }
                current_menu->func[current_menu->select_item]();

				device_set_updata_cloud(1);

				system_write_setting();
			}
		}
        else if((event_bit & MENU_TOUCH) == MENU_TOUCH)
        {
            menu_main.range_from = 0;
			menu_main.range_to = 2;
			menu_main.select_item = 0;
			current_menu = &menu_main;
			xTimerStop(oled_timer[0], 300);
			break;
        }
        oled_display_item(current_menu);
        xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, portMAX_DELAY);
		oled_button_init();
    }
}

void oled_timer_reset(void)
{
	if(xTimerIsTimerActive(oled_timer[0]) == pdFALSE)
	{
		xTimerReset(oled_timer[1], 500);
	}
	else 
	{
		xTimerReset(oled_timer[0], 500);
	}
}

void oled_timer_stop(void)
{
	xTimerStop(oled_timer[0], 500);
	xTimerStop(oled_timer[1], 500);
}

/* 更新定时器并重新启动 */
static void oled_sleep_timer(void)
{
	static uint32_t tick_period;
	
	if(config_get_back_light_time() > 3)
	{
		config_set_back_light_time(3);
	}
	tick_period = (config_get_back_light_time() + 1) * 20000;

	if(xTimerChangePeriod(oled_timer[1], tick_period, 500) != pdPASS)
	{	
	    trace_log("OLED TIME ERR\r\n");
	}
	xTimerReset(oled_timer[1], 500);
}

void oled_timer_call_back(TimerHandle_t t_id)
{
	uint32_t timer_id;
	configASSERT(t_id);
	
	timer_id = (uint32_t )pvTimerGetTimerID(t_id);
	switch(timer_id)
	{
		case 1: dialog_exit_menu();break;
		case 2: 
        {
			trace_log("OLED Time_out\r\n");
            task_oled_post_event(EXIT_SHOW);
        }break;

		default:trace_log("OLED time err\r\n");break;	
	}
}

void oled_timer_init(void)
{
	if(oled_timer[0] == NULL && oled_timer[1] == NULL)
	{
		oled_timer[0] = xTimerCreate("oled_menu", 30000, pdFALSE, (void *)1, oled_timer_call_back);
		oled_timer[1] = xTimerCreate("oled_sleep", 20000 * (config_get_back_light_time() + 1), pdFALSE, (void *)2, oled_timer_call_back);
	}
}
/**
  * @brief  OLED_状态处理
  * @param  None
  * @retval None
  */
void oled_display_abnormal()
{

    /* 还需要加入低电量ERR */
	if((config_get_vbat_value() <= 3330) && (FL_GPIO_GetInputPin(GPIOB,FL_GPIO_PIN_7) == 0))
	{
		low_power_dialog();
	}
    net_err_dialog();

}

void oled_event(void)
{
    uint32_t event_bit = 0;
	uint8_t  tick = 0;
    oled_sleep_timer();
    oled_display_on();

	xTaskNotifyWait(0X00, 0XFFFFFFFF, NULL, 1000);

    while(1)
    {
        oled_display_main();

        xTaskNotifyWait(0X00, 0XFFFFFFFF, &event_bit, 1000);

		if(event_bit == 0)
		{
			show_info.sec++;
		}
		if(++tick >= 5)
		{
			oled_restart();
			tick = 0;
		}
		if((MENU_TOUCH & event_bit) == MENU_TOUCH) /* 进入菜单 */
		{
			xTimerStop(oled_timer[1], 100);
			oled_display_menu();
			oled_sleep_timer();
		}
		if((EXIT_SHOW & event_bit) == EXIT_SHOW)
		{
			trace_log("OLED exit\r\n");
			//oled_main_menu_deinit();
			oled_display_clear();
			oled_refresh_gram();
			break;
		}
        if((PAGE_SW & event_bit) == PAGE_SW)
        {
            if(++show_info.show_page >= 2)
            {
                show_info.show_page = 0;
                show_info.sec = 0;
            }
            else 
            {
                show_info.sec = 4;
            }
        }
        if((event_bit & PWR_TOUCH) == PWR_TOUCH)
        {
            dialog_power_down();
        }
		oled_display_abnormal();
		if(pwr_off == 1)
		{
			oled_display_clear();
			oled_refresh_gram();
			break;
		}	
    }
    oled_display_off();
}

void task_oled_post_event(uint32_t event)
{
//	if(oled_power == 0)
//	{
//		OLED_Init();
//	}
	pwr_opppse_halt(PWR_OLED_BIT);
    xTaskNotify(task_oled_handle, event, eSetBits);
}

static char output_buf[50];
void task_oled(void *paras)
{
	uint32_t bits = 0;
	oled_main_menu_init();
	oled_timer_init();
    while(is_sensor_finish() == 0)
    {
        vTaskDelay(500);
    }
//	OLED_Init();
    while(1)
    {
		pwr_opppse_halt(PWR_OLED_BIT);
        if(oled_power == 0)
        {
			OLED_Init();
        }
		vTaskDelay(1000); 
        trace_log("OLED START\r\n");
		if(bits != EXIT_SHOW)
		{
    	    oled_event();
			oled_deinit();
		}
		else
		{
			sprintf(output_buf,"OLED %x\r\n",bits);
			trace_log(output_buf);
		}
		//oled_display_abnormal();
		pwr_arg_halt(PWR_OLED_BIT);
    	xTaskNotifyWait(0X00, 0XFFFFFFFF, &bits, portMAX_DELAY);
    }
}

void oled_create_task(void *params, uint8_t priority)
{
    xTaskCreate(task_oled, "oled", 512, params, priority, &task_oled_handle);
}


