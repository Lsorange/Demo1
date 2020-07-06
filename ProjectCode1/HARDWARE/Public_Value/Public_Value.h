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


/******电机停止按键扫描的函数***********/
extern u8 KEY_Scan(void);

/******定时器轮询的标志位*********/
extern Task_Flag Tsk_Flg;

/****过流、蓄电池电压信号,电流采集DMA转换的结果****/
//extern ADC_FLAG ADCMont_Flg;
extern u16 ADC_Conversion_Value[5];
extern u8 *pADC_Data;
extern TX_Mode T_Data;
//extern u8 Flag_DMA;

/*********数据连接状态************/
//extern const u8 DataCont_Sts;

/*********串口数据接收***********/
extern RecvData_Struct Recv_Data;
extern volatile u8 RX_Flg;
extern volatile u8 Data_Len;
extern volatile u8 State_SW;
extern u8 RX_Data[RX_SIZE];

/*********看门狗的状态***********/
extern u8 WDG_Flg;

#endif

