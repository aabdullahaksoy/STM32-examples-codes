// STM32 ADC IQ Sample @ 50 KHz (PC.1, PC.2)
 
// Assumptions per system_stm32f4xx.c CPU @ 168 MHz, APB2 @ 84 MHz (/2), APB1 @ 42 MHz (/4)
//HSE_VALUE = 8000000
#include <stm32f4xx.h>
#include <stm32f4xx_conf.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_adc.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_dma.h>
#include <stm32f4xx_tim.h> 
#include <misc.h> 
/**************************************************************************************/
 
char dum1=100; 
void RCC_Configuration(void)
{
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD,ENABLE);
}
 
/**************************************************************************************/
 
void GPIO_Configuration(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
 
  /* ADC Channel 11 -> PC1
     ADC Channel 12 -> PC2
  */
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
}
 
/**************************************************************************************/
 
void ADC_Configuration(void)
{
  ADC_CommonInitTypeDef ADC_CommonInitStructure;
  ADC_InitTypeDef ADC_InitStructure;
 
  /* ADC Common Init */
  ADC_CommonInitStructure.ADC_Mode = ADC_DualMode_RegSimult;
  ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div2;
  ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1; // Once ADC1 sonra ADC2 orneklenip ard arda aliniyor
  ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
  ADC_CommonInit(&ADC_CommonInitStructure);
 
  ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
  ADC_InitStructure.ADC_ScanConvMode = DISABLE; // 1 Channel
  ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Conversions Triggered
  ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_Rising;
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T2_TRGO;
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  ADC_InitStructure.ADC_NbrOfConversion = 1;
  ADC_Init(ADC1, &ADC_InitStructure);
  ADC_Init(ADC2, &ADC_InitStructure); //  ADC2 icin ayni ayarlar
 
  /* ADC1 regular channel 11 configuration */
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 1, ADC_SampleTime_15Cycles); // PC1
 
  /* ADC2 regular channel 12 configuration */
  ADC_RegularChannelConfig(ADC2, ADC_Channel_12, 1, ADC_SampleTime_15Cycles); // PC2
 
  /* Enable DMA request after last transfer (Multi-ADC mode)  */
  ADC_MultiModeDMARequestAfterLastTransferCmd(ENABLE);
 
  /* Enable ADC1 */
  ADC_Cmd(ADC1, ENABLE);
 
  /* Enable ADC2 */
  ADC_Cmd(ADC2, ENABLE);
}
 
/**************************************************************************************/
 
#define BUFFERSIZE 800 // I+Q 50KHz x2 at 0.5 KHz
 
short int ADCDualConvertedValues[BUFFERSIZE]; // Filled as pairs ADC1, ADC2
 
static void DMA_Configuration(void)
{
  DMA_InitTypeDef DMA_InitStructure;
 
  DMA_InitStructure.DMA_Channel = DMA_Channel_0;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&ADCDualConvertedValues[0];
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)0x40012308; // CDR_ADDRESS; Packed ADC1, ADC2
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_BufferSize = BUFFERSIZE; // Count of 16-bit words
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream0, &DMA_InitStructure);
 
  /* Enable DMA Stream Half / Transfer Complete interrupt */
  DMA_ITConfig(DMA2_Stream0, DMA_IT_TC, ENABLE);
 
  /* DMA2_Stream0 enable */
  DMA_Cmd(DMA2_Stream0, ENABLE);
}
 
/**************************************************************************************/
 
void TIM2_Configuration(void)
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
 
  /* Time base configuration */
  TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
  TIM_TimeBaseStructure.TIM_Period = (84000000 / 50000) - 1; // 50 KHz, from 84 MHz TIM2CLK (ie APB1 = HCLK/4, TIM2CLK = HCLK/2)
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
 
  /* TIM2 TRGO selection */
  TIM_SelectOutputTrigger(TIM2, TIM_TRGOSource_Update); // ADC_ExternalTrigConv_T2_TRGO
 
  /* TIM2 enable counter */
  TIM_Cmd(TIM2, ENABLE);
}
 
/**************************************************************************************/
 
void NVIC_Configuration(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
 
  /* Enable the DMA Stream IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}
 
/**************************************************************************************/
 
void DMA2_Stream0_IRQHandler(void) // Called at 1 KHz for 200 KHz sample rate, LED Toggles at 200 KHz
{
  /* Test on DMA Stream Transfer Complete interrupt */
  if(DMA_GetITStatus(DMA2_Stream0, DMA_IT_TCIF0))
  {
    /* Clear DMA Stream Transfer Complete interrupt pending bit */
    DMA_ClearITPendingBit(DMA2_Stream0, DMA_IT_TCIF0);
 
    /* Toggle LED3 on: End of Transfer */
		if (!dum1--){
			GPIO_ToggleBits(GPIOD, GPIO_Pin_15);
			dum1=100;
		}

  }
}
 
/**************************************************************************************/
 
int main(void)
{
	GPIO_InitTypeDef LED; //Structure for signal led
	
  RCC_Configuration();
 
  GPIO_Configuration();
 
  NVIC_Configuration();
 
  TIM2_Configuration();
 
  DMA_Configuration();
 
  ADC_Configuration();
	
  GPIO_StructInit(&LED);
	LED.GPIO_Pin = GPIO_Pin_15;//
	LED.GPIO_Mode = GPIO_Mode_OUT; //
	GPIO_Init(GPIOD,&LED);//
 
  GPIO_SetBits(GPIOD, GPIO_Pin_15); /* Turn LED on/off, 500 Hz means it working */
 
  /* Start ADC1 Software Conversion */
  ADC_SoftwareStartConv(ADC1);
 
  while(1); // Don't want to exit
	}
