#include "usefulfunction.h"
#include "Emm_V5.h"
#include "math.h"
extern float ltan;
extern float rtan;
extern volatile uint8_t interrupt, overflag;
//void motor1(float circule, bool raF)
//{
//    Emm_V5_MMCL_Pos_Control(1, 0, 500, 10, (uint32_t)circule/3200, raF, 1);
//}
//void motor2(uint8_t dir, float circule, bool raF)
//{
//    Emm_V5_MMCL_Pos_Control(2, 0, 500, 10, (uint32_t)circule/3200, raF, 1);
//}
void go(uint16_t X, uint16_t Y)//����ƶ�����
{
    int32_t A = (X+Y)*160;
    int32_t B = (X-Y)*160;
    uint8_t dira = 0;
	uint8_t dirb = 0;
    if(A<0) {A=-A; dira=1;}
    if(B<0) {B=-B; dirb=1;}
    Emm_V5_MMCL_Pos_Control(1, dira, 400, 254, A, 1, 1);
    Emm_V5_MMCL_Pos_Control(2, dirb, 400, 254, B, 1, 1);
    Emm_V5_Multi_Motor_Cmd(0);
}
/*
lo���Ͻ�
ld���½�
ro���Ͻ�
rd���½�
*/



void circle(uint16_t lo[2],uint16_t ld[2],uint16_t ro[2],uint16_t rd[2])//ͿĨ����
{
    float dx = (float)ld[0] - lo[0];
    float dy = (float)ld[1] - lo[1];

    uint16_t lines =(uint16_t)(sqrtf(dx*dx + dy*dy) + 0.5f);

    for(uint16_t i = 0; i <= lines; i++)
    {
        if(interrupt)
        {
            interrupt = 0;
            go(rd[0], rd[1]);
            return;
        }
        float t = (float)i / lines;

        uint16_t lx =
            (uint16_t)(lo[0] +
            t * (ld[0] - lo[0]) + 0.5f);

        uint16_t ly =
            (uint16_t)(lo[1] +
            t * (ld[1] - lo[1]) + 0.5f);

        uint16_t rx =
            (uint16_t)(ro[0] +
            t * (rd[0] - ro[0]) + 0.5f);

        uint16_t ry =
            (uint16_t)(ro[1] +
            t * (rd[1] - ro[1]) + 0.5f);

        if(i & 1)
        {    
            go(rx, ry);    
            HAL_Delay(50);    
    
            go(lx, ly);    
            HAL_Delay(350);    
        }    
        else
        {
            go(lx, ly);
            HAL_Delay(50);

            go(rx, ry);
            HAL_Delay(350);
        }
    }
    go(rd[0], rd[1]);
    overflag = 1;
}



//
//void circule(uint16_t lox,uint16_t loy,uint16_t ldx,uint16_t ldy,uint16_t rox,uint16_t roy,uint16_t rdx,uint16_t rdy)
//{
//	  go(rdx,rdy);
//		HAL_Delay(400);
////	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 60);
////	__HAL_TIM_SET_AUTORELOAD(&htim1, 3000);
////	HAL_Delay(500);
////	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
//    go(ldx,ldy);
//    HAL_Delay(350);
//    ldy+=1;
//    rdy+=1;
//    while(ldy<=loy&&rdy<=roy)
//    {
//        go(ldx,ldy);
//        HAL_Delay(50);
//        go(rdx,rdy);
//        HAL_Delay(350);
//        ldy+=1;
//        rdy+=1;
//        go(rdx,rdy);
//        HAL_Delay(50);
//        go(ldx,ldy);
//        HAL_Delay(350);
//        ldy+=1;
//        rdy+=1;
//    }
//}
