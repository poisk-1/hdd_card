#pragma once

#include <stdbool.h>
#include <inttypes.h>

void uart_init(void);
uint8_t uart_read(void);
void uart_write(uint8_t data);
bool try_uart_read(uint8_t *data);