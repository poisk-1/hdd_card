#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

#define error(...) do { fprintf(stderr, __VA_ARGS__); exit(-1); } while(0)

static size_t next_floppy(size_t *floppy_index)
{
  size_t current_floppy_index = *floppy_index;
  (*floppy_index)++;

  if (current_floppy_index == MAX_NUMBER_FLOPPY_DRIVES)
  {
    error("Error: can't have more than %d floppy drives\r\n", MAX_NUMBER_FLOPPY_DRIVES);
  }

  return current_floppy_index;
}

static size_t next_hard(size_t *hard_index)
{
  size_t current_hard_index = *hard_index;
  (*hard_index)++;

  if (current_hard_index == MAX_NUMBER_HARD_DRIVES)
  {
    error("Error: can't have more than %d hard drives\r\n", MAX_NUMBER_HARD_DRIVES);
  }

  return current_hard_index;
}

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

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    error("Usage: pack <DISK INFO> [<DISK INFO> ...]\r\n"
      "\tExisting image file:                    <DISK INFO> ::= [--read-only] <FILENAME>\r\n"
      "\tEmpty floppy disk:                      <DISK INFO> ::= --floppy-<KILOBYTES>k\r\n"
      "\tEmpty hard disk (16 heads, 63 sectors): <DISK INFO> ::= --hard-<CYLINDERS>c\r\n"
      "\tSupported floppy disk sizes:            <KILOBYTES> ::= 1440 | 1200 | 720 | 360\r\n"
      "\tSupported hard disk cylinders:          0 < <CYLINDERS> <= 1024\r\n"
    );
  }

  struct ImageInfo ii;
  memset(&ii, 0, sizeof(struct ImageInfo));
  memcpy(ii.magic, IMAGE_MAGIC_STR, IMAGE_MAGIC_SIZE);

  size_t current_floppy_index = 0;
  size_t current_hard_index = 0;
  bool current_read_only = false;
  uint32_t current_image_offset = 1; // First block contains card info

  char* floppy_filenames[MAX_NUMBER_FLOPPY_DRIVES];
  char* hard_filenames[MAX_NUMBER_HARD_DRIVES];

  memset(floppy_filenames, 0, sizeof(char*) * MAX_NUMBER_FLOPPY_DRIVES);
  memset(hard_filenames, 0, sizeof(char*) * MAX_NUMBER_HARD_DRIVES);

  for (size_t i = 1; i < argc; i++)
  {
    struct stat st;
    uint16_t number_of_cylinders;
    uint32_t floppy_size_kibytes;

    if (strcmp(argv[i], "--read-only") == 0) {
      current_read_only = true;
    }
    else {
      if (sscanf(argv[i], "--floppy-%dk", &floppy_size_kibytes) == 1) {
        const struct FloppyFormat* floppy_format = find_floppy_format(floppy_size_kibytes * 1024);
        if (floppy_format != NULL) {
          size_t j = next_floppy(&current_floppy_index);
          ii.floppy_dis[j].read_only = false;
          ii.floppy_dis[j].drive_type_fun8h = floppy_format->drive_type_fun8h;
          ii.floppy_dis[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
          ii.floppy_dis[j].number_of_heads = floppy_format->number_of_heads;
          ii.floppy_dis[j].number_of_sectors = floppy_format->number_of_sectors;
          ii.floppy_dis[j].number_of_cylinders = floppy_format->number_of_cylinders;
          ii.floppy_dis[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_dis[j]);
        }
        else {
          error("Error: unexpected floppy size %d KiB in --floppy-NNNNk\r\n", floppy_size_kibytes);
        }
      }
      else if (sscanf(argv[i], "--hard-%hdc", &number_of_cylinders) == 1) {
        if (number_of_cylinders > 0 && number_of_cylinders <= MAX_HARD_NUMBER_OF_CYLINDERS) {
          size_t j = next_hard(&current_hard_index);
          ii.hard_dis[j].read_only = false;
          ii.hard_dis[j].drive_type_fun8h = 0;
          ii.hard_dis[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
          ii.hard_dis[j].number_of_heads = HARD_NUMBER_OF_HEADS;
          ii.hard_dis[j].number_of_sectors = HARD_NUMBER_OF_SECTORS;
          ii.hard_dis[j].number_of_cylinders = number_of_cylinders;
          ii.hard_dis[j].image_offset = next_image_offset(&current_image_offset, &ii.hard_dis[j]);
        }
        else {
          error("Error: invalid number of cylinders in --hard-NNNNc: 0 < %hd <= %hd\r\n", number_of_cylinders, MAX_HARD_NUMBER_OF_CYLINDERS);
        }
      }
      else if (stat(argv[i], &st) == 0)
      {
        const struct FloppyFormat* floppy_format = find_floppy_format(st.st_size);
        if (floppy_format != NULL) {
          size_t j = next_floppy(&current_floppy_index);
          floppy_filenames[j] = argv[i];
          ii.floppy_dis[j].read_only = current_read_only;
          ii.floppy_dis[j].drive_type_fun8h = floppy_format->drive_type_fun8h;
          ii.floppy_dis[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
          ii.floppy_dis[j].number_of_heads = floppy_format->number_of_heads;
          ii.floppy_dis[j].number_of_sectors = floppy_format->number_of_sectors;
          ii.floppy_dis[j].number_of_cylinders = floppy_format->number_of_cylinders;
          ii.floppy_dis[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_dis[j]);
        }
        else if (
              st.st_size > HARD_CYLINDER_SIZE_BYTES &&
              st.st_size % HARD_CYLINDER_SIZE_BYTES == 0 &&
              st.st_size <= HARD_CYLINDER_SIZE_BYTES * MAX_HARD_NUMBER_OF_CYLINDERS)
        {
          size_t j = next_hard(&current_hard_index);
          hard_filenames[j] = argv[i];
          ii.hard_dis[j].read_only = current_read_only;
          ii.hard_dis[j].drive_type_fun8h = 0;
          ii.hard_dis[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
          ii.hard_dis[j].number_of_heads = HARD_NUMBER_OF_HEADS;
          ii.hard_dis[j].number_of_sectors = HARD_NUMBER_OF_SECTORS;
          ii.hard_dis[j].number_of_cylinders = st.st_size / HARD_CYLINDER_SIZE_BYTES;
          ii.hard_dis[j].image_offset = next_image_offset(&current_image_offset, &ii.hard_dis[j]);
        }
        else
        {
          error("Error: unexpected image file size %ld Bytes\r\n", st.st_size);
        }
      }
      else
      {
        error("Error: can't open image file %s\r\n", argv[i]);
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

  for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES + MAX_NUMBER_HARD_DRIVES; i++) {
    struct DiskInfo* di = (i < MAX_NUMBER_FLOPPY_DRIVES) ? &ii.floppy_dis[i] : &ii.hard_dis[i - MAX_NUMBER_FLOPPY_DRIVES];
    if (di->image_offset != 0) {
      char* filename = (i < MAX_NUMBER_FLOPPY_DRIVES) ? floppy_filenames[i] : hard_filenames[i - MAX_NUMBER_FLOPPY_DRIVES];

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
        }
        else
        {
          error("Error: can't open image file %s\r\n", filename);
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
