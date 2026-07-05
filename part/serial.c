#include "serial.h"
#include "parse.h"
#include "protocol.h"
#include <string.h>
volatile uint16_t uart_rx_len = 0;
uint8_t uart_rx_buf[64];

/*==================== RX buffer ====================*/

static char line_buf[128];
static uint16_t line_index = 0;
static volatile bool line_ready = false;

/*==================== IDLE callback ====================*/

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
                extern volatile uint32_t dbg_serial_rx;
                dbg_serial_rx++;
            }
            break;
        }
        else
        {
            if(line_index < sizeof(line_buf)-1)
            {
                line_buf[line_index++] = c;
            }
            else
            {
                line_index = 0;
            }
        }
    }
}

/*==================== Main loop processing ====================*/

void serial_process(void)
{
    if(!line_ready)
        return;

    ParserResult result = parser_parse_line(line_buf);

    if(result == PARSER_QUEUE_FULL)
    {
        /* Queue full: don't consume line, don't send ACK.
         * Will retry next main loop iteration when a slot opens. */
        return;
    }

    /* Consume line and send ACK */
    line_ready = false;

    {
        extern volatile uint32_t dbg_parse_ok, dbg_parse_fail;
        if(result == PARSER_OK) dbg_parse_ok++;
        else dbg_parse_fail++;
    }

    protocol_handle_parser_result(result);
}
