//笔控制+舵机驱动
//GPIO PB3方向+PWM脉冲控制抬笔落笔
#ifndef __NORMAL_H
#define __NORMAL_H

void penrise(void);
void pendown(void);
void penstop(void);
void motor_pulse_start(uint16_t pulse_count);

#endif
