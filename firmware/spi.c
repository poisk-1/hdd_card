#include <xc.h>

void spi_init(void)
{
    SPI1CON0bits.BMODE = 0; // Bit-Length Mode Select: SPIxTWIDTH setting applies only to the last byte exchanged; total bits sent is SPIxTWIDTH + (SPIxTCNT*8)
    SPI1CON0bits.MST = 1; // SPI Host Operating Mode Select: SPI module operates as the bus host
    SPI1CON0bits.LSBF = 0; // LSb-First Data Exchange Select: Data are exchanged MSb first
    SPI1CON0bits.EN = 0; // SPI Enable: SPI is disabled

    SPI1CON1bits.SDOP = 0; // SPI Output Polarity Control: SDO output is active-high
    SPI1CON1bits.SDIP = 0; // SPI Input Polarity Control: SDI input is active-high
    SPI1CON1bits.SSP = 0; // Client Select Input/Output Polarity Control: SS is active-high
    SPI1CON1bits.FST = 0; // Fast Start Enable: Delay to first SCK will be at least ½ baud period
    SPI1CON1bits.CKP = 1; // Clock Polarity Select: Idle state for SCK is high level
    SPI1CON1bits.CKE = 0; // Clock Edge Select: Output data changes on transition from Idle to Active clock state
    SPI1CON1bits.SMP = 0; // SPI Input Sample Phase Control: SDI input is sampled in the middle of data output time

    SPI1CON2bits.RXR = 1; // Receive FIFO Space-Required Control: Data transfers are suspended when RxFIFO is full
    SPI1CON2bits.TXR = 1; // Transmit Data-Required Control: TxFIFO data are required for a transfer
    SPI1CON2bits.SSET = 0; // Client Select Enable: SS_out is driven to the Active state while the transmit counter is not zero
    SPI1CON2bits.SSFLT = 0; // SS_in Fault Status: SS_in ended normally
    SPI1CON2bits.BUSY = 0; // SPI Module Busy Status: Data exchange is not taking place

    SPI1CLK = 0x00; // SPI Clock Source Selection: FOSC (System Clock)

    RC0PPS = 0x31;   //RC0->SPI1:SCK1;    
    SPI1SCKPPS = 0x10;   //RC0->SPI1:SCK1;    
    RC2PPS = 0x32;   //RC2->SPI1:SDO1;    
    SPI1SDIPPS = 0x11;   //RC1->SPI1:SDI1;    

    SLRCONCbits.SLRC0 = 0; // SCK: PORT pin slews at maximum rate
    SLRCONCbits.SLRC1 = 0; // SDI: PORT pin slews at maximum rate
    SLRCONCbits.SLRC2 = 0; // SDO: PORT pin slews at maximum rate
}

void spi_enable_fast(void)
{
    if(!SPI1CON0bits.EN)
    {
        SPI1BAUD = 0x03; //  Baud Clock Prescaler Select: 64MHz / 2 * (1 + 3) = 8MHz

        TRISCbits.TRISC0 = 0; // SCK        

        SPI1CON0bits.EN = 1; // SPI Enable: SPI is enabled
    }
}

void spi_enable_slow(void)
{
    if(!SPI1CON0bits.EN)
    {
        SPI1BAUD = 0x4f; //  Baud Clock Prescaler Select: 64MHz / 2 * (1 + 79) = 400KHz

        TRISCbits.TRISC0 = 0; // SCK

        SPI1CON0bits.EN = 1; // SPI Enable: SPI is enabled
    }
}

void spi_disable(void)
{
    SPI1CON0bits.EN = 0;
}

uint8_t spi_exchange_byte(uint8_t data)
{
    SPI1TCNTL = 1;
    SPI1TXB = data;
    while(!PIR3bits.SPI1RXIF);
    data = SPI1RXB; // This avoids unused return optimization removing SPI1RXB read
    return data;
}

void spi_read_block(void *buffer, size_t size)
{
    uint8_t *ptr = buffer;
    for (size_t i = 0; i < size; i++, ptr++) {
        SPI1TCNTL = 1;
        SPI1TXB = 0xFF;
        while(!PIR3bits.SPI1RXIF);
        *ptr = SPI1RXB;
    }
}

void spi_write_block(void *buffer, size_t size)
{
    uint8_t *ptr = buffer;
    uint8_t dummy;
    for (size_t i = 0; i < size; i++, ptr++) {
        SPI1TCNTL = 1;
        SPI1TXB = *ptr;
        while(!PIR3bits.SPI1RXIF);
        dummy = SPI1RXB;
    }
}
