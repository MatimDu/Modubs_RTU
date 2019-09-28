#include "ADG1409.h"
#include "stm32f10x.h"
//初始化使能 选择模式1
void ADG1409GpioInit(void)
{
	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 
		
	//推挽输出
	 GPIO_InitStructure.GPIO_Pin = MUX_EN|MUX_A1|MUX_A0;				
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 
	 GPIO_Init(ADG1409_PORT, &GPIO_InitStructure);		
	
	//初始化使能 选择模式1
	 GPIO_SetBits(ADG1409_PORT,MUX_EN);			
	 GPIO_ResetBits(ADG1409_PORT,MUX_A1|MUX_A0);							
}

/***********************************************************************
*       ADG1409 Truth Table
*  		A1		    	A0			    EN			    On Switch Pair
*		x				x				0					NONE
*		0				0				1					 1
*		0				1				1					 2
*		1				0				1					 3
*		1				1				1					 4
*****************************************************************/

void ADG1409ModeChooseFunction(ADG1409ModeTypedef ADG1409ModeChoose)
{
	GPIO_SetBits(ADG1409_PORT,MUX_EN);	
    
	switch(ADG1409ModeChoose)
	{
		case ADG1409ModeOne:            
			GPIO_ResetBits(ADG1409_PORT,MUX_A1|MUX_A0);	
		break;		
		case ADG1409ModeTwo:
			GPIO_ResetBits(ADG1409_PORT,MUX_A1);	
			GPIO_SetBits(ADG1409_PORT,MUX_A0);
		break;
		case ADG1409ModeThree:
			GPIO_SetBits(ADG1409_PORT,MUX_A1);
			GPIO_ResetBits(ADG1409_PORT,MUX_A0);				
		break;
		case ADG1409ModeFour:
			GPIO_SetBits(ADG1409_PORT,MUX_A1|MUX_A0);
		break;	
		case ADG1409ModeOff:
			GPIO_ResetBits(ADG1409_PORT,MUX_EN);
        break;
		default:
			GPIO_ResetBits(ADG1409_PORT,MUX_A1|MUX_A0);
		break;
	}
}

