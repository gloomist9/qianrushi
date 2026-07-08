#include "main.h"
#include "planner.h"
#include "command_queue.h"
#include "motioncmd.h"
#include "motion.h"
#include "normaldj.h"
#include "Emm_V5.h"
#include "motor_serial.h"
#include <stdbool.h>

extern volatile uint16_t pulse_remaining;//舵机脉冲计数，main.c定义

static uint32_t last_poll = 0;
static uint8_t poll_motor = 0;

static bool planner_busy = false;//当前是否正在运动

void planner_init(void)
{
    planner_busy = false;
}

void planner_process(void)
{
    MotionCmd cmd;

    //等上一段走完
    if(planner_busy)
    {
        if(motion_is_busy())
            return;

        planner_busy = false;
    }

    //取一条指令
    if(!queue_pop(&cmd))
        return;

    //笔控制由M3/M5在parse阶段处理

    //等舵机到位
    while (pulse_remaining > 0);

    //执行
    motion_execute(&cmd);

    planner_busy = true;
}

//等所有运动走完（阻塞）
void planner_wait_idle(void)
{
    while (1)
    {
        planner_process();
        motor_poll();

        if (queue_is_empty() && !motion_is_busy())
            break;
    }
}

//轮询电机状态+看门狗
void motor_poll(void)
{
    if(HAL_GetTick() - last_poll < 2)
        return;

    last_poll = HAL_GetTick();

    //两个电机共用RS-485，交替查询
    if(poll_motor == 0)
    {
        Emm_V5_Read_Sys_Params(1, S_FLAG);
        poll_motor = 1;
    }
    else
    {
        Emm_V5_Read_Sys_Params(2, S_FLAG);
        poll_motor = 0;
    }

    motor_watchdog();
}
