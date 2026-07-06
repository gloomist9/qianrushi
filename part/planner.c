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
        {
            return;
        }

        planner_busy = false;
    }

    /* 没有新的运动指令 */
    if(!queue_pop(&cmd))
        return;


    /* 笔控制由 M3/M5 在 parse 阶段直接处理 */

    /* 等待舵机到位 */
    while (pulse_remaining > 0);

    /* 执行运动指令 */
    motion_execute(&cmd);

    planner_busy = true;
}

void motor_poll(void)//轮询电机状态 + 看门狗
{
    if(HAL_GetTick() - last_poll < 10)
        return;

    last_poll = HAL_GetTick();


    /* 交替查询：两个电机共用 RS-485 总线，同时查询会导致冲突
     * 注意：必须用直接发送版本，MMCL 批量模式下 Read 子命令电机不回复 */
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

    /* 检测电机是否超时无响应 */
    motor_watchdog();
}
