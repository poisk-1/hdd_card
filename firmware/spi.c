#include "spi.h"

#include <xc.h>

#include "assert.h"

void spi_init(void)
{
    // SDCARD_SCK -> RB0
    LATBbits.LATB0 = 0;
    TRISBbits.TRISB0 = 0;
    ANSELBbits.ANSELB0 = 0;
    RB0PPS = 0x31;
    SPI1SCKPPSbits.PIN = 0;
    SPI1SCKPPSbits.PORT = 1; // B
    SLRCONBbits.SLRB0 = 0; // slew at maximum rate

    // SDCARD_SDI -> RB1
    LATBbits.LATB1 = 0;
    TRISBbits.TRISB1 = 1;
    ANSELBbits.ANSELB1 = 0;
    SPI1SDIPPSbits.PIN = 1;
    SPI1SDIPPSbits.PORT = 1; // B
    SLRCONBbits.SLRB1 = 0; // slew at maximum rate

    // SDCARD_SDO -> RB2
    LATBbits.LATB2 = 0;
    TRISBbits.TRISB2 = 0;
    ANSELBbits.ANSELB2 = 1;
    RB2PPS = 0x32;
    SLRCONBbits.SLRB2 = 0; // slew at maximum rate

    // SDCARD_CS -> RB3
    SPI_CS = 0;
    TRISBbits.TRISB3 = 0;
    ANSELBbits.ANSELB3 = 0;

    // SDCARD_CD -> RC5
    LATCbits.LATC5 = 0;
    TRISCbits.TRISC5 = 1;
    ANSELCbits.ANSELC5 = 0;

    SPI1CON0bits.EN = 0; // SPI Enable: SPI is disabled

    SPI1CON0bits.BMODE = 0; // Bit-Length Mode Select: SPIxTWIDTH setting applies only to the last byte exchanged; total bits sent is SPIxTWIDTH + (SPIxTCNT*8)
    SPI1CON0bits.MST = 1; // SPI Host Operating Mode Select: SPI module operates as the bus host
    SPI1CON0bits.LSBF = 0; // LSb-First Data Exchange Select: Data are exchanged MSb first

    SPI1CON1bits.SDOP = 0; // SPI Output Polarity Control: SDO output is active-high
    SPI1CON1bits.SDIP = 0; // SPI Input Polarity Control: SDI input is active-high
    SPI1CON1bits.SSP = 0; // Client Select Input/Output Polarity Control: SS is active-high
    SPI1CON1bits.FST = 0; // Fast Start Enable: Delay to first SCK will be at least ½ baud period
    SPI1CON1bits.CKP = 1; // Clock Polarity Select: Idle state for SCK is high level
    SPI1CON1bits.CKE = 0; // Clock Edge Select: Output data changes on transition from Idle to Active clock state
    SPI1CON1bits.SMP = 0; // SPI Input Sample Phase Control: SDI input is sampled in the middle of data output time

    // Transfer Off Mode
    SPI1CON2bits.RXR = 0;
    SPI1CON2bits.TXR = 0;

    SPI1TCNTH = 0; // Bits 13-11 of the transfer bit count
    SPI1TCNTL = 0; // Bits 10-3 of the transfer bit count
    SPI1TWIDTHbits.TWIDTH = 0; // Bits 2-0 of the transfer bit count

    SPI1CLK = 0x00; // SPI Clock Source Selection: FOSC (System Clock)

    // Reset errors
    SPI1STATUSbits.RXRE = 0;
    SPI1STATUSbits.TXWE = 0;
}

void spi_enable_fast(void)
{
    if(!SPI1CON0bits.EN)
    {
        SPI1BAUD = 0x03; //  Baud Clock Prescaler Select: 64MHz / 2 * (1 + 3) = 8MHz
        SPI1CON0bits.EN = 1; // SPI Enable: SPI is enabled
    }
}

void spi_enable_slow(void)
{
    if(!SPI1CON0bits.EN)
    {
        SPI1BAUD = 0x4f; //  Baud Clock Prescaler Select: 64MHz / 2 * (1 + 79) = 400KHz
        SPI1CON0bits.EN = 1; // SPI Enable: SPI is enabled
    }
}

void spi_disable(void)
{
    SPI1CON0bits.EN = 0;
}

void spi_write_byte(uint8_t data)
{
    spi_write_block(&data, 1);
}

uint8_t spi_read_byte(void)
{
    uint8_t data;

    spi_read_block(&data, 1);
    
    return data;
}

void spi_read_block(void *buffer, size_t size)
{
    // Receive Only Mode
    SPI1CON2bits.RXR = 1;

    ASSERT(SPI1TCNT == 0);

    SPI1TCNTH = (uint8_t)(size >> 8);
    SPI1TCNTL = (uint8_t)(size);

    // Add padding to TX FIFO to transmit while receiving
    SPI1TXB = 0xFF;

    ASSERT(SPI1STATUSbits.RXRE == 0 && SPI1STATUSbits.TXWE == 0);

    uint8_t *ptr = buffer;
    for (size_t i = 0; i < size; i++, ptr++) {

        // Wait for data in RX FIFO
        while(!PIR3bits.SPI1RXIF);

        *ptr = SPI1RXB;

        ASSERT(SPI1STATUSbits.RXRE == 0 && SPI1STATUSbits.TXWE == 0);
    }

    ASSERT(SPI1TCNT == 0);

    // Clear padding from TX FIFO
    SPI1STATUSbits.CLRBF = 1;

    // Transfer Off Mode
    SPI1CON2bits.RXR = 0;
}

void spi_write_block(void *buffer, size_t size)
{
    // Transmit Only Mode
    SPI1CON2bits.TXR = 1;

    ASSERT(SPI1TCNT == 0);

    SPI1TCNTH = (uint8_t)(size >> 8);
    SPI1TCNTL = (uint8_t)(size);

    ASSERT(SPI1STATUSbits.RXRE == 0 && SPI1STATUSbits.TXWE == 0);

    uint8_t *ptr = buffer;
    uint8_t dummy;
    for (size_t i = 0; i < size; i++, ptr++) {

        // Wait for space in TX FIFO
        while(!PIR3bits.SPI1TXIF);

        SPI1TXB = *ptr;

        ASSERT(SPI1STATUSbits.RXRE == 0 && SPI1STATUSbits.TXWE == 0);
    }

    // Flush TX FIFO
    while (!SPI1STATUSbits.TXBE);

    ASSERT(SPI1TCNT == 0);

    // Transfer Off Mode
    SPI1CON2bits.TXR = 0;
}

