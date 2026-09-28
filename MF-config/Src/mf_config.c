/**
******************************************************************************
* @file           : mf_config.c
* @brief          : MCU FUNCTION CONFIG               
******************************************************************************
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
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "mf_config.h"

/* Private function prototypes -----------------------------------------------*/

 
/**
  * @brief  GPIO Initialization function
  * @param  void
  * @retval None
  */
void MF_GPIO_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    /* PA1 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA5 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA7 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA8 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA11 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PA12 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* PB6 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB12 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_12;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PC0 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_0;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC6 OUTPUT */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_6;
    GPIO_InitStruct.mode = FL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);    
}
 
/**
  * @brief  PMU Initialization function
  * @param  void
  * @retval None
  */
void MF_PMU_Init(void)
{
    FL_PMU_SleepInitTypeDef    PMU_InitStruct;

    PMU_InitStruct.powerMode = FL_PMU_POWER_MODE_SLEEP_OR_DEEPSLEEP;
    PMU_InitStruct.deepSleep = FL_PMU_SLEEP_MODE_NORMAL;
    PMU_InitStruct.LDOLowPowerMode = FL_PMU_LDO_LPM_DISABLE;
    PMU_InitStruct.wakeupFrequency = FL_PMU_RCHF_WAKEUP_FREQ_8MHZ;
    PMU_InitStruct.wakeupDelay = FL_PMU_WAKEUP_DELAY_0US;
    PMU_InitStruct.coreVoltageScaling = FL_ENABLE;

    FL_PMU_Sleep_Init(PMU, &PMU_InitStruct);    
}
 
/**
  * @brief  ADC_Common Initialization function
  * @param  void
  * @retval None
  */
void MF_ADC_Common_Init(void)
{
    FL_ADC_CommonInitTypeDef    Common_InitStruct;

    Common_InitStruct.clockSource = FL_CMU_ADC_CLK_SOURCE_RCHF;
    Common_InitStruct.clockPrescaler = FL_ADC_CLK_PSC_DIV8;
    Common_InitStruct.referenceSource = FL_ADC_REF_SOURCE_VDDA;
    Common_InitStruct.bitWidth = FL_ADC_BIT_WIDTH_12B;

    FL_ADC_CommonInit(&Common_InitStruct);    
}
 
/**
  * @brief  ADC_Sampling Initialization function
  * @param  void
  * @retval None
  */
void MF_ADC_Sampling_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_ADC_InitTypeDef    Sampling_InitStruct;

    /* PD11 ADC_IN0 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_ANALOG;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    Sampling_InitStruct.conversionMode = FL_ADC_CONV_MODE_SINGLE;
    Sampling_InitStruct.autoMode = FL_ADC_SINGLE_CONV_MODE_AUTO;
    Sampling_InitStruct.waitMode = FL_ENABLE;
    Sampling_InitStruct.overrunMode = FL_ENABLE;
    Sampling_InitStruct.scanDirection = FL_ADC_SEQ_SCAN_DIR_FORWARD;
    Sampling_InitStruct.externalTrigConv = FL_ADC_TRIGGER_EDGE_NONE;
    Sampling_InitStruct.triggerSource = FL_ADC_TRGI_LUT0;
    Sampling_InitStruct.fastChannelTime = FL_ADC_FAST_CH_SAMPLING_TIME_2_ADCCLK;
    Sampling_InitStruct.lowChannelTime = FL_ADC_SLOW_CH_SAMPLING_TIME_512_ADCCLK;
    Sampling_InitStruct.oversamplingMode = FL_DISABLE;
    Sampling_InitStruct.overSampingMultiplier = FL_ADC_OVERSAMPLING_MUL_128X;
    Sampling_InitStruct.oversamplingShift = FL_ADC_OVERSAMPLING_SHIFT_8B;

    FL_ADC_Init(ADC, &Sampling_InitStruct);

    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_INTERNAL_VREF1P2);

    FL_ADC_EnableSequencerChannel(ADC, FL_ADC_EXTERNAL_CH0);    
}
 
/**
  * @brief  BSTIM32_TimerBase Initialization function
  * @param  void
  * @retval None
  */
void MF_BSTIM32_TimerBase_Init(void)
{
    FL_BSTIM32_InitTypeDef    TimerBase_InitStruct;

    TimerBase_InitStruct.prescaler = 7999;
    TimerBase_InitStruct.autoReload = 20;
    TimerBase_InitStruct.autoReloadState = FL_ENABLE;
    TimerBase_InitStruct.clockSource = FL_CMU_BSTIM32_CLK_SOURCE_APBCLK;

    FL_BSTIM32_Init(BSTIM32, &TimerBase_InitStruct);    
}
 
/**
  * @brief  BSTIM32_TriggerOutput Initialization function
  * @param  void
  * @retval None
  */
void MF_BSTIM32_TriggerOutput_Init(void)
{

    FL_BSTIM32_SetTriggerOutput(BSTIM32, FL_BSTIM32_TRGO_UPDATE);    
}
 
/**
  * @brief  BSTIM32 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_BSTIM32_Interrupt_Init(void)
{
    FL_BSTIM32_ClearFlag_Update(BSTIM32);
    FL_BSTIM32_EnableIT_Update(BSTIM32);  
}
 
/**
  * @brief  DMA_Channel2 Initialization function
  * @param  void
  * @retval None
  */
void MF_DMA_Channel2_Init(void)
{
    FL_DMA_InitTypeDef    DMA_InitStruct;

    DMA_InitStruct.circMode = FL_DISABLE;
    DMA_InitStruct.dataSize = FL_DMA_BANDWIDTH_8B;
    DMA_InitStruct.direction = FL_DMA_DIR_RAM_TO_PERIPHERAL;
    DMA_InitStruct.periphAddress = FL_DMA_PERIPHERAL_FUNCTION1;
    DMA_InitStruct.priority = FL_DMA_PRIORITY_HIGH;
    DMA_InitStruct.memoryAddressIncMode = FL_DMA_MEMORY_INC_MODE_INCREASE;
    DMA_InitStruct.flashAddressIncMode = FL_DMA_CH7_FLASH_INC_MODE_INCREASE;

    FL_DMA_Init(DMA, &DMA_InitStruct, FL_DMA_CHANNEL_2);    
}
 
/**
  * @brief  EXTI_Common Initialization function
  * @param  void
  * @retval None
  */
void MF_EXTI_Common_Init(void)
{
    FL_EXTI_CommonInitTypeDef    Common_InitStruct;

    Common_InitStruct.clockSource = FL_CMU_EXTI_CLK_SOURCE_LSCLK;

    FL_EXTI_CommonInit(&Common_InitStruct);    
}
 
/**
  * @brief  EXTI_Line2 Initialization function
  * @param  void
  * @retval None
  */
void MF_EXTI_Line2_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_EXTI_InitTypeDef    EXTI_InitStruct;

    /* PA9 EXTI2_Select1 */ 
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
 
/**
  * @brief  EXTI_Line5 Initialization function
  * @param  void
  * @retval None
  */
void MF_EXTI_Line5_Init(void)
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
 
/**
  * @brief  EXTI_Line8 Initialization function
  * @param  void
  * @retval None
  */
void MF_EXTI_Line8_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_EXTI_InitTypeDef    EXTI_InitStruct;

    /* PC1 EXTI8_Select1 */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_1;
    GPIO_InitStruct.mode = FL_GPIO_MODE_INPUT;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    EXTI_InitStruct.filter = FL_DISABLE;
    EXTI_InitStruct.input = FL_GPIO_EXTI_INPUT_GROUP1;
    EXTI_InitStruct.triggerEdge = FL_GPIO_EXTI_TRIGGER_EDGE_FALLING;

    FL_EXTI_Init(FL_GPIO_EXTI_LINE_8, &EXTI_InitStruct);    
}
 
/**
  * @brief  LPUART2 Initialization function
  * @param  void
  * @retval None
  */
void MF_LPUART2_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_LPUART_InitTypeDef    LPUART2_InitStruct;

    /* PB4 LPUART2_RX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_4;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB5 LPUART2_TX */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_5;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    LPUART2_InitStruct.baudRate = FL_LPUART_BAUDRATE_9600;
    LPUART2_InitStruct.dataWidth = FL_LPUART_DATA_WIDTH_8B;
    LPUART2_InitStruct.stopBits = FL_LPUART_STOP_BIT_WIDTH_1B;
    LPUART2_InitStruct.parity = FL_LPUART_PARITY_NONE;
    LPUART2_InitStruct.transferDirection = FL_LPUART_DIRECTION_TX_RX;
    LPUART2_InitStruct.clockSrc = FL_CMU_LPUART_CLK_SOURCE_LSCLK;

    FL_LPUART_Init(LPUART2, &LPUART2_InitStruct);    
}
 
/**
  * @brief  LPUART2 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_LPUART2_Interrupt_Init(void)
{
    FL_LPUART_ClearFlag_RXBuffFull(LPUART2);
    FL_LPUART_EnableIT_RXBuffFull(LPUART2);  
}
 
/**
  * @brief  SPI0 Initialization function
  * @param  void
  * @retval None
  */
void MF_SPI0_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_SPI_InitTypeDef    SPI0_InitStruct;

    /* PB8 SPI0_SSN */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB9 SPI0_SCK */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB10 SPI0_MISO */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_DISABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* PB11 SPI0_MOSI */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_11;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    SPI0_InitStruct.softControl = FL_DISABLE;
    SPI0_InitStruct.mode = FL_SPI_WORK_MODE_MASTER;
    SPI0_InitStruct.baudRate = FL_SPI_BAUDRATE_DIV2;
    SPI0_InitStruct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    SPI0_InitStruct.dataWidth = FL_SPI_DATA_WIDTH_8B;
    SPI0_InitStruct.clockPhase = FL_SPI_PHASE_EDGE1;
    SPI0_InitStruct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    SPI0_InitStruct.clockPolarity = FL_SPI_POLARITY_NORMAL;

    FL_SPI_Init(SPI0, &SPI0_InitStruct);    
}
 
/**
  * @brief  SPI2 Initialization function
  * @param  void
  * @retval None
  */
void MF_SPI2_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_SPI_InitTypeDef    SPI2_InitStruct;

    /* PC7 SPI2_SSN */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_7;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC8 SPI2_SCK */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_8;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC9 SPI2_MISO */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_9;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* PC10 SPI2_MOSI */ 
    GPIO_InitStruct.pin = FL_GPIO_PIN_10;
    GPIO_InitStruct.mode = FL_GPIO_MODE_DIGITAL;
    GPIO_InitStruct.outputType = FL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.pull = FL_ENABLE;
    GPIO_InitStruct.remapPin = FL_DISABLE;
    GPIO_InitStruct.analogSwitch = FL_DISABLE;
    FL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    SPI2_InitStruct.softControl = FL_ENABLE;
    SPI2_InitStruct.mode = FL_SPI_WORK_MODE_MASTER;
    SPI2_InitStruct.baudRate = FL_SPI_BAUDRATE_DIV2;
    SPI2_InitStruct.bitOrder = FL_SPI_BIT_ORDER_MSB_FIRST;
    SPI2_InitStruct.dataWidth = FL_SPI_DATA_WIDTH_8B;
    SPI2_InitStruct.clockPhase = FL_SPI_PHASE_EDGE1;
    SPI2_InitStruct.transferMode = FL_SPI_TRANSFER_MODE_FULL_DUPLEX;
    SPI2_InitStruct.clockPolarity = FL_SPI_POLARITY_NORMAL;

    FL_SPI_Init(SPI2, &SPI2_InitStruct);    
}
 
/**
  * @brief  UART0 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART0_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART0_InitStruct;

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

    UART0_InitStruct.clockSrc = FL_CMU_UART0_CLK_SOURCE_APBCLK;
    UART0_InitStruct.baudRate = 115200;
    UART0_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART0_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART0_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART0_InitStruct.parity = FL_UART_PARITY_NONE;

    FL_UART_Init(UART0, &UART0_InitStruct);    
}
 
/**
  * @brief  UART0 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART0_Interrupt_Init(void)
{
    FL_UART_ClearFlag_RXBuffTimeout(UART0);
    FL_UART_EnableIT_RXTimeout(UART0);

    FL_UART_ClearFlag_RXBuffFull(UART0);
    FL_UART_EnableIT_RXBuffFull(UART0);  
}
 
/**
  * @brief  UART1 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART1_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART1_InitStruct;

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

    UART1_InitStruct.clockSrc = FL_CMU_UART1_CLK_SOURCE_APBCLK;
    UART1_InitStruct.baudRate = 115200;
    UART1_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART1_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART1_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART1_InitStruct.parity = FL_UART_PARITY_NONE;

    FL_UART_Init(UART1, &UART1_InitStruct);    
}
 
/**
  * @brief  UART1 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART1_Interrupt_Init(void)
{
    FL_UART_ClearFlag_RXBuffTimeout(UART1);
    FL_UART_EnableIT_RXTimeout(UART1);

    FL_UART_ClearFlag_RXBuffFull(UART1);
    FL_UART_EnableIT_RXBuffFull(UART1);  
}
 
/**
  * @brief  UART4 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART4_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART4_InitStruct;

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

    UART4_InitStruct.clockSrc = NULL;
    UART4_InitStruct.baudRate = 115200;
    UART4_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART4_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART4_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART4_InitStruct.parity = FL_UART_PARITY_NONE;

    FL_UART_Init(UART4, &UART4_InitStruct);    
}
 
/**
  * @brief  UART4 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART4_Interrupt_Init(void)
{
    FL_UART_ClearFlag_RXBuffFull(UART4);
    FL_UART_EnableIT_RXBuffFull(UART4);  
}
 
/**
  * @brief  UART5 Initialization function
  * @param  void
  * @retval None
  */
void MF_UART5_Init(void)
{
    FL_GPIO_InitTypeDef    GPIO_InitStruct;

    FL_UART_InitTypeDef    UART5_InitStruct;

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

    UART5_InitStruct.clockSrc = NULL;
    UART5_InitStruct.baudRate = 115200;
    UART5_InitStruct.transferDirection = FL_UART_DIRECTION_TX_RX;
    UART5_InitStruct.dataWidth = FL_UART_DATA_WIDTH_8B;
    UART5_InitStruct.stopBits = FL_UART_STOP_BIT_WIDTH_1B;
    UART5_InitStruct.parity = FL_UART_PARITY_NONE;

    FL_UART_Init(UART5, &UART5_InitStruct);    
}
 
/**
  * @brief  UART5 Interrupt Initialization function
  * @param  void
  * @retval None
  */
void MF_UART5_Interrupt_Init(void)
{
    FL_UART_ClearFlag_RXBuffFull(UART5);
    FL_UART_EnableIT_RXBuffFull(UART5);  
}
 
/**
  * @brief  NVIC Initialization function
  * @param  void
  * @retval None
  */
void MF_NVIC_Init(void)
{
    FL_NVIC_ConfigTypeDef    InterruptConfigStruct;

    InterruptConfigStruct.preemptPriority = 0x00;
    FL_NVIC_Init(&InterruptConfigStruct, UART0_IRQn);

    InterruptConfigStruct.preemptPriority = 0x01;
    FL_NVIC_Init(&InterruptConfigStruct, UART1_IRQn);

    InterruptConfigStruct.preemptPriority = 0x00;
    FL_NVIC_Init(&InterruptConfigStruct, UART4_IRQn);

    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, UART5_IRQn);

    InterruptConfigStruct.preemptPriority = 0x00;
    FL_NVIC_Init(&InterruptConfigStruct, LPUARTx_IRQn);

    InterruptConfigStruct.preemptPriority = 0x00;
    FL_NVIC_Init(&InterruptConfigStruct, DMA_IRQn);

    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, BSTIM_IRQn);

    InterruptConfigStruct.preemptPriority = 0x02;
    FL_NVIC_Init(&InterruptConfigStruct, GPIO_IRQn);    
}

/**
  * @brief  FDET_IRQHandler
  * @param  void
  * @retval None
  */
void FDET_IRQHandler(void)
{
    //停振检测中断处理
    if((FL_CMU_IsEnabledIT_XTHFFail() == 0x01UL)
        && (FL_CMU_IsActiveFlag_XTHFFail() == 0x01UL))
    {
        FL_CMU_ClearFlag_XTHFFail();
         /*应用处理逻辑*/
    }
}


/**
  * @brief  Clock Initialization.
  * @retval None
  */
void MF_Clock_Init(void)
{
	uint16_t i;
    /* Initial CDIF */
    FL_CDIF_EnableCPUToVAO(CDIF);
    FL_CDIF_EnableVAOToCPU(CDIF);

    /* Initial RTCA Adjust Value */
    FL_CMU_EnableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);
    FL_RTCA_WriteAdjustValue(RTCA, 0);
    FL_CMU_DisableGroup1BusClock(FL_CMU_GROUP1_BUSCLK_RTCA);

    /* Initial RTCB Adjust Value */
    FL_RTCB_WriteAdjustValue(RTCB, 0);

    /* Initial XTLF */
    FL_VAO_XTLF_Enable(VAO);
//    FL_DelayMs(1000);
	for(i = 0;i<1000;i++)
	{
		;
	}
	
    /* Initial Low Speed Clock */
    FL_CMU_SetLSCLKClockSource(FL_CMU_LSCLK_CLK_SOURCE_RCLP);

    /* Initial RCHF */
    FL_CMU_RCHF_WriteTrimValue(RCHF24M_TRIM);
    FL_CMU_RCHF_SetFrequency(FL_CMU_RCHF_FREQUENCY_24MHZ);
    FL_CMU_RCHF_Enable();

    /* Initial System Clock */
    FL_FLASH_SetReadWait(FLASH, FL_FLASH_READ_WAIT_0CYCLE);    /* 设置FLASH读等待为 0 个周期 */
    FL_CMU_SetSystemClockSource(FL_CMU_SYSTEM_CLK_SOURCE_RCHF);    /* 设置系统主时钟为 RCHF */
    FL_CMU_SetAHBPrescaler(FL_CMU_AHBCLK_PSC_DIV1);
    FL_CMU_SetAPBPrescaler(FL_CMU_APBCLK_PSC_DIV1);

    /* Initial CDIF Clock Prescaler */
    FL_CDIF_SetPrescaler(CDIF, FL_CDIF_PSC_DIV2);

    SystemCoreClockUpdate(); 
}

void MF_Config_Init(void)
{
//    /* Initial GPIO */
//    MF_GPIO_Init();

//    /* Initial PMU */
//    MF_PMU_Init();

//    /* Initial ADC */
//    MF_ADC_Common_Init();
//    MF_ADC_Sampling_Init();

//    /* Initial BSTIM32 */
//    MF_BSTIM32_TimerBase_Init();
//    MF_BSTIM32_TriggerOutput_Init();
//    MF_BSTIM32_Interrupt_Init();

//    /* Initial DMA */
//    MF_DMA_Channel2_Init();

//    /* Initial EXTI */
//    MF_EXTI_Common_Init();
//    MF_EXTI_Line2_Init();
//    MF_EXTI_Line5_Init();
//    MF_EXTI_Line8_Init();

//    /* Initial LPUART2 */
//    MF_LPUART2_Init();
//    MF_LPUART2_Interrupt_Init();

//    /* Initial SPI0 */
//    MF_SPI0_Init();

//    /* Initial SPI2 */
//    MF_SPI2_Init();

//    /* Initial UART0 */
//    MF_UART0_Init();
//    MF_UART0_Interrupt_Init();

//    /* Initial UART1 */
//    MF_UART1_Init();
//    MF_UART1_Interrupt_Init();

//    /* Initial UART4 */
//    MF_UART4_Init();
//    MF_UART4_Interrupt_Init();

//    /* Initial UART5 */
//    MF_UART5_Init();
//    MF_UART5_Interrupt_Init();

//    /* Initial NVIC */
//    MF_NVIC_Init();
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{

}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 

}
#endif /* USE_FULL_ASSERT */

/*************************(C) COPYRIGHT Fudan Microelectronics **** END OF FILE*************************/
