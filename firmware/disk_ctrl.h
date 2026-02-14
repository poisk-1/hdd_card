#pragma once

#include <inttypes.h>

#include "service.h"

enum DiskRequest {
    CTRL_REQUEST_CHECK = 0x1,
    CTRL_REQUEST_SCAN = 0x2,
    CTRL_REQUEST_RESET = 0x3,
    CTRL_REQUEST_READ = 0x4,
    CTRL_REQUEST_READ_NEXT = 0x5,
    CTRL_REQUEST_WRITE = 0x6,
    CTRL_REQUEST_WRITE_NEXT = 0x7,
    CTRL_REQUEST_VERIFY = 0x8,
    CTRL_REQUEST_READ_PARAMS_FUN8H = 0x9,
    CTRL_REQUEST_READ_PARAMS_FUN15H = 0xa,
    CTRL_REQUEST_DETECT_MEDIA_CHANGE = 0xb,
};

enum DriveReqStatus {
    STATUS_NO_ERROR = 0,
    STATUS_BAD_SECTOR = 0x2,
    STATUS_WRITE_PROTECTED = 0x3,
    STATUS_DISK_CHANGED = 0x6,
    STATUS_CONTROLLER_FAILED = 0x20
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

struct DiskCtrl {
    struct BaseCtrl base;
    union Req req;
};

