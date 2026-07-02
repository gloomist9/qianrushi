#ifndef PLANNER_H
#define PLANNER_H

void planner_init(void);

void planner_process(void);

typedef enum
{
    PLANNER_IDLE,        // 空闲
    PLANNER_RUNNING,     // 当前正在执行
} PlannerState;

void motor_poll(void);

#endif
