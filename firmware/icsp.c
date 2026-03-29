#include "icsp.h"

#include <xc.h>

void init_icsp(void) {
    // ICSP_CLK -> RB6
    LATBbits.LATB6 = 0;
    TRISBbits.TRISB6 = 1;
    ANSELBbits.ANSELB6 = 1;

    // ICSP_DAT -> RB7
    LATBbits.LATB7 = 0;
    TRISBbits.TRISB7 = 1;
    ANSELBbits.ANSELB7 = 1;
}
