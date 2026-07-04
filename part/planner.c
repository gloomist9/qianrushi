#include "main.h"
#include "planner.h"
#include "command_queue.h"
#include "motioncmd.h"
#include "motion.h"
#include "normaldj.h"
#include "Emm_V5.h"
#include "motor_serial.h"
#include <stdbool.h>

/* main.c 定义的脉冲计数器，用于等待舵机到位 */
extern volatile uint16_t pulse_remaining;

static uint32_t last_poll = 0;//上次轮询时间
static uint8_t poll_motor = 0;

/* 当前是否正在运动 */
static bool planner_busy = false;

void planner_init(void)
{
    planner_busy = false;
}

void planner_process(void)
{
    MotionCmd cmd;

    /* 等待运动完成 */
    if(planner_busy)
    {
        if(motion_is_busy())
            return;

        planner_busy = false;
    }

    /* 没有新的运动指令 */
    if(!queue_pop(&cmd))
        return;

    /* G0 抬笔（快速移动），G1 落笔（画线） */
    if (cmd.type == MOTION_G0)
        penrise();
    else if (cmd.type == MOTION_G1)
        pendown();

    /* 等待舵机到位（255 个 PWM 脉冲 ≈ 51ms） */
    while (pulse_remaining > 0);

    /* 执行运动指令 */
    motion_execute(&cmd);

    planner_busy = true;
}

void motor_poll(void)//轮询电机状态 + 看门狗
{
    if(HAL_GetTick() - last_poll < 5)
        return;

    last_poll = HAL_GetTick();

    /* 交替查询两个电机的状态标志 */
    if(poll_motor == 0)
    {
        Emm_V5_Read_Sys_Params(1,S_FLAG);//发送查询命令
        poll_motor = 1;
    }
    else
    {
        Emm_V5_Read_Sys_Params(2,S_FLAG);
        poll_motor = 0;
    }

    /* 检测电机是否超时无响应 */
    motor_watchdog();
}
