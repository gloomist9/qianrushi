//运动指令环形队列
//Parser→queue_push→队列→queue_pop→Planner
#ifndef COMMAND_QUEUE_H
#define COMMAND_QUEUE_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>
#include "motioncmd.h"

//队列最大长度，需要更大的缓存改这里
#define COMMAND_QUEUE_SIZE    8

void queue_init(void);
void queue_clear(void);
bool queue_push(const MotionCmd *cmd);
bool queue_pop(MotionCmd *cmd);
bool queue_is_empty(void);
bool queue_is_full(void);
uint16_t queue_size(void);

#endif
