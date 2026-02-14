#pragma once

#define CTRL_BUFFER_SIZE 0x200
#define SECTOR_SIZE 0x200
#define BUFFER_SECTORS 15
#define DATA_BUFFER_SIZE (SECTOR_SIZE * BUFFER_SECTORS)

void init_buffer(void);

void *get_ctrl_buffer(void);
void *get_data_buffer(void);
