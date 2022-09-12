#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
float temp;
// firstly define a timer
int msTicks;
void RCC_Configuration(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);
}
void GPIO_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
}
void ADC_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_NbrOfConversion = 1;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_Cmd(ADC2,ENABLE);
	ADC_RegularChannelConfig(ADC2,ADC_Channel_6,1,ADC_SampleTime_144Cycles);
}
void Systick_Handler(void)
{
	msTicks ++;
	ADC_SoftwareStartConv(ADC2);
}

int main()
{
	RCC_Configuration();
	GPIO_Configuration();
	ADC_Configuration();

	SystemCoreClockUpdate();
	if(SystemCoreClock/1000)
	{
		while(1);
	}
	
	while(1)
	{
		if(ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC))
		{
			temp = ADC_GetConversionValue(ADC2)*(float)(3.0/4096);
		}
	}
}
