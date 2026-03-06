/**
 * @file main.c
 * @author peter
 * @date 2025-10-29
 * @brief Main function
 */

#include <xc.h>
#include <xc8debug.h>
#include <string.h>

#include "icsp.h"
#include "uart.h"
#include "buffer.h"
#include "service.h"
#include "log.h"
#include "spi.h"
#include "sd.h"
#include "int13h_service.h"

struct Int13hService int13h_service;

void wait_no_media_present() {
    int13_service_wait_no_media_present(&int13h_service);
}

void handle_no_media_present(struct ServiceCtrlBase *ctrl) {
    int13_service_handle_no_media_present(&int13h_service, ctrl);
}

void wait_media_present() {
    int13_service_wait_media_present(&int13h_service);
}

void handle_media_present(struct ServiceCtrlBase *ctrl) {
    int13_service_handle_media_present(&int13h_service, ctrl);
}

int main(){
    init_icsp();
    uart_init();
    buffer_init();
    spi_init();
    service_init();
    int13_service_init(&int13h_service);

    LOG("INIT\r\n");

    struct ServiceCtrlBase *ctrl = (struct ServiceCtrlBase *)buffer_get_ctrl();
    
    while(1) {
        if (sd_card_detected() && int13_service_mount_media(&int13h_service)) {
            while (sd_card_detected()) {
                service_wait_or_handle_ctrl_request(
                        ctrl,
                        wait_media_present,
                        handle_media_present
                        );
            }

            int13_service_unmount_media(&int13h_service);
        } else {
            service_wait_or_handle_ctrl_request(
                    ctrl,
                    wait_no_media_present,
                    handle_no_media_present
                    );
        }
    }

    return 0;
}
