#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	 
#include "adc.h"
#include "wdg.h"
#include "433m.h"
#include "timer.h"
#include "Public_Value.h"
 
/********************************************************************
�����ܣ�ϵͳ�ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/30
��    �ߣ�Orange
��    �ģ���
********************************************************************/
void System_Init(void)
{
	MLED_Init();
	MKEY_Init();
	MUSART_Init();
	MADC_Init();
	MCom_Init();
	MWDG_Init();
	MTIMER_Init();
}

/********************************************************************
�����ܣ��ص������Ĵ���
����汾��V1.0
��    �ڣ� 2019/5/30
��    �ߣ�Orange
��    �ģ���
********************************************************************/

void Handle_Func(void (*CallBack_Func)(void), u8 *Timer_Flg)
{
	if(NULL != CallBack_Func)
	{
		if(1 == (*Timer_Flg))
		{
			(*CallBack_Func)();
			(*Timer_Flg) = 0;
		}
		else
		{
			//do nothing
		}
	}
	else
	{
		return;
	}
} 

/********************************************************************
�����ܣ�ң��ģ���������
����汾��V1.0
��    �ڣ� 2019/5/30
��    �ߣ�Orange
��    �ģ���
********************************************************************/

int main(void)
{
	//����
	//char KeySts[] = "KeySts = ";
	
	System_Init();
	
	while(1)
	{
		/**************************************************
							ʱ����ѯ����
		***************************************************/
		/************10ms task-433M control************/
		Handle_Func(MCom_CallBack, (u8*)&Tsk_Flg.USART_Control);
		
		/************20ms task-ADC control************/
		Handle_Func(MADC_CallBack, (u8*)&Tsk_Flg.ADC_Voltage);
		
		/************10ms task-KEY control************/
		Handle_Func(MKEY_CallBack, (u8*)&Tsk_Flg.KEY_SCAN);
		
		/************100ms task-LED control************/
		Handle_Func(MLED_CallBack, (u8*)&Tsk_Flg.LED_Display);
		
		/*****************task-WDG******************/
		Handle_Func(MWDG_CallBack, (u8*)&WDG_Flg);
		WDG_Flg = 1;  //������λ
		
		//Send_data(KeySts, KEY_Scan());
	}
}

