//运动命令数据结构
//Parser→MotionCmd→Queue→Planner→Motor
#ifndef MOTIONCMD_H
#define MOTIONCMD_H
#include "main.h"

typedef enum
{
    MOTION_G0=0,  //快速移动
    MOTION_G1     //直线运动
} MotionType;

typedef struct
{
    MotionType type;
    float x;        //目标X坐标(mm)
    float y;        //目标Y坐标(mm)
    float speed;    //进给速度(mm/min)，0表示用默认值
} MotionCmd;
#endif
