#pragma once

#include <inttypes.h>
#include <stdbool.h>

#define NEXT_TIMEOUT 20000

struct MultiblockTransfer {
    void (*stop)(void);
    bool(*start)(uint32_t);
    bool(*tx)(void *);

    uint32_t last_block_address;
    uint32_t timeout;
};

void mb_transfer_init(struct MultiblockTransfer *mbt,
        void (*stop)(void),
        bool(*start)(uint32_t),
        bool(*tx)(void *)
        );

bool mb_transfer_stop_if_expired(struct MultiblockTransfer *mbt);
void mb_transfer_stop(struct MultiblockTransfer *mbt);
void mb_transfer_abort(struct MultiblockTransfer *mbt);
bool mb_transfer_next_sector(struct MultiblockTransfer *mbt, uint32_t block_address, void *sector_buffer);