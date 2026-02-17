#include "multiblock_transfer.h"

void mb_transfer_init(struct MultiblockTransfer *mbt,
        void (*stop)(void),
        bool(*start)(uint32_t),
        bool(*tx)(void *)
        ) {
    mbt->stop = stop;
    mbt->start = start;
    mbt->tx = tx;

    mbt->last_block_address = 0;
    mbt->timeout = 0;
}

bool mb_transfer_stop_if_expired(struct MultiblockTransfer *mbt) {
    if (mbt->timeout) {
        if (!--mbt->timeout) {
            mbt->stop();
            return true;
        }
    }
    return false;
}

void mb_transfer_stop(struct MultiblockTransfer *mbt) {
    if (mbt->timeout) {
        mbt->stop();
        mbt->timeout = 0;
    }
}

void mb_transfer_abort(struct MultiblockTransfer *mbt) {
    mbt->timeout = 0;
}

bool mb_transfer_next_sector(struct MultiblockTransfer *mbt, uint32_t block_address, void *block_buffer) {
    if (mbt->timeout) {
        if (mbt->last_block_address + 1 == block_address) {
            if (mbt->tx(block_buffer)) {
                mbt->last_block_address = block_address;
                mbt->timeout = NEXT_TIMEOUT;
                return true;
            } else {
                mbt->stop();
                mbt->timeout = 0;
            }
        } else {
            mbt->stop();

            if (mbt->start(block_address) && mbt->tx(block_buffer)) {
                mbt->last_block_address = block_address;
                mbt->timeout = NEXT_TIMEOUT;
                return true;
            } else {
                mbt->stop();
                mbt->timeout = 0;
            }
        }
    } else {
        if (mbt->start(block_address) && mbt->tx(block_buffer)) {
            mbt->last_block_address = block_address;
            mbt->timeout = NEXT_TIMEOUT;
            return true;
        } else {
            mbt->stop();
        }
    }
    return false;
}
