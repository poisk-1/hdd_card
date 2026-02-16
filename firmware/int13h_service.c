#include "int13h_service.h"

#include <inttypes.h>

#include "disk_service_check.h"
#include "log.h"


enum Int13hServiceRequest {
    INT13H_REQUEST_CHECK = 0x1,
    INT13H_REQUEST_SCAN = 0x2,
    INT13H_REQUEST_RESET = 0x3,
    INT13H_REQUEST_READ = 0x4,
    INT13H_REQUEST_READ_NEXT = 0x5,
    INT13H_REQUEST_WRITE = 0x6,
    INT13H_REQUEST_WRITE_NEXT = 0x7,
    INT13H_REQUEST_VERIFY = 0x8,
    INT13H_REQUEST_READ_PARAMS_FUN8H = 0x9,
    INT13H_REQUEST_READ_PARAMS_FUN15H = 0xa,
    INT13H_REQUEST_DETECT_MEDIA_CHANGE = 0xb,
};

enum Int13hStatus {
    INT13H_STATUS_NO_ERROR = 0,
    INT13H_STATUS_BAD_SECTOR = 0x2,
    INT13H_STATUS_WRITE_PROTECTED = 0x3,
    INT13H_STATUS_DISK_CHANGED = 0x6,
    INT13H_STATUS_CONTROLLER_FAILED = 0x20
};

struct RWVReq {
    uint8_t low_cylinder_number;
    uint8_t sector_and_high_cylinder_numbers;
    uint8_t head_number;
    uint8_t drive_number;

    uint8_t sectors_count;

    uint8_t status;
    uint8_t sectors_last_r_next_w_count;
};

struct ScanReq {
    uint8_t number_of_floppy_drives;
    uint8_t number_of_hard_drives;
};

struct ReadParamsFun8hReq {
    uint8_t drive_number;

    uint8_t success;

    uint8_t drive_type;
    uint8_t max_low_cylinder_number;
    uint8_t max_sector_and_high_cylinder_numbers;
    uint8_t max_head_number;
    uint8_t number_of_drives;
};

struct ReadParamsFun15hReq {
    uint8_t drive_number;

    uint8_t success;

    uint8_t drive_type;
};

struct DetectMediaChangeReq {
    uint8_t drive_number;

    uint8_t status;
};

union Req {
    struct RWVReq rwv_req;
    struct ScanReq scan_req;
    struct ReadParamsFun8hReq read_params_fun8h_req;
    struct ReadParamsFun15hReq read_params_fun15h_req;
    struct DetectMediaChangeReq detect_media_change;
};

struct Int13hCtrl {
    struct ServiceCtrlBase base;
    union Req req;
};

void wait_no_media_present() {}

void handle_no_media_present(struct ServiceCtrlBase *ctrl) {
    struct Int13hCtrl *int13h_ctrl = (struct Int13hCtrl *) ctrl;

    switch ((enum Int13hServiceRequest)int13h_ctrl->base.request) {
        case INT13H_REQUEST_CHECK:
            check();
            break;

        case INT13H_REQUEST_SCAN:
            LOG("SCAN [no media]\r\n");
            int13h_ctrl->req.scan_req.number_of_floppy_drives = 0;
            int13h_ctrl->req.scan_req.number_of_hard_drives = 0;
            break;

        case INT13H_REQUEST_RESET:
        case INT13H_REQUEST_READ:
        case INT13H_REQUEST_READ_NEXT:
        case INT13H_REQUEST_WRITE:
        case INT13H_REQUEST_WRITE_NEXT:
        case INT13H_REQUEST_VERIFY:
            LOG("RESET/READ/WRITE/VERIFY [no media]\r\n");
            int13h_ctrl->req.rwv_req.status = INT13H_STATUS_CONTROLLER_FAILED;
            break;

        case INT13H_REQUEST_READ_PARAMS_FUN8H:
            LOG("READ_PARAMS_FUN8H [no media]\r\n");
            int13h_ctrl->req.read_params_fun8h_req.number_of_drives = 0;
            int13h_ctrl->req.read_params_fun8h_req.success = 0;
            break;

        case INT13H_REQUEST_READ_PARAMS_FUN15H:
            LOG("READ_PARAMS_FUN15H [no media]\r\n");
            int13h_ctrl->req.read_params_fun15h_req.success = 0;
            break;

        case INT13H_REQUEST_DETECT_MEDIA_CHANGE:
            LOG("DETECT_MEDIA_CHANGE [no media]\r\n");
            int13h_ctrl->req.detect_media_change.status = INT13H_STATUS_CONTROLLER_FAILED;
            break;

        default:
            LOG("UNKNOWN REQUEST %d [no media]\r\n", ctrl->request);
            break;
    }
}
