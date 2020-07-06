#include "timer.h"
#include "led.h"
#include "sys.h"
#include "Public_Value.h"

/***********************************************************************
全局变量的Tsk_Flg
************************************************************************/
Task_Flag Tsk_Flg;

static void TskFlg_Init(void);
static void TIM1_Init(u16 arr, u16 psc);


/*******************************************************************
程序功能：定时器模块的初始化
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

void MTIMER_Init(void)
{
	TskFlg_Init();
	TIM1_Init(TIMER_PRSC, TIMER_PERD);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
}

/*******************************************************************
程序功能：定时器轮询标志位的初始化
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void TskFlg_Init(void)
{
	u8 *pTskFlg = (u8*)&Tsk_Flg;
	u8 i;

	for(i = 0; i < sizeof(Tsk_Flg)/sizeof(u8); i++)
	{
		pTskFlg[i] = 0x00;
	}
}

/*******************************************************************
程序功能：高级定时器TIM1初始化
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void TIM1_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //时钟使能
	
	//定时器TIM1初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式 
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;//重复计数设置，这里值决定多少次计数才进入中断，此时计数一次进入中断
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);     //清除中断标志位，防止开启中断就进入一次中断
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE ); //使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM1中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //初始化NVIC寄存器


	TIM_Cmd(TIM1, ENABLE);  //使能TIMx
}

/*******************************************************************
程序功能：高级定时器TIM1的中断处理函数
程序版本：V1.0
日    期： 2019/5/30
作    者：Orange
修    改：无
*******************************************************************/

void TIM1_UP_IRQHandler(void) 
{ 
	static u8 Tsk10ms_Cnt = 0;
	static u8 Tsk20ms_Cnt = 0;
	static u8 Tsk100ms_Cnt = 0;
	//static u8 Tsk1000ms_Cnt = 0;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)//检查指定的TIM中断发生与否:TIM 中断源 
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);//清除TIMx的中断待处理位:TIM 中断源 
		
		/****433M模块&按键模块 10ms任务定时轮询的flag****/
		Tsk10ms_Cnt++;
		if(Tsk10ms_Cnt >= Task1_Cnt)
		{
			Tsk_Flg.USART_Control = 0x01;
			Tsk_Flg.KEY_SCAN = 0x01;
			Tsk10ms_Cnt = 0;
		}
		else
		{
			//do nothing
		}
		
		/*****摇杆挡位的检测模块20ms任务定时轮询的flag*****/
		Tsk20ms_Cnt++;
		if(Tsk20ms_Cnt >= Task2_Cnt)
		{
			Tsk_Flg.ADC_Voltage = 0x01;
			//Tsk_Flg.LED_Display = 0x01;
			Tsk20ms_Cnt = 0;
		}
		else
		{
			//do nothing
		}
		
		/****电量显示模块100ms任务定时轮询的flag****/
		Tsk100ms_Cnt++;
		if(Tsk100ms_Cnt >= Task3_Cnt)
		{
			Tsk_Flg.LED_Display = 0x01;
			Tsk100ms_Cnt = 0;
		}
		else
		{
			//do nothing
		}
	}	     
} 

