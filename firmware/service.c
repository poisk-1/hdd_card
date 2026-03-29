#include <xc.h>

#include "service.h"

#define LED LATEbits.LATE0
#define REQ_COMPLETE LATEbits.LATE2

void service_init(void) {
    // LED -> RE0
    LED = 1;
    TRISEbits.TRISE0 = 0;
    ANSELEbits.ANSELE0 = 1;

    // REQ_COMPLETE -> RE2
    REQ_COMPLETE = 0;
    TRISEbits.TRISE2 = 0;
    ANSELEbits.ANSELE2 = 1;
}

void service_wait_or_handle_ctrl_request(struct ServiceCtrlBase *ctrl, void(*wait)(void), void(*handle)(struct ServiceCtrlBase *)) {
    if (ctrl->request == SERVICE_REQUEST_DONE) {
        wait();
    } else {
        ctrl->status = SERVICE_STATUS_BUSY;
        LED = 0;
        handle(ctrl);
        ctrl->status = SERVICE_STATUS_READY;
        REQ_COMPLETE = 1;
        while (ctrl->request != SERVICE_REQUEST_DONE);
        REQ_COMPLETE = 0;
        LED = 1;
    }
}
