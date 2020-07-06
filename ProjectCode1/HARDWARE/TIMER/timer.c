#include "timer.h"
#include "led.h"
#include "sys.h"
#include "Public_Value.h"

/***********************************************************************
ȫ�ֱ�����bit����
------------------------------------------------------------------------
Name							bit					Defination
------------------------------------------------------------------------
Task_Flag	  					7					reserved
								6					reserved
								5					reserved
								4					reserved
								3					USART_Control
								2					KEY_SCAN
								1					LED_Display
								0					ADC_Voltage
------------------------------------------------------------------------
************************************************************************/
Task_Flag Tsk_Flg;

static void TskFlg_Init(void);
static void TIM1_Init(u16 arr, u16 psc);


/*******************************************************************
�����ܣ���ʱ��ģ��ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void MTIMER_Init(void)
{
	TskFlg_Init();
	TIM1_Init(TIMER_PRSC, TIMER_PERD);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
}

/*******************************************************************
�����ܣ���ʱ����ѯ��־λ�ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
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
�����ܣ��߼���ʱ��TIM1��ʼ��
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void TIM1_Init(u16 arr, u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //ʱ��ʹ��
	
	//��ʱ��TIM1��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ 
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;//�ظ��������ã�����ֵ�������ٴμ����Ž����жϣ���ʱ����һ�ν����ж�
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ClearFlag(TIM1, TIM_FLAG_Update);     //����жϱ�־λ����ֹ�����жϾͽ���һ���ж�
	TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM3�ж�,��������ж�

	//�ж����ȼ�NVIC����
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 	 //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;  //TIM1�ж�
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;  //��ռ���ȼ�0��
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  //�����ȼ�3��
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQͨ����ʹ��
	NVIC_Init(&NVIC_InitStructure);  //��ʼ��NVIC�Ĵ���


	TIM_Cmd(TIM1, ENABLE);  //ʹ��TIMx
}

/*******************************************************************
�����ܣ��߼���ʱ��TIM1���жϴ�����
����汾��V1.0
��    �ڣ� 2019/5/30
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void TIM1_UP_IRQHandler(void) 
{ 
	static u8 Tsk10ms_Cnt = 0;
	static u8 Tsk20ms_Cnt = 0;
	static u8 Tsk100ms_Cnt = 0;
	//static u8 Tsk1000ms_Cnt = 0;
	
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)//���ָ����TIM�жϷ������:TIM �ж�Դ 
	{
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);//���TIMx���жϴ�����λ:TIM �ж�Դ 
		
		/****433Mģ��&����ģ�� 10ms����ʱ��ѯ��flag****/
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
		
		/*****ҡ�˵�λ�ļ��ģ��20ms����ʱ��ѯ��flag*****/
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
		
		/****������ʾģ��1000ms����ʱ��ѯ��flag****/
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

