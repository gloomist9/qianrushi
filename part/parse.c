#include "parse.h"
#include "command_queue.h"
#include "motioncmd.h"
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

/*======================== 核心解析 ========================*/

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

    if(strncmp(line, "G21", 3) == 0)
    {
        is_mm = true;
        return PARSER_OK;
    }

    /*==================== 运动指令 ====================*/

    if(strncmp(line, "G0", 2) == 0 || strncmp(line, "G1", 2) == 0)
    {
        cmd.type = (line[1] == '0') ? MOTION_G0 : MOTION_G1;

        const char *p = line;

        bool ok;

        float x = current_x;
        float y = current_y;
        float feed = 0;

        while(*p)
        {
            p = skip_space(p);

            if(*p == 'X')
            { 
                p++;
                x = parse_float(p, &ok);
                if(!ok) return PARSER_FORMAT_ERROR;
            }
            else if(*p == 'Y')
            {
                p++;
                y = parse_float(p, &ok);
                if(!ok) return PARSER_FORMAT_ERROR;
            }
            else if(*p == 'F')
            {
                p++;
                feed = parse_float(p, &ok);
                if(!ok) return PARSER_FORMAT_ERROR;
            }

            while(*p && *p != ' ')
                p++;
        }

        /*================ 坐标模式处理 ================*/

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

        cmd.feed = feed;

        current_x = cmd.x;
        current_y = cmd.y;

        /*================ 入队 ================*/

        if(queue_push(&cmd))
        {
            return PARSER_OK;
        }
        else
        {
            return PARSER_QUEUE_FULL;
        }
    }

    /*==================== 暂不支持 ====================*/

    if(strncmp(line, "M", 1) == 0)
    {
        // M3 / M5 / M2 暂时不处理
        return PARSER_OK;
    }

    return PARSER_UNSUPPORTED_GCODE;
}

