#ifndef __MOTOR_SERIAL_H
#define __MOTOR_SERIAL_H

#include "main.h"

#define MOTOR_RX_BUF_SIZE 128

uint8_t usart1_rx_buf[MOTOR_RX_BUF_SIZE];
uint8_t usart1_frame_buf[MOTOR_RX_BUF_SIZE];

/*======================== 电机状态 ========================*/

/**
 * @brief 电机运行状态
 */
typedef enum
{
    MOTOR_IDLE = 0,
    MOTOR_RUNNING,
    MOTOR_ERROR,
    MOTOR_TIMEOUT
} MotorState;

/**
 * @brief 电机信息（给上层用）
 */
typedef struct
{
    MotorState state;        // 当前状态

    uint8_t online;          // 是否通信正常

    uint32_t last_tick;      // 最后一次收到回复时间（ms）

} MotorInfo;


#endif