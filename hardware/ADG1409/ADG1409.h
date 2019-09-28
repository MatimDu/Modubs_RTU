#ifndef ADG1409_H
#define ADG1409_H


#define  ADG1409_PORT 	GPIOC

#define  MUX_EN					GPIO_Pin_2
#define  MUX_A0					GPIO_Pin_0
#define  MUX_A1					GPIO_Pin_1

typedef enum
{
	ADG1409ModeOff=0,
	ADG1409ModeOne=1,
	ADG1409ModeTwo=2,
	ADG1409ModeThree=3,
	ADG1409ModeFour=4,
}
ADG1409ModeTypedef;

void ADG1409GpioInit(void);
void ADG1409ModeChooseFunction(ADG1409ModeTypedef);





#endif
