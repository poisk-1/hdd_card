#pragma once

#include <inttypes.h>

enum BaseRequest : uint8_t {
    CTRL_REQUEST_DONE = 0,
};

enum Status : uint8_t {
    CTRL_STATUS_READY = 0,
    CTRL_STATUS_BUSY = 0xff
};

struct BaseCtrl {
    enum Status status;
    enum BaseRequest request;
};

void wait_or_handle_ctrl_request(struct BaseCtrl *ctrl, void(*wait)(void), void(*handle)(struct BaseCtrl *));
