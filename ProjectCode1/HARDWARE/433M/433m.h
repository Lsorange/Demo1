#ifndef   __433M_H
#define   __433M_H
#include "sys.h"

#define  MAX_SIZE    8
#define  TX_SIZE     10
#define  RX_SIZE     4
#define  CRC_SIZE    2
#define  EMPTY_ERR   0
#define  FULL_ERR    0
#define  RW_OK       1
#define  MAX_QLEN    8     //double memery blocks of RX_SIZE(include crc data)
#define  EMPTY_SIZE  0
#define  CONT_OK     1
#define  CONT_ERR    0
#define  CRC_OK      1
#define  CRC_ERR     0
#define  CRC_STA     1
#define  CRC_END     1

typedef u8 boolean;

void MCom_CallBack(void);
void MCom_Init(void);
//void RData_Init(void);
//boolean QueueRecv_Insert(u8 *DataArr);
//boolean QueueRecv_Del(u8 *data);
//boolean Is_Concted(void);
//boolean Dect_CRC(void);
//void Send_Data(void);
//void Deal_RecvData(void);

typedef struct
{
	u8 RCRC_Start[CRC_STA];
	u8 RDataCont_Sts;
	u8 RADCMont_Flg;        //电压的数据，位于高四位
	u8 RCRC_End[CRC_END];
	
}RecvData_Struct;

#endif


