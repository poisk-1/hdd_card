#include <pic18f47q83.h>
#include <xc.h>
#include <string.h>

#include "buffer.h"

#define BUFFER_SIZE (CTRL_BUFFER_SIZE + DATA_BUFFER_SIZE)

static uint8_t buffer[BUFFER_SIZE];

void buffer_init(void) {
    memset(buffer, 0, BUFFER_SIZE);

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

    // SEL0 -> RB0
    LATBbits.LATB0 = 0;
    TRISBbits.TRISB0 = 1;
    ANSELBbits.ANSELB0 = 0;

    // SEL1 -> RB1
    LATBbits.LATB1 = 0;
    TRISBbits.TRISB1 = 1;
    ANSELBbits.ANSELB1 = 0;

    // SEL2 -> RB2
    LATBbits.LATB2 = 0;
    TRISBbits.TRISB2 = 1;
    ANSELBbits.ANSELB2 = 0;

    // SEL3 -> RB3
    LATBbits.LATB3 = 0;
    TRISBbits.TRISB3 = 1;
    ANSELBbits.ANSELB3 = 0;

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

void *buffer_get_ctrl(void) { return &buffer[0]; }
void *buffer_get_data(void) { return &buffer[CTRL_BUFFER_SIZE]; }

#define ACK_IO LATEbits.LATE1
#define ADDRESS ((((PORTB & 0xf) | (PORTC & 0x10)) << 8) | PORTD)

void __interrupt(irq(INT0)) handle_read(void) {
    PIR1bits.INT0IF = 0;

    TRISA = 0x00;
    LATA = buffer[ADDRESS];
    ACK_IO = 0;
    ACK_IO = 1;
    TRISA = 0xff;
}

void __interrupt(irq(INT1)) handle_write(void) {
    PIR6bits.INT1IF = 0;

    buffer[ADDRESS] = PORTA;
    ACK_IO = 0;
    ACK_IO = 1;
}

void __interrupt(irq(default)) handle_undefined(unsigned char src) {
    __builtin_software_breakpoint();
}
