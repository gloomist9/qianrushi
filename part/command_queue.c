#include "command_queue.h"

//环形队列缓冲区
static MotionCmd queue_buffer[COMMAND_QUEUE_SIZE];

//tail: Planner取走的位置
static uint16_t tail = 0;

//head: 下一个写入位置
static uint16_t head = 0;

//当前队列里的指令数
static uint16_t count = 0;


void queue_init(void)
{
    head = 0;
    tail = 0;
    count = 0;
}


void queue_clear(void)
{
    head = 0;
    tail = 0;
    count = 0;
}


bool queue_is_empty(void)
{
    return (count == 0);
}


bool queue_is_full(void)
{
    return (count >= COMMAND_QUEUE_SIZE);
}


uint16_t queue_size(void)
{
    return count;
}


bool queue_push(const MotionCmd *cmd)
{
    if(cmd == NULL)
        return false;

    if(queue_is_full())
        return false;

    //写入
    queue_buffer[head] = *cmd;

    //head前移，到末尾回0
    head++;
    if(head >= COMMAND_QUEUE_SIZE)
        head = 0;

    count++;
    return true;
}


bool queue_pop(MotionCmd *cmd)
{
    if(cmd == NULL)
        return false;

    if(queue_is_empty())
        return false;

    //取出
    *cmd = queue_buffer[tail];

    //tail前移，到末尾回0
    tail++;
    if(tail >= COMMAND_QUEUE_SIZE)
        tail = 0;

    count--;
    return true;
}
