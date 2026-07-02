#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "motion.h"
#include "motioncmd.h"
#include "Emm_V5.h"
#include "motor_serial.h"

#define STEPS_PER_MM 160.0f//细分

/* 引用 motor_serial.c 中定义的电机实例 */
extern MotorInfo motor1;
extern MotorInfo motor2;

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

    /* 使用 motor_serial 的状态管理：标记电机正在运行 */
    motor1.state = MOTOR_RUNNING;
    motor1.last_tick = HAL_GetTick();
    motor2.state = MOTOR_RUNNING;
    motor2.last_tick = HAL_GetTick();
}

void motion_execute(MotionCmd *cmd)
{
    go((float)cmd->x,(float)cmd->y);
}
