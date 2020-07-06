#include "433m.h"
#include "usart.h"
#include "sys.h"
#include "Public_Value.h"
#include "led.h"

/*********************************************************************************
433M收发的信号
----------------------------------------------------------------------------------
TX_Message                             Value                 Discription
----------------------------------------------------------------------------------
CRC_Start                              CRC_Start[0]          0x48
DataCont_Sts                           DataCont_Sts          DataConnect_Sts
TMainMotor_State                       TMainMotor_State      MainMotor_ModeSelect
TMainMotor_Spd                         TMainMotor_Spd        MainMotor_SpdMode
TExtdMotor_State                       TExtdMotor_State      ExtdMotor_ModeSelect
TExtdMotor_Spd                         TExtdMotor_Spd        ExtdMotor_SpdMode
TKEY_State                             TKEY_State            LED_Control
TSteer1_Angle                          TSteer1_Angle         Steer_Control
TSteer2_Angle                          TSteer2_Angle         Steer_Control
CRC_End                                CRC_End[0]            0x45	
----------------------------------------------------------------------------------

RX_Message                             Value                 Discription
----------------------------------------------------------------------------------
CRC_Start                              CRC_Start[0]          0x48
RDataCont_Sts                          RDataCont_Sts         DataConnect_Sts
ADCMont_Flg(高四位)                     *pADCFlg     	   Battery value
CRC_End                                CRC_End[0]            0x45	
*********************************************************************************/

u8 TX_Data[TX_SIZE] = {0};
u8 RX_Data[RX_SIZE] = {0};
volatile u8 State_SW = 0x01;        //0x01(bit0)表示开启数据接收，0x02(bit1)表示开启数据发送，同一时刻只有一种状态
const u8 DataCont_Sts = 0x01;  //433M模块连接状态 
const u8 CRC_Data[CRC_SIZE] = 
{
	0x48, //对应ASCII 'H' -> Head
	0x45, //对应ASCII 'E' -> End
};

//测试
const u8 Data[RX_SIZE] = 
{
	0x48, 0x01, 
	0x10, 0x45
};

RecvData_Struct Recv_Data;

typedef struct
{
	u8 MemFrnt_SendIndex;
	u8 MemRear_RecvIndex;
	u8 MemData_Len;
	u8 MemDataBuf[MAX_QLEN];
}Queue_Mem_Struct;

Queue_Mem_Struct Queue_Recv;

enum RxIndex
{
	CrcStrt = 0,
	DataSts = 1,
	CrcEnd  = 3
};

static void Send_Data(void);
static void RData_Init(void);
static void Deal_RecvData(void);
static boolean Dect_CRC(void);


/*****************************************************************************
程序功能：433M模块的回调函数, 10ms task
程序版本：V1.0
入口参数：data
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

void MCom_CallBack(void)
{
	//QueueRecv_Insert(RX_Data);
	Deal_RecvData();
	Send_Data();
	DMAEN_Config();
}

/*****************************************************************************
程序功能：433M模块的初始化
程序版本：V1.0
入口参数：data
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

void MCom_Init(void)
{
	RData_Init();
}

/*****************************************************************************
程序功能：Recv_Data结构体成员初始化
程序版本：V1.0
入口参数：data
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

static void RData_Init(void)
{
	u8 *pData = (u8*)&Recv_Data;
	u8 i;

	for(i = 0; i < sizeof(Recv_Data)/sizeof(u8); i++)
	{
		pData[i] = 0x00;
	}
}


#if TEST_END

	/*****************************************************************************
	程序功能：将接收的数据插入到环形缓冲队列中
	程序版本：V1.0
	入口参数：*DataArr
	返回参数：FULL_ERR & RW_OK
	日    期：2019/6/18
	作    者：Orange
	修    改：无
	******************************************************************************/

		boolean QueueRecv_Insert(u8* DataArr)
		{
			if(MAX_QLEN == Queue_Recv.MemData_Len)
			{
				return FULL_ERR;
			}
			else
			{
				Queue_Recv.MemDataBuf[Queue_Recv.MemRear_RecvIndex] = DataArr[Queue_Recv.MemRear_RecvIndex];
				
				if(++Queue_Recv.MemRear_RecvIndex >= RX_SIZE)
				{
					Queue_Recv.MemRear_RecvIndex = 0;
				}
				//Queue_Recv.MemRear_RecvIndex = (Queue_Recv.MemRear_RecvIndex+1) % MAX_QLEN;
				Queue_Recv.MemData_Len++;
				
				return RW_OK;
			}
			
			//return 0;
		}

	/*****************************************************************************
	程序功能：将环形缓冲队列中的数据取出并释放内存
	程序版本：V1.0
	入口参数：*data
	返回参数：EMPTY_ERR & RW_OK
	日    期：2019/6/18
	作    者：Orange
	修    改：无
	******************************************************************************/

		boolean QueueRecv_Del(u8 *data)
		{
			if((EMPTY_SIZE == Queue_Recv.MemData_Len))
			{
				return EMPTY_ERR;
			}
			else
			{
				data[Queue_Recv.MemFrnt_SendIndex] = Queue_Recv.MemDataBuf[Queue_Recv.MemFrnt_SendIndex];
				
				if(++Queue_Recv.MemFrnt_SendIndex >= RX_SIZE)
				{
					Queue_Recv.MemFrnt_SendIndex = 0;
				}
				Queue_Recv.MemData_Len--;
				
				return RW_OK;
			}
		}

#endif

/*****************************************************************************
程序功能：433M发送的数据
程序版本：V1.0
入口参数：无
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

static void Send_Data(void)
{
	u8 i;
	
	TX_Data[0] = CRC_Data[0];
	TX_Data[1] = DataCont_Sts;
	TX_Data[2] = KEY_Scan();

	/*
	TX_Data[3] = pADC_Data[0];   //TMainMotor_State
	TX_Data[4] = pADC_Data[1];   //TMainMotor_Spd
	TX_Data[5] = pADC_Data[2];   //TExtdMotor_State
	TX_Data[6] = pADC_Data[3];   //TExtdMotor_Spd
	TX_Data[7] = pADC_Data[4];   //TSteer1_Angle
	TX_Data[8] = pADC_Data[5];   //TSteer2_Angle
	*/
	for(i = 0; i < sizeof(T_Data)/sizeof(u8); i++)
	{
		TX_Data[i+3] = pADC_Data[i];
	}
	
	TX_Data[9] = CRC_Data[1];
	
	if(1 == (State_SW>>1))
	{
		uart_tx_bytes(TX_Data, TX_SIZE);
		//测试
		KEY_Two_LED = ~KEY_Two_LED;
	}
}

#if TEST_START
/*****************************************************************************
程序功能：处理433M接收的数据
程序版本：V1.0
入口参数：无
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

	static void Deal_RecvData(void)
	{
		u8 i;
		//u8 R_Data[RX_SIZE] = {0};
		u8* pReData = (u8*)&Recv_Data;  //取结构体第一个元素的地址
		
		if((1 == RX_Flg) && (SET == DMA_GetFlagStatus(DMA1_FLAG_TC5)))   //判断是否接收和DMA传输完成
		{
			
			if((CONT_OK == Is_Concted()) && (CRC_OK == Dect_CRC() && (RX_SIZE == Data_Len)))  //检查数据连接状态和CRC位数据是否正确以及数据长度
			{
				for(i = 0; i < RX_SIZE; i++)
				{
					/*
					if(RW_OK == QueueRecv_Insert(&RX_Data[i]))
					{
						KEY_Test_LED = ~KEY_Test_LED;
						
						//pReData[i] = RX_Data[i];
						if(RW_OK == QueueRecv_Del(&R_Data[i]))
						{
							pReData[i] = R_Data[i]; //分别对应给结构体的元素赋值
						}
						else
						{
							//do nothing
						}
					}
					else
					{
						//do nothing;
					}
					*/
					
					pReData[i] = RX_Data[i]; //分别对应给结构体的元素赋值
				}
				
				//测试
				for(i = 0; i < RX_SIZE; i++)
				{
					if(pReData[i] != Data[i])
					{
						break;
					}
				}
				
				if((Data[CrcStrt] == Recv_Data.RCRC_Start[0]) && (Data[CrcEnd]) == Recv_Data.RCRC_End[0] && (RX_SIZE == i))
				{
					KEY_Test_LED = ~KEY_Test_LED;
				}
				else
				{
					//do nothing;
				}
			}
			
			RX_Flg = 0;     //接收完成标志清零
			DMA_ClearFlag(DMA1_FLAG_TC5); //清除DMA传输完成标志位
			State_SW = (State_SW&0x00) | 0x02;  //将接收数据标志位置0，将发送数据标志置1
		}
		else
		{
			//do nothing
		}
	}

/*****************************************************************************
程序功能：判断433M模块连接的状态
程序版本：V1.0
入口参数：无
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

static boolean Is_Concted(void)
{
	if(DataCont_Sts == RX_Data[DataSts])
	{
		return CONT_OK;
	}
	else
	{
		//do nothing
	}
	
	return CONT_ERR;
}

/*****************************************************************************
程序功能：核对数据校验位的结果
程序版本：V1.0
入口参数：无
返回参数：无
日    期：2019/6/18
作    者：Orange
修    改：无
******************************************************************************/

static boolean Dect_CRC(void)
{
	if((CRC_Data[0] == RX_Data[CrcStrt]) && 
		 (CRC_Data[1] == RX_Data[CrcEnd]))
	{
		return CRC_OK;
	}
	else
	{
		//do nothing
	}
	
	return CRC_ERR;
}

#endif





