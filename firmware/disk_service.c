#include "disk_service.h"

#include "disk_ctrl.h"
#include "disk_service_check.h"
#include "log.h"

void wait_no_media_present() {}

void handle_no_media_present(struct BaseCtrl *ctrl) {
    struct DiskCtrl *disk_ctrl = (struct DiskCtrl *) ctrl;

    switch ((enum DiskRequest)disk_ctrl->base.request) {
        case CTRL_REQUEST_CHECK:
            check();
            break;

        case CTRL_REQUEST_SCAN:
            LOG("SCAN [no media]\r\n");
            disk_ctrl->req.scan_req.number_of_floppy_drives = 0;
            disk_ctrl->req.scan_req.number_of_hard_drives = 0;
            break;

        case CTRL_REQUEST_RESET:
        case CTRL_REQUEST_READ:
        case CTRL_REQUEST_READ_NEXT:
        case CTRL_REQUEST_WRITE:
        case CTRL_REQUEST_WRITE_NEXT:
        case CTRL_REQUEST_VERIFY:
            LOG("RESET/READ/WRITE/VERIFY [no media]\r\n");
            disk_ctrl->req.rwv_req.status = STATUS_CONTROLLER_FAILED;
            break;

        case CTRL_REQUEST_READ_PARAMS_FUN8H:
            LOG("READ_PARAMS_FUN8H [no media]\r\n");
            disk_ctrl->req.read_params_fun8h_req.number_of_drives = 0;
            disk_ctrl->req.read_params_fun8h_req.success = 0;
            break;

        case CTRL_REQUEST_READ_PARAMS_FUN15H:
            LOG("READ_PARAMS_FUN15H [no media]\r\n");
            disk_ctrl->req.read_params_fun15h_req.success = 0;
            break;

        case CTRL_REQUEST_DETECT_MEDIA_CHANGE:
            LOG("DETECT_MEDIA_CHANGE [no media]\r\n");
            disk_ctrl->req.detect_media_change.status = STATUS_CONTROLLER_FAILED;
            break;

        default:
            LOG("UNKNOWN REQUEST %d [no media]\r\n", ctrl->request);
            break;
    }
}
