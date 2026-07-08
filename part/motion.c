#include "main.h"
#include "motion.h"
#include "motioncmd.h"
#include "normaldj.h"
#include "Emm_V5.h"
#include "motor_serial.h"

#define STEPS_PER_MM 80.0f  // X=81.8 Y=78.1 校准取平均

//坐标缩放：G-code坐标×SCALE→实际mm，物理行程不够时改小
#define SCALE_FACTOR 0.95f

//默认速度(mm/min)
#define DEFAULT_G0_SPEED 2500.0f  //G0快速移动默认速度
#define DEFAULT_G1_SPEED 1500.0f  //G1画线默认速度

extern MotorInfo motor1;
extern MotorInfo motor2;

//运动到指定位置（CoreXY变换），speed为mm/min
void go(float X, float Y, float speed_mm_min)
{
    //mm/min → step/s
    uint16_t spd = (uint16_t)(speed_mm_min * STEPS_PER_MM / 60.0f);

    Y = -Y;
    int32_t A = (int32_t)((X+Y)*STEPS_PER_MM);
    int32_t B = (int32_t)((X-Y)*STEPS_PER_MM);
    uint8_t dira = 0;
    uint8_t dirb = 0;
    if(A<0) {A=-A; dira=1;}
    if(B<0) {B=-B; dirb=1;}
    Emm_V5_MMCL_Pos_Control(1, dira, spd, 245, A, 1, 1);
    Emm_V5_MMCL_Pos_Control(2, dirb, spd, 245, B, 1, 1);
    Emm_V5_Multi_Motor_Cmd(0);

    motor1.state = MOTOR_RUNNING;
    motor1.last_tick = HAL_GetTick();
    motor2.state = MOTOR_RUNNING;
    motor2.last_tick = HAL_GetTick();
}

extern bool g_manual_pen_mode;
static bool pen_down = false;

void motion_execute(MotionCmd *cmd)
{
    //手动模式(M3/M5)下不自动控制笔
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

    float spd = cmd->speed;
    if (spd <= 0)
        spd = (cmd->type == MOTION_G0) ? DEFAULT_G0_SPEED : DEFAULT_G1_SPEED;

    go(x, y, spd);
}
