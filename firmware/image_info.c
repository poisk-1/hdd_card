#include "image_info.h"

uint32_t size_blocks(struct DiskInfo* di)
{
  return di->number_of_cylinders *
                  di->number_of_heads *
                  di->number_of_sectors;
}

bool has_geometry(const struct DiskInfo* di) {
    return di->number_of_cylinders != 0;
}
