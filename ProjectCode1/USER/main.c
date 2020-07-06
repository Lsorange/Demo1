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
程序功能：系统的初始化
程序版本：V1.0
日    期： 2019/5/30
作    者：Orange
修    改：无
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
程序功能：回调函数的处理
程序版本：V1.0
日    期： 2019/5/30
作    者：Orange
修    改：无
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
程序功能：遥控模块的主函数
程序版本：V1.0
日    期： 2019/5/30
作    者：Orange
修    改：无
********************************************************************/

int main(void)
{
	//测试
	//char KeySts[] = "KeySts = ";
	
	System_Init();
	
	while(1)
	{
		/**************************************************
							时间轮询调度
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
		WDG_Flg = 1;  //重新置位
		
		//Send_data(KeySts, KEY_Scan());
	}
}

