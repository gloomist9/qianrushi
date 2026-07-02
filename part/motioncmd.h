/**
 ******************************************************************************
 * @file    motioncmd.h
 * @author  apex-CoreXY Project
 * @brief   运动命令数据结构定义
 *
 * @details
 * MotionCmd 是整个运动控制系统的数据载体。
 *
 * 数据流：
 *
 *      Parser
 *         │
 *         ▼
 *     MotionCmd
 *         │
 *         ▼
 *   Command Queue
 *         │
 *         ▼
 *      Planner
 *         │
 *         ▼
 *   Motor Driver
 *
 * 本文件只定义运动命令的数据结构，不涉及任何运动控制算法。
 ******************************************************************************
 */
#ifndef MOTIONCMD_H
#define MOTIONCMD_H
//运动命令
#include "main.h"

/**
 * @brief 运动类型
 *
 * 目前仅支持：
 * G0：快速移动
 * G1：直线运动
 *
 * 后续可扩展：
 * G2、G3 等圆弧插补。
 */
typedef enum
{
    MOTION_G0=0,    
    MOTION_G1       
} MotionType;

/**
 * @brief 一条运动命令
 *
 * @note
 * Queue 中保存的永远是完整坐标。
 */
typedef struct
{
    MotionType type;     /**< G0 或 G1 */

    float x;             /**< 目标 X 坐标（mm） */

    float y;             /**< 目标 Y 坐标（mm） */

    float feed;          /**< 运动速度（mm/min） */

} MotionCmd;
#endif
