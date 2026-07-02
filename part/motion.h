#ifndef MOTION_H
#define MOTION_H

typedef struct
{
    bool moving;      // 是否正在运动
    bool online;      // 是否在线
} MotorState;

void circle(uint16_t lo[2],uint16_t ld[2],uint16_t ro[2],uint16_t rd[2]);

void motor_init(void);

bool motion_is_busy(void);

void motion_execute(MotionCmd *cmd);

#endif
