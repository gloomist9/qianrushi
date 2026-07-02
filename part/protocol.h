#ifndef PROTOCOL_H
#define PROTOCOL_H
//通讯协议
#include "main.h"
#include "usart.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief 通信状态码（单字节）
 */
typedef enum
{
    PROTO_OK = 0x00,
    PROTO_BUSY = 0x01,
    PROTO_PARSE_ERROR = 0x02,
    PROTO_UNSUPPORTED = 0x03

} ProtoStatus;



/**
 * @brief 发送状态码给上位机
 *
 * @param status 状态码
 */
void protocol_send_status(ProtoStatus status);

/**
 * @brief 从 Parser 映射并发送状态
 *
 * @param parser_result Parser返回值
 */
void protocol_handle_parser_result(int parser_result);

#endif
