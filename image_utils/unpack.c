#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

#define error(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while(0)

static void print_drive_info(const struct DiskInfo* di) {
    printf("\tDRIVE TYPE FUN8H: 0x%x\r\n", di->drive_type_fun8h);
    printf("\tDRIVE TYPE FUN15H: 0x%x\r\n", di->drive_type_fun15h);

    printf("\tNUM OF HEADS: %d\r\n", di->number_of_heads);
    printf("\tNUM OF CYLINDERS: %d\r\n", di->number_of_cylinders);
    printf("\tNUM OF SECTORS: %d\r\n", di->number_of_sectors);

    printf("\tIMAGE OFFSET: %u\r\n", di->image_offset);
}

static void write_image_file(char* filename, uint8_t *block_buffer, uint32_t size) {
  FILE *file = fopen(filename, "w");

  if (file)
  {
    for (size_t i = 0; i < size; i++) {
      if (fread(block_buffer, BLOCK_BUFFER_SIZE, 1, stdin) != 1)
      {
        error("Error: unexpected EOF\r\n");
      }

      if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, file) != 1)
      {
        error("Error: can't write output\r\n");
      }
    }
  }
  else
  {
    error("Error: can't open image file %s\r\n", filename);
  }
}

int main(int argc, char **argv)
{
  struct ImageInfo ii;
  uint8_t *block_buffer = malloc(BLOCK_BUFFER_SIZE);

  if (fread(block_buffer, BLOCK_BUFFER_SIZE, 1, stdin) != 1) {
    error("Error: unexpected EOF\r\n");
  }

  memcpy(&ii, block_buffer, sizeof(struct ImageInfo));

  if (strncmp(ii.magic, IMAGE_MAGIC_STR, IMAGE_MAGIC_SIZE) != 0) {
    error("Error: unexpected image\r\n");
  }

  char filename[256];

  for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
      if (has_geometry(&ii.floppy_dis[i])) {
          uint32_t size = size_blocks(&ii.floppy_dis[i]);
          sprintf(filename, "floppy%ld.img", i);
          printf("Writing %d blocks to %s\r\n", size, filename);
          write_image_file(filename, block_buffer, size);
      }
  }

  for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
      if (has_geometry(&ii.hard_dis[i])) {
          uint32_t size = size_blocks(&ii.hard_dis[i]);
          sprintf(filename, "hard%ld.img", i);
          printf("Writing %d blocks to %s\r\n", size, filename);
          write_image_file(filename, block_buffer, size);
      }
  }

  free(block_buffer);

  return 0;
}
