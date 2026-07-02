/**
 ******************************************************************************
 * @file    command_queue.c
 * @author  Mini Motion Controller Project
 * @brief   运动命令环形队列实现
 ******************************************************************************
 */

#include "command_queue.h"

/*============================ 私有变量 ============================*/

/**
 * @brief 环形队列缓冲区
 *
 * queue_buffer[] 用于存放等待执行的运动命令。
 *
 * Parser:
 *      queue_push()
 *
 * Planner:
 *      queue_pop()
 */
static MotionCmd queue_buffer[COMMAND_QUEUE_SIZE];

/**
 * @brief 队首索引（出队位置）
 *
 * 指向下一条即将被 Planner 取走的命令。
 */
static uint16_t tail = 0;

/**
 * @brief 队尾索引（入队位置）
 *
 * 指向下一条准备写入的位置。
 */
static uint16_t head = 0;

/**
 * @brief 当前队列中命令数量
 */
static uint16_t count = 0;


/*============================ 接口实现 ============================*/

/**
 * @brief 初始化队列
 */
void queue_init(void)
{
    head = 0;
    tail = 0;
    count = 0;
}


/**
 * @brief 清空队列
 */
void queue_clear(void)
{
    head = 0;
    tail = 0;
    count = 0;
}


/**
 * @brief 判断队列是否为空
 */
bool queue_is_empty(void)
{
    return (count == 0);
}


/**
 * @brief 判断队列是否已满
 */
bool queue_is_full(void)
{
    return (count >= COMMAND_QUEUE_SIZE);
}


/**
 * @brief 获取当前队列长度
 */
uint16_t queue_size(void)
{
    return count;
}


/**
 * @brief 入队
 *
 * @param cmd
 *      待加入队列的运动命令
 *
 * @retval true
 *      入队成功
 *
 * @retval false
 *      队列已满
 */
bool queue_push(const MotionCmd *cmd)
{
    /*------------------------
      参数检查
    ------------------------*/

    if(cmd == NULL)
    {
        return false;
    }

    /*------------------------
      判断队列是否已满
    ------------------------*/

    if(queue_is_full())
    {
        return false;
    }

    /*------------------------
      写入命令
    ------------------------*/

    queue_buffer[head] = *cmd;

    /*------------------------
      head向后移动一格
      超过末尾后自动回到0
    ------------------------*/

    head++;

    if(head >= COMMAND_QUEUE_SIZE)
    {
        head = 0;
    }

    /*------------------------
      当前命令数量+1
    ------------------------*/

    count++;

    return true;
}


/**
 * @brief 出队
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
bool queue_pop(MotionCmd *cmd)
{
    /*------------------------
      参数检查
    ------------------------*/

    if(cmd == NULL)
    {
        return false;
    }

    /*------------------------
      判断队列是否为空
    ------------------------*/

    if(queue_is_empty())
    {
        return false;
    }

    /*------------------------
      取出一条命令
    ------------------------*/

    *cmd = queue_buffer[tail];

    /*------------------------
      tail向后移动
    ------------------------*/

    tail++;

    if(tail >= COMMAND_QUEUE_SIZE)
    {
        tail = 0;
    }

    /*------------------------
      当前命令数量-1
    ------------------------*/

    count--;

    return true;
}

