#pragma once

#include <inttypes.h>
#include <stddef.h>

void init_spi(void);
void enable_fast_spi(void);
void enable_slow_spi(void);
void disable_spi(void);
uint8_t exchange_byte(uint8_t data);
void exchange_block(void *block, size_t blockSize);