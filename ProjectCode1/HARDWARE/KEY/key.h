#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"

#define N_KEY            0        //�ް�������
#define S_KEY            1        //�а���������Ϊ�̰�
#define L_KEY            2        //�а���������Ϊ����

#define N_KEY_State      0        //�ް���
#define S_KEY_State      1        //�̰������ֹͣ
#define L_KEY_State      2        //�����������������
#define KDelay_Time      4000       //3s

#define Stop_KEY  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)
#define Rtn_KEY   GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13)
#define Fled_KEY  GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14)
#define Led1_KEY  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)
#define Led2_KEY  GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)

//����
#define TEST_KEY GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)



//void KEY_Init(void);//IO��ʼ��
//u8 Get_KEY_State(u8 Key_Type, u16 time);
u8 KEY_Scan(void);  	//����ɨ�躯��	
void MKEY_CallBack(void);
void MKEY_Init(void);
//void TestKeyInit(void);
#endif
