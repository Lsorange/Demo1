#include "433m.h"
#include "usart.h"
#include "sys.h"
#include "Public_Value.h"
#include "led.h"

/*********************************************************************************
433M�շ����ź�
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
ADCMont_Flg(����λ)                 	   *pADCFlg     		 Battery value
CRC_End                                CRC_End[0]            0x45	
*********************************************************************************/

u8 TX_Data[TX_SIZE] = {0};
u8 RX_Data[RX_SIZE] = {0};
volatile u8 State_SW = 0x01;        //0x01(bit0)��ʾ�������ݽ��գ�0x02(bit1)��ʾ�������ݷ��ͣ�ͬһʱ��ֻ��һ��״̬
const u8 DataCont_Sts = 0x01;  //433Mģ������״̬ 
const u8 CRC_Data[CRC_SIZE] = 
{
	0x48, //��ӦASCII 'H' -> Head
	0x45, //��ӦASCII 'E' -> End
};

//����
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
�����ܣ�433Mģ��Ļص�����, 10ms task
����汾��V1.0
��ڲ�����data
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
******************************************************************************/

void MCom_CallBack(void)
{
	//QueueRecv_Insert(RX_Data);
	Deal_RecvData();
	Send_Data();
	DMAEN_Config();
}

/*****************************************************************************
�����ܣ�433Mģ��ĳ�ʼ��
����汾��V1.0
��ڲ�����data
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
******************************************************************************/

void MCom_Init(void)
{
	RData_Init();
}

/*****************************************************************************
�����ܣ�Recv_Data�ṹ���Ա��ʼ��
����汾��V1.0
��ڲ�����data
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
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
	�����ܣ������յ����ݲ��뵽���λ��������
	����汾��V1.0
	��ڲ�����*DataArr
	���ز�����FULL_ERR & RW_OK
	��    �ڣ�2019/6/18
	��    �ߣ�Orange
	��    �ģ���
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
	�����ܣ������λ�������е�����ȡ�����ͷ��ڴ�
	����汾��V1.0
	��ڲ�����*data
	���ز�����EMPTY_ERR & RW_OK
	��    �ڣ�2019/6/18
	��    �ߣ�Orange
	��    �ģ���
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
�����ܣ�433M���͵�����
����汾��V1.0
��ڲ�������
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
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
		//����
		KEY_Two_LED = ~KEY_Two_LED;
	}
}

#if TEST_START
/*****************************************************************************
�����ܣ�����433M���յ�����
����汾��V1.0
��ڲ�������
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
******************************************************************************/

	static void Deal_RecvData(void)
	{
		u8 i;
		//u8 R_Data[RX_SIZE] = {0};
		u8* pReData = (u8*)&Recv_Data;  //ȡ�ṹ���һ��Ԫ�صĵ�ַ
		
		if((1 == RX_Flg) && (SET == DMA_GetFlagStatus(DMA1_FLAG_TC5)))   //�ж��Ƿ���պ�DMA�������
		{
			
			if((CONT_OK == Is_Concted()) && (CRC_OK == Dect_CRC() && (RX_SIZE == Data_Len)))  //�����������״̬��CRCλ�����Ƿ���ȷ�Լ����ݳ���
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
							pReData[i] = R_Data[i]; //�ֱ��Ӧ���ṹ���Ԫ�ظ�ֵ
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
					
					pReData[i] = RX_Data[i]; //�ֱ��Ӧ���ṹ���Ԫ�ظ�ֵ
				}
				
				//����
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
			
			RX_Flg = 0;     //������ɱ�־����
			DMA_ClearFlag(DMA1_FLAG_TC5); //���DMA������ɱ�־λ
			State_SW = (State_SW&0x00) | 0x02;  //���������ݱ�־λ��0�����������ݱ�־��1
		}
		else
		{
			//do nothing
		}
	}

/*****************************************************************************
�����ܣ��ж�433Mģ�����ӵ�״̬
����汾��V1.0
��ڲ�������
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
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
�����ܣ��˶�����У��λ�Ľ��
����汾��V1.0
��ڲ�������
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
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





