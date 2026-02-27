#include "int13h_service.h"

#include <inttypes.h>
#include <stdint.h>
#include <string.h>

#include "image_info.h"
#include "log.h"
#include "multiblock_transfer.h"
#include "sd.h"
#include "buffer.h"
#include "uart.h"


enum Int13hServiceRequest {
    INT13H_SERVICE_REQUEST_CHECK = 0x1,
    INT13H_SERVICE_REQUEST_SCAN = 0x2,
    INT13H_SERVICE_REQUEST_RESET = 0x3,
    INT13H_SERVICE_REQUEST_READ = 0x4,
    INT13H_SERVICE_REQUEST_READ_NEXT = 0x5,
    INT13H_SERVICE_REQUEST_WRITE = 0x6,
    INT13H_SERVICE_REQUEST_WRITE_NEXT = 0x7,
    INT13H_SERVICE_REQUEST_VERIFY = 0x8,
    INT13H_SERVICE_REQUEST_READ_PARAMS_FUN8H = 0x9,
    INT13H_SERVICE_REQUEST_READ_PARAMS_FUN15H = 0xa,
    INT13H_SERVICE_REQUEST_DETECT_MEDIA_CHANGE = 0xb,
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

static const struct DiskInfo* find_disk_info(const struct Int13hService* service, uint8_t drive_number) {
    const struct DiskInfo* di = NULL;
    size_t drive_index = drive_number & HARD_DRIVE_NUMBER_MASK;

    if (is_hard_drive(drive_number) && drive_index < MAX_NUMBER_HARD_DRIVES) {
        di = service->current_hard_dis[drive_index];
    } else if (drive_index < MAX_NUMBER_FLOPPY_DRIVES) {
        di = service->current_floppy_dis[drive_index];
    }

    return di;
}

void int13_service_init(struct Int13hService *service) {
    memset(service, 0, sizeof(struct Int13hService));

    mb_transfer_init(&service->read_mbt, sd_stop_read_blocks, sd_start_read_blocks, sd_read_next_block);
    mb_transfer_init(&service->write_mbt, sd_stop_write_blocks, sd_start_write_blocks, sd_write_next_block);
}

void log_drive_info(const struct DiskInfo* di) {
    LOG("\tDRIVE NUMBER: 0x%x\r\n", di->drive_number);
    LOG("\tDRIVE TYPE FUN8H: 0x%x\r\n", di->drive_type_fun8h);
    LOG("\tDRIVE TYPE FUN15H: 0x%x\r\n", di->drive_type_fun15h);

    LOG("\tNUM OF HEADS: %d\r\n", di->number_of_heads);
    LOG("\tNUM OF CYLINDERS: %d\r\n", di->number_of_cylinders);
    LOG("\tNUM OF SECTORS: %d\r\n", di->number_of_sectors);

    LOG("\tIMAGE OFFSET: %lu\r\n", di->image_offset);
}

bool int13_service_mount_media(struct Int13hService *service) {
    struct SDMediaInfo media_info;

    sd_media_init(&media_info);

    if (media_info.error == SD_MEDIA_ERROR_NO_ERROR) {
        LOG("SD MEDIA DETECTED IN %s MODE\r\n", media_info.sd_mode == SD_MODE_NORMAL ? "NORMAL" : "HC");

        if (sd_start_read_blocks(0) && sd_read_next_block(buffer_get_data())) {
            struct ImageInfo *ii = (struct ImageInfo *)buffer_get_data();
            if (memcmp(ii->magic, IMAGE_MAGIC_STR, IMAGE_MAGIC_SIZE) == 0) {
                memcpy(&service->ii, ii, sizeof(struct ImageInfo));

                service->floppy_drive_count = 0;
                service->hard_drive_count = 0;

                for (uint8_t di_index = 0; di_index < MAX_NUMBER_DISKS; di_index++) {
                    struct DiskInfo* di = &service->ii.dis[di_index];
                    if (has_geometry(di)) {
                        uint8_t drive_index = di->drive_number & HARD_DRIVE_NUMBER_MASK;

                        if (is_hard_drive(di->drive_number) && drive_index < MAX_NUMBER_HARD_DRIVES) {
                            LOG("MOUNTED HD%d#%d:\r\n", drive_index, service->hard_di_counts[drive_index]);
                            log_drive_info(di);

                            if (service->current_hard_dis[drive_index] == NULL) {
                                service->current_hard_dis[drive_index] = di;
                                service->hard_drive_count++;
                            }

                            service->hard_di_indexes[drive_index][service->hard_di_counts[drive_index]++] = di_index;
                        }
                        else if (drive_index < MAX_NUMBER_FLOPPY_DRIVES) {
                            LOG("MOUNTED FP%d#%d:\r\n", drive_index, service->floppy_di_counts[drive_index]);
                            log_drive_info(di);

                            if (service->current_floppy_dis[drive_index] == NULL) {
                                service->current_floppy_dis[drive_index] = di;
                                service->floppy_drive_count++;
                            }

                            service->floppy_di_indexes[drive_index][service->floppy_di_counts[drive_index]++] = di_index;
                        }
                    }
                }
            }
            else {
                LOG("MAGIC NOT FOUND");
                return false;
            }
        }
        sd_stop_read_blocks();

        for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
            service->media_changed[i] = true;
        }

        return true;
    }
    else {
        LOG("SD MEDIA INIT FAILED");
    }

    return false;
}

void int13_service_unmount_media(struct Int13hService *service) {
    mb_transfer_abort(&service->read_mbt);
    mb_transfer_abort(&service->write_mbt);    

    LOG("UNMOUNTED\r\n");
}

void int13_service_wait_media_present(struct Int13hService *service) {
    mb_transfer_stop_if_expired(&service->read_mbt);
    mb_transfer_stop_if_expired(&service->write_mbt);

    uint8_t command = 0;
    if (try_uart_read(&command)) {
        if (command >= 'a' && command < 'a' + MAX_NUMBER_FLOPPY_DRIVES) {
            uint8_t drive_index = command - 'a';

            for (uint8_t i = 0; i < service->floppy_di_counts[drive_index]; i++) {
                uint8_t di_index = service->floppy_di_indexes[drive_index][i];
                struct DiskInfo* di = &service->ii.dis[di_index];

                if (di == service->current_floppy_dis[drive_index]) {
                    uint8_t j = (i + 1) % service->floppy_di_counts[drive_index];
                    LOG("FLOPPY DRIVE %d DISK CHANGE FROM %d TO %d\r\n", drive_index, i, j);

                    service->current_floppy_dis[drive_index] = &service->ii.dis[service->floppy_di_indexes[drive_index][j]];
                    service->media_changed[drive_index] = true;
                    break;
                }
            }
        } else if (command >= '0' && command < '0' + MAX_NUMBER_HARD_DRIVES) {
            uint8_t drive_index = command - '0';

            for (uint8_t i = 0; i < service->hard_di_counts[drive_index]; i++) {
                uint8_t di_index = service->hard_di_indexes[drive_index][i];
                struct DiskInfo* di = &service->ii.dis[di_index];

                if (di == service->current_hard_dis[drive_index]) {
                    uint8_t j = (i + 1) % service->hard_di_counts[drive_index];
                    LOG("HARD DRIVE %d DISK CHANGE FROM %d TO %d\r\n", drive_index, i, j);

                    service->current_hard_dis[drive_index] = &service->ii.dis[service->hard_di_indexes[drive_index][j]];
                    break;
                }
            }
        }
    }
}

#define HIGH_CYLINDER_BITS 2
#define HIGH_CYLINDER_NUMBER_MASK ((uint8_t)0xc0)
#define SECTOR_NUMBER_MASK ((uint8_t)~HIGH_CYLINDER_NUMBER_MASK)

static bool chs_to_block_address(const struct DiskInfo* di, const struct RWVReq* disk_req, uint32_t *block_address) {
    uint32_t cylinder_number =
            (uint32_t) disk_req->low_cylinder_number |
            (((uint32_t) disk_req->sector_and_high_cylinder_numbers & HIGH_CYLINDER_NUMBER_MASK) << HIGH_CYLINDER_BITS);

    uint32_t sector_number =
            (uint32_t) disk_req->sector_and_high_cylinder_numbers & SECTOR_NUMBER_MASK;

    if (cylinder_number < di->number_of_cylinders &&
            disk_req->head_number < di->number_of_heads &&
            sector_number <= di->number_of_sectors) {
        *block_address = di->image_offset + ((uint32_t) cylinder_number * di->number_of_heads +
                (uint32_t) disk_req->head_number) * di->number_of_sectors +
                ((uint32_t) sector_number - 1);

        return true;
    }

    return false;
}

static void invert_buffer(uint8_t *buffer, size_t size) {
    for (size_t i = 0; i < size; i++) {
        buffer[i] = ~buffer[i];
    }
}

static void set_params_fun8h(const struct DiskInfo* di, struct ReadParamsFun8hReq* req) {
    req->success = 1;
    req->drive_type = di->drive_type_fun8h;
    req->max_low_cylinder_number = (uint8_t) ((di->number_of_cylinders - 1) & 0xff);
    req->max_head_number = (uint8_t) di->number_of_heads - 1;
    req->max_sector_and_high_cylinder_numbers =
            (uint8_t) (di->number_of_sectors & SECTOR_NUMBER_MASK) |
            (uint8_t) (((di->number_of_cylinders - 1) >> HIGH_CYLINDER_BITS) & HIGH_CYLINDER_NUMBER_MASK);
}

void int13_service_handle_media_present(struct Int13hService *service, struct ServiceCtrlBase *base_ctrl) {
    struct Int13hCtrl *ctrl = (struct Int13hCtrl *) base_ctrl;
    uint8_t *data_buffer = buffer_get_data();

    const struct DiskInfo* di = NULL;

    if (ctrl->base.request != INT13H_SERVICE_REQUEST_READ &&
            ctrl->base.request != INT13H_SERVICE_REQUEST_READ_NEXT) {
        mb_transfer_stop(&service->read_mbt);
    }

    if (ctrl->base.request != INT13H_SERVICE_REQUEST_WRITE &&
            ctrl->base.request != INT13H_SERVICE_REQUEST_WRITE_NEXT) {
        mb_transfer_stop(&service->write_mbt);
    }

    switch ((enum Int13hServiceRequest)ctrl->base.request) {
        case INT13H_SERVICE_REQUEST_CHECK:
            LOG("CHECK\r\n");

            invert_buffer(buffer_get_data(), DATA_BUFFER_SIZE);
            break;

        case INT13H_SERVICE_REQUEST_SCAN:
            ctrl->req.scan_req.number_of_floppy_drives = service->floppy_drive_count;
            ctrl->req.scan_req.number_of_hard_drives = service->hard_drive_count;

            LOG("SCAN [%d FPS, %d HDS]\r\n", ctrl->req.scan_req.number_of_floppy_drives, ctrl->req.scan_req.number_of_hard_drives);
            break;

        case INT13H_SERVICE_REQUEST_RESET:
            LOG("RESET [d=%d]\r\n", ctrl->req.rwv_req.drive_number);
            ctrl->req.rwv_req.status = 0;
            break;

        case INT13H_SERVICE_REQUEST_READ:
            di = find_disk_info(service, ctrl->req.rwv_req.drive_number);

            LOG(
                    "READ [d=%d,lc=%d,h=%d,shc=%d,sct=%d] ",
                    ctrl->req.rwv_req.drive_number,
                    ctrl->req.rwv_req.low_cylinder_number,
                    ctrl->req.rwv_req.head_number,
                    ctrl->req.rwv_req.sector_and_high_cylinder_numbers,
                    ctrl->req.rwv_req.sectors_count
                    );

            if (!(di != NULL && has_geometry(di) && chs_to_block_address(di, &ctrl->req.rwv_req, &service->current_read_block_address))) {
                LOG("[no geometry/bad chs]\r\n");
                ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
                break;
            }

        case INT13H_SERVICE_REQUEST_READ_NEXT:
            if (ctrl->base.request == INT13H_SERVICE_REQUEST_READ_NEXT) {
                LOG("READ_NEXT [lct=%d,sct=%d] ",
                        ctrl->req.rwv_req.sectors_last_r_next_w_count,
                        ctrl->req.rwv_req.sectors_count);
            }

            ctrl->req.rwv_req.sectors_last_r_next_w_count = 0;
            ctrl->req.rwv_req.status = 0;

            if (ctrl->req.rwv_req.sectors_count != 0) {
                uint8_t sectors_to_read = ctrl->req.rwv_req.sectors_count;
                if (sectors_to_read > BUFFER_SECTORS) sectors_to_read = BUFFER_SECTORS;

                for (uint8_t i = 0; i < sectors_to_read; i++) {
                    if (mb_transfer_next_sector(&service->read_mbt, service->current_read_block_address, &data_buffer[i * SECTOR_SIZE])) {
                        ctrl->req.rwv_req.sectors_last_r_next_w_count++;
                        ctrl->req.rwv_req.sectors_count--;

                        LOG("[a=%lu] ", service->current_read_block_address);
                        service->current_read_block_address++;
                    } else {
                        LOG("[read error] ");
                        ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
                        break;
                    }
                }
                LOG("[lct=%d,sct=%d]\r\n",
                        ctrl->req.rwv_req.sectors_last_r_next_w_count,
                        ctrl->req.rwv_req.sectors_count);

            } else {
                LOG("[nothing to read]\r\n");
            }

            break;


        case INT13H_SERVICE_REQUEST_WRITE:
            di = find_disk_info(service, ctrl->req.rwv_req.drive_number);
            LOG(
                    "WRITE [d=%d,lc=%d,h=%d,shc=%d,sct=%d] ",
                    ctrl->req.rwv_req.drive_number,
                    ctrl->req.rwv_req.low_cylinder_number,
                    ctrl->req.rwv_req.head_number,
                    ctrl->req.rwv_req.sector_and_high_cylinder_numbers,
                    ctrl->req.rwv_req.sectors_count
                    );

            ctrl->req.rwv_req.sectors_last_r_next_w_count = 0;
            ctrl->req.rwv_req.status = 0;

            if (di != NULL && has_geometry(di) && chs_to_block_address(di, &ctrl->req.rwv_req, &service->current_write_block_address)) {
                if (di->read_only) {
                    LOG(" [write protected]\r\n");
                    ctrl->req.rwv_req.status = INT13H_STATUS_WRITE_PROTECTED;
                } else {
                    if (ctrl->req.rwv_req.sectors_count != 0) {
                        uint8_t sectors_to_write = ctrl->req.rwv_req.sectors_count;
                        if (sectors_to_write > BUFFER_SECTORS) sectors_to_write = BUFFER_SECTORS;

                        ctrl->req.rwv_req.sectors_last_r_next_w_count += sectors_to_write;
                        ctrl->req.rwv_req.sectors_count -= sectors_to_write;

                        LOG("[nct=%d,sct=%d]\r\n",
                                ctrl->req.rwv_req.sectors_last_r_next_w_count,
                                ctrl->req.rwv_req.sectors_count);

                    } else {
                        LOG("[nothing to write]\r\n");
                    }
                }
            } else {
                LOG("[no lba]\r\n");
                ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
            }

            break;

        case INT13H_SERVICE_REQUEST_WRITE_NEXT:
            LOG("WRITE_NEXT [nct=%d,sct=%d] ",
                    ctrl->req.rwv_req.sectors_last_r_next_w_count,
                    ctrl->req.rwv_req.sectors_count);

            ctrl->req.rwv_req.status = 0;

            if (ctrl->req.rwv_req.sectors_last_r_next_w_count != 0) {
                for (uint8_t i = 0; i < ctrl->req.rwv_req.sectors_last_r_next_w_count; i++) {
                    if (mb_transfer_next_sector(&service->write_mbt, service->current_write_block_address, &data_buffer[i * SECTOR_SIZE])) {
                        LOG("[a=%lu] ", service->current_write_block_address);
                        service->current_write_block_address++;
                    } else {
                        LOG("[write error] ");
                        ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
                        break;

                    }
                }

                ctrl->req.rwv_req.sectors_last_r_next_w_count = 0;

                if (ctrl->req.rwv_req.status == 0 && ctrl->req.rwv_req.sectors_count != 0) {
                    uint8_t sectors_to_write = ctrl->req.rwv_req.sectors_count;
                    if (sectors_to_write > BUFFER_SECTORS) sectors_to_write = BUFFER_SECTORS;

                    ctrl->req.rwv_req.sectors_last_r_next_w_count += sectors_to_write;
                    ctrl->req.rwv_req.sectors_count -= sectors_to_write;

                }
                LOG("[nct=%d,sct=%d]\r\n",
                        ctrl->req.rwv_req.sectors_last_r_next_w_count,
                        ctrl->req.rwv_req.sectors_count);
            } else {
                LOG("[nothing to write]\r\n");
            }

            break;

        case INT13H_SERVICE_REQUEST_VERIFY:
            {
                uint32_t block_address = 0;
                di = find_disk_info(service, ctrl->req.rwv_req.drive_number);
                LOG(
                        "VERIFY [d=%d,lc=%d,h=%d,shc=%d,sct=%d] ",
                        ctrl->req.rwv_req.drive_number,
                        ctrl->req.rwv_req.low_cylinder_number,
                        ctrl->req.rwv_req.head_number,
                        ctrl->req.rwv_req.sector_and_high_cylinder_numbers,
                        ctrl->req.rwv_req.sectors_count
                        );

                if (!(di != NULL && has_geometry(di) && chs_to_block_address(di, &ctrl->req.rwv_req, &block_address))) {
                    LOG("[no geomtery/bad chs]\r\n");
                    ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
                } else {
                    LOG("[a=%lu]\r\n", block_address);
                    ctrl->req.rwv_req.status = 0;
                }
            }

            break;
        case INT13H_SERVICE_REQUEST_READ_PARAMS_FUN8H:
            ctrl->req.read_params_fun8h_req.number_of_drives = 0;

            if (is_hard_drive(ctrl->req.read_params_fun8h_req.drive_number)) {
                ctrl->req.read_params_fun8h_req.number_of_drives = service->hard_drive_count;
            } else {
                ctrl->req.read_params_fun8h_req.number_of_drives = service->floppy_drive_count;
            }

            di = find_disk_info(service, ctrl->req.read_params_fun8h_req.drive_number);

            LOG(
                    "READ_PARAMS_FUN8H [d=%d] ",
                    ctrl->req.read_params_fun8h_req.drive_number
                    );

            if (di != NULL && has_geometry(di)) {
                set_params_fun8h(di, &ctrl->req.read_params_fun8h_req);
                LOG(
                        "[mlc=%d,mh=%d,mshc=%d]\r\n",
                        ctrl->req.read_params_fun8h_req.max_low_cylinder_number,
                        ctrl->req.read_params_fun8h_req.max_head_number,
                        ctrl->req.read_params_fun8h_req.max_sector_and_high_cylinder_numbers
                        );
            } else {
                LOG("[no geometry]\r\n");
                ctrl->req.read_params_fun8h_req.success = 0;
                ctrl->req.read_params_fun8h_req.drive_type = 0;
                ctrl->req.read_params_fun8h_req.max_low_cylinder_number = 0;
                ctrl->req.read_params_fun8h_req.max_head_number = 0;
                ctrl->req.read_params_fun8h_req.max_sector_and_high_cylinder_numbers = 0;
            }

            break;

        case INT13H_SERVICE_REQUEST_READ_PARAMS_FUN15H:
            di = find_disk_info(service, ctrl->req.read_params_fun8h_req.drive_number);
            LOG(
                    "READ_PARAMS_FUN15H [d=%d] ",
                    ctrl->req.read_params_fun15h_req.drive_number
                    );

            if (di != NULL && has_geometry(di)) {
                LOG("[t=%d]\r\n", di->drive_type_fun15h);
                ctrl->req.read_params_fun15h_req.success = 1;
                ctrl->req.read_params_fun15h_req.drive_type = di->drive_type_fun15h;
            } else {
                LOG("[no geometry]\r\n");
                ctrl->req.read_params_fun15h_req.success = 0;
                ctrl->req.read_params_fun15h_req.drive_type = 0;
            }

            break;

        case INT13H_SERVICE_REQUEST_DETECT_MEDIA_CHANGE:
            LOG(
                    "DETECT_MEDIA_CHANGE [d=%d] ",
                    ctrl->req.detect_media_change.drive_number
                    );

            if (!is_hard_drive(ctrl->req.read_params_fun8h_req.drive_number) && ctrl->req.read_params_fun8h_req.drive_number < MAX_NUMBER_FLOPPY_DRIVES) {
                if (service->media_changed[ctrl->req.read_params_fun8h_req.drive_number]) {
                    service->media_changed[ctrl->req.read_params_fun8h_req.drive_number] = false;
                    LOG("[media changed]\r\n");
                    ctrl->req.detect_media_change.status = INT13H_STATUS_DISK_CHANGED;
                } else {
                    LOG("[media not changed]\r\n");
                    ctrl->req.detect_media_change.status = 0;
                }
            }
            else {
                LOG("[fixed media]\r\n");
                ctrl->req.rwv_req.status = INT13H_STATUS_BAD_SECTOR;
            }

            break;

        default:
            LOG("UNKNOWN REQUEST %d [no media]\r\n", ctrl->base.request);
            break;
    }
}

void int13_service_wait_no_media_present(struct Int13hService *service) {}

void int13_service_handle_no_media_present(struct Int13hService *service, struct ServiceCtrlBase *base_ctrl) {
    struct Int13hCtrl *ctrl = (struct Int13hCtrl *) base_ctrl;

    switch ((enum Int13hServiceRequest)ctrl->base.request) {
        case INT13H_SERVICE_REQUEST_CHECK:
            LOG("CHECK\r\n");

            invert_buffer(buffer_get_data(), DATA_BUFFER_SIZE);
            break;

        case INT13H_SERVICE_REQUEST_SCAN:
            LOG("SCAN [no media]\r\n");
            ctrl->req.scan_req.number_of_floppy_drives = 0;
            ctrl->req.scan_req.number_of_hard_drives = 0;
            break;

        case INT13H_SERVICE_REQUEST_RESET:
        case INT13H_SERVICE_REQUEST_READ:
        case INT13H_SERVICE_REQUEST_READ_NEXT:
        case INT13H_SERVICE_REQUEST_WRITE:
        case INT13H_SERVICE_REQUEST_WRITE_NEXT:
        case INT13H_SERVICE_REQUEST_VERIFY:
            LOG("RESET/READ/WRITE/VERIFY [no media]\r\n");
            ctrl->req.rwv_req.status = INT13H_STATUS_CONTROLLER_FAILED;
            break;

        case INT13H_SERVICE_REQUEST_READ_PARAMS_FUN8H:
            LOG("READ_PARAMS_FUN8H [no media]\r\n");
            ctrl->req.read_params_fun8h_req.number_of_drives = 0;
            ctrl->req.read_params_fun8h_req.success = 0;
            break;

        case INT13H_SERVICE_REQUEST_READ_PARAMS_FUN15H:
            LOG("READ_PARAMS_FUN15H [no media]\r\n");
            ctrl->req.read_params_fun15h_req.success = 0;
            break;

        case INT13H_SERVICE_REQUEST_DETECT_MEDIA_CHANGE:
            LOG("DETECT_MEDIA_CHANGE [no media]\r\n");
            ctrl->req.detect_media_change.status = INT13H_STATUS_CONTROLLER_FAILED;
            break;

        default:
            LOG("UNKNOWN REQUEST %d [no media]\r\n", ctrl->base.request);
            break;
    }
}
