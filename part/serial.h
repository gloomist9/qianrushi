#ifndef SERIAL_H
#define SERIAL_H

//与上位机通讯
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define LINE_BUF_SIZE 64//最大限制
#define CMD_QUEUE_SIZE 32//最大限制

/**
 * @brief 初始化串口接收（DMA + IDLE）
 */
void serial_init(void);

/**
 * @brief 串口接收处理函数
 *
 * @note
 * 在 USART IDLE 中断里调用
 */
void serial_rx_idle_callback(void);

/**
 * @brief 主循环调用（处理完整行）
 */
void serial_process(void);

//typedef struct {
//    MotionCmd buf[CMD_QUEUE_SIZE];
//    uint8_t head;
//    uint8_t tail;
//    uint8_t count;
//} CmdQueue;



#endif
