#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/image_info.h"

#define BLOCK_BUFFER_SIZE SECTOR_SIZE_BYTES

static size_t next_floppy(size_t *floppy_index)
{
  size_t current_floppy_index = *floppy_index;
  *floppy_index++;

  if (current_floppy_index == MAX_NUMBER_FLOPPY_DRIVES)
  {
    printf("Error: can't have more than %d floppy drives\r\n", MAX_NUMBER_FLOPPY_DRIVES);
    exit(-1);
  }

  return current_floppy_index;
}

static size_t next_hard(size_t *hard_index)
{
  size_t current_hard_index = *hard_index;
  *hard_index++;

  if (current_hard_index == MAX_NUMBER_HARD_DRIVES)
  {
    printf("Error: can't have more than %d hard drives\r\n", MAX_NUMBER_HARD_DRIVES);
    exit(-1);
  }

  return current_hard_index;
}

static uint32_t size_blocks(struct DiskInfo* di)
{
  return di->number_of_cylinders *
                  di->number_of_heads *
                  di->number_of_sectors;
}

static uint32_t next_image_offset(uint32_t *image_offset, struct DiskInfo* di)
{
  uint32_t current_image_offset = *image_offset;

  *image_offset += size_blocks(di);

  return current_image_offset;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Usage: pack <image> [<image>...]\r\n");
    exit(-1);
  }

  struct ImageInfo ii;
  memset(&ii, 0, sizeof(struct ImageInfo));
  memcpy(ii.magic, IMAGE_MAGIC_STR, IMAGE_MAGIC_SIZE);

  size_t current_floppy_index = 0;
  size_t current_hard_index = 0;
  uint32_t current_image_offset = 1; // First block contains card info

  char* floppy_filenames[MAX_NUMBER_FLOPPY_DRIVES];
  char* hard_filenames[MAX_NUMBER_HARD_DRIVES];

  memset(floppy_filenames, 0, sizeof(char*) * MAX_NUMBER_FLOPPY_DRIVES);
  memset(hard_filenames, 0, sizeof(char*) * MAX_NUMBER_HARD_DRIVES);

  for (size_t i = 1; i < argc; i++)
  {
    struct stat st;
    uint16_t number_of_cylinders;

    if (sscanf(argv[i], "--hard-%hdcyl", &number_of_cylinders) == 1) {
      if (number_of_cylinders > 0 && number_of_cylinders <= MAX_HARD_NUMBER_OF_CYLINDERS) {
        size_t j = next_hard(&current_hard_index);
        ii.hard_drives[j].drive_type_fun8h = 0;
        ii.hard_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
        ii.hard_drives[j].number_of_heads = HARD_NUMBER_OF_HEADS;
        ii.hard_drives[j].number_of_sectors = HARD_NUMBER_OF_SECTORS;
        ii.hard_drives[j].number_of_cylinders = number_of_cylinders;
        ii.hard_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.hard_drives[j]);
      }
      else {
        printf("Error: invalid number of cylinders: 0 < %hd <= %hd\r\n", number_of_cylinders, MAX_HARD_NUMBER_OF_CYLINDERS);
        exit(-1);
      }
    }
    else if (stat(argv[i], &st) == 0)
    {
      switch (st.st_size)
      {
      case FLOPPY_1440_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1440;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_1440_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_1440_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_1440_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_720_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_720;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_720_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_720_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_720_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_1200_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1200;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_1200_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_1200_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_1200_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_360_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_360_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_360_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_360_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_180_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_180_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_180_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_180_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_320_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_320_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_320_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_320_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      case FLOPPY_160_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        floppy_filenames[j] = argv[i];
        ii.floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        ii.floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        ii.floppy_drives[j].number_of_heads = FLOPPY_160_NUMBER_OF_HEADS;
        ii.floppy_drives[j].number_of_sectors = FLOPPY_160_NUMBER_OF_SECTORS;
        ii.floppy_drives[j].number_of_cylinders = FLOPPY_160_NUMBER_OF_CYLINDERS;
        ii.floppy_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.floppy_drives[j]);
      }
      break;
      default:
        if (
            st.st_size > HARD_CYLINDER_SIZE_BYTES &&
            st.st_size % HARD_CYLINDER_SIZE_BYTES == 0 &&
            st.st_size <= HARD_CYLINDER_SIZE_BYTES * MAX_HARD_NUMBER_OF_CYLINDERS)
        {
          size_t j = next_hard(&current_hard_index);
          hard_filenames[j] = argv[i];
          ii.hard_drives[j].drive_type_fun8h = 0;
          ii.hard_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
          ii.hard_drives[j].number_of_heads = HARD_NUMBER_OF_HEADS;
          ii.hard_drives[j].number_of_sectors = HARD_NUMBER_OF_SECTORS;
          ii.hard_drives[j].number_of_cylinders = st.st_size / HARD_CYLINDER_SIZE_BYTES;
          ii.hard_drives[j].image_offset = next_image_offset(&current_image_offset, &ii.hard_drives[j]);
        }
        else
        {
          printf("Error: unexpected image file size %ld\r\n", st.st_size);
          exit(-1);
        }
      }
    }
    else
    {
      printf("Error: can't open image file %s\r\n", argv[i]);
      exit(-1);
    }
  }

  uint8_t *block_buffer = malloc(BLOCK_BUFFER_SIZE);
  memset(block_buffer, 0, BLOCK_BUFFER_SIZE);
  memcpy(block_buffer, &ii, sizeof(struct ImageInfo));

  if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
  {
    printf("Error: can't write output\r\n");
    exit(-1);
  }

  for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES + MAX_NUMBER_HARD_DRIVES; i++) {
    struct DiskInfo* di = (i < MAX_NUMBER_FLOPPY_DRIVES) ? &ii.floppy_drives[i] : &ii.hard_drives[i - MAX_NUMBER_FLOPPY_DRIVES];
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
              printf("Error: can't write output\r\n");
              exit(-1);
            }
          }
        }
        else
        {
          printf("Error: can't open image file %s\r\n", filename);
          exit(-1);
        }
      }
      else {
        memset(block_buffer, 0, BLOCK_BUFFER_SIZE);

        for (uint32_t i = 0; i < size_blocks(di); i++) {
          if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
          {
            printf("Error: can't write output\r\n");
            exit(-1);
          }
        }
      }
    }
  }

  free(block_buffer);

  return 0;
}
