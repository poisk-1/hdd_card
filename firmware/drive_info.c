#include "drive_info.h"

uint32_t disk_size_bytes(const struct DriveInfo* di) {
    return (uint32_t) di->number_of_cylinders *
            (uint32_t) di->number_of_heads *
            (uint32_t) di->number_of_sectors *
            SECTOR_SIZE_BYTES;
}
