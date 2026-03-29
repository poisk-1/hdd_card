#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

#define error(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while(0)

static uint32_t next_image_offset(uint32_t *image_offset, struct DiskInfo* di)
{
  uint32_t current_image_offset = *image_offset;

  *image_offset += size_blocks(di);

  return current_image_offset;
}

struct FloppyFormat {
    uint32_t size_bytes;

    uint8_t drive_type_fun8h;

    uint8_t number_of_heads;
    uint8_t number_of_sectors;
    uint16_t number_of_cylinders;
};

const struct FloppyFormat floppy_formats[] = {
  {FLOPPY_1440_SIZE_BYTES, FUN8H_DRIVE_TYPE_FLOPPY_1440, FLOPPY_1440_NUMBER_OF_HEADS, FLOPPY_1440_NUMBER_OF_SECTORS, FLOPPY_1440_NUMBER_OF_CYLINDERS},
  {FLOPPY_1200_SIZE_BYTES, FUN8H_DRIVE_TYPE_FLOPPY_1200, FLOPPY_1200_NUMBER_OF_HEADS, FLOPPY_1200_NUMBER_OF_SECTORS, FLOPPY_1200_NUMBER_OF_CYLINDERS},
  {FLOPPY_720_SIZE_BYTES, FUN8H_DRIVE_TYPE_FLOPPY_720, FLOPPY_720_NUMBER_OF_HEADS, FLOPPY_720_NUMBER_OF_SECTORS, FLOPPY_720_NUMBER_OF_CYLINDERS},
  {FLOPPY_360_SIZE_BYTES, FUN8H_DRIVE_TYPE_FLOPPY_360, FLOPPY_360_NUMBER_OF_HEADS, FLOPPY_360_NUMBER_OF_SECTORS, FLOPPY_360_NUMBER_OF_CYLINDERS},
  {0, 0, 0, 0, 0},
};

const struct FloppyFormat* find_floppy_format(uint32_t size_bytes) {
  size_t i = 0;
  while (floppy_formats[i].size_bytes != 0) {
    if (floppy_formats[i].size_bytes == size_bytes) {
      return &floppy_formats[i];
    }

    i++;
  }

  return NULL;
}

void usage(void) {
  error("Usage: pack <DISK INFO> [<DISK INFO> ...]\r\n"
    "\tExisting image file:                    <DISK INFO> ::= [--read-only] <FILENAME>\r\n"
    "\tEmpty floppy disk:                      <DISK INFO> ::= --floppy-<KILOBYTES>k\r\n"
    "\tEmpty hard disk (16 heads, 63 sectors): <DISK INFO> ::= --hard-<CYLINDERS>c\r\n"
    "\tSupported floppy disk sizes:            <KILOBYTES> ::= 1440 | 1200 | 720 | 360\r\n"
    "\tSupported hard disk cylinders:          0 < <CYLINDERS> <= 1024\r\n"
  );
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    usage();
  }

  struct ImageInfo ii;
  memset(&ii, 0, sizeof(struct ImageInfo));
  memcpy(ii.magic, IMAGE_MAGIC_STR, IMAGE_MAGIC_SIZE);

  size_t current_floppy_index = 0;
  size_t current_hard_index = 0;
  bool current_read_only = false;
  uint32_t current_image_offset = 1; // First block contains card info

  char* image_filenames[MAX_NUMBER_DISKS];
  memset(image_filenames, 0, sizeof(char*) * (MAX_NUMBER_DISKS));

  size_t disk_index = 0;
  size_t image_index = 0;
  for (size_t i = 1; i < argc; i++)
  {
    struct stat st;
    uint8_t drive_index;
    uint16_t number_of_cylinders;
    uint32_t floppy_size_kibytes;

    if (strcmp(argv[i], "--read-only") == 0) {
      current_read_only = true;
    }
    else {
      if (disk_index >= MAX_NUMBER_DISKS) {
        error("Error: can't have more than %d disks\r\n", MAX_NUMBER_DISKS);
      }

      if (sscanf(argv[i], "--floppy%hhd-%dk", &drive_index, &floppy_size_kibytes) == 2) {
        if (drive_index >= MAX_NUMBER_FLOPPY_DRIVES) {
          error("Error: can't have more than %d floppy drives\r\n", MAX_NUMBER_FLOPPY_DRIVES);
        }

        const struct FloppyFormat* floppy_format = find_floppy_format(floppy_size_kibytes * 1024);
        if (floppy_format != NULL) {
          ii.dis[disk_index].read_only = false;
          ii.dis[disk_index].drive_number = drive_index;
          ii.dis[disk_index].drive_type_fun8h = floppy_format->drive_type_fun8h;
          ii.dis[disk_index].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
          ii.dis[disk_index].number_of_heads = floppy_format->number_of_heads;
          ii.dis[disk_index].number_of_sectors = floppy_format->number_of_sectors;
          ii.dis[disk_index].number_of_cylinders = floppy_format->number_of_cylinders;
          ii.dis[disk_index].image_offset = next_image_offset(&current_image_offset, &ii.dis[disk_index]);
          disk_index++;
        }
        else {
          error("Error: unexpected floppy size %d KiB in --floppy-NNNNk\r\n", floppy_size_kibytes);
        }
      }
      else if (sscanf(argv[i], "--hard%hhd-%hdc", &drive_index, &number_of_cylinders) == 2) {
        if (drive_index >= MAX_NUMBER_HARD_DRIVES) {
          error("Error: can't have more than %d hard drives\r\n", MAX_NUMBER_HARD_DRIVES);
        }

        if (number_of_cylinders > 0 && number_of_cylinders <= MAX_HARD_NUMBER_OF_CYLINDERS) {
          ii.dis[disk_index].read_only = false;
          ii.dis[disk_index].drive_number = HARD_DRIVE_NUMBER_BASE | drive_index;
          ii.dis[disk_index].drive_type_fun8h = 0;
          ii.dis[disk_index].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
          ii.dis[disk_index].number_of_heads = HARD_NUMBER_OF_HEADS;
          ii.dis[disk_index].number_of_sectors = HARD_NUMBER_OF_SECTORS;
          ii.dis[disk_index].number_of_cylinders = number_of_cylinders;
          ii.dis[disk_index].image_offset = next_image_offset(&current_image_offset, &ii.dis[disk_index]);
          disk_index++;
        }
        else {
          error("Error: invalid number of cylinders in --hard-NNNNc: 0 < %hd <= %hd\r\n", number_of_cylinders, MAX_HARD_NUMBER_OF_CYLINDERS);
        }
      }
      else if (sscanf(argv[i], "--floppy%hhd-image", &drive_index) == 1 && i + 1 < argc)
      {
        char* filename = argv[++i];

        if (stat(filename, &st) == 0) {
          const struct FloppyFormat* floppy_format = find_floppy_format(st.st_size);
          if (floppy_format != NULL) {
            image_filenames[image_index++] = filename;

            ii.dis[disk_index].read_only = current_read_only;
            ii.dis[disk_index].drive_number = drive_index;
            ii.dis[disk_index].drive_type_fun8h = floppy_format->drive_type_fun8h;
            ii.dis[disk_index].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
            ii.dis[disk_index].number_of_heads = floppy_format->number_of_heads;
            ii.dis[disk_index].number_of_sectors = floppy_format->number_of_sectors;
            ii.dis[disk_index].number_of_cylinders = floppy_format->number_of_cylinders;
            ii.dis[disk_index].image_offset = next_image_offset(&current_image_offset, &ii.dis[disk_index]);
            disk_index++;
          }
          else {
            error("Error: unexpected floppy image file size %ld Bytes\r\n", st.st_size);
          }
        }
        else {
          error("Error: can't open floppy image file %s\r\n", argv[i + 1]);
        }
      }
      else if (sscanf(argv[i], "--hard%hhd-image", &drive_index) == 1 && i + 1 < argc) {
        char* filename = argv[++i];

        if (stat(filename, &st) == 0) {
          if (
                st.st_size > HARD_CYLINDER_SIZE_BYTES &&
                st.st_size % HARD_CYLINDER_SIZE_BYTES == 0 &&
                st.st_size <= HARD_CYLINDER_SIZE_BYTES * MAX_HARD_NUMBER_OF_CYLINDERS)
          {
            image_filenames[image_index++] = filename;

            ii.dis[disk_index].read_only = current_read_only;
            ii.dis[disk_index].drive_number = HARD_DRIVE_NUMBER_BASE | drive_index;
            ii.dis[disk_index].drive_type_fun8h = 0;
            ii.dis[disk_index].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
            ii.dis[disk_index].number_of_heads = HARD_NUMBER_OF_HEADS;
            ii.dis[disk_index].number_of_sectors = HARD_NUMBER_OF_SECTORS;
            ii.dis[disk_index].number_of_cylinders = st.st_size / HARD_CYLINDER_SIZE_BYTES;
            ii.dis[disk_index].image_offset = next_image_offset(&current_image_offset, &ii.dis[disk_index]);
            disk_index++;
          }
          else
          {
            error("Error: unexpected hard image file size %ld Bytes\r\n", st.st_size);
          }
        }
        else
        {
          error("Error: can't open hard image file %s\r\n", argv[i + 1]);
        }
      }
      else {
        error("Error: invalid arguments\r\n");
      }

      current_read_only = false;
    }
  }

  uint8_t *block_buffer = malloc(BLOCK_BUFFER_SIZE);
  memset(block_buffer, 0, BLOCK_BUFFER_SIZE);
  memcpy(block_buffer, &ii, sizeof(struct ImageInfo));

  if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
  {
    error("Error: can't write output\r\n");
  }

  for (size_t i = 0, j = 0; i < disk_index; i++) {
    struct DiskInfo* di = &ii.dis[i];

    if (has_geometry(di)) {
      char* filename = image_filenames[j++];

      if (filename != NULL) {
        FILE *file = fopen(filename, "r");

        if (file)
        {
          while (fread(block_buffer, BLOCK_BUFFER_SIZE, 1, file) == 1)
          {
            if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
            {
              error("Error: can't write output\r\n");
            }
          }

          fclose(file);
        }
        else
        {
          error("Error: can't read image file %s\r\n", filename);
        }
      }
      else {
        memset(block_buffer, 0, BLOCK_BUFFER_SIZE);

        for (uint32_t i = 0; i < size_blocks(di); i++) {
          if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
          {
            error("Error: can't write output\r\n");
          }
        }
      }
    }
  }

  free(block_buffer);

  return 0;
}
