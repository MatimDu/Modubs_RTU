#include "AD7192.h"


void led_init(void);
void FirstTestRun(void);

extern BoolFlagTypedef  ReceiveDataCompleteFlag;
extern BoolFlagTypedef  SalveTimeOutCheckFlag;
extern unsigned char   ReceiveArrayData[256];
int main(void)
{
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    UartInit();
    ReceiveDataCompleteTimerInit();
    SalveTimeOutCheckTimerInit();
	  ADG1409GpioInit();
	  SpiGpioInit();   
	  AD7192Initialization(); //初始化包括 复位 内部零校准 内部全量程校准
      
    ReceiveDataCompleteFlag =False; 
    SalveTimeOutCheckFlag =False;
    
    while (1)
    {		 

        if(True == ReceiveDataCompleteFlag)
        {
            ReceiveRtuDataFromModbus();
        }   
    }
}

void FirstTestRun(void)
{
	int i;
	//读取寄存器上电后的值
	ReadFromAD7192ViaSPI(REG_COM_STA, 8, AD7192Registers, REG_COM_STA);
	for(i=0; i < 8; i++)
	printf("AD7192Register[%d] = 0x%06X \r\n",i+REG_COM_STA,AD7192Registers[i+REG_COM_STA]);
	//写值到模式寄存器和配置寄存器
	AD7192InternalZeroScaleCalibration();
	AD7192InternalFullScaleCalibration();
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	AD7192Registers[REG_MODE] = MODE_CONT|DAT_STA_DIS|INCLK_MCLK2EN|SINC_4|ENPAR_EN|CLK_DIV_DIS|SINGLECYCLE_DIS|REJ60_DIS|0x080;		//Output Rate =	MCLK/1024/128 without chop
	AD7192Registers[REG_CONF] = CHOP_DIS|REF_IN2|AIN1_AIN2|BURN_DIS|REFDET_DIS|BUF_DIS|UB_BI|GAIN_1;	// Gain = 1
	printf("\r\nWrite 0x0C2080  0x100100 to AD7192 Mode Register and Configuration Register and read back.\r\n");
	WriteToAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);
	//读模式寄存器和配置寄存器
	AD7192Registers[REG_MODE] = 0;
	AD7192Registers[REG_CONF] = 0;
	ReadFromAD7192ViaSPI(REG_MODE, 2, AD7192Registers, REG_MODE);
	printf("Mode Register=0x%06X \r\nConfiguration Register=0x%06X \r\n", AD7192Registers[REG_MODE], AD7192Registers[REG_CONF]);
	//读取寄存器上电后的值
	ReadFromAD7192ViaSPI(REG_COM_STA, 8, AD7192Registers, REG_COM_STA);
	for(i=0; i < 8; i++)
	printf("AD7192Register[%d] = 0x%06X \r\n",i+REG_COM_STA,AD7192Registers[i+REG_COM_STA]);

}

void led_init(void)
{
	GPIO_InitTypeDef GPIO_InitTypeDef;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	
	GPIO_InitTypeDef.GPIO_Pin=GPIO_Pin_9;
	GPIO_InitTypeDef.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitTypeDef.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitTypeDef);
	GPIO_SetBits(GPIOA,GPIO_Pin_9);
}

