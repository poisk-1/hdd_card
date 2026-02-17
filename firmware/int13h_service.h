#pragma once

#include "drive_info.h"
#include "service.h"
#include "multiblock_transfer.h"

struct Int13hService {
    struct CardInfo card_info;
    
    uint32_t current_read_block_address;
    struct MultiblockTransfer read_mbt;

    uint32_t current_write_block_address;
    struct MultiblockTransfer write_mbt;

    bool media_changed[MAX_NUMBER_FLOPPY_DRIVES];
};

void int13_service_init(struct Int13hService *int13h_service);

bool int13_service_mount_media(struct Int13hService *int13h_service);
void int13_service_unmount_media(struct Int13hService *int13h_service);

void int13_service_wait_media_present(struct Int13hService *int13h_service);
void int13_service_handle_media_present(struct Int13hService *int13h_service, struct ServiceCtrlBase *ctrl);

void int13_service_wait_no_media_present(struct Int13hService *int13h_service);
void int13_service_handle_no_media_present(struct Int13hService *int13h_service, struct ServiceCtrlBase *ctrl);