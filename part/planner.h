//运动规划器
//从队列取指令→等上一段走完→等舵机到位→下发执行
#ifndef PLANNER_H
#define PLANNER_H

void planner_init(void);
void planner_process(void);
void planner_wait_idle(void);

typedef enum
{
    PLANNER_IDLE,
    PLANNER_RUNNING,
} PlannerState;

void motor_poll(void);

#endif
