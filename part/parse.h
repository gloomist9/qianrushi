//G-code解析器
//串口收一行→解析→入队/执行
#ifndef PARSE_H
#define PARSE_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PARSER_OK = 0,
    PARSER_EMPTY_LINE,
    PARSER_UNSUPPORTED_GCODE,
    PARSER_FORMAT_ERROR,
    PARSER_QUEUE_FULL
} ParserResult;

ParserResult parser_parse_line(const char *line);

#endif
