#pragma once

#include <xc.h>
#include <inttypes.h>
#include <stddef.h>

#define SPI_CS LATBbits.LATB3

#define spi_chip_select() do { SPI_CS = 0; } while(0)
#define spi_chip_deselect() do { SPI_CS = 1; } while(0);

void spi_init(void);
void spi_enable_fast(void);
void spi_enable_slow(void);
void spi_disable(void);
void spi_read_block(void *buffer, size_t size);
uint8_t spi_read_byte(void);
void spi_write_block(void *buffer, size_t size);
void spi_write_byte(uint8_t data);