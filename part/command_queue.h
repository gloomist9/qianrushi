/**
 ******************************************************************************
 * @file    command_queue.h
 * @author  apex-CoreXY Project
 * @brief   运动命令环形队列
 *
 * @details
 * 本模块负责管理 MotionCmd 队列。
 *
 * 数据流：
 *
 *      Parser
 *         │
 *         ▼
 *     queue_push()
 *         │
 *         ▼
 *   Command Queue
 *         │
 *         ▼
 *     queue_pop()
 *         │
 *         ▼
 *      Planner
 *
 * 本模块仅负责队列管理。
 *
 ******************************************************************************
 */
#ifndef COMMANDE_QUEUE_H
#define COMMANDE_QUEUE_H
//保存等待执行的运动命令

#include "main.h"

#include <stdbool.h>
#include <stdint.h>

#include "motioncmd.h"


/*============================ 宏定义 ============================*/

/**
 * @brief 命令队列长度
 *
 * 当前版本：
 * 32条运动命令
 *
 * 后续如果需要更大的缓存，
 * 只需修改此宏即可。
 */
#define COMMAND_QUEUE_SIZE    32


/*============================ 对外接口 ============================*/


/**
 * @brief 初始化命令队列
 *
 * @note
 * 系统上电后调用一次即可。
 */
void queue_init(void);

/**
 * @brief 清空命令队列
 *
 * @note
 * 已缓存但尚未执行的命令将全部丢弃。
 */
void queue_clear(void);

/**
 * @brief 命令入队
 *
 * @param cmd
 *      指向待入队运动命令
 *
 * @retval true
 *      入队成功
 *
 * @retval false
 *      队列已满
 */
bool queue_push(const MotionCmd *cmd);

/**
 * @brief 命令出队
 *
 * @param cmd
 *      输出一条运动命令
 *
 * @retval true
 *      出队成功
 *
 * @retval false
 *      队列为空
 */
bool queue_pop(MotionCmd *cmd);

/**
 * @brief 判断队列是否为空
 *
 * @retval true
 *      队列为空
 *
 * @retval false
 *      队列非空
 */
bool queue_is_empty(void);

/**
 * @brief 判断队列是否已满
 *
 * @retval true
 *      队列已满
 *
 * @retval false
 *      队列未满
 */
bool queue_is_full(void);

/**
 * @brief 获取当前队列中的命令数量
 *
 * @return
 * 当前缓存的命令数量
 */
uint16_t queue_size(void);

#endif
