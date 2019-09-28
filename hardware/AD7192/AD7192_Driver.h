#ifndef AD7192_DRIVER_H
#define AD7192_DRIVER_H
/********************************************************************************
 Author : CAC (China Applications Support Team) 

 Date :   January, 2012

 File name :   ADuC7026Driver.h

 Description :	 Use the GPIO to simulate the SPI communication of AD7192

 Hardware plateform : 	ADuC7026 and AD7190/92EBZ
********************************************************************************/

/*
#define CS			0x40	// pin CS = PE10
#define SYNC		0x42	// pin SYNC  = PA4//¬ﬂº≠ ‰»Î
#define SCLK		0x44	// pin SCLK  = PE11
#define	SDI			0x45	// pin SDI   = PE12//DIN
#define	SDO			0x05	// pin SDO   = PE13//DOUT
*/
//GPIO Define

#define SPI_PORT		GPIOA

#define CS					GPIO_Pin_4	// pin CS 	 = PA4

#define SCLK				GPIO_Pin_5	// pin SCLK  = PA5
#define	SDO					GPIO_Pin_6	// pin SDO   = PA6//DOUT
#define	SDI					GPIO_Pin_7	// pin SDI   = PA7//DIN

#define SYNC_PORT		GPIOB
#define SYNC				GPIO_Pin_0	// pin SYNC  = PB0//¬ﬂº≠ ‰»Î

#define SDO_H       GPIO_SetBits(SPI_PORT, SDO)  
#define SDO_L       GPIO_ResetBits(SPI_PORT, SDO)  

#define SDI_H       GPIO_SetBits(SPI_PORT, SDI)  
#define SDI_L       GPIO_ResetBits(SPI_PORT, SDI)  

#define CS_H        GPIO_SetBits(SPI_PORT, CS) 
#define CS_L        GPIO_ResetBits(SPI_PORT, CS) 

#define SCLK_H        GPIO_SetBits(SPI_PORT, SCLK) 
#define SCLK_L        GPIO_ResetBits(SPI_PORT, SCLK)

#define SDI_V       GPIO_ReadInputDataBit(SPI_PORT, SDI)   
#define SDO_V       GPIO_ReadInputDataBit(SPI_PORT, SDO)   

#define SYNC_H      GPIO_SetBits(SYNC_PORT, SYNC) 
#define SYNC_L      GPIO_ResetBits(SYNC_PORT, SYNC)
	
void SpiOperation(unsigned char* WriteBuffer, unsigned char *ReadBuffer, unsigned char NumberOfByte);
void SpiGpioInit(void);

#endif
