#ifndef PERSE_H
#define PERSE_H

#include "main.h"

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Parser执行结果
 */
typedef enum
{
    PARSER_OK = 0,

    PARSER_EMPTY_LINE,

    PARSER_UNSUPPORTED_GCODE,

    PARSER_FORMAT_ERROR,

    PARSER_QUEUE_FULL
} ParserResult;

/**
 * @brief 初始化Parser
 */
void parser_init(void);

/**
 * @brief 解析一行G-code
 *
 * @param line 输入字符串
 */
ParserResult parser_parse_line(const char *line);

#endif
