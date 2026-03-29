#pragma once

#include <xc.h>
#include <stdbool.h>
#include <inttypes.h>

enum SDMediaError
{
    SD_MEDIA_ERROR_NO_ERROR = 0,
    SD_MEDIA_ERROR_DEVICE_NOT_PRESENT,
    SD_MEDIA_ERROR_CANNOT_INITIALIZE
};

enum SDMode {
    SD_MODE_NORMAL = 0,
    SD_MODE_HC
};

struct SDMediaInfo
{
    enum SDMediaError error;
    enum SDMode sd_mode;
};

#define SDCARD_CD PORTCbits.RC5

#define sd_card_detected() SDCARD_CD

void sd_media_init(struct SDMediaInfo* media_info);
bool sd_start_read_blocks(uint32_t address);
bool sd_start_write_blocks(uint32_t address);
bool sd_read_next_block(void *buffer);
bool sd_write_next_block(void *buffer);
void sd_stop_read_blocks(void);
void sd_stop_write_blocks(void);