#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_adc.h"
float temp;
//CMSIS
// load the ADC.c,GPIO.c,RCC.c
// load the header files to PATH
// include the header files


int main()
{
	GPIO_InitTypeDef PIN;	// PIN STRUUCT IS SET
	ADC_InitTypeDef ADCAYAR;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);	//RCC->APB2ENR |= (1<<8); C DOSYASINDAN RCC'YI EKLE --> void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState NewState)
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC,ENABLE);//RCC->AHB1ENR |= 0x00000002 PORTC
	
	//PORT AYARLARKEN STRUCTURE AYARLA GPIO ILE ILGILI (GPIO_INIT)
	
	
	PIN.GPIO_Pin = GPIO_Pin_4; // PC4 IS ACTIVATED - BIRDEN FAZLA PIN KULLANMAK ISTERSEK OR KAPISI KULLANIRIZ
	PIN.GPIO_Mode = GPIO_Mode_AN;
	
	//BU AYARLAMALARI PORT C'YE AKTARMAMIZ LAZIM (GPIO_INIT) GPIO.c'DE
	GPIO_Init(GPIOC,&PIN); //PIN'IN ADRESINI GÖNDERMEMIZ LAZIM 
	
	
	//ADC AYARLARI
	ADCAYAR.ADC_NbrOfConversion = 1;
	ADCAYAR.ADC_DataAlign = ADC_DataAlign_Right; //EN SAGDAN BASLA
	ADCAYAR.ADC_Resolution = ADC_Resolution_12b;
	ADC_Init(ADC1,&ADCAYAR);
	ADC_Cmd(ADC1,ENABLE); // ADC1->CR2 = (1<<0) ADC1'I ENABLE EDIYOR
	
	//ADC1->SQR
	ADC_RegularChannelConfig(ADC1,ADC_Channel_14,1,ADC_SampleTime_112Cycles); // 1--> KACINCI SIRADA OLDUGU 
																																						//2 TANE KANAL OLSAYDI NbrOf = 2
	while(1)
	{
		ADC_SoftwareStartConv(ADC1); // ADC1->CR2 |= (1<<30)
		if(ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC)) //if(ADC->SR & 2)
		{
			temp = (ADC_GetConversionValue(ADC1))*(float)(3.0/4096);
		}

	}		
			
	
	
	
	
	
	
}
