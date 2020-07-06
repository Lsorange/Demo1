#include "key.h"
#include "sys.h" 
#include "Public_Value.h"

static void MKEY_Init(void);
static void KEY_Init(void); 
static u8 Get_KEY_State(u8 Key_Type, u16 time);

								    
/*******************************************************************
�����ܣ�KEYģ��Ļص�������100ms task
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

void MKEY_CallBack(void)
{
	KEY_Scan();
}

/*******************************************************************
�����ܣ�KEYģ��ĳ�ʼ��
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void MKEY_Init(void)
{
	KEY_Init();
	
	#if TEST_START
		TestKeyInit();
	#endif
}

//����
#if TEST_END
	void TestKeyInit(void)
	{
		GPIO_InitTypeDef GPIO_InitStructure;
 
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);   //ʹ��PORTAʱ��
		GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;              
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //���ó���������
		GPIO_Init(GPIOE, &GPIO_InitStructure);
	}
#endif

/*******************************************************************
�����ܣ�Motor_Stop_KEY��ʼ������
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
*******************************************************************/

static void KEY_Init(void) 
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);   //ʹ��PORTAʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14;              
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //���ó���������
 	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4 | GPIO_Pin_5;              
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //���ó���������
 	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*******************************************************************
�����ܣ�������ʽ�İ���ʶ��
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
				  
*******************************************************************/

static u8 Get_KEY_State(u8 Key_Type, u16 time)
{
		static u8 Key_State = N_KEY_State;         //����״̬��ʼ��
    static u8 Key_Return = N_KEY;                //��������ֵ��ʼ��
    static u16 num = 0;
    
    switch(Key_State)
    {
        case N_KEY_State:
        {
            Key_Return = N_KEY;
            
            if(Key_Type == 0)
            {
                Key_State = S_KEY_State;                   //����������ϵͳִ��һ������ʱ��
            }
            
            break;
        }
        
        case S_KEY_State:
        {
            if(Key_Type == 0)
            {
                Key_State = L_KEY_State;                //��������������ϵͳִ��һ������ʱ��
            }
            else
            {
                Key_State = N_KEY_State;
            }
            
            break;
        }
        
        case L_KEY_State:
        {        
            if((Key_Type == 1) && (Key_Return == N_KEY))        //��ֹ���������ͷź������̰�״̬�����Լ���һ��Key_Return����
            {
                Key_Return = S_KEY;
                Key_State = N_KEY_State;
                num = 0;
            }
            
            else if(Key_Type == 0)
            {
                num++;
                
                if(num >= time)                           //����ͨ���޸���ڲ���time�Ĵ�С��������������ʱ�䣬time = 20ʱ��ʱ��Լ1s
                {
                    Key_Return = L_KEY;
                    num = 0;
                }
				else
				{
						//do nothing;
				}
            }
            
            else if((Key_Type == 1) && (Key_Return == L_KEY))  //�ȴ������������ͷ�
            {
                Key_State = N_KEY_State;              
            }
						
			else
			{
					//do nothing;
			}
            
            break;
        }
				
		default:
		{
				break;
		}
    }
    
    return Key_Return;
}

/*******************************************************************
�����ܣ�����ɨ�裬�̰�enable������disable
����汾��V1.0
��    �ڣ� 2019/6/10
��    �ߣ�Orange
��    �ģ���
				  
*******************************************************************/

u8 KEY_Scan(void)
{
	static u8 Key_Sts = 0;
	
	if(S_KEY == Get_KEY_State(Rtn_KEY, KDelay_Time))
	{
		Key_Sts |= 0x02;                      
	}
	else if(L_KEY == Get_KEY_State(Rtn_KEY, KDelay_Time))
	{
		Key_Sts &= 0xfd; 
	}
	
	if(S_KEY == Get_KEY_State(Fled_KEY, KDelay_Time))
	{
		Key_Sts |= 0x04; 
	}
	else if(L_KEY == Get_KEY_State(Fled_KEY, KDelay_Time))
	{
		Key_Sts &= 0xfb;
	}
	
	if(S_KEY == Get_KEY_State(Led1_KEY, KDelay_Time))
	{
		Key_Sts |= 0x08;
	}
	else
	{
		Key_Sts &= 0xf7;
	}
	
	if(S_KEY == Get_KEY_State(Led2_KEY, KDelay_Time))
	{
		Key_Sts |= 0x10;
	}
	else
	{
		Key_Sts &= 0xef;
	}
	
	if(S_KEY == Get_KEY_State(Stop_KEY, KDelay_Time))
	{
		Key_Sts |= 0x40;                      //��λ
	}
	else if(L_KEY == Get_KEY_State(Stop_KEY, KDelay_Time))
	{
		Key_Sts &= 0xbf;                      //����
	}

	
	#if TEST_END
		if(S_KEY == Get_KEY_State(TEST_KEY, KDelay_Time))
		{
			Key_Sts |= 0x02;                      
		}
		else if(L_KEY == Get_KEY_State(TEST_KEY, KDelay_Time))
		{
			Key_Sts &= 0xfd; 
		}
	#endif
	
	return Key_Sts;
}

