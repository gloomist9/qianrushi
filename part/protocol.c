#include "protocol.h"
#include "parse.h"


/*======================== µ×²ã·¢ËÍ ========================*/

void protocol_send_status(ProtoStatus status)
{
    HAL_UART_Transmit(&huart3,
                      (uint8_t *)&status,
                      1,
                      100);
}

/*======================== Parser½á¹ûÓ³Éä ========================*/

void protocol_handle_parser_result(int parser_result)
{
    switch(parser_result)
    {
        case PARSER_OK:
            protocol_send_status(PROTO_OK);
            break;

        case PARSER_QUEUE_FULL:
            protocol_send_status(PROTO_BUSY);
            break;

        case PARSER_FORMAT_ERROR:
            protocol_send_status(PROTO_PARSE_ERROR);
            break;

        case PARSER_UNSUPPORTED_GCODE:
            protocol_send_status(PROTO_UNSUPPORTED);
            break;

        default:
            protocol_send_status(PROTO_PARSE_ERROR);
            break;
    }
}
