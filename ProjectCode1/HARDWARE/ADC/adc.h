#ifndef __ADC_H
#define __ADC_H	
#include "sys.h"

//#define ADC_FACTOR   1000U
#define ADC_OFFSET   20U
#define MOTR_MIDVAL  1639U
#define MOTR_MAXVAL  3278U
#define MOTR_ADDVAL  273U
#define STER_MIDVAL  1638U
#define STER_MAXVAL  3277U
#define STER_ADDVAL  409U
#define MODE_STS     0U
#define MODE_VAL     1U
#define MAIN_MOTR    1U
#define EXTD_MOTR    2U


void MADC_CallBack(void);
void MADC_Init(void);
//void ADC1_GPIO_Config(void);
//void ADC1_Config(void);
//void ADC1_DMA_Config(void);
//void ADC_Convert(void);
//u8 *SpdMode_Select(u16 adc_val, u8 mtr_type);
//u8 StrMode_Select(u16 adc_val);
//void MTORMode_Anly(void);
//void STERMode_Anly(void);
//u16 R_BatVol(void);

//测试
extern u16 ADC_Conversion_Value[5]; 

typedef struct
{
	u8 MMTOR_Sts;
	u8 MMTOR_Spd;
	u8 EMTOR_Sts;
	u8 EMTOR_Spd;
	u8 STER1_Mode;
	u8 STER2_Mode;
}TX_Mode;

#endif 

