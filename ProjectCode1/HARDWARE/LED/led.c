#include "led.h"
#include "Public_Value.h"

/*************************************************************************************************
LED硬件层的索引：
Serial_Num    Name   			GPIO    INIT     Function
------------------------------------------------------------------------------------------------
    1         LowVol_LED        PC6     0(OFF)   蓄电池30%电量点亮
	2         MidVol_LED   		PC7		0(OFF)   蓄电池50%电量点亮
	3		  HigVol_LED   		PC8     0(OFF)	 蓄电池80%电量点亮
	4		  FulVol_LED        PC9     0(OFF)   蓄电池满电时点亮
	5		  Rtn_LED           PC10    0(OFF)	 返航指示灯
	6         MCU_LED   		PC11	1(ON)	 初始化完成后点亮
	7		  Data_LED  		PC12	1(快闪)	 433M等待连接时，快闪状态，连接成功时常亮状态	 
-------------------------------------------------------------------------------------------------
*************************************************************************************************/

/***********************************************************************
全局变量的bit分配
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
程序功能：LED模块回调函数，20ms task
程序版本：V1.0
日    期： 2019/5/31
作    者：Orange
修    改：无
*************************************************************************/

void MLED_CallBack(void)
{
	DataConnect_Sts();
	Return_LED();
	BatVol_LED();
}

/*************************************************************************
程序功能：LED模块的初始化
程序版本：V1.0
日    期： 2019/5/31
作    者：Orange
修    改：无
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
程序功能：全局变量RLED_State的初始化
程序版本：V1.0
日    期： 2019/5/31
作    者：Orange
修    改：无
*************************************************************************/

static void RLEDSts_Init(void)
{
	ReadLed_Sts.DSts_LED = 0x01;   //实际值0x01
	ReadLed_Sts.Rtrn_LED = 0X01;    //0x02
}

/*************************************************************************
程序功能：LED显示/LED照明/点按LED初始化配置
程序版本：V1.0
日    期： 2019/5/31
作    者：Orange
修    改：无
*************************************************************************/

static void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	                               //使能PC端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8
							 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;	  //LED1-->PE.5 端口配置, 推挽输出
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		                                  //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		                                  //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 			                                  //根据设定参数初始化GPIOB.11
 GPIO_SetBits(GPIOC,GPIO_Pin_11);						 					                                  //PB.11 输出高
 GPIO_ResetBits(GPIOC,GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_12); 						 
}

#if TEST_START

	static void LED_Test(void)
	{

	 GPIO_InitTypeDef  GPIO_InitStructure;
		
	 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);	 //使能PB,PE端口时钟

	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6;	    		 //LED1-->PE.5 端口配置, 推挽输出
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //推挽输出 ，IO口速度为50MHz
	 GPIO_SetBits(GPIOE,GPIO_Pin_2 | GPIO_Pin_5 | GPIO_Pin_6); 						 //PE.5 输出高 
	}
	
#endif

/*************************************************************************
程序功能：数据状态连接的LED指示
程序版本：V1.0
日    期： 2019/6/10
作    者：Orange
修    改：无
*************************************************************************/

static void DataConnect_Sts(void)
{
	//连接成功，常亮状态
	if(ReadLed_Sts.DSts_LED == (Recv_Data.RDataCont_Sts&0x01))
	{
		Data_LED = 1;
	}
	else //等待连接，快闪状态
	{
		Data_LED = ~Data_LED;
	}
}

/*************************************************************************
程序功能：返航状态的LED指示
程序版本：V1.0
日    期： 2019/6/10
作    者：Orange
修    改：无
说    明：采用定时来控制灯的慢闪，避免用延时函数减低了系统的实时性
*************************************************************************/

static void Return_LED(void)
{
	static u8 cnt = 0;
	
	//返航信号，慢闪状态,慢闪周期4s
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
程序功能：蓄电池电量的指示灯
程序版本：V1.0
日    期： 2019/6/10
作    者：Orange
修    改：无
说    明：无
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

