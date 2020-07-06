#ifndef __Public_Value_H
#define __Public_Value_H
#include "sys.h"
#include "timer.h"
#include "adc.h"
#include "433m.h"
#include "usart.h"
#include "wdg.h"

#define TEST_START  1
#define TEST_END    0


/******���ֹͣ����ɨ��ĺ���***********/
extern u8 KEY_Scan(void);

/******��ʱ����ѯ�ı�־λ*********/
extern Task_Flag Tsk_Flg;

/****���������ص�ѹ�ź�,�����ɼ�DMAת���Ľ��****/
//extern ADC_FLAG ADCMont_Flg;
extern u16 ADC_Conversion_Value[5];
extern u8 *pADC_Data;
extern TX_Mode T_Data;
//extern u8 Flag_DMA;

/*********��������״̬************/
//extern const u8 DataCont_Sts;

/*********�������ݽ���***********/
extern RecvData_Struct Recv_Data;
extern volatile u8 RX_Flg;
extern volatile u8 Data_Len;
extern volatile u8 State_SW;
extern u8 RX_Data[RX_SIZE];

/*********���Ź���״̬***********/
extern u8 WDG_Flg;

#endif

