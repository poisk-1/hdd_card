#pragma once

#include <inttypes.h>

enum BaseRequest {
    CTRL_REQUEST_DONE = 0,
};

enum Status {
    CTRL_STATUS_READY = 0,
    CTRL_STATUS_BUSY = 0xff
};

struct BaseCtrl {
    uint8_t status;
    uint8_t request;
};

void wait_or_handle_ctrl_request(struct BaseCtrl *ctrl, void(*wait)(void), void(*handle)(struct BaseCtrl *));
