#include "parse.h"
#include "command_queue.h"
#include "motioncmd.h"
#include "normaldj.h"
#include "planner.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>




/*======================== 内部状态 ========================*/

/**
 * @brief 坐标模式
 * true = 绝对坐标 (G90)
 * false = 相对坐标 (G91)
 */
static bool is_absolute = true;

/**
 * @brief 当前单位是否为mm
 * 第一版默认只支持mm
 */
static bool is_mm = true;

/**
  * @brief 当前坐标（用于补全G-code）
 */
static float current_x = 0.0f;
static float current_y = 0.0f;

/**
 * @brief 手动笔模式：M3/M5 触发后置 true，G00/G01 不再自动控制笔
 */
bool g_manual_pen_mode = false;

/*======================== 工具函数 ========================*/

/**
 * @brief 跳过空格
 */
static const char* skip_space(const char *str)
{
    while(*str == ' ' || *str == '\t')
        str++;
    return str;
}

/**
 * @brief 解析浮点数参数
 */
static float parse_float(const char *str, bool *ok)
{
    char *end;
    float val = strtof(str, &end);

    if(end == str)
    {
        *ok = false;
        return 0;
    }

    *ok = true;
    return val;
}

/*========================  核心解析 ========================*/

ParserResult parser_parse_line(const char *line)
{
    if(line == NULL)
        return PARSER_EMPTY_LINE;

    line = skip_space(line);

    if(*line == '\0')
        return PARSER_EMPTY_LINE;

    MotionCmd cmd;
    memset(&cmd, 0, sizeof(cmd));

    /*==================== G-code解析 ====================*/

    if(strncmp(line, "G90", 3) == 0)
    {
        is_absolute = true;
        return PARSER_OK;
    }

    if(strncmp(line, "G91", 3) == 0)
    {
        is_absolute = false;
        return PARSER_OK;
    }

    if(strncmp(line, "G17", 3) == 0)
    {
        return PARSER_OK;
    }

    if(strncmp(line, "G21", 3) == 0)
    {
        is_mm = true;
        return PARSER_OK;
    }

    /* G4 / G04 延时 */
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
                    /* 先把队列里的抬笔/移动指令执行完，再延时 */
                    planner_process();
                    HAL_Delay((uint32_t)(delay * 1000.0f));
                }
                break;
            }
            p++;
        }
        return PARSER_OK;
    }

    /*==================== M代码 ====================*/

    if(strncmp(line, "M3", 2) == 0)
    {
        g_manual_pen_mode = true;
        /* 先把队列里排队的 G0 抬笔移动执行完，再落笔 */
        planner_process();
        pendown();
        return PARSER_OK;
    }

    if(strncmp(line, "M5", 2) == 0)
    {
        g_manual_pen_mode = true;
        /* 先把队列里排队的 G1 画线执行完，再抬笔 */
        planner_process();
        penrise();
        return PARSER_OK;
    }

    /*====================运动指令====================*/

    if(strncmp(line, "G0", 2) == 0 || strncmp(line, "G1", 2) == 0)
    {
        const char *g = line + 1;
        while(*g == '0') g++;
        cmd.type = (*g == '1') ? MOTION_G1 : MOTION_G0;

        const char *p = line;
        bool ok;
        float x = current_x;
        float y = current_y;

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
            else
            {
                p++;
            }
        }

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
        {
            return PARSER_OK;
        }
        else
        {
            return PARSER_QUEUE_FULL;
        }
    }

    /* F 单独一行（如 F1500），忽略，速度由固件控制 */
    if(strncmp(line, "F", 1) == 0)
    {
        return PARSER_OK;
    }

    /* 其他 M 代码直接忽略 */
    if(strncmp(line, "M", 1) == 0)
    {
        return PARSER_OK;
    }

    return PARSER_UNSUPPORTED_GCODE;
}
