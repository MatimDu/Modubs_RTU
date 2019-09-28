#include "time.h"
#include "Modbus.h"

extern BoolFlagTypedef  ReceiveDataCompleteFlag ;//一帧数据接收完成
extern BoolFlagTypedef  SalveTimeOutCheckFlag;

/*******************************************************************************
* 函 数 名         : ReceiveDataCompleteTimerInit
* 函数功能		   : 定时器3端口初始化函数	   
* 输    入         : 无
* 输    出         : 无
* 说    明 ：帧结束检测 当串口接收到数据后开启定时器 若在一定时间没有接收到数据 则判定位帧结束
***********************************************************************************/
void ReceiveDataCompleteTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	 //声明一个结构体变量，用来初始化定时器

	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开启定时器3时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);

	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);//清除TIMx的中断待处理位:TIM 中断源
	TIM_TimeBaseInitStructure.TIM_Period = 17;//设置自动重装载寄存器周期的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 3599;//设置用来作为TIMx时钟频率预分频值，10khz计数频率
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1 ; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInit(TIM3,&TIM_TimeBaseInitStructure);
	
	TIM_Cmd(TIM3,DISABLE); //使能或者失能TIMx外设
	/* 设置中断参数，并打开中断 */
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE );	//使能或者失能指定的TIM中断
       
	/* 设置NVIC参数 */
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn; //打开TIM3_IRQn的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0;  //响应优先级为0
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能
	NVIC_Init(&NVIC_InitStructure);	
    
    TIM_ClearFlag(TIM3,TIM_FLAG_Update);
}
void TIM3_IRQHandler()	  //定时器3中断函数
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
* 函 数 名         : SalveTimeOutCheckTimerInit
* 函数功能		   : 定时器2端口初始化函数	   
* 输    入         : 无
* 输    出         : 无
* 说    明         ：等待从设备操作 超时则操作失败 
***********************************************************************************/
void SalveTimeOutCheckTimerInit(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;	 //声明一个结构体变量，用来初始化定时器

	NVIC_InitTypeDef NVIC_InitStructure;

	/* 开启定时器2时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);

	TIM_ClearITPendingBit(TIM2,TIM_IT_Update);//清除TIMx的中断待处理位:TIM 中断源
	TIM_TimeBaseInitStructure.TIM_Period = 1000;//设置自动重装载寄存器周期的值
	TIM_TimeBaseInitStructure.TIM_Prescaler = 35999;//设置用来作为TIMx时钟频率预分频值，1khz计数频率
	TIM_TimeBaseInitStructure.TIM_ClockDivision =TIM_CKD_DIV1 ; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;//TIM向上计数模式
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStructure);
		
	/* 设置中断参数，并打开中断 */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE );	//使能或者失能指定的TIM中断
    
    TIM_Cmd(TIM2,DISABLE); //使能或者失能TIMx外设  

	/* 设置NVIC参数 */
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn; //打开TIM2_IRQn的全局中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;	//抢占优先级为0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;  //响应优先级为1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;	//使能
	NVIC_Init(&NVIC_InitStructure);	
    
    TIM_ClearFlag(TIM2,TIM_FLAG_Update);
}
void TIM2_IRQHandler()	  //定时器3中断函数
{
    if(TIM_GetITStatus(TIM2,TIM_FLAG_Update))
    {
        SalveTimeOutCheckFlag=True; //标识操作超时
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
