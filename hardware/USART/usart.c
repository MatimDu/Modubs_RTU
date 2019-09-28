#include "usart.h"
#include "stm32f10x.h"

#if 1    
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((UART4->SR&0X40)==0);//循环发送,直到发送完毕   
    UART4->DR = (u8) ch;      
	return ch;
}
#endif 



#define USARTBaudRate       115200

#define NoneCheck   0
#define EvenCheck   1       //0: OddCheck      1: EvenCheck


extern unsigned char * ReceiveArrayDataPoniter;//接收缓存区的指针
extern unsigned char ReceiveArrayData[256];
void UartInit()	//printf初始化
{
	GPIO_InitTypeDef	 		GPIO_InitStructure;	//声明一个结构体变量，用来初始化GPIO
	NVIC_InitTypeDef 			NVIC_InitStructure;	 //中断结构体定义
	USART_InitTypeDef  		    USART_InitStructure;	  //串口结构体定义

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_AFIO,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4,ENABLE);

	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;//TX
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;//RX
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
 
	USART_InitStructure.USART_BaudRate=USARTBaudRate;   //波特率设置为115200

    
#if NoneCheck
	USART_InitStructure.USART_StopBits=USART_StopBits_2;
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;//这里为8位数据位 
    USART_InitStructure.USART_Parity=USART_Parity_No;
#else
#if EvenCheck
    USART_InitStructure.USART_Parity=USART_Parity_Even;
#else
    USART_InitStructure.USART_Parity=USART_Parity_Odd;
#endif
    USART_InitStructure.USART_StopBits=USART_StopBits_1;
    USART_InitStructure.USART_WordLength=USART_WordLength_9b;//这里为9位数据位 
#endif
	
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;//收发模式
	USART_Init(UART4,&USART_InitStructure);	
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//使能或者失能指定的USART中断 接收中断
    
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);
    
	USART_ClearFlag(UART4,USART_FLAG_TC);//清除USARTx的待处理标志位
    USART_ClearFlag(UART4,USART_FLAG_RXNE);//清除USARTx的待处理标志位	
    USART_Cmd(UART4, ENABLE);
}
void UART4_Send_Byte(u8 Data) //发送一个字节；
{
	USART_SendData(UART4,Data);
    while(USART_GetFlagStatus(UART4, USART_FLAG_TC) != SET);
}

void UART4_Send_String(u8 *Data) //发送字符串；
{
	while(*Data)
	UART4_Send_Byte(*Data++);
}


void UART4_IRQHandler(void)	
{   
	if(USART_GetITStatus(UART4,USART_IT_RXNE)!=RESET)//检查指定的USART接收中断发生与否	
	{     
        TIM3->CNT=0;
        TIM_Cmd(TIM3,ENABLE); //使能TIMx外设 开始检测 超过1.75ms则帧完成       
        
        if(ReceiveArrayDataPoniter == ReceiveArrayData+255)       //防止越界
            *ReceiveArrayDataPoniter=USART_ReceiveData(UART4);
        else
            *ReceiveArrayDataPoniter++=USART_ReceiveData(UART4); //接收数据
        
         USART_ClearFlag(UART4, USART_IT_RXNE); //清除标志位；       
	}
}
