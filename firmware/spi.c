#include <xc.h>

void init_spi(void)
{
    RC0PPS = 0x31;   //RC0->SPI1:SCK1;    
    SPI1SCKPPS = 0x10;   //RC0->SPI1:SCK1;    
    RC2PPS = 0x32;   //RC2->SPI1:SDO1;    
    SPI1SDIPPS = 0x11;   //RC1->SPI1:SDI1;    

    //EN disabled; LSBF MSb first; MST bus slave; BMODE last byte; 
    SPI1CON0 = 0x02;
    //SMP Middle; CKE Active to idle; CKP Idle:Low, Active:High; FST disabled; SSP active high; SDIP active high; SDOP active high; 
    SPI1CON1 = 0x40;
    //SSET disabled; TXR not required for a transfer; RXR data is not stored in the FIFO; 
    SPI1CON2 = 0x00;
    //CLKSEL FOSC; 
    SPI1CLK = 0x00;
    //BAUD 0; 
    SPI1BAUD = 0x00;
    TRISCbits.TRISC0 = 0;
}


void enable_fast_spi(void)
{
    if(!SPI1CON0bits.EN)
    {
        //EN disabled; LSBF MSb first; MST bus slave; BMODE last byte; 
        SPI1CON0 = 0x02;
        //SMP Middle; CKE Active to idle; CKP Idle:Low, Active:High; FST disabled; SSP active high; SDIP active high; SDOP active high; 
        SPI1CON1 = 0x20;
        //SSET disabled; TXR not required for a transfer; RXR data is not stored in the FIFO; 
        SPI1CON2 = 0x00 | (_SPI1CON2_SPI1RXR_MASK | _SPI1CON2_SPI1TXR_MASK);
        //CLKSEL FOSC; 
        //SPI1CLK = 0x00;
        //BAUD 0; 
        SPI1BAUD = 0x03;
        TRISCbits.TRISC0 = 0;
        SPI1CON0bits.EN = 1;
    }
}

void enable_slow_spi(void)
{
    if(!SPI1CON0bits.EN)
    {
        //EN disabled; LSBF MSb first; MST bus slave; BMODE last byte; 
        SPI1CON0 = 0x02;
        //SMP Middle; CKE Active to idle; CKP Idle:Low, Active:High; FST disabled; SSP active high; SDIP active high; SDOP active high; 
        SPI1CON1 = 0x20;
        //SSET disabled; TXR not required for a transfer; RXR data is not stored in the FIFO; 
        SPI1CON2 = 0x00 | (_SPI1CON2_SPI1RXR_MASK | _SPI1CON2_SPI1TXR_MASK);
        //CLKSEL FOSC; 
        //SPI1CLK = 0x00;
        //BAUD 0; 
        SPI1BAUD = 0x4f;
        TRISCbits.TRISC0 = 0;
        SPI1CON0bits.EN = 1;
    }
}

void disable_spi(void)
{
    SPI1CON0bits.EN = 0;
}

uint8_t exchange_byte(uint8_t data)
{
    SPI1TCNTL = 1;
    SPI1TXB = data;
    while(!PIR3bits.SPI1RXIF);
    return SPI1RXB;
}

void exchange_block(void *block, size_t blockSize)
{
    uint8_t *data = block;
    while(blockSize--)
    {
        SPI1TCNTL = 1;
        SPI1TXB = *data;
        while(!PIR3bits.SPI1RXIF);
        *data++ = SPI1RXB;
    }
}
