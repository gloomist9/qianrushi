//电机串口通讯
//USART1 DMA+IDLE收电机回复，交替轮询两个电机状态
#ifndef __MOTOR_SERIAL_H
#define __MOTOR_SERIAL_H

#include "main.h"

typedef enum
{
    MOTOR_IDLE = 0,
    MOTOR_RUNNING,
    MOTOR_ERROR,
    MOTOR_TIMEOUT
} MotorState;

typedef struct
{
    uint8_t id;
    MotorState state;
    uint8_t online;
    uint32_t last_tick;
    uint8_t expect_reply;
} MotorInfo;

void motor_serial_init(void);
void motor_frame_parse(uint8_t *buf, uint16_t len);
uint8_t motion_is_busy(void);
void motor_watchdog(void);
MotorInfo* get_motor_by_id(uint8_t id);

#endif
