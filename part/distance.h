//距离传感器
//解析串口数据提取距离值，连续3次小于阈值判定
#ifndef __DISTANCE_H
#define __DISTANCE_H
#include "main.h"
#define buffer 64

extern uint8_t distance_buffer[buffer];
extern uint16_t distance;
uint16_t Get_distance(void);
uint8_t distancejudge(void);
#endif
