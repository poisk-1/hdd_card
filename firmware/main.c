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
#include "int13h_service.h"
#include "drive_info.h"
#include "spi.h"
#include "sd.h"

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

void log_drive_info(const struct DriveInfo* drive_info) {
    LOG("\tDRIVE TYPE FUN8H: 0x%x\r\n", drive_info->drive_type_fun8h);
    LOG("\tDRIVE TYPE FUN15H: 0x%x\r\n", drive_info->drive_type_fun15h);

    LOG("\tNUM OF HEADS: %d\r\n", drive_info->number_of_heads);
    LOG("\tNUM OF CYLINDERS: %d\r\n", drive_info->number_of_cylinders);
    LOG("\tNUM OF SECTORS: %d\r\n", drive_info->number_of_sectors);

    LOG("\tOFFSET: %lu\r\n", drive_info->card_offset);
}

int main(){
    init();
    uart_init();
    buffer_init();
    spi_init();

    LOG("INIT\r\n");

    if (sd_card_detected()) {
        struct SDMediaInfo media_info;

        sd_media_init(&media_info);

        if (media_info.error == SD_MEDIA_ERROR_NO_ERROR) {
            LOG("SD MEDIA DETECTED IN %s MODE\r\n", media_info.sd_mode == SD_MODE_NORMAL ? "NORMAL" : "HC");
        }

        if (sd_start_read_blocks(0) && sd_read_next_block(buffer_get_data())) {
            struct CardInfo *card_info = (struct CardInfo *)buffer_get_data();
            if (memcmp(card_info->magic, MAGIC_STR, MAGIC_SIZE) == 0) {
                for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                    LOG("FP%d:\r\n", i);
                    log_drive_info(&card_info->floppy_drives[i]);
                }

                for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                    LOG("HD%d:\r\n", i);
                    log_drive_info(&card_info->hard_drives[i]);
                }
            }
        }
        sd_stop_read_blocks();
    }
    
    while(1) {
        service_wait_or_handle_ctrl_request((struct ServiceCtrlBase *) buffer_get_ctrl(), wait_no_media_present, handle_no_media_present);
    }

    return 0;
}
