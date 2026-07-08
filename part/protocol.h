//通讯协议
//单字节状态码经USART3回传上位机
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "main.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    PROTO_OK = 0x00,
    PROTO_BUSY = 0x01,
    PROTO_PARSE_ERROR = 0x02,
    PROTO_UNSUPPORTED = 0x03
} ProtoStatus;

void protocol_send_status(ProtoStatus status);
void protocol_handle_parser_result(int parser_result);

#endif
