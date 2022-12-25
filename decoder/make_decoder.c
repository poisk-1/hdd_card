#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t address_bits = 20;
const size_t rom_bits = 13;
const size_t rom_size = 1 << rom_bits;

const size_t bios_base = 0xe0000;
const size_t bios_size = 0x1000;

const size_t io_base = bios_base + bios_size;
const size_t io_size = 0x300;

const size_t sel_io_bit = 6;
const size_t sel_bios_bit = 7;

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: encode <target>\r\n");
    return 1;
  }

  uint8_t *rom_buffer = malloc(rom_size);
  memset(rom_buffer, 0, rom_size);

  if (!rom_buffer) {
    printf("error: can't allocate rom buffer\r\n");
    return 1;
  }

  FILE *tgt_file = fopen(argv[1], "w");

  if (!tgt_file) {
    printf("error: can't open target file\r\n");
    return 1;
  }

  for (size_t address = 0; address < (1 << 20); address++) {
    uint8_t data = 0xff;

    if (address >= bios_base && address < (bios_base + bios_size)) {
      data = (~(1 << sel_bios_bit) & 0xff);
    }
    else if (address >= io_base && address < (io_base + io_size)) {
      data = (~(1 << sel_io_bit) & 0xf0) | ((address >> 8) & 0xf);
    }

    rom_buffer[address >> (address_bits - rom_bits)] = data;
  }

  fwrite(rom_buffer, sizeof(uint8_t), rom_size, tgt_file);

  free(rom_buffer);

  return 0;
}
