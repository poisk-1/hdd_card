#pragma once

#include <inttypes.h>

enum ServiceRequestBase {
    SERVICE_REQUEST_DONE = 0,
};

enum ServiceStatus {
    SERVICE_STATUS_READY = 0,
    SERVICE_STATUS_BUSY = 0xff
};

struct ServiceCtrlBase {
    uint8_t status;
    uint8_t request;
};

void service_wait_or_handle_ctrl_request(struct ServiceCtrlBase *ctrl, void(*wait)(void), void(*handle)(struct ServiceCtrlBase *));
