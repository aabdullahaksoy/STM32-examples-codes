#include "STM32F4xx.h"
#include <misc.h>
#include <stm32f4xx_rcc.h>
#include <stm32f4xx_spi.h>
#include <stm32f4xx_gpio.h>

	unsigned char dataX,dataY,dataZ;
	unsigned char status;



void mySPI_Init(void);
uint8_t mySPI_GetData(uint8_t);
void mySPI_SendData(uint8_t, uint8_t);


int main(void)
{

	int i=0;
	mySPI_Init();

	//mySPI_SendData(0x20, 0xC0); //LIS302D Config
  mySPI_SendData(0x20, 0x77);  // Data Rate=400Hz, Full Scale=2g, Activate, x,y,z enable 20h adresini 400 hz ile kullaniyor 
  while(!(mySPI_GetData(0x27)&1));   // status register'in birinci byte'i ile and'liyor yani x boyutunda ivme var mi 
	dataX=mySPI_GetData(0x29 <<8)| mySPI_GetData(0x28); // hem 28'den hem de29'dan okuma yapmak için
          
	while(!(mySPI_GetData(0x27)&2)); 
	dataY=mySPI_GetData(0x2B);

    while(1)
    {
			status=mySPI_GetData(0x27);   // Statusu ogrenelim. Kim hazir kim degil?
			while (status && 0x08){
				if (status&&0x80)
					i=80;// okuma hizi ivme degisiminden dusuk overrun sorunu
				dataX = mySPI_GetData(0x29);				
				dataY = mySPI_GetData(0x2B);
				dataZ = mySPI_GetData(0x2D);
			i=0;
	 }
    }

}





 
uint8_t mySPI_GetData(uint8_t adress){ //ivme bilgisini hangi register'dan okuyacagiz. x 'deki verileri okumak için 28h ve 29h 

	GPIO_ResetBits(GPIOE, GPIO_Pin_3);

	adress = 0x80 | adress;

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));  //transmit buffer empty?
	SPI_I2S_SendData(SPI1, adress);
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); //data received?
	SPI_I2S_ReceiveData(SPI1);	//Clear RXNE bit

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));  //transmit buffer empty?
	SPI_I2S_SendData(SPI1, 0x00);	//Dummy byte to generate clock
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); //data received?
	GPIO_SetBits(GPIOE, GPIO_Pin_3);

	return SPI_I2S_ReceiveData(SPI1); //return reveiced data
}

void mySPI_SendData(uint8_t adress, uint8_t data){ //önce adresi gönder sonra datayi 

	GPIO_ResetBits(GPIOE, GPIO_Pin_3); // clock'un düsen kenarinda okudugumuz için sifir yapiyoruz 

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));  //transmit buffer empty?
	SPI_I2S_SendData(SPI1, adress);
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); //data received?
	SPI_I2S_ReceiveData(SPI1);

	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE));  //transmit buffer empty?
	SPI_I2S_SendData(SPI1, data);
	while(!SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE)); //data received?
	SPI_I2S_ReceiveData(SPI1);

	GPIO_SetBits(GPIOE, GPIO_Pin_3);
}


void mySPI_Init(void){
	SPI_InitTypeDef SPI_InitTypeDefStruct;
	GPIO_InitTypeDef GPIO_InitTypeDefStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_InitTypeDefStruct.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitTypeDefStruct.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitTypeDefStruct.SPI_Mode = SPI_Mode_Master;
	SPI_InitTypeDefStruct.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitTypeDefStruct.SPI_NSS = SPI_NSS_Soft; // Slave selection pin SOFTWARE seçiyoruz
	SPI_InitTypeDefStruct.SPI_FirstBit = SPI_FirstBit_MSB; 
	SPI_InitTypeDefStruct.SPI_CPOL = SPI_CPOL_High; //clock polarity'i high 
	SPI_InitTypeDefStruct.SPI_CPHA = SPI_CPHA_2Edge; //clock'un düsen kenarinda veri okunuyor 

	SPI_Init(SPI1, &SPI_InitTypeDefStruct);


	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOE , ENABLE);

	GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_6;
	GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitTypeDefStruct);

	GPIO_InitTypeDefStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitTypeDefStruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitTypeDefStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitTypeDefStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitTypeDefStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOE, &GPIO_InitTypeDefStruct);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);

	GPIO_SetBits(GPIOE, GPIO_Pin_3);


	SPI_Cmd(SPI1, ENABLE);

}