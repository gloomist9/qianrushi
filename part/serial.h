//串口上位机通讯
//USART3 DMA+IDLE收G-code，按行拆分交Parser
#ifndef SERIAL_H
#define SERIAL_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define LINE_BUF_SIZE 64

void serial_init(void);
void serial_rx_callback(uint8_t *buf, uint16_t size);
void serial_process(void);

#endif
