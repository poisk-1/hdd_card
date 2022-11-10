#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const size_t address_bits = 20;
const size_t rom_bits = 13;
const size_t rom_size = 1 << rom_bits;

const size_t io_base = 0xe0000;
const size_t io_data_size = 0x200;
const size_t io_ctrl_size = 0x80;

const size_t sel_io_bit = 7;
const size_t sel_io_data_bit = 6;
const size_t sel_io_ctrl_bit = 5;

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

  for (size_t i = 0; i < (1 << 20); i++) {
    size_t j = i >> (address_bits - rom_bits);
    uint8_t flags = 0;

    if (i >= io_base && i < (io_base + io_data_size + io_ctrl_size)) {
      flags |= (1 << sel_io_bit);
    }
    if (i >= io_base && i < (io_base + io_data_size)) {
      flags |= (1 << sel_io_data_bit);
    }
    if (i >= (io_base + io_data_size) &&
        i < (io_base + io_data_size + io_ctrl_size)) {
      flags |= (1 << sel_io_ctrl_bit);
    }

    rom_buffer[j] = flags;
  }

  fwrite(rom_buffer, sizeof(uint8_t), rom_size, tgt_file);

  free(rom_buffer);

  return 0;
}
