#include <xc.h>
#include <string.h>

#include "buffer.h"

#define BUFFER_SIZE (CTRL_BUFFER_SIZE + DATA_BUFFER_SIZE)

static uint8_t buffer[BUFFER_SIZE];

void init_buffer(void) {
    memset(buffer, 0, BUFFER_SIZE);

    INT0PPS = 0x0C;   //RB4->EXT_INT:INT0;    
    INT1PPS = 0x0D;   //RB5->EXT_INT:INT1;    

    PIR1bits.INT0IF = 0;
    INTCON0bits.INT0EDG = 0;
    PIE1bits.INT0IE = 1;


    PIR6bits.INT1IF = 0;
    INTCON0bits.INT1EDG = 0;
    PIE6bits.INT1IE = 1;

    INTCON0bits.GIE = 1; 
}

void *get_ctrl_buffer(void) { return &buffer[0]; }
void *get_data_buffer(void) { return &buffer[CTRL_BUFFER_SIZE]; }

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
