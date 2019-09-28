#include "usart.h"
#include "stm32f10x.h"

#if 1    
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((UART4->SR&0X40)==0);//ѭ������,ֱ���������   
    UART4->DR = (u8) ch;      
	return ch;
}
#endif 



#define USARTBaudRate       115200

#define NoneCheck   0
#define EvenCheck   1       //0: OddCheck      1: EvenCheck


extern unsigned char * ReceiveArrayDataPoniter;//���ջ�������ָ��
extern unsigned char ReceiveArrayData[256];
void UartInit()	//printf��ʼ��
{
	GPIO_InitTypeDef	 		GPIO_InitStructure;	//����һ���ṹ�������������ʼ��GPIO
	NVIC_InitTypeDef 			NVIC_InitStructure;	 //�жϽṹ�嶨��
	USART_InitTypeDef  		    USART_InitStructure;	  //���ڽṹ�嶨��

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
 
	USART_InitStructure.USART_BaudRate=USARTBaudRate;   //����������Ϊ115200

    
#if NoneCheck
	USART_InitStructure.USART_StopBits=USART_StopBits_2;
    USART_InitStructure.USART_WordLength=USART_WordLength_8b;//����Ϊ8λ����λ 
    USART_InitStructure.USART_Parity=USART_Parity_No;
#else
#if EvenCheck
    USART_InitStructure.USART_Parity=USART_Parity_Even;
#else
    USART_InitStructure.USART_Parity=USART_Parity_Odd;
#endif
    USART_InitStructure.USART_StopBits=USART_StopBits_1;
    USART_InitStructure.USART_WordLength=USART_WordLength_9b;//����Ϊ9λ����λ 
#endif
	
	USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode=USART_Mode_Rx|USART_Mode_Tx;//�շ�ģʽ
	USART_Init(UART4,&USART_InitStructure);	
	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);//ʹ�ܻ���ʧ��ָ����USART�ж� �����ж�
    
    NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; 
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1; 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; 
	NVIC_Init(&NVIC_InitStructure);
    
	USART_ClearFlag(UART4,USART_FLAG_TC);//���USARTx�Ĵ������־λ
    USART_ClearFlag(UART4,USART_FLAG_RXNE);//���USARTx�Ĵ������־λ	
    USART_Cmd(UART4, ENABLE);
}
void UART4_Send_Byte(u8 Data) //����һ���ֽڣ�
{
	USART_SendData(UART4,Data);
    while(USART_GetFlagStatus(UART4, USART_FLAG_TC) != SET);
}

void UART4_Send_String(u8 *Data) //�����ַ�����
{
	while(*Data)
	UART4_Send_Byte(*Data++);
}


void UART4_IRQHandler(void)	
{   
	if(USART_GetITStatus(UART4,USART_IT_RXNE)!=RESET)//���ָ����USART�����жϷ������	
	{     
        TIM3->CNT=0;
        TIM_Cmd(TIM3,ENABLE); //ʹ��TIMx���� ��ʼ��� ����1.75ms��֡���       
        
        if(ReceiveArrayDataPoniter == ReceiveArrayData+255)       //��ֹԽ��
            *ReceiveArrayDataPoniter=USART_ReceiveData(UART4);
        else
            *ReceiveArrayDataPoniter++=USART_ReceiveData(UART4); //��������
        
         USART_ClearFlag(UART4, USART_IT_RXNE); //�����־λ��       
	}
}
