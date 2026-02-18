/**
 * @file main.c
 * @author peter
 * @date 2025-10-29
 * @brief Main function
 */

#include <xc.h>
#include <xc8debug.h>
#include <string.h>

#include "uart.h"
#include "buffer.h"
#include "service.h"
#include "log.h"
#include "spi.h"
#include "sd.h"
#include "int13h_service.h"

void init() {
    /**
    LATx registers
    */
    LATE = 0x03;
    LATD = 0x00;
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;

    /**
    TRISx registers
    */
    TRISE = 0x00;
    TRISA = 0xFF;
    TRISB = 0xFF;
    TRISC = 0xB2;
    TRISD = 0xFF;

    /**
    ANSELx registers
    */
    ANSELD = 0x00;
    ANSELC = 0x44;
    ANSELB = 0xC0;
    ANSELE = 0x07;
    ANSELA = 0x00;

    /**
    WPUx registers
    */
    WPUD = 0x00;
    WPUE = 0x00;
    WPUB = 0x00;
    WPUA = 0x00;
    WPUC = 0x00;

    /**
    ODx registers
    */
    ODCONE = 0x00;
    ODCONA = 0x00;
    ODCONB = 0x00;
    ODCONC = 0x00;
    ODCOND = 0x00;

    /**
    SLRCONx registers
    */
    SLRCONA = 0xFF;
    SLRCONB = 0xFF;
    SLRCONC = 0xFF;
    SLRCOND = 0xFF;
    SLRCONE = 0x07;

    /**
    INLVLx registers
    */
    INLVLA = 0xFF;
    INLVLB = 0xFF;
    INLVLC = 0xFF;
    INLVLD = 0xFF;
    INLVLE = 0x0F;
}

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
    init();
    uart_init();
    buffer_init();
    spi_init();
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
