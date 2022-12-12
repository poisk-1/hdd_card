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

#define CTRL_BUFFER_SIZE 0x100
#define DATA_BUFFER_SIZE 0x200

static uint8_t ctrl_buffer[CTRL_BUFFER_SIZE];
static uint8_t data_buffer[DATA_BUFFER_SIZE];

void handle_read(void) {
    size_t sel = (size_t) (PORTB & 0xf);
    size_t address = (size_t) PORTD;
    uint8_t data = 0;

    switch (sel) {
        case 0:
        case 1:
            data = data_buffer[address | ((size_t) sel << 8)];
            break;
        case 2:
            data = ctrl_buffer[address];
            break;
    }

    TRISA = 0x00;
    PORTA = data;

    ACK_IO_SetLow();
    ACK_IO_SetHigh();

    TRISA = 0xff;
}

void handle_write(void) {
    size_t sel = (size_t) (PORTB & 0xf);
    size_t address = (size_t) PORTD;
    uint8_t data = PORTA;

    switch (sel) {
        case 0:
        case 1:
            data_buffer[address | ((size_t) sel << 8)] = data;
            break;
        case 2:
            ctrl_buffer[address] = data;
            break;
    }

    ACK_IO_SetLow();
    ACK_IO_SetHigh();
}

enum Request {
    REQUEST_DONE = 0,
    REQUEST_CHECK = 0x1,
    REQUEST_SCAN = 0x2,
    REQUEST_RESET = 0x3,
    REQUEST_READ = 0x4,
    REQUEST_WRITE = 0x5,
    REQUEST_READ_PARAMS_FUN8H = 0x6,
    REQUEST_READ_PARAMS_FUN15H = 0x7,
};

enum Status {
    STATUS_NO_ERROR = 0,
    STATUS_BAD_COMMAND = 0x1,
    STATUS_BAD_SECTOR = 0x2,
    STATUS_RESET_FAILED = 0x5
};

struct DriveReq {
    uint8_t status;

    uint8_t cylinder_number;
    uint8_t sector_number;
    uint8_t head_number;
    uint8_t drive_number;
};

struct ScanReq {
    uint8_t number_of_floppy_drives;
    uint8_t number_of_hard_drives;
};

struct ReadParamsFun8hReq {
    uint8_t drive_number;

    uint8_t success;

    uint8_t drive_type;
    uint8_t max_cylinder_number;
    uint8_t max_sector_number;
    uint8_t max_head_number;
    uint8_t number_of_drives;
};

struct ReadParamsFun15hReq {
    uint8_t drive_number;

    uint8_t success;

    uint8_t drive_type;
};

union Req {
    struct DriveReq drive_req;
    struct ScanReq scan_req;
    struct ReadParamsFun8hReq read_params_fun8h_req;
    struct ReadParamsFun15hReq read_params_fun15h_req;
};

struct Ctrl {
    uint8_t request;
    union Req req;
};

enum DriveTypeFun8h {
    FUN8H_DRIVE_TYPE_FLOPPY_360 = 1,
    FUN8H_DRIVE_TYPE_FLOPPY_1200 = 2,
    FUN8H_DRIVE_TYPE_FLOPPY_720 = 3,
    FUN8H_DRIVE_TYPE_FLOPPY_1440 = 4
};

enum DriveTypeFun15h {
    FUN15H_DRIVE_TYPE_FLOPPY_DISK = 2,
    FUN15H_DRIVE_TYPE_HARD_DISK = 3
};

struct Drive {
    const char* file_name;

    uint8_t drive_type_fun8h;
    uint8_t drive_type_fun15h;

    size_t number_of_heads;
    size_t number_of_sectors;
    size_t number_of_cylinders;
};

#define MAX_NUMBER_FLOPPY_DRIVES 2
#define MAX_NUMBER_HARD_DRIVES 2

const char* floppy_file_names[MAX_NUMBER_FLOPPY_DRIVES] = {"FLOPPY0.IMG", "FLOPPY1.IMG"};
const char* hard_file_names[MAX_NUMBER_HARD_DRIVES] = {"HARD0.IMG", "HARD1.IMG"};

struct Drive floppy_drives[MAX_NUMBER_FLOPPY_DRIVES];
struct Drive hard_drives[MAX_NUMBER_HARD_DRIVES];

#define FLOPPY_1440_SECTOR_SIZE_BYTES 512
#define FLOPPY_1440_NUMBER_OF_HEADS 2
#define FLOPPY_1440_NUMBER_OF_SECTORS 18
#define FLOPPY_1440_NUMBER_OF_CYLINDERS 80
#define FLOPPY_1440_SIZE_BYTES ((FSIZE_t)FLOPPY_1440_NUMBER_OF_HEADS * FLOPPY_1440_NUMBER_OF_SECTORS * FLOPPY_1440_NUMBER_OF_CYLINDERS * FLOPPY_1440_SECTOR_SIZE_BYTES)

bool is_hard_drive(uint8_t drive_number) {
    return drive_number & 0x80;
}

struct Drive* find_drive(uint8_t drive_number) {
    struct Drive* drive = NULL;
    size_t i = drive_number & 0xf;

    if (is_hard_drive(drive_number) && i < MAX_NUMBER_HARD_DRIVES) {
        drive = &hard_drives[i];
    } else if (i < MAX_NUMBER_FLOPPY_DRIVES) {
        drive = &floppy_drives[i];
    }

    if (drive && drive->file_name) {
        return drive;
    }

    return NULL;
}

uint32_t chs_to_lba_sector_number(struct Drive* drive, struct DriveReq* disk_req) {
    return ((uint32_t) disk_req->cylinder_number * drive->number_of_heads +
            (uint32_t) disk_req->head_number) * drive->number_of_sectors +
            ((uint32_t) disk_req->sector_number - 1);
}

void putch(char c) {
    EUSART1_Write(c);
}

static FATFS fs;
static FIL floppy_file;
static FILINFO file_info;

/*
                         Main application
 */
void main(void) {
    // Initialize the device
    SYSTEM_Initialize();
    INT0_SetInterruptHandler(handle_read);
    INT1_SetInterruptHandler(handle_write);

    memset(ctrl_buffer, 0, CTRL_BUFFER_SIZE);
    memset(data_buffer, 0, DATA_BUFFER_SIZE);

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
        if (SD_SPI_IsMediaPresent() && f_mount(&fs, "0:", 1) == FR_OK) {
            memset(&floppy_drives, 0, sizeof (struct Drive) * MAX_NUMBER_FLOPPY_DRIVES);
            memset(&hard_drives, 0, sizeof (struct Drive) * MAX_NUMBER_HARD_DRIVES);

            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                if (f_stat(floppy_file_names[i], &file_info) == FR_OK) {
                    switch (file_info.fsize) {
                        case FLOPPY_1440_SIZE_BYTES:
                            floppy_drives[i].file_name = floppy_file_names[i];

                            floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1440;
                            floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                            floppy_drives[i].number_of_heads = FLOPPY_1440_NUMBER_OF_HEADS;
                            floppy_drives[i].number_of_sectors = FLOPPY_1440_NUMBER_OF_SECTORS;
                            floppy_drives[i].number_of_cylinders = FLOPPY_1440_NUMBER_OF_CYLINDERS;
                    }
                }
            }

            printf("MOUNTED\r\n");

            while (SD_SPI_IsMediaPresent()) {
                if (ctrl->request != REQUEST_DONE) {
                    LED_SetLow();

                    struct Drive* drive = NULL;

                    switch (ctrl->request) {
                        case REQUEST_CHECK:
                            printf("CHECK\r\n");

                            for (size_t i = 0; i < DATA_BUFFER_SIZE; i++) {
                                data_buffer[i] = ~data_buffer[i];
                            }

                            break;

                        case REQUEST_SCAN:
                            printf("SCAN\r\n");

                            memset(&ctrl->req.scan_req, 0, sizeof (struct ScanReq));

                            ctrl->req.scan_req.number_of_floppy_drives = 0;

                            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                                if (floppy_drives[i].file_name) {
                                    ctrl->req.scan_req.number_of_floppy_drives++;
                                }
                            }

                            ctrl->req.scan_req.number_of_hard_drives = 0;

                            for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                                if (hard_drives[i].file_name) {
                                    ctrl->req.scan_req.number_of_hard_drives++;
                                }
                            }

                            break;

                        case REQUEST_RESET:
                            printf("RESET[d=%d]\r\n", ctrl->req.drive_req.drive_number);

                            drive = find_drive(ctrl->req.drive_req.drive_number);

                            if (drive) {
                                ctrl->req.drive_req.status = (
                                        f_open(&floppy_file, drive->file_name, FA_READ) == FR_OK &&
                                        f_close(&floppy_file) == FR_OK) ? 0 : STATUS_RESET_FAILED;
                            } else {
                                ctrl->req.drive_req.status = STATUS_RESET_FAILED;
                            }

                            break;

                        case REQUEST_READ:
                            printf(
                                    "READ[d=%d,c=%d,h=%d,s=%d]\r\n",
                                    ctrl->req.drive_req.drive_number,
                                    ctrl->req.drive_req.cylinder_number,
                                    ctrl->req.drive_req.head_number,
                                    ctrl->req.drive_req.sector_number
                                    );

                            drive = find_drive(ctrl->req.drive_req.drive_number);

                            if (drive) {
                                UINT read_bytes = 0;
                                ctrl->req.drive_req.status = (
                                        f_open(&floppy_file, drive->file_name, FA_READ) == FR_OK &&
                                        f_lseek(&floppy_file, (FSIZE_t) DATA_BUFFER_SIZE * chs_to_lba_sector_number(drive, &ctrl->req.drive_req)) == FR_OK &&
                                        f_read(&floppy_file, data_buffer, DATA_BUFFER_SIZE, &read_bytes) == FR_OK &&
                                        f_close(&floppy_file) == FR_OK &&
                                        read_bytes == DATA_BUFFER_SIZE) ? 0 : STATUS_BAD_SECTOR;

                                ctrl->req.drive_req.sector_number++;
                            } else {
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }

                            break;

                        case REQUEST_WRITE:
                            printf(
                                    "WRITE[d=%d,c=%d,h=%d,s=%d]\r\n",
                                    ctrl->req.drive_req.drive_number,
                                    ctrl->req.drive_req.cylinder_number,
                                    ctrl->req.drive_req.head_number,
                                    ctrl->req.drive_req.sector_number
                                    );

                            drive = find_drive(ctrl->req.drive_req.drive_number);

                            if (drive) {
                                UINT written_bytes = 0;
                                ctrl->req.drive_req.status = (
                                        f_open(&floppy_file, drive->file_name, FA_WRITE) == FR_OK &&
                                        f_lseek(&floppy_file, (FSIZE_t) DATA_BUFFER_SIZE * chs_to_lba_sector_number(drive, &ctrl->req.drive_req)) == FR_OK &&
                                        f_write(&floppy_file, data_buffer, DATA_BUFFER_SIZE, &written_bytes) == FR_OK &&
                                        f_close(&floppy_file) == FR_OK &&
                                        written_bytes == DATA_BUFFER_SIZE) ? 0 : STATUS_BAD_SECTOR;

                                ctrl->req.drive_req.sector_number++;
                            } else {
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }

                            break;

                        case REQUEST_READ_PARAMS_FUN8H:
                            printf(
                                    "REQUEST_READ_PARAMS_FUN8H[d=%d]\r\n",
                                    ctrl->req.read_params_fun8h_req.drive_number
                                    );

                            ctrl->req.read_params_fun8h_req.number_of_drives = 0;

                            if (is_hard_drive(ctrl->req.read_params_fun8h_req.drive_number)) {
                                for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                                    if (hard_drives[i].file_name) {
                                        ctrl->req.read_params_fun8h_req.number_of_drives++;
                                    }
                                }
                            } else {
                                for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                                    if (floppy_drives[i].file_name) {
                                        ctrl->req.read_params_fun8h_req.number_of_drives++;
                                    }
                                }
                            }

                            drive = find_drive(ctrl->req.read_params_fun8h_req.drive_number);

                            if (drive) {
                                ctrl->req.read_params_fun8h_req.success = 1;
                                ctrl->req.read_params_fun8h_req.drive_type = drive->drive_type_fun8h;
                                ctrl->req.read_params_fun8h_req.max_cylinder_number = (uint8_t) drive->number_of_cylinders - 1;
                                ctrl->req.read_params_fun8h_req.max_head_number = (uint8_t) drive->number_of_heads - 1;
                                ctrl->req.read_params_fun8h_req.max_sector_number = (uint8_t) drive->number_of_sectors;
                            } else {
                                ctrl->req.read_params_fun8h_req.success = 0;
                                ctrl->req.read_params_fun8h_req.drive_type = 0;
                                ctrl->req.read_params_fun8h_req.max_cylinder_number = 0;
                                ctrl->req.read_params_fun8h_req.max_head_number = 0;
                                ctrl->req.read_params_fun8h_req.max_sector_number = 0;
                            }

                            break;

                        case REQUEST_READ_PARAMS_FUN15H:
                            printf(
                                    "REQUEST_READ_PARAMS_FUN15H[d=%d]\r\n",
                                    ctrl->req.read_params_fun15h_req.drive_number
                                    );

                            drive = find_drive(ctrl->req.read_params_fun15h_req.drive_number);

                            if (drive) {
                                ctrl->req.read_params_fun15h_req.success = 1;
                                ctrl->req.read_params_fun15h_req.drive_type = drive->drive_type_fun15h;
                            } else {
                                ctrl->req.read_params_fun15h_req.success = 0;
                                ctrl->req.read_params_fun15h_req.drive_type = 0;
                            }

                            break;

                        default:
                            printf("UNKNOWN REQUEST %d\r\n", ctrl->request);

                            break;
                    }

                    ctrl->request = REQUEST_DONE;
                    LED_SetHigh();
                }
            }

            f_mount(0, "0:", 0);

            printf("UNMOUNTED\r\n");
        } else {
            if (ctrl->request) {
                ctrl->req.drive_req.status = STATUS_RESET_FAILED;
                ctrl->request = REQUEST_DONE;
            }

        }
    }
}
/**
 End of File
 */