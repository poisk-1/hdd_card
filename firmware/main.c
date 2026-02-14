/**
 * @file main.c
 * @author peter
 * @date 2025-10-29
 * @brief Main function
 */

#include <xc.h>
#include <xc8debug.h>

#include "uart.h"
#include "buffer.h"
#include "service.h"
#include "log.h"
#include "disk_service.h"

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

	
    RC0PPS = 0x31;   //RC0->SPI1:SCK1;    
    SPI1SCKPPS = 0x10;   //RC0->SPI1:SCK1;    
    RC2PPS = 0x32;   //RC2->SPI1:SDO1;    
    SPI1SDIPPS = 0x11;   //RC1->SPI1:SDI1;    
        
    // Set the UART1 module to the options selected in the user interface.
}

int main(){
    init();
    init_uart();
    init_buffer();

    LOG("INIT\r\n");
    
    while(1) {
        wait_or_handle_ctrl_request((struct BaseCtrl *) get_ctrl_buffer(), wait_no_media_present, handle_no_media_present);
    }

    return 0;
}
