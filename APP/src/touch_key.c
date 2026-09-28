#include "touch_key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "display.h"
#include "sys_setting.h"
#include "display.h"
#include "usart.h"
#include "main.h"
#include "oled.h"

static touch_t    touch_pad[TOUCH_CNT];
static touch_fifo_t touch_fifo;

static uint8_t is_key_menu_down(void)
{
    if(FL_GPIO_GetInputPin(GPIOC,FL_GPIO_PIN_0))
    {
        return 0;
    }
    return 1;
}

static uint8_t is_key_pwr_down(void)
{
    if(FL_GPIO_GetInputPin(GPIOC,FL_GPIO_PIN_4))
    {    
        return 0;
    }
    return 1;
}

static void touch_put(uint8_t key_code)
{
    touch_fifo.buf[touch_fifo.write_offset] = key_code;
    if(++touch_fifo.write_offset >= 4)
    {
        touch_fifo.write_offset = 0;
    }
}

static void touch_detect(uint8_t i)
{
    touch_t* p_touch_pad;

    p_touch_pad = &touch_pad[i];
    if(p_touch_pad->is_key_down_fun())
    {
        if(p_touch_pad->press_cnt < KEY_FILTER_TIME)
        {
            p_touch_pad->press_cnt = KEY_FILTER_TIME;
        }
		else if(p_touch_pad->press_cnt < KEY_FILTER_TIME * 2)
		{
			p_touch_pad->press_cnt++;
		}
        else
        {
            if(p_touch_pad->status == 0)
            {
                p_touch_pad->status = 1;
                touch_put(3 * i + 1);
            }
            if(p_touch_pad->long_time > 0)
            {
                if((p_touch_pad->long_cnt) < (p_touch_pad->long_time))
                {
                    if(++p_touch_pad->long_cnt == p_touch_pad->long_time)
                    {
                        touch_put(3 * i + 3);  /* Êä³ö³¤°´BUTTON */
                    }
                }
            }
        }
    }
    else
    {
        if(p_touch_pad->status > KEY_FILTER_TIME)
        {
           p_touch_pad->press_cnt = KEY_FILTER_TIME; 
        }
        else if(p_touch_pad->press_cnt != 0)
        {
            p_touch_pad->press_cnt--;
        }
        else
        {
            if(p_touch_pad->status == 1)
            {
                p_touch_pad->status = 0;
                touch_put(3 * i + 2);
            }
        }
        p_touch_pad->long_cnt = 0;
    }
}

static void touch_init_fifo(void)
{
    touch_fifo.read_offset = 0;
    touch_fifo.write_offset = 0;

    for(uint8_t i = 0;i < TOUCH_CNT; ++i)
    {
        touch_pad[i].long_time= KEY_LONG_TIME;
        touch_pad[i].status   = 0;
        touch_pad[i].long_cnt = 0;
        touch_pad[i].press_cnt = 0;
    }
    touch_pad[0].is_key_down_fun = is_key_menu_down;
    touch_pad[1].is_key_down_fun = is_key_pwr_down;
}

uint8_t touch_get(void)
{
    uint8_t res;
    if(touch_fifo.read_offset == touch_fifo.write_offset)
    {
        return KEY_NONE;
    }
    else 
    {
        res = touch_fifo.buf[touch_fifo.read_offset];
        if(++touch_fifo.read_offset >= 4)
        {
            touch_fifo.read_offset = 0;
        }
    }
    return res;
}

void touch_clear(void)
{
    touch_fifo.read_offset = touch_fifo.write_offset;
}

void touch_scan(void)
{
    for(uint8_t i = 0; i < TOUCH_CNT; ++i)
    {
        touch_detect(i);
    }
}

void beep_and_led_deinit(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_4;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_4);
	FL_GPIO_ResetOutputPin(GPIOA, FL_GPIO_PIN_8);		
}

void beep_and_led_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
//	GPIO_InitStruct.pin = FL_GPIO_PIN_4 | FL_GPIO_PIN_8;
	GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.pin = FL_GPIO_PIN_4;
	FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void beep(uint16_t ms)
{
	FL_GPIO_SetOutputPin(GPIOA,FL_GPIO_PIN_8);
	FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_4);
    vTaskDelay(ms);
	FL_GPIO_ResetOutputPin(GPIOA,FL_GPIO_PIN_8);
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_4);
}

void chip_touch_pad_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
	
	/* PC0 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_0 | FL_GPIO_PIN_4;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    touch_init_fifo();
}

void touch_exit_init(void)
{   
	FL_EXTI_InitTypeDef    EXTI_InitStruct;
	
	EXTI_InitStruct.filter = FL_DISABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP0;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_FALLING;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_8, &EXTI_InitStruct);  

	EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP0;
    FL_EXTI_Init(FL_GPIO_EXTI_LINE_9, &EXTI_InitStruct); 	
}

void touch_exit_deinit(void)
{
	FL_EXTI_DeInit(FL_GPIO_EXTI_LINE_8);
	FL_EXTI_DeInit(FL_GPIO_EXTI_LINE_9);	
	//FL_GPIO_SetTriggerEdge0(GPIO, FL_GPIO_EXTI_LINE_8, FL_GPIO_EXTI_TRIGGER_EDGE_DISABLE);
}

void task_touch_pad(void *param)
{
    uint8_t pree_key;
    chip_touch_pad_init();
    while(1)
    {
        vTaskDelay(10);
        touch_scan();
        pree_key = touch_get();

        switch(pree_key)
        {
            case KEY_NONE:
            {
                
            }break;
            case MENU_DOWN:
            {
				beep(20);
				pwr_off = 0;
                task_oled_post_event(NEXT_TOUCH);          
            }break;

            case MENU_LONG:
            {
				beep(20);
				pwr_off = 0;
                task_oled_post_event(MENU_TOUCH);    
            }break;

            case MENU_UP:
            {
				pwr_off = 0;
                task_oled_post_event(PAGE_SW);
            }break;

            case POWER_DOWN:
            {
				beep(20);
				pwr_off = 0;
                task_oled_post_event(CONFIRM_TOUCH);                    
            }break;

            case POWER_LONG:
            {
				beep(20);
				pwr_off = 0;
                task_oled_post_event(PWR_TOUCH);               
            }break;
        }
		if(config_get_back_light_time() == 3)
        {
            oled_timer_reset();
        }
    }
}

void button_create_task(void *params, uint8_t priority)
{
    xTaskCreate(task_touch_pad, "touch_pad", 128, params, priority, NULL);
}


