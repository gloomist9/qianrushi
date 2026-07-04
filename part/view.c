#include "usart.h"
#include "main.h"

extern UART_HandleTypeDef huart4;

uint8_t RxByte;
volatile uint8_t viewflag = 0;
float ltan=0;
float rtan=0;
extern uint16_t lo[];
extern uint16_t ld[];
extern uint16_t ro[];
extern uint16_t rd[];
volatile uint8_t interrupt = 0, overflag = 0;//����Ƿ��ƶ��Ӷ���Ҫ���,overflag��ʾ�Ƿ����
uint16_t PointData[8];

uint8_t getflag(void){
	uint8_t t=0;
	if(viewflag){viewflag=0;t=1;}
	return t;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint8_t RxState = 0;
    static uint8_t Index = 0;
    static uint8_t RxBuf[16];
    static uint8_t time=0;
    if(huart == &huart4)
    {
        uint8_t RxData = RxByte;

        switch(RxState)
        {
            case 0:
            {
                if(RxData == 0xAA)
                {
                    Index = 0;
                    RxState = 1;
                }
                break;
            }

            case 1:
            {
                RxBuf[Index++] = RxData;

                if(Index >= 16)
                {
                    RxState = 2;
                }

                break;
            }

            case 2:
            {
                if(RxData == 0xFF)
                {
                    lo[0] = RxBuf[0]  | (RxBuf[1]  << 8);
					lo[1] = RxBuf[2]  | (RxBuf[3]  << 8);
			            
			        ro[0] = RxBuf[4]  | (RxBuf[5]  << 8);
			        ro[1] = RxBuf[6]  | (RxBuf[7]  << 8);
			            
			        ld[0] = RxBuf[8]  | (RxBuf[9]  << 8);
			        ld[1] = RxBuf[10] | (RxBuf[11] << 8);
			            
			        rd[0] = RxBuf[12] | (RxBuf[13] << 8);
			        rd[1] = RxBuf[14] | (RxBuf[15] << 8);

                    viewflag =1;
                    
                    time++;
                    if(overflag){overflag=0;time=0;}//����Ѿ�������˶����Ͱ�time����
                    if(time>=1){interrupt=1;}//���ͳ����������Ϊ��ݱ��ƶ��ˣ���ϵ�ǰ���˶�

                }
				else
				{
					RxState = 0; 
				}
                RxState = 0;
                break;
            }
        }
    }

    HAL_UART_Receive_IT(&huart4, &RxByte, 1);
}
