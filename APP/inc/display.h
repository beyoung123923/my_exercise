#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "fm33lg0xx_fl.h"

#define CONFIRM_TOUCH   (0x01 << 1)
#define MENU_TOUCH      (0x01 << 2)
#define NEXT_TOUCH      (0x01 << 3)
#define EXIT_SHOW       (0x01 << 4)
#define PAGE_SW         (0x01 << 5)
#define PWR_TOUCH       (0x01 << 6)

#define PAGE_1          (0x00)
#define PAGE_2          (0x01)

typedef struct 
{
    uint8_t show_page;
    uint8_t sec;
}show_info_t;

typedef struct menu
{
	uint8_t range_from;
	uint8_t range_to;               /* 当前显示的起始和结束项 */	
	uint8_t item_cnt;           	/* item总数   */
	uint8_t select_item;              /* 当前选择项 */
	uint8_t is_button;               /* 选择按键同时作为底部按键选择列表*/
	uint8_t *item_code_cn[10];            /* 列表code */
	uint8_t *item_code_en[10];            /* 列表code */
	uint16_t item_width_cn[10];          /* 列表长度 */  
	uint16_t item_width_en[10];          /* 列表长度 */ 
	uint8_t *cn_menu_bar;    /* 指向menubar的字库 */
	uint8_t *en_menu_bar;
	uint8_t bar_width_cn;   /* menu的width */
	uint8_t bar_width_en;
	struct menu **sub_menu;       /* 子菜单 后续去掉 应该只有触发按键即可*/    
	struct menu *parent;          /* 上级菜单 */
	void  (**func)();             /* 确认键功能 */
}menu_t;

void task_oled_post_event(uint32_t event);
void oled_create_task(void *params, uint8_t priority);
void oled_timer_reset(void);
void oled_main_menu_init(void);
#endif

