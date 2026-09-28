/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "main.h"

/* 函数原型 ----------------------------------------------------------*/
/* 宏  ---------------------------------------------------------------*/
/* 变量 --------------------------------------------------------------*/
static uart_t  usb_uart;
static uart_t  gps_uart;
static uart_t  iot_uart;
static uart_t  sensor_uart;
static uint8_t iot_buf[IOT_BUF_SIZE];
static uint8_t gps_buf[USB_BUF_SIZE];
static uint8_t usb_buf[USB_BUF_SIZE];
static uint8_t sensor_buff[SENSOR_BUF_SIZE];
static uint8_t debug = 0;
TimerHandle_t  OneShotTimer_Handle = NULL; //单次定时器句柄
TimerHandle_t  OneShotTimer1_Handle = NULL;

/* 函数代码 ----------------------------------------------------------*/
/**
 * @description: 串口超时中断回调
 * @param  uart_id 串口的ID
 * @return None
 */
void uart_isr_rx_timeout(uint8_t uart_id)
{
    uart_t *uart;

    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;
        
        default: break;
    }
    uart->rx_end = 1;
}

/**
 * @description: 串口接收中断回调
 * @param  uart_id 串口的ID
 * @return None
 */
void uart_recv_irq_proc(uint8_t uart_id)
{
    uint8_t rd;
    uart_t *uart;

    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;
        
        default: break;
    }
	
	rd = FL_UART_ReadRXBuff(uart->uart);
	
	if(uart->rx_count < uart->rx_size)
	{
		uart->rx_end = 0;
		uart->rx_count++;
		uart->rx_buf[uart->write_pos] = rd;
		
		if(++uart->write_pos >= uart->rx_size)
		{
			uart->write_pos = 0;
		}
	}
	uart->rx_end = 0;
}

//WIFI
void OneShotCallback1(TimerHandle_t xTimer)
{
	uart_isr_rx_timeout(WIFI_UART);
}
void UART4_IRQHandler(void)
{
	BaseType_t task_woken;
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART4))
		&&(FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART4)))
	{
		//中断转发接收到的数据
		uart_recv_irq_proc(WIFI_UART);
		xTimerResetFromISR(OneShotTimer1_Handle, &task_woken);//复位软件定时器
		portYIELD_FROM_ISR(task_woken);     
	}
}

//4G
void UART0_IRQHandler(void)
{
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART0))
		&&(FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART0)))
	{ 
		FL_UART_ClearFlag_RXBuffFull(UART0);///
		uart_recv_irq_proc(IOT_4G_UART);
	}
	
	if(FL_ENABLE == FL_UART_IsActiveFlag_RXBuffTimeout(UART0))
	{
		FL_UART_ClearFlag_RXBuffTimeout(UART0);
		uart_isr_rx_timeout(IOT_4G_UART);		
	}
}

//USB
void UART1_IRQHandler(void)
{	
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART1))   
		&&(FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART1)))
	{
		uart_recv_irq_proc(USB_UART);
	}
	if(FL_ENABLE == FL_UART_IsActiveFlag_RXBuffTimeout(UART1))
	{
		FL_UART_ClearFlag_RXBuffTimeout(UART1);
		uart_isr_rx_timeout(USB_UART);		
	}
}

//GPS
void OneShotCallback(TimerHandle_t xTimer)
{
	uart_isr_rx_timeout(GPS_UART);
}
void UART3_IRQHandler(void)
{
	BaseType_t task_woken;
	//接收中断处理
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART3))
		&&(FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART3)))
	{
		//中断转发接收到的数据
		uart_recv_irq_proc(GPS_UART);
		xTimerResetFromISR(OneShotTimer_Handle, &task_woken);//复位软件定时器
		portYIELD_FROM_ISR(task_woken);     
	}
}

//SENSOR
void BSTIM_IRQHandler(void)
{
	if(FL_BSTIM32_IsEnabledIT_Update(BSTIM32) && FL_BSTIM32_IsActiveFlag_Update(BSTIM32))
	{
		FL_BSTIM32_ClearFlag_Update(BSTIM32);
		FL_BSTIM32_Disable(BSTIM32);
		uart_isr_rx_timeout(SENSOR_UART);
	}
}

void MF_BSTIM32_Init2(void)
{
	FL_BSTIM32_InitTypeDef    TimerBase_InitStruct;
	
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
	
    TimerBase_InitStruct.prescaler = 7999;
    TimerBase_InitStruct.autoReload = 20;  //40
    TimerBase_InitStruct.autoReloadState = FL_ENABLE;
    TimerBase_InitStruct.clockSource = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;
    FL_BSTIM32_Init(BSTIM32, &TimerBase_InitStruct);

	FL_BSTIM32_ClearFlag_Update(BSTIM32);
	FL_BSTIM32_EnableIT_Update(BSTIM32);
	
    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);
	
	FL_BSTIM32_Enable(BSTIM32);
}

void UART5_IRQHandler(void)
{
	if((FL_ENABLE == FL_UART_IsEnabledIT_RXBuffFull(UART5))
		&&(FL_SET == FL_UART_IsActiveFlag_RXBuffFull(UART5)))
	{	uart_recv_irq_proc(SENSOR_UART);
		MF_BSTIM32_Init2();	
	}
}

/**
 * @description: 串口初始化
 * @param  uart_id 串口的ID
 * @return None
 */
void iot_4g_uart_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART0_InitStruct; 
	
	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

    /* PA2 UART0_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA3 UART0_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	
    iot_uart.uart               = UART0;
    iot_uart.rx_size            = IOT_BUF_SIZE;
    iot_uart.rx_buf             = iot_buf;
    iot_uart.rx_count           = 0;
    iot_uart.rx_end             = 0;
    iot_uart.read_pos           = 0;
    iot_uart.write_pos          = 0;
	
	UART0_InitStruct.clockSrc = FL_CMU_UART0_CLK_SOURCE_APBCLK;
    UART0_InitStruct.baudRate = 115200;
    UART0_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART0_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART0_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART0_InitStruct.parity = FL_UART_PARITY_NONE;
	FL_UART_Init(UART0, &UART0_InitStruct);   
	
	/*NVIC中断配置*/
	FL_UART_EnableRXTimeout(UART0);
	FL_UART_EnableIT_RXTimeout(UART0);
	
	FL_UART_ClearFlag_RXBuffFull(UART0);
    FL_UART_EnableIT_RXBuffFull(UART0);  
	
	FL_UART_WriteRXTimeout(UART0,50);
	FL_UART_ClearFlag_RXBuffTimeout(UART0);
	
    FL_UART_EnableTX(UART0);
    FL_UART_EnableRX(UART0);

    InterruptConfigStruct.preemptPriority = 0x00;
    FL_NVIC_Init(&InterruptConfigStruct, UART0_IRQn);
}


/**
 * @description: 串口初始化
 * @param  uart_id 串口的ID
 * @return None
 */
void usb_uart_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART1_InitStruct;

	FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
    /* PC2 UART1_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC3 UART1_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
    usb_uart.uart               = UART1;
    usb_uart.rx_size            = USB_BUF_SIZE;
    usb_uart.rx_buf             = usb_buf;
    usb_uart.rx_count           = 0;
    usb_uart.rx_end             = 0;
    usb_uart.read_pos           = 0;
    usb_uart.write_pos          = 0;
	
    UART1_InitStruct.clockSrc = FL_CMU_UART1_CLK_SOURCE_APBCLK;
    UART1_InitStruct.baudRate = 115200;
    UART1_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART1_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART1_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART1_InitStruct.parity = FL_UART_PARITY_NONE;
	FL_UART_Init(UART1, &UART1_InitStruct);    
	
	/*NVIC中断配置*/
	FL_UART_EnableRXTimeout(UART1);
	FL_UART_EnableIT_RXTimeout(UART1);
	
	FL_UART_ClearFlag_RXBuffFull(UART1);
    FL_UART_EnableIT_RXBuffFull(UART1);  
	
	FL_UART_WriteRXTimeout(UART1,50);
	FL_UART_ClearFlag_RXBuffTimeout(UART1);

    FL_UART_EnableTX(UART1);
    FL_UART_EnableRX(UART1);
	
	InterruptConfigStruct.preemptPriority = 0x01;
    FL_NVIC_Init(&InterruptConfigStruct, UART1_IRQn);


    NVIC_DisableIRQ(UART1_IRQn);
    NVIC_SetPriority(UART1_IRQn,1);//中断优先级配置
    NVIC_EnableIRQ(UART1_IRQn); 
}

/**
 * @description: 串口初始化
 * @param  uart_id 串口的ID
 * @return None
 */
void wifi_uart_init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART4_InitStruct;

    FL_NVIC_ConfigTypeDef  InterruptConfigStruct;
	
    /* PB2 UART4_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_2;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB3 UART4_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_3;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);	
	
    iot_uart.uart            	= UART4;
    iot_uart.rx_size            = IOT_BUF_SIZE;
    iot_uart.rx_buf             = iot_buf;
    iot_uart.rx_count           = 0;
    iot_uart.rx_end             = 0;
    iot_uart.read_pos           = 0;
    iot_uart.write_pos          = 0;
	
    UART4_InitStruct.clockSrc = NULL;
    UART4_InitStruct.baudRate = 115200;
    UART4_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART4_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART4_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART4_InitStruct.parity = FL_UART_PARITY_NONE;
    FL_UART_Init(UART4, &UART4_InitStruct); 
	
	if(OneShotTimer1_Handle == NULL)
	{
		OneShotTimer1_Handle = xTimerCreate("OneShotTimer1", 100, pdFALSE, (void *)0, OneShotCallback1);
	}
			
	/*NVIC中断配置*/
	FL_UART_ClearFlag_RXBuffFull(UART4);
    FL_UART_EnableIT_RXBuffFull(UART4);  
	
    FL_UART_EnableTX(UART4);
    FL_UART_EnableRX(UART4);

    InterruptConfigStruct.preemptPriority = 0;
    FL_NVIC_Init(&InterruptConfigStruct, UART4_IRQn); 
}

/**
 * @description: 串口初始化
 * @param  uart_id 串口的ID
 * @return None
 */
void gps_uart_init(void)
{
	FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART3_InitStruct;

    FL_NVIC_ConfigTypeDef  InterruptConfigStruct;
	
    /* PB2 UART3_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_ENABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* PB3 UART3_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_ENABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
    gps_uart.uart               = UART3;
    gps_uart.rx_size            = USB_BUF_SIZE;
    gps_uart.rx_buf             = gps_buf;
    gps_uart.rx_count           = 0;
    gps_uart.rx_end             = 0;
    gps_uart.read_pos           = 0;
    gps_uart.write_pos          = 0;
	
    UART3_InitStruct.clockSrc = NULL;
    UART3_InitStruct.baudRate = 9600;
    UART3_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART3_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART3_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART3_InitStruct.parity = FL_UART_PARITY_NONE;
    FL_UART_Init(UART3, &UART3_InitStruct);  
	
	if(OneShotTimer_Handle == NULL)
	{
		OneShotTimer_Handle = xTimerCreate("OneShotTimer", 100, pdFALSE, (void *)0, OneShotCallback);	
	}
		
	/*NVIC中断配置*/
    FL_UART_ClearFlag_RXBuffFull(UART3);
    FL_UART_EnableIT_RXBuffFull(UART3);  
	
    FL_UART_EnableTX(UART3);
    FL_UART_EnableRX(UART3);

    InterruptConfigStruct.preemptPriority = 0x01;
    FL_NVIC_Init(&InterruptConfigStruct, UART3_IRQn); 
}

/**
 * @description: 串口初始化
 * @param  uart_id 串口的ID
 * @return None
 */
void sensor_uart_init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART5_InitStruct;

    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;
	
    /* PD0 UART5_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_0;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* PD1 UART5_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);
	
    sensor_uart.uart               = UART5;
    sensor_uart.rx_size            = SENSOR_BUF_SIZE;
    sensor_uart.rx_buf             = sensor_buff;
    sensor_uart.rx_count           = 0;
    sensor_uart.rx_end             = 0;
    sensor_uart.read_pos           = 0;
    sensor_uart.write_pos          = 0;
	
    UART5_InitStruct.clockSrc = NULL;
    UART5_InitStruct.baudRate = 115200;
    UART5_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART5_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART5_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART5_InitStruct.parity = FL_UART_PARITY_NONE;
    FL_UART_Init(UART5, &UART5_InitStruct); 
	
	/*NVIC中断配置*/
    FL_UART_ClearFlag_RXBuffFull(UART5);
    FL_UART_EnableIT_RXBuffFull(UART5);  
	
    FL_UART_EnableTX(UART5);
    FL_UART_EnableRX(UART5);

    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, UART5_IRQn);
}

/**
 * @description: 串口推出休眠模式
 * @param  uart_id 串口的ID
 * @return None
 */
/* void uart_exti_sleep_mode(uint8_t uart_id)
{
	FL_GPIO_InitTypeDef GPIO_InitStructure;
	if(uart_id == USB_UART)
	{
        GPIO_InitStructure.Pin       = FL_GPIO_PIN_2;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
        FL_GPIO_Init(GPIOC, &GPIO_InitStructure);

        GPIO_InitStructure.Pin        = FL_GPIO_PIN_3;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
        FL_GPIO_Init(GPIOC, &GPIO_InitStructure);
        USART_Enable(UART1, ENABLE);
	}
	else if(uart_id == IOT_4G_UART)  
	{
        GPIO_InitStructure.Pin            = FL_GPIO_PIN_3|FL_GPIO_PIN_2;    
        GPIO_InitStructure.GPIO_Mode      = GPIO_MODE_AF_PP;
        GPIO_InitStructure.GPIO_Alternate = GPIO_AF4_USART1;
        FL_GPIO_Init(IOT_GPIO, &GPIO_InitStructure);
        USART_Enable(IOT, ENABLE);
	}
} */

/**
 * @description: 串口进入休眠模式
 * @param  uart_id 串口的ID
 * @return None
 */
void uart_enter_sleep_mode(uint8_t uart_id)
{
	FL_GPIO_InitTypeDef io_config = {0}; 
	if(uart_id == IOT_4G_UART)// 
	{
        io_config.pin        = FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
		io_config.mode       = FL_GPIO_MODE_INPUT;
		io_config.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
		io_config.pull       = FL_DISABLE;
		io_config.remapPin   = FL_DISABLE;
		
		FL_GPIO_Init(GPIOA, &io_config);
		FL_GPIO_DisablePinInput(GPIOA, FL_GPIO_PIN_2 | FL_GPIO_PIN_3);
		FL_UART_DisableRX(UART0);
		FL_UART_DisableTX(UART0);
	}
	else if(uart_id == USB_UART) 
	{
        io_config.pin        = FL_GPIO_PIN_3 | FL_GPIO_PIN_2;  
		io_config.mode       = FL_GPIO_MODE_INPUT;
		io_config.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
		io_config.pull       = FL_DISABLE;
		io_config.remapPin   = FL_DISABLE;
		
		FL_GPIO_Init(GPIOC, &io_config);
		FL_GPIO_DisablePinInput(GPIOC, FL_GPIO_PIN_3|FL_GPIO_PIN_2);
		FL_UART_DisableRX(UART1);
		FL_UART_DisableTX(UART1);
	}
	else if(uart_id == WIFI_UART)
	{
        io_config.pin        = FL_GPIO_PIN_2 | FL_GPIO_PIN_3;
		io_config.mode       = FL_GPIO_MODE_INPUT;
		io_config.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
		io_config.pull       = FL_DISABLE;
		io_config.remapPin   = FL_DISABLE;
		
		FL_GPIO_Init(GPIOB, &io_config);
		FL_GPIO_DisablePinInput(GPIOB, FL_GPIO_PIN_2 | FL_GPIO_PIN_3);
		FL_UART_DisableRX(UART4);
		FL_UART_DisableTX(UART4);
	}
	else if(uart_id == GPS_UART) 
	{
        io_config.pin        = FL_GPIO_PIN_7 | FL_GPIO_PIN_8;  
		io_config.mode       = FL_GPIO_MODE_INPUT;
		io_config.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
		io_config.pull       = FL_DISABLE;
		io_config.remapPin   = FL_DISABLE;
		
		FL_GPIO_Init(GPIOD, &io_config);
		FL_GPIO_DisablePinInput(GPIOD, FL_GPIO_PIN_7 | FL_GPIO_PIN_8);
		FL_UART_DisableRX(UART3);
		FL_UART_DisableTX(UART3);
	}
	else if(uart_id == SENSOR_UART) 
	{
        io_config.pin        = FL_GPIO_PIN_0|FL_GPIO_PIN_1;  
		io_config.mode       = FL_GPIO_MODE_INPUT;
		io_config.outputType = FL_GPIO_OUTPUT_OPENDRAIN;
		io_config.pull       = FL_DISABLE;
		io_config.remapPin   = FL_DISABLE;
		
		FL_GPIO_Init(GPIOD, &io_config);
		FL_GPIO_DisablePinInput(GPIOD, FL_GPIO_PIN_0|FL_GPIO_PIN_1);
		FL_UART_DisableRX(UART5);
		FL_UART_DisableTX(UART5);
	}
}

/**
 * @description: 串口发送BUF
 * @param  串口的ID
 * @param  要发送的buff 
 * @param  发送的长度
 * @return None
 */
void uart_transmit_buf(uint8_t uart_id, uint8_t *tx_buf, uint16_t tx_size)
{
	//uint16_t wait_cont = 0;
    uart_t *uart;
    uint16_t tx_cnt = 0;
    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;
        
        default: break;
    }
	
    if(uart == NULL)
    {
        return; 
    }
    else
    {
		while(tx_cnt < tx_size)
		{
			FL_UART_WriteTXBuff(uart->uart, tx_buf[tx_cnt++]);
			while(FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(uart->uart));		//等待发送完成
//			while(FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(uart->uart))		//等待发送完成
//			{
//				if(wait_cont++ > 50000)
//				{
//					break;
//				}
//			}
//			wait_cont = 0;
		}	
    } 
}
/**
 * @description: 串口发送字符串
 * @param  串口的ID
 * @param  要发送的字符串
 * @return None
 */
void uart_transmit_str(uint8_t uart_id, uint8_t *tx_buf)
{
	//uint16_t wait_cont = 0;
    uart_t *uart;
    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;
        
        default: break;
    }

    if(uart == NULL)
    {
        return; 
    }
    else
    {
		while(*tx_buf != '\0')
		{
			FL_UART_WriteTXBuff(uart->uart, *tx_buf++);
			while(FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(uart->uart));		//等待发送完成
//			while(FL_SET != FL_UART_IsActiveFlag_TXBuffEmpty(uart->uart))		//等待发送完成
//			{
//				if(wait_cont++ > 50000)
//				{
//					break;
//				}	
//			}
//			wait_cont = 0;
		}	
    } 
}

/**
 * @description: 获取接收包长度
 * @param  串口的ID
 * @return 已接收的长度
 */
uint16_t uart_get_rx_cnt(uint8_t uart_id)
{
    uart_t *uart;
    uint16_t rx_cnt = 0;
    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;

        default: break;
    }

    if(uart == NULL)
        return 0; 
    if(uart->rx_end == 1)
    {
        taskENTER_CRITICAL();
        uart->rx_end = 0;
        rx_cnt = uart->rx_count;
        taskEXIT_CRITICAL();
    }
    return rx_cnt;
}

/**
 * @description: 串口接收数据
 * @param  串口的ID
 * @param  接收缓存区
 * @param  要接收的最大长度
 * @param  等待时长
 * @return 接收的长度
 */
uint16_t uart_recv_data(uint8_t uart_id, uint8_t *recv_buf ,uint16_t recv_len, uint16_t wait_tick)
{
	uint16_t rx_count = 0,save_count = 0;
    uart_t *uart;
    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;

        default: break;
    }

    if(uart == NULL)
        return 0; 

	wait_tick /= 10;
	while(wait_tick--)   /*等待串口数据接受完毕*/
	{
		//uart_transmit_str(USB_UART, (uint8_t *)"usb\n");
		if((rx_count = uart_get_rx_cnt(uart_id)) != 0)
			break;
		vTaskDelay(10);
	}
	
	if(rx_count > 0)
	{
		memset(recv_buf, 0, recv_len);
		
		if(rx_count >= recv_len)
			rx_count = recv_len;
	
		for(; save_count < rx_count; save_count++)
		{
			recv_buf[save_count] = uart->rx_buf[uart->read_pos];
			if(++uart->read_pos >= uart->rx_size)
			{
				uart->read_pos = 0;
			}
		}
		taskENTER_CRITICAL();
		uart->rx_count -= rx_count;
		taskEXIT_CRITICAL();
		
		recv_buf[save_count] = '\0';
	}
	return rx_count;
}

/**
 * @description: 串口接收并对比
 * @param  串口的ID
 * @param  接收缓存区
 * @param  要对比的buf
 * @param  接收长度
 * @param  接收等待
 * @return 0:接收到的数据存在str 1:串口接收到的数据不存在STR1
 */
uint8_t uart_strstr(uint8_t uart_id, uint8_t *recv_buf ,const char *str, uint16_t rd_size ,uint16_t wait_tick)
{
	uint16_t rx_size = 0;
	uint8_t ret = 1;
			
	wait_tick /= 10;
	while(wait_tick--)
	{
		rx_size = uart_recv_data(uart_id, recv_buf ,rd_size, 10);
		
		if(rx_size)
		{
			trace_log(recv_buf);
			if(strstrlen(recv_buf, (uint8_t *)str , rx_size) == 0)
			{
				ret = 0;
				break;
			}
		}
	}
	return ret;
}
/**
 * @description: 串口接收并对比
 * @param  串口的ID
 * @param  接收缓存区
 * @param  要对比的buf1
 * @param  要对比的buf2
 * @param  接收长度
 * @param  接收等待
 * @return -1:串口接收中不包括buf1和buf2 0:接收到的数据存在str1 1:接收到的数据存在BUF2
 */
int8_t uart_recv_str_double(uint8_t uart_id, uint8_t *recv_buf ,const char *str1, const char *str2, uint16_t rd_size ,uint16_t wait_tick)
{
	uint16_t rx_size = 0;
	int8_t ret = -1;
	
	wait_tick /= 10;
	
	while(wait_tick--)
	{
		rx_size = uart_recv_data(uart_id, recv_buf ,rd_size, 10);
		
		if(rx_size)
		{
			trace_log(recv_buf);
			if(strstrlen(recv_buf, (uint8_t *)str1 , rx_size) == 0)
			{
				ret = 0;
				break;
			}
			if(strstrlen(recv_buf, (uint8_t *)str2 , rx_size) == 0)
			{
				ret = 1;
				break;
			}
		}
	}
	return ret;
}
/**
 * @description: 串口接收并对比
 * @param  串口的ID
 * @param  接收缓存区
 * @param  要对比的buf1
 * @param  要对比的buf2
 * @param  要对比的buf3
 * @param  接收长度
 * @param  接收等待
 * @return -1:串口接收中不包括buf1和buf2 0:接收到的数据存在str1 1:接收到的数据存在str2 2:接收到的数据存在str3
 */
int8_t uart_recv_str_three(uint8_t uart_id, uint8_t *recv_buf ,const char *str1, const char *str2, const char *str3, uint16_t rd_size ,uint16_t wait_tick)
{
	uint16_t rx_size = 0;
	int8_t ret = -1;

	wait_tick /= 10;
	while(wait_tick--)
	{
		rx_size = uart_recv_data(uart_id, recv_buf ,rd_size, 10);
		
		if(rx_size)
		{
			trace_log(recv_buf);
			if(strstrlen(recv_buf, (uint8_t *)str1 , rx_size) == 0)
			{
				ret = 0;
				break;
			}
			if(strstrlen(recv_buf, (uint8_t *)str2 , rx_size) == 0)
			{
				ret = 1;
				break;
			}
			if(strstrlen(recv_buf, (uint8_t *)str3 , rx_size) == 0)
			{
				ret = 2;
				break;
			}
		}
	}
	return ret;
}
/**
 * @description: 串口发送字符串并等待应答
 * @param  串口的ID
 * @param  发送数据
 * @param  接收缓存区
 * @param  对比得字符串
 * @param  接收长度
 * @param  接收等待
 * @return 0:接收到的数据存在str 1:串口接收到的数据不存在STR1
 */
uint8_t uart_send_ack(uint8_t uart_id, void *send_buf, uint8_t *recv_buf ,const char *str, uint16_t rd_size ,uint16_t wait_tick)
{
	uart_transmit_str(uart_id, send_buf);
	
	return uart_strstr(uart_id, recv_buf, str, rd_size, wait_tick);
}

/**
 * @description: 串口发送数据并等待应答
 * @param  串口的ID
 * @param  发送数据
 * @param  发送数据得长度
 * @param  接收缓存区
 * @param  对比得字符串
 * @param  接收长度
 * @param  接收等待
 * @return 0:接收到的数据存在str 1:串口接收到的数据不存在STR1
 */
uint8_t uart_transmit_wait_ack(uint8_t uart_id, void *send_buf, uint16_t send_size,uint8_t *recv_buf ,uint16_t rd_size ,const char *str, uint16_t wait_tick)
{
	if(send_size != 0)
	{
		uart_transmit_buf(uart_id, send_buf, send_size);
	}
	else 
	{
		uart_transmit_str(uart_id, send_buf);
	}
	
	return uart_strstr(uart_id, recv_buf, str, rd_size, wait_tick);
}

/**
 * @description: 清空串口接收缓存区
 * @param  串口的ID
 * @return None
 */
void uart_clear_recv(uint8_t uart_id)
{
    uart_t *uart;
    switch (uart_id)
    {
        case IOT_4G_UART: uart = &iot_uart; break;
		
		case WIFI_UART: uart = &iot_uart;break;
		
		case GPS_UART: uart = &gps_uart;break;

        case USB_UART: uart = &usb_uart; break;
		
		case SENSOR_UART: uart = &sensor_uart;break;

        default: break;
    }

    if(uart == NULL)
        return ; 

	uart->read_pos  = 0;
	uart->write_pos = 0;
	uart->rx_end    = 0;
	uart->rx_count  = 0;
}

/**
 * @description: 日志打印
 * @param  发送得字符串
 * @return None
 */
void trace_log(void *x)
{
	if(debug)
	//if(1)
	{
	    uart_transmit_str(USB_UART, (uint8_t *)x);
	}
}

void trace_enable(uint8_t enable)
{
	debug = enable;
}

uint8_t get_trace_log(void)
{
	return debug;
}

