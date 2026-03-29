#include <xc.h>

#include "uart.h"

void uart_init(void) {
    // UART_TX -> RC6
    LATCbits.LATC6 = 0;
    TRISCbits.TRISC6 = 0;
    ANSELCbits.ANSELC6 = 1;
    RC6PPS = 0x20;

    // UART_RX -> RC7
    LATCbits.LATC7 = 0;
    TRISCbits.TRISC7 = 1;
    ANSELCbits.ANSELC7 = 0;
    U1RXPPSbits.PIN = 7;
    U1RXPPSbits.PORT = 2; // C

    // P1L 0; 
    U1P1L = 0x00;
    // P1H 0; 
    U1P1H = 0x00;
    // P2L 0; 
    U1P2L = 0x00;
    // P2H 0; 
    U1P2H = 0x00;
    // P3L 0; 
    U1P3L = 0x00;
    // P3H 0; 
    U1P3H = 0x00;
    // BRGS high speed; MODE Asynchronous 8-bit mode; RXEN enabled; TXEN enabled; ABDEN disabled; 
    U1CON0 = 0xB0;
    // RXBIMD Set RXBKIF on rising RX input; BRKOVR disabled; WUE disabled; SENDB disabled; ON enabled; 
    U1CON1 = 0x80;
    // TXPOL not inverted; FLO off; C0EN Checksum Mode 0; RXPOL not inverted; RUNOVF RX input shifter stops all activity; STP Transmit 1Stop bit, receiver verifies first Stop bit; 
    U1CON2 = 0x00;
    // BRGL 138; 
    U1BRGL = 0x8A;
    // BRGH 0; 
    U1BRGH = 0x00;
    // STPMD in middle of first Stop bit; TXWRE No error; 
    U1FIFO = 0x00;
    // ABDIF Auto-baud not enabled or not complete; WUIF WUE not enabled by software; ABDIE disabled; 
    U1UIR = 0x00;
    // ABDOVF Not overflowed; TXCIF 0; RXBKIF No Break detected; RXFOIF not overflowed; CERIF No Checksum error; 
    U1ERRIR = 0x00;
    // TXCIE disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; RXFOIE disabled; PERIE disabled; RXBKIE disabled; 
    U1ERRIE = 0x00;
}

uint8_t uart_read(void)
{
    while(!PIR4bits.U1RXIF)
    {
    }

    if(U1ERRIRbits.FERIF){
    }

    if(U1ERRIRbits.RXFOIF){
    }

    return U1RXB;
}

bool try_uart_read(uint8_t *data)
{
    if (PIR4bits.U1RXIF)
    {
        *data = U1RXB;
        return true;
    }

    return false;
}

void uart_write(uint8_t data)
{
    while(0 == PIR4bits.U1TXIF)
    {
    }

    U1TXB = data;
}

int getch(void)
{
    return uart_read();
}

void putch(char data)
{
    uart_write(data);
}
