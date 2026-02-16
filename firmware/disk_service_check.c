#include <inttypes.h>
#include <stdlib.h>

#include "buffer.h"
#include "log.h"

void invert_buffer(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = ~buffer[i];
    }
}

void check(void) {
    LOG("CHECK\r\n");

    invert_buffer(buffer_get_data(), DATA_BUFFER_SIZE);
}