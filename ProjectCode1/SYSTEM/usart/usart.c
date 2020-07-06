#include "sys.h"
#include "usart.h"
#include "433m.h"
#include "Public_Value.h"

/*******************************************
USART���ݽ�����ɱ�־λ��RX_Flg
�������ͣ�global volatile value
����λ�ã�Public_Value.h
********************************************/
volatile u8 RX_Flg = 0;
volatile u8 Data_Len = 0;

static void usart_init(u32 bound);
static void Mode_Init(void);
static void USART_DMA_Config(void);


/****************************************************************************************
�����ܣ�USARTģ��ĳ�ʼ��
����汾��V1.0
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
****************************************************************************************/

void MUSART_Init(void)
{
	usart_init(BOUND);
	Mode_Init();
	USART_DMA_Config();
}

/****************************************************************************************
�����ܣ�USART������
����汾��V1.0
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
****************************************************************************************/
  
static void usart_init(u32 bound)
{
  	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART1��GPIOAʱ��
  
	//USART1_TX   GPIOA.9
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.9
   
  	//USART1_RX	  GPIOA.10��ʼ��
  	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;//PA10
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
  	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.10  

  	//Usart1 NVIC ����
  	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   	//USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;//���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

 	USART_Init(USART1, &USART_InitStructure);      //��ʼ������1
  	USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); //�����߿��У������ж� 
  	USART_DMACmd(USART1,USART_DMAReq_Rx,ENABLE);   //��������DMA����
	
  	USART_Cmd(USART1, ENABLE);                     //ʹ�ܴ���1 
}

/*****************************************************************************
�����ܣ�433Mģʽ��ʼ������
����汾��V1.0
��ڲ�������
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
******************************************************************************/

static void Mode_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	                           //ʹ��PB�˿�ʱ��
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		                             //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		                             //IO���ٶ�Ϊ50MHz
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11;            //PB�˿����ã�MD0��MD1��AUX
	GPIO_Init(GPIOB, &GPIO_InitStructure);					                                 //�����趨������ʼ��GPIOB
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_1 | GPIO_Pin_10 | GPIO_Pin_11);
}

/*******************************************************************
�����ܣ�USART_DMA����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void USART_DMA_Config(void)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	//ʹ��DMA����
	
	DMA_DeInit(DMA1_Channel5);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(USART1 -> DR));  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&(RX_Data[0]);  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  //���ݴ��䷽�򣬴����赽�ڴ�
	DMA_InitStructure.DMA_BufferSize = RX_SIZE;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;      //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //������ѭ��ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh; //DMAͨ�� xӵ��������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(DMA1_Channel5, &DMA_InitStructure);  //
	
	DMA_Cmd(DMA1_Channel5, ENABLE);
}

/*****************************************************************************
�����ܣ�USART1��������
����汾��V1.0
��ڲ�����TxBuffer���������ݵ��׵�ַ��Length�����ݵĳ���
���ز�������
��    �ڣ�2019/6/18
��    �ߣ�Orange
��    �ģ���
******************************************************************************/

void uart_tx_bytes( u8* TxBuffer, u8 Length)
{
	while(Length--)
	{
		while(RESET == USART_GetFlagStatus(USART1, USART_FLAG_TXE));
		USART1->DR = *TxBuffer++;
	}
	
	State_SW = (State_SW&0x00) | 0x01;
}

/*******************************************************************
�����ܣ�DMA_ENABLE����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void DMAEN_Config(void)
{
	if((1 == State_SW) && (1 == RX_Flg))
	{ 
		DMA_Cmd(DMA1_Channel5, ENABLE);  //����DMA
	}
	else
	{
		//do nothing
	}
}

/*******************************************************************
�����ܣ�USART1_ISR����
����汾��V1.0
��    �ڣ� 2019/5/30 
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void USART1_IRQHandler(void)
{
    if(USART_GetITStatus(USART1,USART_IT_IDLE) == SET)
    {
       Data_Len = USART1 -> SR;
       Data_Len = USART1-> DR; //��USART_IT_IDLE��־
			 DMA_Cmd(DMA1_Channel5, DISABLE);    //�ر�DMA
		   Data_Len = RX_SIZE - DMA_GetCurrDataCounter(DMA1_Channel5);      //�õ������������ݸ���  
		   DMA1_Channel5->CNDTR = RX_SIZE;                        //�������ý������ݸ���  
			
       RX_Flg = 1;                      //�������ݱ�־λ��1
    }
}
