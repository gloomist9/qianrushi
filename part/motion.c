#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "motion.h"
#include "motioncmd.h"
#include "Emm_V5.h"

//void motor1(float circule, bool raF)
//{
//    Emm_V5_MMCL_Pos_Control(1, 0, 500, 10, (uint32_t)circule/3200, raF, 1);
//}
//void motor2(uint8_t dir, float circule, bool raF)
//{
//    Emm_V5_MMCL_Pos_Control(2, 0, 500, 10, (uint32_t)circule/3200, raF, 1);
//}
#define STEPS_PER_MM 160.0f//细分

MotorState motor1;
MotorState motor2;

static void go(float X, float Y)//运动到指定位置
{
    int32_t A = (int32_t)((X+Y)*STEPS_PER_MM);
    int32_t B = (int32_t)((X-Y)*STEPS_PER_MM);
    uint8_t dira = 0;
	uint8_t dirb = 0;
    if(A<0) {A=-A; dira=1;}
    if(B<0) {B=-B; dirb=1;}
    Emm_V5_MMCL_Pos_Control(1, dira, 400, 254, A, 1, 1);
    Emm_V5_MMCL_Pos_Control(2, dirb, 400, 254, B, 1, 1);
    Emm_V5_Multi_Motor_Cmd(0);

    motor1.moving = true;
    motor2.moving = true;
}

void motor_init(void)
{
    motor1.moving = false;
    motor2.moving = false;

    motor1.online = true;
    motor2.online = true;
}

void motion_execute(MotionCmd *cmd)
{
    go((float)cmd->x,(float)cmd->y);
}

bool motion_is_busy(void)
{
    return motor1.moving || motor2.moving;
}

