#pragma once

#include <xc.h>
#include <inttypes.h>
#include <stddef.h>

#define SPI_CS LATCbits.LATC3

#define spi_chip_select() do { SPI_CS = 0; } while(0)
#define spi_chip_deselect() do { SPI_CS = 1; } while(0);

void spi_init(void);
void spi_enable_fast(void);
void spi_enable_slow(void);
void spi_disable(void);
uint8_t spi_exchange_byte(uint8_t data);
void spi_read_block(void *block, size_t block_size);
void spi_write_block(void *block, size_t block_size);