#ifndef __TIMER_H
#define __TIMER_H
#include "sys.h"
#define TIMER_PRSC 99
#define TIMER_PERD 7199

#define Task1_Cnt 1
#define Task2_Cnt 2
#define Task3_Cnt 10
//#define Task4_Cnt 100

void MTIMER_Init(void);
//void TskFlg_Init(void);
//void TIM1_Init(u16 arr, u16 psc);

typedef volatile struct
{
	u8 ADC_Voltage;
	u8 LED_Display;
	u8 KEY_SCAN;
	u8 USART_Control;
}Task_Flag;


#endif
