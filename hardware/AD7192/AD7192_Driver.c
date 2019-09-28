#include "ad7192_driver.h"
#include "stm32f10x.h"
#include "stdio.h"
#include "delay.h"
/********************************************************************************
 Author : CAC (China Applications Support Team) 

 Date :   January, 2012

 File name :   ADuC7026Driver.c

 Description :	 Use the GPIO to simulate the SPI communication of AD7192

 Hardware plateform : 	ADuC7026 and AD7190/92EBZ
********************************************************************************/
void SPI1_Init(void);
u8 SPI1_ReadWriteByte(u8 TxData);
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler);


void SpiGpioInit(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
		
		//推挽输出
	 GPIO_InitStructure.GPIO_Pin = CS|SCLK|SDO|SDI;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 	
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(SPI_PORT, &GPIO_InitStructure);							
	
	 GPIO_InitStructure.GPIO_Pin = SDO;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 	
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	 GPIO_Init(SPI_PORT, &GPIO_InitStructure);
		
//	 GPIO_InitStructure.GPIO_Pin = SYNC;
//	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 	
//	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	 GPIO_Init(SYNC_PORT, &GPIO_InitStructure);	
	
	 //SDO_H;
//   SDI_H;  
	 CS_H;    
	 SCLK_H;

}

//下降沿读 	miso  sdo
//上升沿写  mosi  sdi

void SpiOperation(unsigned char* WriteBuffer, unsigned char *ReadBuffer, unsigned char NumberOfByte)
{

	unsigned	char	WriteData, ReadData;
  unsigned	char	i, j;
	
	SCLK_H;
	for(i=0; i<NumberOfByte; i++)
 	{
	 	WriteData = *(WriteBuffer + i);
		ReadData = 0;

		for(j=0; j<8; j++)
		{					
   		SCLK_H;	
			delay_us(60);		
			if(0x80 == (WriteData & 0x80))
			{
				SDI_H;//Send one to SDI pin
			}
			else
			{
				SDI_L;	  //Send zero to SDI pin
			}
			WriteData = WriteData << 1;	
			delay_us(80);
			SCLK_L;			
			ReadData = (ReadData<<1) | SDO_V;
			delay_us(60);	
			
		}
		*(ReadBuffer + i)= ReadData;
	}  
	
	SCLK_H;
}


//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{		
		u8 retry=0;				 	
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
		{
			retry++;
			if(retry>200)return 0;
		}			  
		SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
		retry=0;

		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
		{
			retry++;
			if(retry>200)return 0;
		}	  						    
		return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据					    
}
