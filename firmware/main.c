/**
 * @file main.c
 * @author peter
 * @date 2025-10-29
 * @brief Main function
 */

#include <stdio.h>
#include <string.h>
#include <xc.h>
#include <xc8debug.h>

// CONFIG1
#pragma config FEXTOSC = OFF
#pragma config RSTOSC = HFINTOSC_64MHZ

// CONFIG2
#pragma config CLKOUTEN = OFF
#pragma config PR1WAY = ON
#pragma config CSWEN = ON
#pragma config JTAGEN = OFF
#pragma config FCMEN = ON
#pragma config FCMENP = ON
#pragma config FCMENS = ON

// CONFIG3
#pragma config MCLRE = EXTMCLR
#pragma config PWRTS = PWRT_OFF
#pragma config MVECEN = ON
#pragma config IVT1WAY = ON
#pragma config LPBOREN = OFF
#pragma config BOREN = SBORDIS

// CONFIG4
#pragma config BORV = VBOR_1P9
#pragma config ZCD = OFF
#pragma config PPS1WAY = ON
#pragma config STVREN = ON
#pragma config LVP = ON
#pragma config XINST = OFF

// CONFIG5
#pragma config WDTCPS = WDTCPS_31
#pragma config WDTE = OFF

// CONFIG6
#pragma config WDTCWS = WDTCWS_7
#pragma config WDTCCS = SC

// CONFIG7
#pragma config BBSIZE = BBSIZE_512
#pragma config BBEN = OFF
#pragma config SAFEN = OFF

// CONFIG8
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config WRTSAF = OFF
#pragma config WRTAPP = OFF

// CONFIG9
#pragma config BOOTPINSEL = RC5
#pragma config BPEN = OFF
#pragma config ODCON = OFF

// CONFIG10
#pragma config CP = OFF

// CONFIG11
#pragma config BOOTSCEN = OFF
#pragma config BOOTCOE = HALT
#pragma config APPSCEN = OFF
#pragma config SAFSCEN = OFF
#pragma config DATASCEN = OFF
#pragma config CFGSCEN = OFF
#pragma config COE = HALT
#pragma config BOOTPOR = OFF

// CONFIG12
#pragma config BCRCPOLT = hFF

// CONFIG13
#pragma config BCRCPOLU = hFF

// CONFIG14
#pragma config BCRCPOLH = hFF

// CONFIG15
#pragma config BCRCPOLL = hFF

// CONFIG16
#pragma config BCRCSEEDT = hFF

// CONFIG17
#pragma config BCRCSEEDU = hFF

// CONFIG18
#pragma config BCRCSEEDH = hFF

// CONFIG19
#pragma config BCRCSEEDL = hFF

// CONFIG24
#pragma config CRCPOLT = hFF

// CONFIG25
#pragma config CRCPOLU = hFF

// CONFIG26
#pragma config CRCPOLH = hFF

// CONFIG27
#pragma config CRCPOLL = hFF

// CONFIG28
#pragma config CRCSEEDT = hFF

// CONFIG29
#pragma config CRCSEEDU = hFF

// CONFIG30
#pragma config CRCSEEDH = hFF

// CONFIG31
#pragma config CRCSEEDL = hFF

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
    INT0PPS = 0x0C;   //RB4->EXT_INT:INT0;    
    RC2PPS = 0x32;   //RC2->SPI1:SDO1;    
    INT1PPS = 0x0D;   //RB5->EXT_INT:INT1;    
    RC6PPS = 0x20;   //RC6->UART1:TX1;    
    U1RXPPS = 0x17;   //RC7->UART1:RX1;    
    SPI1SDIPPS = 0x11;   //RC1->SPI1:SDI1;    
        
    // Set the UART1 module to the options selected in the user interface.

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

    PIR1bits.INT0IF = 0;
    INTCON0bits.INT0EDG = 0;
    PIE1bits.INT0IE = 1;


    PIR6bits.INT1IF = 0;
    INTCON0bits.INT1EDG = 0;
    PIE6bits.INT1IE = 1;

    INTCON0bits.GIE = 1; 
}

uint8_t read_uart1(void)
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

void write_uart1(uint8_t txData)
{
    while(0 == PIR4bits.U1TXIF)
    {
    }

    U1TXB = txData;    // Write the data byte to the USART.
}

int getch(void)
{
    return read_uart1();
}

void putch(char txData)
{
    write_uart1(txData);
}

#define CTRL_BUFFER_SIZE 0x200
#define SECTOR_SIZE 0x200
#define BUFFER_SECTORS 15
#define DATA_BUFFER_SIZE (SECTOR_SIZE * BUFFER_SECTORS)
#define IO_BUFFER_SIZE (CTRL_BUFFER_SIZE + DATA_BUFFER_SIZE)

static uint8_t io_buffer[IO_BUFFER_SIZE];

static uint8_t *ctrl_buffer = &io_buffer[0];
static uint8_t *data_buffer = &io_buffer[CTRL_BUFFER_SIZE];

#define LED LATEbits.LATE0
#define ACK_IO LATEbits.LATE1
#define REQ_COMPLETE LATEbits.LATE2

#define IO_ADDRESS ((((PORTB & 0xf) | (PORTC & 0x10)) << 8) | PORTD)

void __interrupt(irq(INT0)) handle_io_read(void) {
    PIR1bits.INT0IF = 0;

    TRISA = 0x00;
    LATA = io_buffer[IO_ADDRESS];
    ACK_IO = 0;
    ACK_IO = 1;
    TRISA = 0xff;
}

void __interrupt(irq(INT1)) handle_io_write(void) {
    PIR6bits.INT1IF = 0;

    io_buffer[IO_ADDRESS] = PORTA;
    ACK_IO = 0;
    ACK_IO = 1;
}

void __interrupt(irq(default)) handle_undefined(unsigned char src) {
    __builtin_software_breakpoint();
}

enum Request {
    CTRL_REQUEST_DONE = 0,
};

enum Status {
    CTRL_STATUS_READY = 0,
    CTRL_STATUS_BUSY = 0xff
};

struct Ctrl {
    uint8_t status;
    uint8_t request;
};

int main(){
    init();

    memset(ctrl_buffer, 0, CTRL_BUFFER_SIZE);
    struct Ctrl *ctrl = (struct Ctrl *) ctrl_buffer;

    printf("Init\r\n");
    
    while(1) {
        if (ctrl->request == CTRL_REQUEST_DONE) {
            ;
        } else {
            printf("Req %d\r\n", ctrl->request);
            ctrl->status = CTRL_STATUS_BUSY;
            LED = 0;

            ctrl->status = CTRL_STATUS_READY;
            REQ_COMPLETE = 1;
            while (ctrl->request != CTRL_REQUEST_DONE);
            REQ_COMPLETE = 0;
            LED = 1;
            printf("Done\r\n");
        }
    }

    return 0;
}
