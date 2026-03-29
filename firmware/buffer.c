#include <language_support.h>
#include <xc.h>
#include <string.h>

#include "buffer.h"

#define BUFFER_SIZE (CTRL_BUFFER_SIZE + DATA_BUFFER_SIZE)

#define BUFFER_BANK_BASE 0x12
#define BUFFER_BANK_MASK 0x1f

#define BUFFER_BASE ((uint8_t*)(BUFFER_BANK_BASE << 8))

void buffer_init(void) {

    memset(BUFFER_BASE, 0, BUFFER_SIZE);

    // IO_RE(INT0) -> RB4
    LATBbits.LATB4 = 0;
    TRISBbits.TRISB4 = 1;
    ANSELBbits.ANSELB4 = 0;
    INT0PPSbits.PIN = 4;   
    INT0PPSbits.PORT = 1; // B
    PIR1bits.INT0IF = 0;
    INTCON0bits.INT0EDG = 0;
    PIE1bits.INT0IE = 1;

    // IO_WE(INT1) -> RB5
    LATBbits.LATB5 = 0;
    TRISBbits.TRISB5 = 1;
    ANSELBbits.ANSELB5 = 0;
    INT1PPSbits.PIN = 5;
    INT1PPSbits.PORT = 1; // B
    PIR6bits.INT1IF = 0;
    INTCON0bits.INT1EDG = 0;
    PIE6bits.INT1IE = 1;

    // DAT.0 - DAT.7 -> PORTA
    LATA = 0x00;
    TRISA = 0xFF;
    ANSELA = 0x00;

    // ADDR.0 - ADDR.7 -> PORTD
    LATD = 0x00;
    TRISD = 0xFF;
    ANSELD = 0x00;

    // SEL0 -> RC0
    LATCbits.LATC0 = 0;
    TRISCbits.TRISC0 = 1;
    ANSELCbits.ANSELC0 = 0;

    // SEL1 -> RC1
    LATCbits.LATC1 = 0;
    TRISCbits.TRISC1 = 1;
    ANSELCbits.ANSELC1 = 0;

    // SEL2 -> RC2
    LATCbits.LATC2 = 0;
    TRISCbits.TRISC2 = 1;
    ANSELCbits.ANSELC2 = 0;

    // SEL3 -> RC3
    LATCbits.LATC3 = 0;
    TRISCbits.TRISC3 = 1;
    ANSELCbits.ANSELC3 = 0;

    // SEL4 -> RC4
    LATCbits.LATC4 = 0;
    TRISCbits.TRISC4 = 1;
    ANSELCbits.ANSELC4 = 0;

    // ACK_IO -> RE1
    LATEbits.LATE1 = 1;
    TRISEbits.TRISE1 = 0;
    ANSELEbits.ANSELE1 = 1;

    INTCON0bits.GIE = 1;
}

void *buffer_get_ctrl(void) { return &BUFFER_BASE[0]; }
void *buffer_get_data(void) { return &BUFFER_BASE[CTRL_BUFFER_SIZE]; }

extern void __interrupt(irq(INT0)) handle_read(void) {
    asm("bcf            PIR1,0"); // INT0IF = 0

    asm("clrf           TRISA");

    asm("movf           PORTC,w");
    asm("andlw          0x1f");
    asm("addlw          0x12");
    asm("movwf          fsr2h");

    asm("movf           PORTD,w");
    asm("movwf          fsr2l");

    asm("movff          indf2,PORTA");

    asm("bcf            LATE,1"); // ACK_IO = 0;
    asm("bsf            LATE,1"); // ACK_IO = 1;

    asm("setf           TRISA");

    
}

extern void __interrupt(irq(INT1)) handle_write(void) {
    asm("bcf            PIR6,0"); // INT1IF = 0

    asm("movf           PORTC,w");
    asm("andlw          0x1f");
    asm("addlw          0x12");
    asm("movwf          fsr2h");

    asm("movf           PORTD,w");
    asm("movwf          fsr2l");

    asm("movff          PORTA,indf2");

    asm("bcf            LATE,1"); // ACK_IO = 0;
    asm("bsf            LATE,1"); // ACK_IO = 1;
}

void __interrupt(irq(default)) handle_undefined(unsigned char src) {
    __builtin_software_breakpoint();
}
