#include "time.h"
#include "Modbus.h"

extern BoolFlagTypedef  ReceiveDataCompleteFlag ;//һ֡���ݽ������
extern BoolFlagTypedef  SalveTimeOutCheckFlag;

/*******************************************************************************
* �� �� ��         : ReceiveDataCompleteTimerInit
* ��������		   : ��ʱ��3�˿ڳ�ʼ������	   
* ��    ��         : ��
* ��    ��         : ��
* ˵    �� ��֡������� �����ڽ��յ����ݺ�����ʱ�� ����һ��ʱ��û�н��յ����� ���ж�λ֡����
***********************************************************************************/
void ReceiveDataCompleteTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	 //����һ���ṹ�������������ʼ����ʱ��

	NVIC_InitTypeDef NVIC_InitStructure;

	/* ������ʱ��3ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//���TIMx���жϴ�����λ:TIM �ж�Դ
	TIM_TimeBaseInitStructure.TIM_Period = 17;//�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = 3599;//����������ΪTIMxʱ��Ƶ��Ԥ��Ƶֵ��10khz����Ƶ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1 ; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_Cmd(TIM3,DISABLE); //ʹ�ܻ���ʧ��TIMx����
	/* �����жϲ����������ж� */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );	//ʹ�ܻ���ʧ��ָ����TIM�ж�
       
	/* ����NVIC���� */
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //��TIM3_IRQn��ȫ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//��ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;  //��Ӧ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//ʹ��
	NVIC_Init(&NVIC_InitStructure);	
    
    TIM_ClearFlag(TIM3,TIM_FLAG_Update);
}
void TIM3_IRQHandler()	  //��ʱ��3�жϺ���
{
    if(TIM_GetITStatus(TIM3,TIM_FLAG_Update))
    {
        ReceiveDataCompleteFlag=True;
        
        TIM_Cmd(TIM3,DISABLE);
        TIM3->CNT=0;      
        TIM_ClearITPendingBit(TIM3,TIM_IT_Update); 
    }   
}

/*******************************************************************************
* �� �� ��         : SalveTimeOutCheckTimerInit
* ��������		   : ��ʱ��2�˿ڳ�ʼ������	   
* ��    ��         : ��
* ��    ��         : ��
* ˵    ��         ���ȴ����豸���� ��ʱ�����ʧ�� 
***********************************************************************************/
void SalveTimeOutCheckTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	 //����һ���ṹ�������������ʼ����ʱ��

	NVIC_InitTypeDef NVIC_InitStructure;

	/* ������ʱ��2ʱ�� */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//���TIMx���жϴ�����λ:TIM �ж�Դ
	TIM_TimeBaseInitStructure.TIM_Period = 1000;//�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseInitStructure.TIM_Prescaler = 35999;//����������ΪTIMxʱ��Ƶ��Ԥ��Ƶֵ��1khz����Ƶ��
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1 ; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
		
	/* �����жϲ����������ж� */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );	//ʹ�ܻ���ʧ��ָ����TIM�ж�
    
    TIM_Cmd(TIM2,DISABLE); //ʹ�ܻ���ʧ��TIMx����  

	/* ����NVIC���� */
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //��TIM2_IRQn��ȫ���ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//��ռ���ȼ�Ϊ0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;  //��Ӧ���ȼ�Ϊ1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//ʹ��
	NVIC_Init(&NVIC_InitStructure);	
    
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
}
void TIM2_IRQHandler()	  //��ʱ��3�жϺ���
{
    if(TIM_GetITStatus(TIM2,TIM_FLAG_Update))
    {
        SalveTimeOutCheckFlag=True; //��ʶ������ʱ
        TIM2->CNT=0;
        TIM_Cmd(TIM2,DISABLE);
        TIM_ClearITPendingBit(TIM2,TIM_IT_Update); 
    }   
}

void  TimerInit(void)
{
     SalveTimeOutCheckTimerInit();
     ReceiveDataCompleteTimerInit();
}
