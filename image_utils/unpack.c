#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

#define error(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while(0)

static void write_image_file(char* filename, uint8_t *block_buffer, bool header_only, struct DiskInfo* di) {
  uint32_t size = size_blocks(di);
  printf("Found %d blocks at offset %d", size, di->image_offset);

  if (!header_only) {
    printf(": writing to %s", filename);
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
  printf("\r\n");
}

int main(int argc, char **argv)
{
  bool header_only = false;
  if (argc > 1) {
    if (strcmp(argv[1], "--header-only") == 0) {
      header_only = true;
    }
    else {
      error("Usage: unpack [--header-only]\r\n");
    }
  }
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
          sprintf(filename, "floppy%ld.img", i);
          write_image_file(filename, block_buffer, header_only, &ii.floppy_dis[i]);
      }
  }

  for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
      if (has_geometry(&ii.hard_dis[i])) {
          sprintf(filename, "hard%ld.img", i);
          write_image_file(filename, block_buffer, header_only, &ii.hard_dis[i]);
      }
  }

  free(block_buffer);

  return 0;
}
