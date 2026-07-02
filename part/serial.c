#include "serial.h"
#include "parse.h"
#include "protocol.h"
#include <string.h>
volatile uint16_t uart_rx_len = 0;
uint8_t uart_rx_buf[64];

/*==================== 接收缓冲区 ====================*/

/**
 * 行缓冲区（拼完整一行G-code）
 */
static char line_buf[128];

/**
 * 当前写入位置
 */
static uint16_t line_index = 0;

/**
 * 是否有完整一行数据
 */
static volatile bool line_ready = false;

/*==================== IDLE中断回调 ====================*/



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
        }
        else
        {
            if(line_index < sizeof(line_buf)-1)
            {
                line_buf[line_index++] = c;
            }
            else
            {
                /* 一行太长，丢弃 */
                line_index = 0;
            }
        }
    }

    HAL_UARTEx_ReceiveToIdle_DMA(
        &huart3,
        uart_rx_buf,
        sizeof(uart_rx_buf));
}

/*==================== 主循环处理 ====================*/

void serial_process(void)
{
    if(!line_ready)
        return;

    line_ready = false;

    /*==================== 交给Parser ====================*/
    ParserResult result = parser_parse_line(line_buf);

    /*==================== 交给Protocol ====================*/
    protocol_handle_parser_result(result);
}




//void serial_process(void)
//{
//    HAL_UART_DMAStop(&huart3);

//    uint16_t len = LINE_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);//求数据长度(后者是未传输数据长度的函数)

//    uart_rx_buf[len] = '\0';

//    char line[LINE_BUF_SIZE];//临时行缓存（一条G-code）
//    uint16_t idx = 0;

//    for(uint16_t i = 0; i < len; i++)//遍历本次数据
//    {
//        char c = uart_rx_buf[i];

//        if(c == '\n' || idx >= LINE_BUF_SIZE - 1)//一行结束
//        {
//            line[idx] = '\0';//给当前行字符串加上结束符

//            if(idx > 0)
//            {
//                parser_parse_line(line);//解析器传入一整行
//            }

//            idx = 0;//刷新缓存
//        }
//        else if(c != '\r')//普通字符存入缓存
//        {
//            line[idx++] = c;
//        }
//    }
//    HAL_UART_Receive_DMA(&huart3, uart_rx_buf, LINE_BUF_SIZE);
//}
//void USART3_IRQHandler(void)
//{
//    if(__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE))
//    {
//        __HAL_UART_CLEAR_IDLEFLAG(&huart3);

//        serial_process();
//    }

//    HAL_UART_IRQHandler(&huart3);
//}
