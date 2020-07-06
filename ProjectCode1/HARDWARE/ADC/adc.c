#include "adc.h"
#include "sys.h"
#include "Public_Value.h"

/***********************************************************************
全局变量的bit分配
------------------------------------------------------------------------
Name		bit					Defination
------------------------------------------------------------------------
Task_Flag	7					BAT_FULVOL
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

//创建的全局变量
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

u16 ADC_Conversion_Value[NUM];       //ADC采集值存储的数组

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
程序功能：AD采集的回调函数，100ms task
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

void MADC_CallBack(void)
{
	ADC_Convert();
	MTORMode_Anly();
	STERMode_Anly();
	R_BatVol();
}

/*******************************************************************
程序功能：ADC模块的初始化
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

void MADC_Init(void)
{
	//ADCFlg_Init();
	ADC1_GPIO_Config();
	ADC1_Config();
	ADC1_DMA_Config();
}

/*******************************************************************
程序功能：ADC1的GPIO配置
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
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
程序功能：ADC1配置
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void ADC1_Config(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);	  //使能ADC1通道时钟
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M

	ADC_DeInit(ADC1);  //复位ADC1 

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;	      //模数转换工作在扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 5;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器 

	ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 2, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 4, ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 5, ADC_SampleTime_239Cycles5);
	
	ADC_DMACmd(ADC1, ENABLE);
	ADC_Cmd(ADC1, ENABLE);	    //使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
	
}

/*******************************************************************
程序功能：ADC1_DMA配置
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void ADC1_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//使能DMA传输
	
	DMA_DeInit(DMA1_Channel1);   //将DMA的通道1寄存器重设为缺省值

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADC1 -> DR));  //DMA外设基地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&(ADC_Conversion_Value[0]);  //DMA内存基地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //数据传输方向，从外设到内存
	DMA_InitStructure.DMA_BufferSize = 5;  //DMA通道的DMA缓存的大小
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  //数据宽度为16位
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;      //数据宽度为16位
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  //工作在循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; //DMA通道 x拥有高优先级 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);  //
	
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

/*******************************************************************
程序功能：ADC采集值转换
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void ADC_Convert(void)
{
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
		
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC1));
	DMA_ClearFlag(DMA1_FLAG_TC1);
}
	   
/*******************************************************************
程序功能：电机速度挡位的计算
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static u8 *SpdMode_Select(u16 adc_val, u8 mtr_type)
{
	static u8 Spd_Mode[2] = {0x00};
	u8 tmp = 0x02;
	u8 cnt = 0x00;
	
	Spd_Mode[MODE_STS] |= mtr_type;   //0x01代表主电机，0x02代表扩展电机
	
	if(0x02 == KEY_Scan())
	{
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x08;                 //停止模式
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
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x20;                 //正转模式
	}
	else
	{
		cnt = (MOTR_MIDVAL-adc_val+ADC_OFFSET) / MOTR_ADDVAL;
		Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x10;                 //反转模式
	}
	
	if(cnt <= (MOTR_MAXVAL-MOTR_MIDVAL)/MOTR_ADDVAL)
	{
		if(0 == cnt)
		{
			Spd_Mode[MODE_STS] = (Spd_Mode[MODE_STS]&mtr_type) | 0x04;               //初始化模式
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
程序功能：舵机角度的计算
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
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
			Str_Mode = 0x00;       //中位
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
			Str_Mode = 0x00;       //中位
		}
		else
		{
			Str_Mode = (Str_Mode&0x00) | (tmp << (cnt-1+4));
		}
	}
	
	return Str_Mode;
}

/*******************************************************************
程序功能：电机状态、速度挡位的解析
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
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
程序功能：舵机状态、速度挡位的解析
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static void STERMode_Anly(void)
{
	T_Data.STER1_Mode = StrMode_Select(ADC_Conversion_Value[STER_1]);
	
	T_Data.STER2_Mode = StrMode_Select(ADC_Conversion_Value[STER_2]);
}

/*******************************************************************
程序功能：蓄电池电压的读取
程序版本：V1.0
日    期： 2019/5/30 
作    者：Orange
修    改：无
*******************************************************************/

static u16 R_BatVol(void)
{
	u16 Result = 0;
	
	Result = ADC_Conversion_Value[BATVAL];
	
	return Result;
}
















