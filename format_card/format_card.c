#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../firmware/drive_info.h"

#define BLOCK_BUFFER_SIZE 512

size_t next_floppy(size_t *floppy_index)
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

size_t next_hard(size_t *hard_index)
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

uint32_t next_card_offset(uint32_t *card_offset, struct DriveInfo *drive_info)
{
  uint32_t current_card_offset = *card_offset;

  *card_offset += drive_info->number_of_cylinders *
                  drive_info->number_of_heads *
                  drive_info->number_of_sectors;

  return current_card_offset;
}

int main(int argc, char **argv)
{
  if (argc < 2)
  {
    printf("Usage: format_card <image> [<image>...]\r\n");
    exit(-1);
  }

  uint8_t *block_buffer = malloc(BLOCK_BUFFER_SIZE);
  memset(block_buffer, 0, BLOCK_BUFFER_SIZE);

  struct CardInfo *card_info = (struct CardInfo *)block_buffer;
  memcpy(card_info->magic, MAGIC_STR, MAGIC_SIZE);

  size_t current_floppy_index = 0;
  size_t current_hard_index = 0;
  uint32_t current_card_offset = 1; // First block contains card info

  for (size_t i = 1; i < argc; i++)
  {
    struct stat st;
    if (stat(argv[i], &st) == 0)
    {
      switch (st.st_size)
      {
      case FLOPPY_1440_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1440;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_1440_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_1440_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_1440_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_720_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_720;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_720_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_720_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_720_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_1200_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1200;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_1200_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_1200_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_1200_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_360_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_360_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_360_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_360_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_180_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_180_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_180_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_180_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_320_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_320_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_320_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_320_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      case FLOPPY_160_SIZE_BYTES:
      {
        size_t j = next_floppy(&current_floppy_index);
        card_info->floppy_drives[j].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
        card_info->floppy_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;
        card_info->floppy_drives[j].number_of_heads = FLOPPY_160_NUMBER_OF_HEADS;
        card_info->floppy_drives[j].number_of_sectors = FLOPPY_160_NUMBER_OF_SECTORS;
        card_info->floppy_drives[j].number_of_cylinders = FLOPPY_160_NUMBER_OF_CYLINDERS;
        card_info->floppy_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->floppy_drives[j]);
      }
      break;
      default:
        if (
            st.st_size > HARD_CYLINDER_SIZE_BYTES &&
            st.st_size % HARD_CYLINDER_SIZE_BYTES == 0 &&
            st.st_size <= HARD_CYLINDER_SIZE_BYTES * MAX_HARD_NUMBER_OF_CYLINDERS)
        {
          size_t j = next_hard(&current_hard_index);
          card_info->hard_drives[j].drive_type_fun8h = 0;
          card_info->hard_drives[j].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;
          card_info->hard_drives[j].number_of_heads = HARD_NUMBER_OF_HEADS;
          card_info->hard_drives[j].number_of_sectors = HARD_NUMBER_OF_SECTORS;
          card_info->hard_drives[j].number_of_cylinders = st.st_size / HARD_CYLINDER_SIZE_BYTES;
          card_info->hard_drives[j].card_offset = next_card_offset(&current_card_offset, &card_info->hard_drives[j]);
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

  if (fwrite(block_buffer, BLOCK_BUFFER_SIZE, 1, stdout) != 1)
  {
    printf("Error: can't write output\r\n");
    exit(-1);
  }

  for (size_t i = 1; i < argc; i++)
  {
    FILE *file = fopen(argv[i], "r");

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
      printf("Error: can't open image file %s\r\n", argv[i]);
      exit(-1);
    }
  }

  free(block_buffer);

  return 0;
}
