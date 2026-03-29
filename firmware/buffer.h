#pragma once

#define CTRL_BUFFER_SIZE 0x200
#define SECTOR_SIZE 0x200
#define BUFFER_SECTORS 15
#define DATA_BUFFER_SIZE (SECTOR_SIZE * BUFFER_SECTORS)

void buffer_init(void);

void *buffer_get_ctrl(void);
void *buffer_get_data(void);
