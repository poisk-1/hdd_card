#include "drive_info.h"

bool is_hard_drive(uint8_t drive_number) {
    return drive_number & 0x80;
}

bool has_geometry(const struct DriveInfo* drive_info) {
    return drive_info->number_of_cylinders != 0;
}

const struct DriveInfo* find_drive_info(const struct CardInfo* card_info, uint8_t drive_number) {
    const struct DriveInfo* drive_info = NULL;
    size_t i = drive_number & 0xf;

    if (is_hard_drive(drive_number) && i < MAX_NUMBER_HARD_DRIVES) {
        drive_info = &card_info->hard_drives[i];
    } else if (i < MAX_NUMBER_FLOPPY_DRIVES) {
        drive_info = &card_info->floppy_drives[i];
    }

    return drive_info;
}
