#include "parse.h"
#include "command_queue.h"
#include "motioncmd.h"
#include "normaldj.h"
#include "planner.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


//当前坐标模式：true=绝对(G90)，false=相对(G91)
static bool is_absolute = true;

//当前单位：true=mm(G21)，false=inch(G20)
static bool is_mm = true;

//当前坐标（G-code补全用）
static float current_x = 0.0f;
static float current_y = 0.0f;

//当前进给速度(mm/min)
static float current_f = 1500.0f;

//手动笔模式：收到M3/M5后置true，G0/G1不再自动控制笔
bool g_manual_pen_mode = false;

//跳过空格
static const char* skip_space(const char *str)
{
    while(*str == ' ' || *str == '\t')
        str++;
    return str;
}

//去掉行内的括号注释，如 "G1 X10 (cut here)" → "G1 X10 "
static void strip_comment(char *line)
{
    char *s = line;
    char *d = line;
    int depth = 0;
    while (*s)
    {
        if (*s == '(') { depth++; s++; continue; }
        if (depth > 0)
        {
            if (*s == ')') { depth--; s++; continue; }
            s++;
            continue;
        }
        *d++ = *s++;
    }
    *d = '\0';
}

//解析一行G-code
ParserResult parser_parse_line(const char *line_in)
{
    if(line_in == NULL)
        return PARSER_EMPTY_LINE;

    //拷贝到可修改的缓冲区，去掉注释
    char line_buf[128];
    strncpy(line_buf, line_in, sizeof(line_buf)-1);
    line_buf[sizeof(line_buf)-1] = '\0';
    strip_comment(line_buf);

    const char *line = skip_space(line_buf);

    if(*line == '\0')
        return PARSER_EMPTY_LINE;

    MotionCmd cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.speed = 0;

    //========== G-code ==========

    //G90 绝对坐标
    if(strncmp(line, "G90", 3) == 0)
    {
        is_absolute = true;
        return PARSER_OK;
    }

    //G91 相对坐标
    if(strncmp(line, "G91", 3) == 0)
    {
        is_absolute = false;
        return PARSER_OK;
    }

    //G17 XY平面（2D绘图默认）
    if(strncmp(line, "G17", 3) == 0)
        return PARSER_OK;

    //G20 英寸（不支持，但明确拒绝）
    if(strncmp(line, "G20", 3) == 0)
        return PARSER_UNSUPPORTED_GCODE;

    //G21 毫米
    if(strncmp(line, "G21", 3) == 0)
    {
        is_mm = true;
        return PARSER_OK;
    }

    //G4/G04 延时
    if(strncmp(line, "G4", 2) == 0 || strncmp(line, "G04", 3) == 0)
    {
        const char *p = line;
        while(*p)
        {
            p = skip_space(p);
            if(*p == 'P')
            {
                p++;
                char *end;
                float delay = strtof(p, &end);
                if(end != p)
                {
                    planner_wait_idle();
                    HAL_Delay((uint32_t)(delay * 1000.0f));
                }
                break;
            }
            p++;
        }
        return PARSER_OK;
    }

    //G28 回零（空实现，防止报错）
    if(strncmp(line, "G28", 3) == 0)
        return PARSER_OK;

    //G92 设置当前位置
    if(strncmp(line, "G92", 3) == 0)
    {
        //解析X/Y参数设置坐标原点偏移
        const char *p = line;
        while(*p)
        {
            p = skip_space(p);
            if(*p == 'X') { p++; char *e; current_x = strtof(p, &e); if(e!=p) p=e; else p++; }
            else if(*p == 'Y') { p++; char *e; current_y = strtof(p, &e); if(e!=p) p=e; else p++; }
            else p++;
        }
        return PARSER_OK;
    }

    //========== M代码 ==========

    //M2/M30 程序结束
    if(strncmp(line, "M2", 2) == 0 || strncmp(line, "M30", 3) == 0)
    {
        planner_wait_idle();
        return PARSER_OK;
    }

    //M3 落笔（支持 M3Sxxx 格式）
    if(strncmp(line, "M3", 2) == 0)
    {
        g_manual_pen_mode = true;
        //顺便解析S参数（只存不实际用，笔没有转速概念）
        const char *p = line;
        while(*p)
        {
            p = skip_space(p);
            if(*p == 'S') { p++; char *e; strtof(p, &e); if(e!=p) p=e; else p++; }
            else p++;
        }
        planner_wait_idle();
        pendown();
        return PARSER_OK;
    }

    //M5 抬笔
    if(strncmp(line, "M5", 2) == 0)
    {
        g_manual_pen_mode = true;
        planner_wait_idle();
        penrise();
        return PARSER_OK;
    }

    //M84 释放电机
    if(strncmp(line, "M84", 3) == 0)
        return PARSER_OK;

    //========== 运动指令 G0/G1 ==========

    if(strncmp(line, "G0", 2) == 0 || strncmp(line, "G1", 2) == 0)
    {
        const char *g = line + 1;
        while(*g == '0') g++;
        cmd.type = (*g == '1') ? MOTION_G1 : MOTION_G0;

        const char *p = line;
        float x = current_x;
        float y = current_y;
        float f = 0;//本行指定的F值，0表示沿用上一次

        while(*p)
        {
            p = skip_space(p);

            if(*p == 'X')
            {
                p++;
                char *end;
                x = strtof(p, &end);
                if(end == p) return PARSER_FORMAT_ERROR;
                p = end;
            }
            else if(*p == 'Y')
            {
                p++;
                char *end;
                y = strtof(p, &end);
                if(end == p) return PARSER_FORMAT_ERROR;
                p = end;
            }
            else if(*p == 'F')
            {
                p++;
                char *end;
                f = strtof(p, &end);
                if(end != p) current_f = f;
                p = end;
            }
            else
            {
                p++;
            }
        }

        cmd.speed = (f > 0) ? f : current_f;

        if(is_absolute)
        {
            cmd.x = x;
            cmd.y = y;
        }
        else
        {
            cmd.x = current_x + x;
            cmd.y = current_y + y;
        }

        current_x = cmd.x;
        current_y = cmd.y;

        if(queue_push(&cmd))
            return PARSER_OK;
        else
            return PARSER_QUEUE_FULL;
    }

    //F 单独一行（如 "F1500"），更新默认进给速度
    if(strncmp(line, "F", 1) == 0)
    {
        const char *p = line + 1;
        p = skip_space(p);
        char *end;
        float f = strtof(p, &end);
        if(end != p) current_f = f;
        return PARSER_OK;
    }

    //其他 M 代码，忽略
    if(strncmp(line, "M", 1) == 0)
        return PARSER_OK;

    return PARSER_UNSUPPORTED_GCODE;
}
