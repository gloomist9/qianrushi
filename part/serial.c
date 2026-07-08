#include "serial.h"
#include "parse.h"
#include "protocol.h"
#include <string.h>

volatile uint16_t uart_rx_len = 0;
uint8_t uart_rx_buf[64];

static char line_buf[128];
static uint16_t line_index = 0;
static volatile bool line_ready = false;

//USART3 IDLE回调，按行拆分
void serial_rx_callback(uint8_t *buf, uint16_t size)
{
    for(uint16_t i = 0; i < size; i++)
    {
        char c = (char)buf[i];

        if(c == '\r')
            continue;

        if(c == '\n')
        {
            if(line_index > 0)
            {
                line_buf[line_index] = '\0';
                line_ready = true;
                line_index = 0;
            }
            break;
        }
        else
        {
            if(line_index < sizeof(line_buf)-1)
                line_buf[line_index++] = c;
            else
                line_index = 0;
        }
    }
}

//主循环调用，每次处理一行
void serial_process(void)
{
    if(!line_ready)
        return;

    ParserResult result = parser_parse_line(line_buf);

    //队列满了就等下一轮，不消费这一行
    if(result == PARSER_QUEUE_FULL)
        return;

    line_ready = false;

    protocol_handle_parser_result(result);
}
