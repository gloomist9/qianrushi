//运动执行模块
//运动指令→CoreXY坐标变换→电机驱动，speed单位mm/min
#ifndef MOTION_H
#define MOTION_H

#include "motioncmd.h"

void go(float X, float Y, float speed_mm_min);

void motion_execute(MotionCmd *cmd);

#endif
