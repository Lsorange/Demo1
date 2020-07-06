#include "adc.h"
#include "sys.h"
#include "Public_Value.h"

/***********************************************************************
ȫ�ֱ�����bit����
------------------------------------------------------------------------
Name								bit					Defination
------------------------------------------------------------------------
Task_Flag	  						7					BAT_FULVOL
									6					BAT_HIGVOL
									5					BAT_MIDVOL
									4					BAT_LOWVOL
									3					M4_OVERCUR
									2					M3_OVERCUR
									1					M2_OVERCUR
									0					M1_OVERCUR
------------------------------------------------------------------------
************************************************************************/
//ADC_FLAG ADCMont_Flg;
//u8 *pADCFlg = (u8*)&ADCMont_Flg;

//������ȫ�ֱ���
TX_Mode T_Data;
u8 *pADC_Data = (u8*)&T_Data;
/*
u8 MMTOR_Sts = 0;
u8 MMTOR_Spd = 0;
u8 EMTOR_Sts = 0;
u8 EMTOR_Spd = 0;
u8 STER1_Mode = 0;
u8 STER2_Mode = 0;
*/

typedef enum
{
	STER_1 = 0x00,
	STER_2,
	MMTOR,
	EMTOR,
	BATVAL,
	
	NUM
}eADC_Cnt;

u16 ADC_Conversion_Value[NUM];       //ADC�ɼ�ֵ�洢������

static void ADC1_GPIO_Config(void);
static void ADC1_Config(void);
static void ADC1_DMA_Config(void);
static void ADC_Convert(void);
static u8 *SpdMode_Select(u16 adc_val, u8 mtr_type);
static u8 StrMode_Select(u16 adc_val);
static void MTORMode_Anly(void);
static void STERMode_Anly(void);
static u16 R_BatVol(void);


/*******************************************************************
�����ܣ�AD�ɼ��Ļص�������100ms task
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void MADC_CallBack(void)
{
	ADC_Convert();
	MTORMode_Anly();
	STERMode_Anly();
	R_BatVol();
}

/*******************************************************************
�����ܣ�ADCģ��ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void MADC_Init(void)
{
	//ADCFlg_Init();
	ADC1_GPIO_Config();
	ADC1_Config();
	ADC1_DMA_Config();
}

/*******************************************************************
�����ܣ�ADC1��GPIO����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void ADC1_GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	      
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; 		               
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4; 
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}
 
/*******************************************************************
�����ܣ�ADC1����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void ADC1_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	  //ʹ��ADC1ͨ��ʱ��
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	ADC_DeInit(ADC1);  //��λADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	      //ģ��ת��������ɨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 5;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ��� 

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_239Cycles5);
	
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);	    //ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
	
}

/*******************************************************************
�����ܣ�ADC1_DMA����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void ADC1_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
	DMA_DeInit(DMA1_Channel1);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADC1 -> DR));  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&(ADC_Conversion_Value[0]);  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = 5;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;      //���ݿ��Ϊ16λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMAͨ�� xӵ�и����ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

/*******************************************************************
�����ܣ�ADC�ɼ�ֵת��
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void ADC_Convert(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
	DMA_ClearFlag(DMA1_FLAG_TC1);
}
	   
/*******************************************************************
�����ܣ�����ٶȵ�λ�ļ���
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static u8 *SpdMode_Select(u16 adc_val, u8 mtr_type)
{
	static u8 Spd_Mode[2] = {0x00};
	u8 tmp = 0x02;
	u8 cnt = 0x00;
	
	Spd_Mode[MODE_STS] |= mtr_type;   //0x01�����������0x02������չ���
	
	if(0x02 == KEY_Scan())
	{
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x08;                 //ֹͣģʽ
	}
	else
	{
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) & 0xf7;
	}
	
	if(adc_val >= MOTR_MAXVAL)
	{
		adc_val = MOTR_MAXVAL;
	}
	else
	{
		//do nothing
	}
	
	if(adc_val >= MOTR_MIDVAL)
	{
		cnt = (adc_val-MOTR_MIDVAL+ADC_OFFSET) / MOTR_ADDVAL;
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x20;                 //��תģʽ
	}
	else
	{
		cnt = (MOTR_MIDVAL-adc_val+ADC_OFFSET) / MOTR_ADDVAL;
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x10;                 //��תģʽ
	}
	
	if(cnt <= (MOTR_MAXVAL-MOTR_MIDVAL)/MOTR_ADDVAL)
	{
		if(0 == cnt)
		{
			Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x04;               //��ʼ��ģʽ
			Spd_Mode[MODE_VAL] &= 0xfe;
		}
		else
		{
			Spd_Mode[MODE_VAL] |= 0x01;
		}
		
		Spd_Mode[MODE_VAL] = (Spd_Mode[MODE_VAL]&0x01) | (tmp << cnt);
	}
	else
	{
		//do nothing
	}
	
	return Spd_Mode;
}

/*******************************************************************
�����ܣ�����Ƕȵļ���
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static u8 StrMode_Select(u16 adc_val)
{
	u8 Str_Mode = 0x00;
	u8 tmp = 0x01;
	u8 cnt = 0x00;
	        
	if(adc_val >= STER_MAXVAL)
	{
			adc_val = STER_MAXVAL;
	}
	else
	{
		//do nothing
	}
	
	if(adc_val >= STER_MIDVAL)
	{
		cnt = (adc_val-STER_MIDVAL+ADC_OFFSET) / STER_ADDVAL;
		
		if(0 == cnt)
		{
			Str_Mode = 0x00;       //��λ
		}
		else
		{
			Str_Mode = (Str_Mode&0x00) | (tmp << (cnt-1));
		}
	}
	else
	{
		cnt = (STER_MIDVAL-adc_val+ADC_OFFSET) / STER_ADDVAL;
		
		if(0 == cnt)
		{
			Str_Mode = 0x00;       //��λ
		}
		else
		{
			Str_Mode = (Str_Mode&0x00) | (tmp << (cnt-1+4));
		}
	}
	
	return Str_Mode;
}

/*******************************************************************
�����ܣ����״̬���ٶȵ�λ�Ľ���
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void MTORMode_Anly(void)
{
	u8 *MResult, *EResult;
	
	MResult = SpdMode_Select(ADC_Conversion_Value[MMTOR], MAIN_MOTR);
	T_Data.MMTOR_Sts = MResult[MODE_STS];
	T_Data.MMTOR_Spd = MResult[MODE_VAL];
	
	EResult = SpdMode_Select(ADC_Conversion_Value[EMTOR], EXTD_MOTR);
	T_Data.EMTOR_Sts = EResult[MODE_STS];
	T_Data.EMTOR_Spd = EResult[MODE_VAL];
}

/*******************************************************************
�����ܣ����״̬���ٶȵ�λ�Ľ���
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void STERMode_Anly(void)
{
	T_Data.STER1_Mode = StrMode_Select(ADC_Conversion_Value[STER_1]);
	
	T_Data.STER2_Mode = StrMode_Select(ADC_Conversion_Value[STER_2]);
}

/*******************************************************************
�����ܣ����ص�ѹ�Ķ�ȡ
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static u16 R_BatVol(void)
{
	u16 Result = 0;
	
	Result = ADC_Conversion_Value[BATVAL];
	
	return Result;
}
















