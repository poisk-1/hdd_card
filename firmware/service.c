#include <xc.h>

#include "service.h"

#define LED LATEbits.LATE0
#define REQ_COMPLETE LATEbits.LATE2

void wait_or_handle_ctrl_request(struct BaseCtrl *ctrl, void(*wait)(void), void(*handle)(struct BaseCtrl *)) {
    if (ctrl->request == CTRL_REQUEST_DONE) {
        wait();
    } else {
        ctrl->status = CTRL_STATUS_BUSY;
        LED = 0;
        handle(ctrl);
        ctrl->status = CTRL_STATUS_READY;
        REQ_COMPLETE = 1;
        while (ctrl->request != CTRL_REQUEST_DONE);
        REQ_COMPLETE = 0;
        LED = 1;
    }
}
