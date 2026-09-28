#include "oled.h"
#include "FreeRTOS.h"
#include "task.h"
#include "string.h"
#include "oled_font.h"

const uint8_t oled_init_cmds[] = 
{
	0xAE, // 关闭显示
	0x00, // 设置列地址低4位
	0x10, // 设置列地址高4位
	0x40, // 设置显示起始行
	0x81, // 设置对比度
	0xCF, // 对比度值（0~255）
	0xA1, // 段重映射（左右翻转可选）
	0xC0, // 行重映射（上下翻转可选）
	0xA6, // 正常显示（0xA7=反显）
	0xA8, // 设置多路复用率
	0x3F, // 64行显示
	0xD3, // 设置显示偏移
	0x00, // 偏移0
	0xD5, // 设置振荡频率
	0x80, // 默认频率
	0xD9, // 设置预充电周期
	0xF1, // 预充电周期
	0xDA, // 设置COM引脚配置
	0x12, // COM引脚配置
	0xDB, // 设置VCOMH
	0x40, // VCOMH值
	0x20, // 设置内存地址模式
	0x02, // 页地址模式（适合128×64）
	0x8D, // 启用电荷泵
	0x14, // 电荷泵开启
	0xA4, // 启用电荷泵
	0xA6, // 电荷泵开启
	0xAF // 开启显示
};

const uint8_t oled_restar_cmds[] = 
{
	0xAE,//--关闭OLED显示
	0x00,//--设置列第地址
	0x10,//--设置列高地址
	0x40,//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	0x81,//--设置控制寄存器
	0xCF,//--设置SEG输出电流亮度。
	0xA1,//--Set SEG/Column Mapping
	0xC0,//--设置COM/ROW扫描方向
	0xA6,//--设置正常显示
	0xA8,//--设置多重比率(1至64
	0x3f,//--1/64占空比
	0xD3,//--设置显示偏移位移映射RAM计数器(0x00~0x3F
	0x00,//--不偏移
	0xd5,//--设置显示时钟分比/振荡器频率。
	0x80,//--设置分配比，设置时钟为100帧/秒。
	0xD9,//--设置pre-charge时期
	0xF1,//--Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	0xDA,//--设置com插头硬件配置。
	0x12,
	0xDB,//--设置vcomh
	0x40,//--Set VCOM Deselect Level
	0x20,//--Set Page Addressing Mode (0x00/0x01/0x02
	0x02,//
	0x8D,//--set Charge Pump enable/disable
	0x14,//--set(0x10 disable
	0xA4,//--Disable Entire Display On (0xa4/0xa5
	0xA6,//--Disable Inverse Display On (0xa6/a7 
	0xAF //--turn on oled panel
};

const uint8_t oled_display_on_cmds[] = 
{
	0X8D, /* set dcdc命令 */
	0X14, /* dcdc on*/
	0XAF, /* display on*/
};

const uint8_t oled_display_off_cmds[] = 
{
	0X8D, /* set dcdc命令 */
	0X10, /* dcdc off*/
	0XAF, /* display off*/
};

uint8_t oled_refresh_gram_cmds[] = 
{
	0xb0 , 	  /*设置页地址 0 - 7 */
	0x02,     /*设置显示位置  列低位*/
	0x10,    /* 设置显示位置  列高位*/
};


volatile uint8_t dma_tx_complete = 0; // DMA 传输完成标记（中断中更新）
uint8_t oled_power = 0;
static uint8_t oled_gram[128][8];
static uint8_t row_buf[128];

void init_print_buf(void)
{
	memset(oled_gram,0x01,sizeof(oled_gram));
}

void DMA_IRQHandler(void)
{
    if(FL_DMA_IsActiveFlag_TransferComplete(DMA, FL_DMA_CHANNEL_2))
    {
		FL_DMA_ClearFlag_TransferComplete(DMA, FL_DMA_CHANNEL_2);
	    dma_tx_complete = 1; // 设置发送完成标记	
		FL_DMA_DisableChannel(DMA,FL_DMA_CHANNEL_2);
    }
}

void GPIO_Init_OLED(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;
    
	/* PB9：SPI0_SCK 	PB11:SPI0_MOSI */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_9 | FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	/*PB8:DC	PB12:RES */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8 | FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void GPIO_DeInit_OLED(void)
{
    FL_GPIO_InitTypeDef gpioInitStruct = {0};

    gpioInitStruct.pin = FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_11 | FL_GPIO_PIN_12;
    gpioInitStruct.mode = FL_GPIO_MODE_INPUT;
    gpioInitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    gpioInitStruct.pull = FL_DISABLE;
    gpioInitStruct.remapPin = FL_DISABLE;
    gpioInitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &gpioInitStruct);
	FL_GPIO_DisablePinInput(GPIOB, FL_GPIO_PIN_8 | FL_GPIO_PIN_9 | FL_GPIO_PIN_11 | FL_GPIO_PIN_12);
}

void SPI0_Init(void) 
{
    FL_SPI_InitTypeDef    SPI0_InitStruct;
	
	SPI0_InitStruct.softControl = FL_DISABLE;
    SPI0_InitStruct.mode = FL_SPI_WORK_MODE_MASTER;
    SPI0_InitStruct.baudRate = FL_SPI_BAUDRATE_DIV2;
    SPI0_InitStruct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    SPI0_InitStruct.dataWidth = FL_SPI_DATA_WIDTH_8B;
    SPI0_InitStruct.clockPhase = FL_SPI_PHASE_EDGE1;
    SPI0_InitStruct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    SPI0_InitStruct.clockPolarity = FL_SPI_POLARITY_NORMAL;
    FL_SPI_Init(SPI0, &SPI0_InitStruct); 
    FL_SPI_ClearTXBuff(SPI0);
}

void SPI0_DMA_TX_Init(void) 
{
	FL_DMA_InitTypeDef dmaInitStruct = {0};
	
    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
	
    dmaInitStruct.circMode = FL_DISABLE;
    dmaInitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    dmaInitStruct.direction = FL_DMA_DIR_RAM_TO_PERIPHERAL;
    dmaInitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    dmaInitStruct.priority = FL_DMA_PRIORITY_HIGH ;
    dmaInitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;
    FL_DMA_Init(DMA, &dmaInitStruct, FL_DMA_CHANNEL_2);
    
	FL_DMA_EnableIT_TransferComplete(DMA,FL_DMA_CHANNEL_2);						//使能DMA+SPI中断
	
	FL_DMA_Enable(DMA);

    InterruptConfigStruct.preemptPriority = 1;
    FL_NVIC_Init(&InterruptConfigStruct, DMA_IRQn);
}

// 等待 DMA 传输完成
static void OLED_DMA_WaitComplete(void) 
{
	uint16_t wait_cont = 0;
	while (dma_tx_complete == 0)
	{
		if(++wait_cont > 1000)
		{
			break;
		}
		vTaskDelay(1);
	}
	dma_tx_complete = 0; // 重置标记
}

// DMA 发送单个命令
void OLED_DMA_SendCmd(uint32_t *cmd, uint16_t len) 
{
	/*if(oled_power == 0)
	{
		OLED_Init();
	}*/
	
	if(oled_power == 1)
	{
        if(len == 0)
        {
            return;
        }
		FL_SPI_ClearTXBuff(SPI0);
		FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_8); // DC=0 → 命令模式
		// 配置 DMA 存储器地址和传输长度
		FL_DMA_WriteMemoryAddress(DMA, (uint32_t)cmd, FL_DMA_CHANNEL_2);
		FL_DMA_WriteTransmissionSize(DMA, len-1, FL_DMA_CHANNEL_2);
		// 启动 DMA 传输
		FL_DMA_EnableChannel(DMA,FL_DMA_CHANNEL_2);
		// 等待传输完成
		OLED_DMA_WaitComplete();	
	}
}

// DMA 发送数据块
void OLED_DMA_SendData(uint32_t *data, uint16_t len) 
{
	/*if(oled_power == 0)
	{
		OLED_Init();
	}*/
	if(oled_power == 1)
	{
        if(len == 0)
        {
            return;
        }
		FL_SPI_ClearTXBuff(SPI0);
		FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_8); // DC=1 → 数据模式
		// 配置 DMA 存储器地址和传输长度
		FL_DMA_WriteMemoryAddress(DMA, (uint32_t)data, FL_DMA_CHANNEL_2);
		FL_DMA_WriteTransmissionSize(DMA, len-1, FL_DMA_CHANNEL_2);
		// 启动 DMA 传输
		FL_DMA_EnableChannel(DMA,FL_DMA_CHANNEL_2);
		// 等待传输完成
		OLED_DMA_WaitComplete();							
	}	
}

//开启显示
void oled_display_on(void)
{
	uint16_t cmd_len = sizeof(oled_display_on_cmds)/sizeof(oled_display_on_cmds[0]); 
	OLED_DMA_SendCmd((uint32_t *)oled_display_on_cmds,cmd_len);
	
//	OLED_DMA_SendCmd(0X8D); /* set dcdc命令 */
//	OLED_DMA_SendCmd(0X14); /* dcdc on*/
//	OLED_DMA_SendCmd(0XAF); /* display on*/
}

//关闭显示
void oled_display_off(void)
{
	uint16_t cmd_len = sizeof(oled_display_off_cmds)/sizeof(oled_display_off_cmds[0]); 
	OLED_DMA_SendCmd((uint32_t *)oled_display_off_cmds,cmd_len);
//	OLED_DMA_SendCmd(0X8D); /* set dcdc命令 */
//	OLED_DMA_SendCmd(0X10); /* dcdc off*/
//	OLED_DMA_SendCmd(0XAF); /* display on*/	
	//oled_write_rst(0);
}

//刷新屏幕
void oled_refresh_gram(void)
{
	uint8_t row, column; /*行与列*/ 
	uint16_t cmd_len = sizeof(oled_refresh_gram_cmds)/sizeof(oled_refresh_gram_cmds[0]); 
	
	for(row = 0; row < 8; row++)
	{
		oled_refresh_gram_cmds[0] = 0xb0 + row;
		OLED_DMA_SendCmd((uint32_t *)oled_refresh_gram_cmds,cmd_len);
		
//		OLED_DMA_SendCmd(0xb0 + row); /*设置页地址 0 - 7 */
//		OLED_DMA_SendCmd(0x02);     /*设置显示位置  列低位*/
//		OLED_DMA_SendCmd(0x10);    /* 设置显示位置  列高位*/
		for(column = 0; column < 128; column++)
        {
			row_buf[column] = oled_gram[column][row];
            //OLED_DMA_SendData(&oled_gram[column][row], 1);
        }
		OLED_DMA_SendData((uint32_t*)row_buf,128);
	}
}

//清除缓存
void oled_display_clear(void)
{
	uint8_t i,n;
	for(i = 0;i < 8;i++)
	{
		for(n = 0;n < 128; n++)
		{
			oled_gram[n][i] = 0X00;
		}
	}
}

//加载缓存
void oled_display_point(uint8_t x, uint8_t y, uint8_t t)
{
    uint8_t pos, bx, temp = 0;
    if(x > 127 || y > 63)
		return ;
    
	pos = 7 - y/8;   /*求出在第几行 因为是相反的所以要用7*/
	bx = y%8;
	temp = 1 << (7 - bx);  /* 根据OLED来做偏移 */
	if(t)
	{
		oled_gram[x][pos] |= temp;
	}else
	{
		oled_gram[x][pos] &= ~temp;
	}
}

//全显
void oled_fill(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t dot)
{
	uint8_t x, y;
	for(x = x1; x <= x2; x++)
	{
		for(y = y1; y<= y2; y++)
		{
			oled_display_point(x, y, dot);
		}
	}
}

//加载线条
void oled_display_line(uint8_t s_x, uint8_t s_y, uint8_t e_x, uint8_t e_y, uint8_t mode)
{
	int32_t dx, dy;
	int32_t tx, ty;
	int32_t inc1, inc2;
	int32_t d, itag;
	int32_t x, y;
	
	oled_display_point(s_x, s_y, mode);
	
	if(s_x == e_x && s_y == e_y)
	{
		return ;
	}
	
	itag = 0;
		/* dx = abs ( _usX2 - _usX1 ); */
	if( e_x > s_x )
	{
		dx = e_x - s_x;
	}else 
	{
		dx = s_x - e_x;
	}
	
	/* dy = abs ( _usY2 - _usY1 ); */
	if (e_y >= s_y)
	{
		dy = e_y - s_y;
	}
	else
	{
		dy = s_y - e_y;
	}
	
	if(dx < dy)  /* 如果dy为计长方向，则交换纵横坐标。 */
	{
		uint16_t temp;
		
		itag = 1;
		temp = s_x; s_x = s_y; s_y = temp;
		temp = e_x; e_x = e_y; e_y = temp;
		
		temp = dx; dx = dy; dy = temp;
	}
	tx = e_x > s_x ? 1 : -1;  /* 确定是增1还是减1 */
	ty = e_y > s_y ? 1 : -1;
	
	x = s_x;
	y = s_y;
	
	inc1 = 2 * dy;
	inc2 = 2 * (dy - dx);
	
	d = inc1 - dx;
	
	while(x != e_x)
	{
		if( d < 0)
		{
			d += inc1;
		}
		else 
		{
			y += ty;
			d += inc2;
		}
		if(itag)
		{
			oled_display_point(y, x, mode);
		}
		else 
		{
			oled_display_point(x, y, mode);
		}
		 x += tx;
	}
	
}

//加载单个字符
void oled_show_char(uint8_t x, uint8_t y, uint8_t chr , uint8_t size, uint8_t mode)
{
	uint8_t temp,t,t1;
	uint8_t y0 = y;
	uint8_t chr_size = (size/8 + ((size % 8) ? 1 : 0)) * (size / 2);   /* 得到字符对应点阵集所占字节数 */
	chr -= ' ';
	for(t = 0;t < chr_size; t++)
	{
		if(size == 12)
			temp = asc2_1206[chr][t];
		/*else if(size == 16)
			temp = asc2_1608[chr][t];*/
		else if(size == 24)
			temp = asc2_2412[chr][t];
		else 
			return;
		
		for(t1 = 0;t1 < 8; t1++)
		{
			if(temp & 0X80)
				oled_display_point(x, y, mode);
			else 
				oled_display_point(x, y, !mode);
			temp <<= 1;
			y++;
			if((y - y0) == size)  /* 写入行完毕 跳转到下一列 */
			{
				y = y0;
				x++;
				break;
			}
		}
	}
}

//加载字符串
void oled_display_string(uint8_t x,uint8_t y, const uint8_t *string, uint8_t size, uint8_t moth)
{
	while((*string <= '~') && (*string >= ' '))
	{
		if(x > 128 - (size / 2)) /*偏移到下一列显示*/
		{
			x = 0; y+= size;
		}
		if(y > (64-size))
		{
			break;	/* 越界字符直接丢弃,严禁中途清屏破坏已绘制画面 */
		}
		oled_show_char(x, y, *string, size, moth);
		x += size / 2;
		string++;
	}
}

//加载图片
void oled_display_bmp(uint8_t x, uint8_t y,uint8_t width, uint8_t height, uint8_t *ptr, uint8_t mode)
{
	uint8_t temp = 0, y_pos, y_0 = y;
	width += x;
	height += y;
	
	for(; x < width; x++)
	{
		for(y = y_0; y < height;)
		{
			temp = *ptr++;
			
			for(y_pos = 0; y_pos < 8;y_pos++)
			{
				if(temp & 0X80)
					oled_display_point(x, y, mode);
				else 
					oled_display_point(x, y, !mode);
				temp <<= 1;
				y++;
				if(y == height)
				{
					break;
				}
			}
		}
	}
}

void OLED_Init(void) 
{
	uint16_t cmd_len = sizeof(oled_init_cmds)/sizeof(oled_init_cmds[0]);
	oled_power = 1;
	// 底层初始化（GPIO→SPI→DMA）
	GPIO_Init_OLED();
	SPI0_Init();
	SPI0_DMA_TX_Init();

	// OLED 硬件复位
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_12);
	vTaskDelay(10);
	FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_12);
	vTaskDelay(10);
	
	// 发送初始化命令（SSD1306 标准序列）
	OLED_DMA_SendCmd((uint32_t *)oled_init_cmds,cmd_len);
	
//	OLED_DMA_SendCmd(0xAE); // 关闭显示
//	OLED_DMA_SendCmd(0x00); // 设置列地址低4位
//	OLED_DMA_SendCmd(0x10); // 设置列地址高4位
//	OLED_DMA_SendCmd(0x40); // 设置显示起始行
//	OLED_DMA_SendCmd(0x81); // 设置对比度
//	OLED_DMA_SendCmd(0xCF); // 对比度值（0~255）
//	OLED_DMA_SendCmd(0xA1); // 段重映射（左右翻转可选）
//	OLED_DMA_SendCmd(0xC0); // 行重映射（上下翻转可选）
//	OLED_DMA_SendCmd(0xA6); // 正常显示（0xA7=反显）
//	OLED_DMA_SendCmd(0xA8); // 设置多路复用率
//	OLED_DMA_SendCmd(0x3F); // 64行显示
//	OLED_DMA_SendCmd(0xD3); // 设置显示偏移
//	OLED_DMA_SendCmd(0x00); // 偏移0
//	OLED_DMA_SendCmd(0xD5); // 设置振荡频率
//	OLED_DMA_SendCmd(0x80); // 默认频率
//	OLED_DMA_SendCmd(0xD9); // 设置预充电周期
//	OLED_DMA_SendCmd(0xF1); // 预充电周期
//	OLED_DMA_SendCmd(0xDA); // 设置COM引脚配置
//	OLED_DMA_SendCmd(0x12); // COM引脚配置
//	OLED_DMA_SendCmd(0xDB); // 设置VCOMH
//	OLED_DMA_SendCmd(0x40); // VCOMH值
//	OLED_DMA_SendCmd(0x20); // 设置内存地址模式
//	OLED_DMA_SendCmd(0x02); // 页地址模式（适合128×64）
//	OLED_DMA_SendCmd(0x8D); // 启用电荷泵
//	OLED_DMA_SendCmd(0x14); // 电荷泵开启
//	OLED_DMA_SendCmd(0xA4); // 启用电荷泵
//	OLED_DMA_SendCmd(0xA6); // 电荷泵开启
//	OLED_DMA_SendCmd(0xAF); // 开启显示
	
	oled_display_clear();	// 清屏
	oled_refresh_gram(); // 刷新显示
}

void oled_restart(void)
{
	uint16_t cmd_len = sizeof(oled_restar_cmds)/sizeof(oled_restar_cmds[0]);
	// OLED 硬件复位
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_12);
	vTaskDelay(10);
	FL_GPIO_SetOutputPin(GPIOB, FL_GPIO_PIN_12);
	vTaskDelay(10);
	
	OLED_DMA_SendCmd((uint32_t *)oled_restar_cmds,cmd_len);
	
//	OLED_DMA_SendCmd(0xAE);//--关闭OLED显示
//	OLED_DMA_SendCmd(0x00);//--设置列第地址
//	OLED_DMA_SendCmd(0x10);//--设置列高地址
//	OLED_DMA_SendCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
//	OLED_DMA_SendCmd(0x81);//--设置控制寄存器
//	OLED_DMA_SendCmd(0xCF);//--设置SEG输出电流亮度。
//	OLED_DMA_SendCmd(0xA1);//--Set SEG/Column Mapping
//	OLED_DMA_SendCmd(0xC0);//--设置COM/ROW扫描方向
//	OLED_DMA_SendCmd(0xA6);//--设置正常显示
//	OLED_DMA_SendCmd(0xA8);//--设置多重比率(1至64)
//	OLED_DMA_SendCmd(0x3f);//--1/64占空比
//	OLED_DMA_SendCmd(0xD3);//--设置显示偏移位移映射RAM计数器(0x00~0x3F)
//	OLED_DMA_SendCmd(0x00);//--不偏移
//	OLED_DMA_SendCmd(0xd5);//--设置显示时钟分比/振荡器频率。
//	OLED_DMA_SendCmd(0x80);//--设置分配比，设置时钟为100帧/秒。
//	OLED_DMA_SendCmd(0xD9);//--设置pre-charge时期
//	OLED_DMA_SendCmd(0xF1);//--Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
//	OLED_DMA_SendCmd(0xDA);//--设置com插头硬件配置。
//	OLED_DMA_SendCmd(0x12);
//	OLED_DMA_SendCmd(0xDB);//--设置vcomh
//	OLED_DMA_SendCmd(0x40);//--Set VCOM Deselect Level
//	OLED_DMA_SendCmd(0x20);//--Set Page Addressing Mode (0x00/0x01/0x02)
//	OLED_DMA_SendCmd(0x02);//
//	OLED_DMA_SendCmd(0x8D);//--set Charge Pump enable/disable
//	OLED_DMA_SendCmd(0x14);//--set(0x10) disable
//	OLED_DMA_SendCmd(0xA4);//--Disable Entire Display On (0xa4/0xa5)
//	OLED_DMA_SendCmd(0xA6);//--Disable Inverse Display On (0xa6/a7) 
//	OLED_DMA_SendCmd(0xAF);//--turn on oled panel

	oled_display_clear();
}

void oled_deinit(void)
{
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_8);
	FL_GPIO_ResetOutputPin(GPIOB, FL_GPIO_PIN_12);

	if(oled_power == 1)
	{
		oled_power = 0;
		/************
		低功耗前关外设
		*************/

        //关闭DMA
		FL_DMA_DisableChannel(DMA, FL_DMA_CHANNEL_2);
		FL_DMA_Disable(DMA);
		
        //关闭SPI
		FL_SPI_Disable(SPI0);

        //关闭IO
        GPIO_DeInit_OLED();
	}
}




