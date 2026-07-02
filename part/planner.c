#include "main.h"
#include "planner.h"
#include "command_queue.h"
#include "motioncmd.h"
#include "motion.h"
#include "Emm_V5.h"
#include <stdbool.h>

extern volatile uint8_t interrupt, overflag;

static bool busy = false;
static uint32_t last_poll = 0;//上次轮询时刻
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

    /* 电机还没完成 */
    if(planner_busy)
    {
        if(motion_is_busy())
            return;

        planner_busy = false;
    }

    /* 没有新的运动 */
    if(!queue_pop(&cmd))
        return;

    /* 发给运动层 */
    motion_execute(&cmd);

    planner_busy = true;
}

void motor_poll(void)//轮询电机
{
    if(HAL_GetTick() - last_poll < 5)
        return;

    last_poll = HAL_GetTick();

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
}


