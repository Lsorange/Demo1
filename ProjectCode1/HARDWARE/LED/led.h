#ifndef __LED_H
#define __LED_H	 
#include "sys.h"

#define LVol_LED   PCout(6)
#define MVol_LED   PCout(7)
#define HVol_LED   PCout(8)
#define FVol_LED   PCout(9)
#define Rtn_LED    PCout(10)
#define MCU_LED    PCout(11)
#define Data_LED   PCout(12)

//void LED_Init(void);//≥ı ºªØ
//void RLEDSts_Init(void);
//void DataConnect_Sts(void);
//void Return_LED(void);
//void BatVol_LED(void);
void MLED_CallBack(void);
void MLED_Init(void);

	//≤‚ ‘
	#define KEY_Test_LED  PEout(5)
	#define KEY_Two_LED   PEout(6)
	#define KEY_Three_LED PEout(2)
	//void LED_Test(void);
		 				    
#endif
