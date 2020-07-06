#include "led.h"
#include "Public_Value.h"

/*************************************************************************************************
LEDӲ�����������
Serial_Num    Name   			GPIO    INIT     Function
------------------------------------------------------------------------------------------------
    1         LowVol_LED        PC6     0(OFF)   ����30%��������
	2         MidVol_LED   		PC7		0(OFF)   ����50%��������
	3		  HigVol_LED   		PC8     0(OFF)	 ����80%��������
	4		  FulVol_LED        PC9     0(OFF)   ��������ʱ����
	5		  Rtn_LED           PC10    0(OFF)	 ����ָʾ��
	6         MCU_LED   		PC11	1(ON)	 ��ʼ����ɺ����
	7		  Data_LED  		PC12	1(����)	 433M�ȴ�����ʱ������״̬�����ӳɹ�ʱ����״̬	 
-------------------------------------------------------------------------------------------------
*************************************************************************************************/

/***********************************************************************
ȫ�ֱ�����bit����
------------------------------------------------------------------------
Name						bit					Defination
------------------------------------------------------------------------
RLED_State	  				7					reserved
							6					reserved
							5					reserved
							4					reserved
							3					reserved
							2					reserved
							1					Rtrn_LED
							0					DataSts_LED
------------------------------------------------------------------------
************************************************************************/

struct ReadLed_State
{
	u8 DSts_LED : 1;
	u8 Rtrn_LED : 1;
}ReadLed_Sts;

enum BatVol
{
	LowVol_Sts = 0x01,
	MidVol_Sts = 0x02,
	HigVol_Sts = 0x04,
	FulVol_Sts = 0x08
};

static void RLEDSts_Init(void);
static void LED_Init(void);
static void LED_Test(void);
static void DataConnect_Sts(void);
static void Return_LED(void);
static void BatVol_LED(void);


/*************************************************************************
�����ܣ�LEDģ��ص�������20ms task
����汾��V1.0
��    �ڣ� 2019/5/31
��    �ߣ�Orange
��    �ģ���
*************************************************************************/

void MLED_CallBack(void)
{
	DataConnect_Sts();
	Return_LED();
	BatVol_LED();
}

/*************************************************************************
�����ܣ�LEDģ��ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/31
��    �ߣ�Orange
��    �ģ���
*************************************************************************/

void MLED_Init(void)
{
	RLEDSts_Init();
	LED_Init();
	
	#if TEST_START
		LED_Test();
	#endif
}

/*************************************************************************
�����ܣ�ȫ�ֱ���RLED_State�ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/5/31
��    �ߣ�Orange
��    �ģ���
*************************************************************************/

static void RLEDSts_Init(void)
{
	ReadLed_Sts.DSts_LED = 0x01;   //ʵ��ֵ0x01
	ReadLed_Sts.Rtrn_LED = 0X01;    //0x02
}

/*************************************************************************
�����ܣ�LED��ʾ/LED����/�㰴LED��ʼ������
����汾��V1.0
��    �ڣ� 2019/5/31
��    �ߣ�Orange
��    �ģ���
*************************************************************************/

static void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	                               //ʹ��PC�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8
							 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;	  //LED1-->PE.5 �˿�����, �������
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		                                  //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		                                  //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 			                                  //�����趨������ʼ��GPIOB.11
 GPIO_SetBits(GPIOC,GPIO_Pin_11);						 					                                  //PB.11 �����
 GPIO_ResetBits(GPIOC,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_12); 						 
}

#if TEST_START

	static void LED_Test(void)
	{

	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��PB,PE�˿�ʱ��

	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6;	    		 //LED1-->PE.5 �˿�����, �������
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //������� ��IO���ٶ�Ϊ50MHz
	 GPIO_SetBits(GPIOE,GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6); 						 //PE.5 ����� 
	}
	
#endif

/*************************************************************************
�����ܣ�����״̬���ӵ�LEDָʾ
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
*************************************************************************/

static void DataConnect_Sts(void)
{
	//���ӳɹ�������״̬
	if(ReadLed_Sts.DSts_LED == (Recv_Data.RDataCont_Sts&0x01))
	{
		Data_LED = 1;
	}
	else //�ȴ����ӣ�����״̬
	{
		Data_LED = ~Data_LED;
	}
}

/*************************************************************************
�����ܣ�����״̬��LEDָʾ
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
˵    �������ö�ʱ�����ƵƵ���������������ʱ����������ϵͳ��ʵʱ��
*************************************************************************/

static void Return_LED(void)
{
	static u8 cnt = 0;
	
	//�����źţ�����״̬,��������4s
	if((ReadLed_Sts.Rtrn_LED == (Recv_Data.RADCMont_Flg>>4)) || (1 == ((KEY_Scan()>>1) & 0x01)))
	{
		if(++cnt >= 2)
		{
			Rtn_LED = ~Rtn_LED;
			cnt = 0;
		}
		else
		{
			//do nothing;
		}
	}
	else
	{
		Rtn_LED = 0;
	}
}
 
/*************************************************************************
�����ܣ����ص�����ָʾ��
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
˵    ������
*************************************************************************/

static void BatVol_LED(void)
{
	switch(Recv_Data.RADCMont_Flg>>4)
	{
		case LowVol_Sts:
		{
			FVol_LED = 0;
			HVol_LED = 0;
			MVol_LED = 0;
			LVol_LED = 1;
			break;
		}
		case MidVol_Sts:
		{
			LVol_LED = 0;
			FVol_LED = 0;
			HVol_LED = 0;
			MVol_LED = 1;
			break;
		}
		case HigVol_Sts:
		{
			LVol_LED = 0;
			MVol_LED = 0;
			FVol_LED = 0;
			HVol_LED = 1;
			break;
		}
		case FulVol_Sts:
		{
			LVol_LED = 0;
			MVol_LED = 0;
			HVol_LED = 0;
			FVol_LED = 1;
			break;
		}
		default:
		{
			break;
		}
	}
}

