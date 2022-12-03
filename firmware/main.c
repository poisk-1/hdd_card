/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC18F47Q10
        Driver Version    :  2.00
 */

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
 */

#include <string.h>

#include "mcc_generated_files/mcc.h"

#define DATA_BUFFER_SIZE 0x200

static uint8_t ctrl_buffer[0x80];
static uint8_t data_buffer[DATA_BUFFER_SIZE];

void handle_read(void) {
    size_t address = (((size_t) ADDR_8_GetValue()) << 8) | (size_t) PORTD;
    uint8_t data = SEL_IO_DATA_GetValue() ? ctrl_buffer[address] : data_buffer[address];

    TRISA = 0x00;
    PORTA = data;

    ACK_IO_SetLow();
    ACK_IO_SetHigh();

    TRISA = 0xff;
}

void handle_write(void) {

    size_t address = (((size_t) ADDR_8_GetValue()) << 8) | (size_t) PORTD;
    uint8_t data = PORTA;

    if (SEL_IO_DATA_GetValue())
        ctrl_buffer[address] = data;
    else
        data_buffer[address] = data;

    ACK_IO_SetLow();
    ACK_IO_SetHigh();
}

enum Request {
    REQUEST_DONE = 0,
    REQUEST_RESET = 0x1,
    REQUEST_READ = 0x2,
    REQUEST_WRITE = 0x3,
};

enum Status {
    STATUS_NO_ERROR = 0,
    STATUS_BAD_COMMAND = 0x1,
    STATUS_BAD_SECTOR = 0x2,
    STATUS_RESET_FAILED = 0x5
};

struct Ctrl {
    uint8_t request;
    uint8_t status;

    uint8_t cylinder_number;
    uint8_t sector_number;
    uint8_t head_number;
    uint8_t drive_number;
};

#define NUMBER_OF_HEADS 2
#define NUMBER_OF_SECTORS 18

uint32_t chs_to_lba_sector_number(struct Ctrl* ctrl) {
    return ((uint32_t) ctrl->cylinder_number * NUMBER_OF_HEADS + (uint32_t) ctrl->head_number) * NUMBER_OF_SECTORS + ((uint32_t) ctrl->sector_number - 1);
}

void putch(char c) {
    EUSART1_Write(c);
}

static FATFS fs;
static FIL floppy_file;

/*
                         Main application
 */
void main(void) {
    // Initialize the device
    SYSTEM_Initialize();
    INT0_SetInterruptHandler(handle_read);
    INT1_SetInterruptHandler(handle_write);

    memset(ctrl_buffer, 0x00, 0x80);
    memset(data_buffer, 0x00, DATA_BUFFER_SIZE);

    struct Ctrl *ctrl = (struct Ctrl *) ctrl_buffer;

    // If using interrupts in PIC18 High/Low Priority Mode you need to enable the Global High and Low Interrupts
    // If using interrupts in PIC Mid-Range Compatibility Mode you need to enable the Global and Peripheral Interrupts
    // Use the following macros to:

    // Enable the Global Interrupts
    INTERRUPT_GlobalInterruptEnable();

    // Disable the Global Interrupts
    //INTERRUPT_GlobalInterruptDisable();

    // Enable the Peripheral Interrupts
    INTERRUPT_PeripheralInterruptEnable();

    // Disable the Peripheral Interrupts
    //INTERRUPT_PeripheralInterruptDisable();

    while (1) {
        f_mount(0, "0:", 0);

        if (SD_SPI_IsMediaPresent() && f_mount(&fs, "0:", 1) == FR_OK) {
            while (SD_SPI_IsMediaPresent()) {
                switch (ctrl->request) {
                    case REQUEST_RESET:
                        LED_SetLow();

                        printf("RESET\r\n");

                        ctrl->status = (
                                f_open(&floppy_file, "FLOPPY.IMG", FA_READ) == FR_OK &&
                                f_close(&floppy_file) == FR_OK) ? 0 : STATUS_RESET_FAILED;

                        ctrl->request = REQUEST_DONE;

                        LED_SetHigh();

                        break;
                    case REQUEST_READ:
                        LED_SetLow();

                        printf(
                                "READ[d=%d,c=%d,h=%d,s=%d]\r\n",
                                ctrl->drive_number,
                                ctrl->cylinder_number,
                                ctrl->head_number,
                                ctrl->sector_number
                                );

                        UINT read_bytes = 0;
                        ctrl->status = (
                                f_open(&floppy_file, "FLOPPY.IMG", FA_READ) == FR_OK &&
                                f_lseek(&floppy_file, (FSIZE_t) DATA_BUFFER_SIZE * chs_to_lba_sector_number(ctrl)) == FR_OK &&
                                f_read(&floppy_file, data_buffer, DATA_BUFFER_SIZE, &read_bytes) == FR_OK &&
                                f_close(&floppy_file) == FR_OK &&
                                read_bytes == DATA_BUFFER_SIZE) ? 0 : STATUS_BAD_SECTOR;

                        ctrl->sector_number++;
                        ctrl->request = REQUEST_DONE;

                        LED_SetHigh();
                        break;

                    case REQUEST_WRITE:
                        LED_SetLow();

                        printf(
                                "WRITE[d=%d,c=%d,h=%d,s=%d]\r\n",
                                ctrl->drive_number,
                                ctrl->cylinder_number,
                                ctrl->head_number,
                                ctrl->sector_number
                                );

                        UINT written_bytes = 0;
                        ctrl->status = (
                                f_open(&floppy_file, "FLOPPY.IMG", FA_WRITE) == FR_OK &&
                                f_lseek(&floppy_file, (FSIZE_t) DATA_BUFFER_SIZE * chs_to_lba_sector_number(ctrl)) == FR_OK &&
                                f_write(&floppy_file, data_buffer, DATA_BUFFER_SIZE, &written_bytes) == FR_OK &&
                                f_close(&floppy_file) == FR_OK &&
                                written_bytes == DATA_BUFFER_SIZE) ? 0 : STATUS_BAD_SECTOR;

                        ctrl->sector_number++;
                        ctrl->request = REQUEST_DONE;

                        LED_SetHigh();
                        break;
                }
            }
        } else {
            if (ctrl->request) {
                ctrl->status = STATUS_RESET_FAILED;
                ctrl->request = REQUEST_DONE;
            }

        }
    }
}
/**
 End of File
 */