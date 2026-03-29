#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

#define error(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while(0)

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

  uint8_t floppy_disk_indexes[MAX_NUMBER_FLOPPY_DRIVES];
  uint8_t hard_disk_indexes[MAX_NUMBER_HARD_DRIVES];

  memset(&floppy_disk_indexes, 0, sizeof(uint8_t) * MAX_NUMBER_HARD_DRIVES);
  memset(&hard_disk_indexes, 0, sizeof(uint8_t) * MAX_NUMBER_HARD_DRIVES);

  for (size_t i = 0; i < MAX_NUMBER_DISKS; i++) {
      struct DiskInfo *di =  &ii.dis[i];

      if (has_geometry(di)) {
        char filename[256];
        uint8_t drive_index;
        uint8_t disk_index;

        printf("Found ");

        if (is_hard_drive(di->drive_number)) {
          drive_index = di->drive_number & HARD_DRIVE_NUMBER_MASK;

          if (drive_index < MAX_NUMBER_HARD_DRIVES) {
            disk_index = hard_disk_indexes[drive_index]++;

            printf("hard drive %d disk %d ", drive_index, disk_index);
            sprintf(filename, "hard%d_%d.img", drive_index, disk_index);
          }
          else {
            error("Error: can't have more than %d hard drives\r\n", MAX_NUMBER_HARD_DRIVES);
          }
        }
        else {
          drive_index = di->drive_number;

          if (drive_index < MAX_NUMBER_FLOPPY_DRIVES) {
            disk_index = floppy_disk_indexes[drive_index]++;

            printf("floppy drive %d disk %d ", drive_index, disk_index);
            sprintf(filename, "floppy%d_%d.img", drive_index, disk_index);
          }
          else {
            error("Error: can't have more than %d floppy drives\r\n", MAX_NUMBER_FLOPPY_DRIVES);
          }
        }

        uint32_t size = size_blocks(di);
        printf("having %d blocks at offset %d", size, di->image_offset);

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

            fclose(file);
          }
          else
          {
            error("Error: can't open image file %s\r\n", filename);
          }
        }
        printf("\r\n");
      }
  }

  free(block_buffer);

  return 0;
}
