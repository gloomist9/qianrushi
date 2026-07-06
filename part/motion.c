#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "motion.h"
#include "motioncmd.h"
#include "normaldj.h"
#include "Emm_V5.h"
#include "motor_serial.h"

#define STEPS_PER_MM 80.0f  // X=81.8 Y=78.1 校准取平均

/*
 * 坐标缩放系数：G-code 坐标 × SCALE → 实际 mm
 * 如果 G-code 的活动范围大于物理行程，设一个小于 1 的值等比缩小
 * 例如物理 Y 最大 192mm，G-code Y 最大 300 → SCALE = 192/300 = 0.64
 */
#define SCALE_FACTOR 0.95f

/* 引用 motor_serial.c 中定义的电机实例 */
extern MotorInfo motor1; 
extern MotorInfo motor2;

void go(float X, float Y)//运动到指定位置
{
    
    
	  Y = -Y;  
    int32_t A = (int32_t)((X+Y)*STEPS_PER_MM);
    int32_t B = (int32_t)((X-Y)*STEPS_PER_MM);
    uint8_t dira = 0;
    uint8_t dirb = 0;
    if(A<0) {A=-A; dira=1;}
    if(B<0) {B=-B; dirb=1;}
    Emm_V5_MMCL_Pos_Control(1, dira, 1000, 230, A, 1, 1);
    Emm_V5_MMCL_Pos_Control(2, dirb, 1000, 230, B, 1, 1);
    Emm_V5_Multi_Motor_Cmd(0);

    /* 使用 motor_serial 的状态管理：标记电机正在运行 */
    motor1.state = MOTOR_RUNNING;
    motor1.last_tick = HAL_GetTick();
    motor2.state = MOTOR_RUNNING;
    motor2.last_tick = HAL_GetTick();
}

extern bool g_manual_pen_mode;
static bool pen_down = false;

void motion_execute(MotionCmd *cmd)
{
    /* 手动模式（M3/M5）下跳过 G-code 自动笔控制 */
    if (!g_manual_pen_mode)
    {
        if (cmd->type == MOTION_G0)
        {
            if (pen_down) { penrise(); pen_down = false; }
        }
        else
        {
            if (!pen_down) { pendown(); pen_down = true; }
        }
    }

    float x = cmd->x * SCALE_FACTOR;
    float y = cmd->y * SCALE_FACTOR;
    go(x, y);
}
