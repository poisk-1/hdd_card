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

static FATFS fs;

#define CTRL_BUFFER_SIZE 0x100
#define DATA_BUFFER_SIZE 0x200

static uint8_t ctrl_buffer[CTRL_BUFFER_SIZE];
#if (FF_MAX_SS == DATA_BUFFER_SIZE)
static uint8_t *data_buffer = fs.win;
#else
static uint8_t data_buffer[DATA_BUFFER_SIZE];
#endif

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
    CTRL_REQUEST_DONE = 0,
    CTRL_REQUEST_CHECK = 0x1,
    CTRL_REQUEST_SCAN = 0x2,
    CTRL_REQUEST_RESET = 0x3,
    CTRL_REQUEST_READ = 0x4,
    CTRL_REQUEST_WRITE = 0x5,
    CTRL_REQUEST_VERIFY = 0x6,
    CTRL_REQUEST_READ_PARAMS_FUN8H = 0x7,
    CTRL_REQUEST_READ_PARAMS_FUN15H = 0x8,
    CTRL_REQUEST_DETECT_MEDIA_CHANGE = 0x9,
};

enum DriveReqStatus {
    STATUS_NO_ERROR = 0,
    STATUS_BAD_SECTOR = 0x2,
    STATUS_WRITE_PROTECTED = 0x3,
    STATUS_DISK_CHANGED = 0x6,
    STATUS_CONTROLLER_FAILED = 0x20
};

void invert_data_buffer() {
    for (size_t i = 0; i < DATA_BUFFER_SIZE; i++) {
        data_buffer[i] = ~data_buffer[i];
    }
}

struct DriveReq {
    uint8_t status;

    uint8_t low_cylinder_number;
    uint8_t sector_and_high_cylinder_numbers;
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
    uint8_t max_low_cylinder_number;
    uint8_t max_sector_and_high_cylinder_numbers;
    uint8_t max_head_number;
    uint8_t number_of_drives;
};

struct ReadParamsFun15hReq {
    uint8_t drive_number;

    uint8_t success;

    uint8_t drive_type;
};

struct DetectMediaChangeReq {
    uint8_t drive_number;

    uint8_t status;
};

union Req {
    struct DriveReq drive_req;
    struct ScanReq scan_req;
    struct ReadParamsFun8hReq read_params_fun8h_req;
    struct ReadParamsFun15hReq read_params_fun15h_req;
    struct DetectMediaChangeReq detect_media_change;
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
    FIL image_file;

    bool read_only;
    bool media_changed;

    uint8_t drive_type_fun8h;
    uint8_t drive_type_fun15h;

    size_t number_of_heads;
    size_t number_of_sectors;
    size_t number_of_cylinders;
};

#define MAX_NUMBER_FLOPPY_DRIVES 4
#define MAX_NUMBER_HARD_DRIVES 2

const char* floppy_image_file_names[MAX_NUMBER_FLOPPY_DRIVES] = {"FLOPPY0.IMG", "FLOPPY1.IMG", "FLOPPY2.IMG", "FLOPPY3.IMG"};
const char* floppy_info_file_names[MAX_NUMBER_FLOPPY_DRIVES] = {"FLOPPY0.TXT", "FLOPPY1.TXT", "FLOPPY2.TXT", "FLOPPY3.TXT"};
const char* floppy_ro_file_names[MAX_NUMBER_FLOPPY_DRIVES] = {"FLOPPY0.RO", "FLOPPY1.RO", "FLOPPY2.RO", "FLOPPY3.RO"};
const char* hard_image_file_names[MAX_NUMBER_HARD_DRIVES] = {"HARD0.IMG", "HARD1.IMG"};

struct Drive floppy_drives[MAX_NUMBER_FLOPPY_DRIVES];
struct Drive hard_drives[MAX_NUMBER_HARD_DRIVES];

#define FLOPPY_IMAGE_FILE_CLTBL_SIZE 8
#define HARD_IMAGE_FILE_CLTBL_SIZE 64

DWORD floppy_image_file_cltbl[MAX_NUMBER_FLOPPY_DRIVES][FLOPPY_IMAGE_FILE_CLTBL_SIZE];
DWORD hard_image_file_cltbl[MAX_NUMBER_HARD_DRIVES][HARD_IMAGE_FILE_CLTBL_SIZE];

#define FLOPPY_SECTOR_SIZE_BYTES 512

#define FLOPPY_1440_NUMBER_OF_HEADS 2
#define FLOPPY_1440_NUMBER_OF_SECTORS 18
#define FLOPPY_1440_NUMBER_OF_CYLINDERS 80

#define FLOPPY_720_SECTOR_SIZE_BYTES 512
#define FLOPPY_720_NUMBER_OF_HEADS 2
#define FLOPPY_720_NUMBER_OF_SECTORS 18
#define FLOPPY_720_NUMBER_OF_CYLINDERS 40

#define FLOPPY_1200_SECTOR_SIZE_BYTES 512
#define FLOPPY_1200_NUMBER_OF_HEADS 2
#define FLOPPY_1200_NUMBER_OF_SECTORS 15
#define FLOPPY_1200_NUMBER_OF_CYLINDERS 80

#define FLOPPY_360_SECTOR_SIZE_BYTES 512
#define FLOPPY_360_NUMBER_OF_HEADS 2
#define FLOPPY_360_NUMBER_OF_SECTORS 9
#define FLOPPY_360_NUMBER_OF_CYLINDERS 40

#define FLOPPY_180_SECTOR_SIZE_BYTES 512
#define FLOPPY_180_NUMBER_OF_HEADS 1
#define FLOPPY_180_NUMBER_OF_SECTORS 9
#define FLOPPY_180_NUMBER_OF_CYLINDERS 40

#define FLOPPY_320_SECTOR_SIZE_BYTES 512
#define FLOPPY_320_NUMBER_OF_HEADS 2
#define FLOPPY_320_NUMBER_OF_SECTORS 8
#define FLOPPY_320_NUMBER_OF_CYLINDERS 40

#define FLOPPY_160_SECTOR_SIZE_BYTES 512
#define FLOPPY_160_NUMBER_OF_HEADS 1
#define FLOPPY_160_NUMBER_OF_SECTORS 8
#define FLOPPY_160_NUMBER_OF_CYLINDERS 40

#define FLOPPY_INFO_TYPE_LENGTH 4
#define FLOPPY_INFO_TYPE_1440 "1440"
#define FLOPPY_INFO_TYPE_720 "0720"
#define FLOPPY_INFO_TYPE_1200 "1200"
#define FLOPPY_INFO_TYPE_360 "0360"
#define FLOPPY_INFO_TYPE_180 "0180"
#define FLOPPY_INFO_TYPE_320 "0320"
#define FLOPPY_INFO_TYPE_160 "0160"

#define HARD_SECTOR_SIZE_BYTES 512
#define HARD_NUMBER_OF_HEADS 16
#define HARD_NUMBER_OF_SECTORS 63
#define HARD_CYLINDER_SECTORS ((FSIZE_t)HARD_NUMBER_OF_HEADS * (FSIZE_t)HARD_NUMBER_OF_SECTORS)
#define HARD_CYLINDER_SIZE_BYTES ((FSIZE_t)HARD_SECTOR_SIZE_BYTES * HARD_CYLINDER_SECTORS)
#define HARD_MAX_NUMBER_OF_CYLINDERS 1024

bool is_hard_drive(uint8_t drive_number) {
    return drive_number & 0x80;
}

bool has_geometry(const struct Drive* drive) {
    return drive->number_of_cylinders != 0;
}

bool has_image(const struct Drive* drive) {
    return drive->image_file.obj.fs != NULL;
}

uint32_t disk_size_bytes(const struct Drive* drive) {
    return (uint32_t) drive->number_of_cylinders *
            (uint32_t) drive->number_of_heads *
            (uint32_t) drive->number_of_sectors *
            FLOPPY_SECTOR_SIZE_BYTES;
}

struct Drive* find_drive(uint8_t drive_number) {
    struct Drive* drive = NULL;
    size_t i = drive_number & 0xf;

    if (is_hard_drive(drive_number) && i < MAX_NUMBER_HARD_DRIVES) {
        drive = &hard_drives[i];
    } else if (i < MAX_NUMBER_FLOPPY_DRIVES) {
        drive = &floppy_drives[i];
    }

    return drive;
}

#define HIGH_CYLINDER_BITS 2
#define HIGH_CYLINDER_NUMBER_MASK ((uint8_t)0xc0)
#define SECTOR_NUMBER_MASK ((uint8_t)~HIGH_CYLINDER_NUMBER_MASK)

bool chs_to_lba(const struct Drive* drive, const struct DriveReq* disk_req, uint32_t *lba) {
    uint32_t cylinder_number =
            (uint32_t) disk_req->low_cylinder_number |
            (((uint32_t) disk_req->sector_and_high_cylinder_numbers & HIGH_CYLINDER_NUMBER_MASK) << HIGH_CYLINDER_BITS);

    uint32_t sector_number =
            (uint32_t) disk_req->sector_and_high_cylinder_numbers & SECTOR_NUMBER_MASK;

    if (cylinder_number < drive->number_of_cylinders &&
            disk_req->head_number < drive->number_of_heads &&
            sector_number <= drive->number_of_sectors) {
        *lba = ((uint32_t) cylinder_number * drive->number_of_heads +
                (uint32_t) disk_req->head_number) * drive->number_of_sectors +
                ((uint32_t) sector_number - 1);

        return true;
    }

    return false;
}

void next_drive_req(const struct Drive* drive, struct DriveReq* disk_req) {
    uint32_t cylinder_number =
            (uint32_t) disk_req->low_cylinder_number |
            (((uint32_t) disk_req->sector_and_high_cylinder_numbers & HIGH_CYLINDER_NUMBER_MASK) << HIGH_CYLINDER_BITS);

    uint32_t head_number = disk_req->head_number;

    uint32_t sector_number =
            (uint32_t) disk_req->sector_and_high_cylinder_numbers & SECTOR_NUMBER_MASK;

    sector_number++;

    if (sector_number > drive->number_of_sectors) {
        sector_number = 1;

        head_number++;

        if (head_number >= drive->number_of_heads) {
            head_number = 0;
            cylinder_number++;

            if (cylinder_number >= drive->number_of_cylinders) {
                cylinder_number = 0;
            }
        }
    }

    disk_req->low_cylinder_number = (uint8_t) (cylinder_number & 0xff);
    disk_req->head_number = (uint8_t) head_number;
    disk_req->sector_and_high_cylinder_numbers =
            (uint8_t) (sector_number & SECTOR_NUMBER_MASK) |
            (uint8_t) ((cylinder_number >> HIGH_CYLINDER_BITS) & HIGH_CYLINDER_NUMBER_MASK);
}

void set_params_fun8h(const struct Drive* drive, struct ReadParamsFun8hReq* req) {
    req->success = 1;
    req->drive_type = drive->drive_type_fun8h;
    req->max_low_cylinder_number = (uint8_t) ((drive->number_of_cylinders - 1) & 0xff);
    req->max_head_number = (uint8_t) drive->number_of_heads - 1;
    req->max_sector_and_high_cylinder_numbers =
            (uint8_t) (drive->number_of_sectors & SECTOR_NUMBER_MASK) |
            (uint8_t) (((drive->number_of_cylinders - 1) >> HIGH_CYLINDER_BITS) & HIGH_CYLINDER_NUMBER_MASK);
}

void putch(char c) {
    EUSART1_Write(c);
}

#if FF_MAX_SS == FF_MIN_SS
#define SS(fs)	((UINT)FF_MAX_SS)	/* Fixed sector size */
#else
#define SS(fs)	((fs)->ssize)	/* Variable sector size */
#endif

static DWORD clmt_clust(/* <2:Error, >=2:Cluster number */
        FIL* fp, /* Pointer to the file object */
        FSIZE_t ofs /* File offset to be converted to cluster# */
        ) {
    DWORD cl, ncl, *tbl;
    FATFS *fs = fp->obj.fs;


    tbl = fp->cltbl + 1; /* Top of CLMT */
    cl = (DWORD) (ofs / SS(fs) / fs->csize); /* Cluster order from top of the file */
    for (;;) {
        ncl = *tbl++; /* Number of cluters in the fragment */
        if (ncl == 0) return 0; /* End of table? (error) */
        if (cl < ncl) break; /* In this fragment? */
        cl -= ncl;
        tbl++; /* Next fragment */
    }
    return cl + *tbl; /* Return the cluster number */
}

static DWORD clst2sect(/* !=0:Sector number, 0:Failed (invalid cluster#) */
        FATFS* fs, /* Filesystem object */
        DWORD clst /* Cluster# to be converted */
        ) {
    clst -= 2; /* Cluster number is origin from 2 */
    if (clst >= fs->n_fatent - 2) return 0; /* Is it invalid cluster number? */
    return fs->database + fs->csize * clst; /* Start sector number of the cluster */
}

static FRESULT offset2sector(FIL *file, FSIZE_t offset, DWORD *sector) {
    DWORD cluster;
    FATFS *fs = file->obj.fs;

    cluster = clmt_clust(file, offset);
    if (cluster < 2) return FR_INT_ERR;

    *sector = clst2sect(fs, cluster);
    if (*sector == 0) return FR_INT_ERR;

    *sector += (DWORD) (offset / SS(fs)) & (fs->csize - 1);

    return FR_OK;
}

static FRESULT setup_clmt(FIL *fp, DWORD *cltbl, DWORD cltbl_size) {
    cltbl[0] = cltbl_size;
    fp->cltbl = cltbl;

    return FR_OK;
}

/*
                         Main application
 */
void main(void) {
    // Initialize the device
    SYSTEM_Initialize();
    ACK_IO_SetHigh();
    INT0_SetInterruptHandler(handle_read);
    INT1_SetInterruptHandler(handle_write);

    memset(ctrl_buffer, 0, CTRL_BUFFER_SIZE);
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

            static FILINFO file_info;

            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                static FIL info_file;
                static char info[FLOPPY_INFO_TYPE_LENGTH + 1];
                memset(info, 0, FLOPPY_INFO_TYPE_LENGTH + 1);
                UINT read_bytes = 0;

                if (
                        f_open(&info_file, floppy_info_file_names[i], FA_READ) == FR_OK &&
                        f_read(&info_file, info, FLOPPY_INFO_TYPE_LENGTH, &read_bytes) == FR_OK) {
                    if (strncmp(info, FLOPPY_INFO_TYPE_1440, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1440;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_1440_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_1440_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_1440_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_720, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_720;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_720_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_720_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_720_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_1200, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_1200;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_1200_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_1200_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_1200_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_360, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_360_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_360_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_360_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_180, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_180_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_180_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_180_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_320, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_320_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_320_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_320_NUMBER_OF_CYLINDERS;
                    } else if (strncmp(info, FLOPPY_INFO_TYPE_160, FLOPPY_INFO_TYPE_LENGTH) == 0) {
                        floppy_drives[i].drive_type_fun8h = FUN8H_DRIVE_TYPE_FLOPPY_360;
                        floppy_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_FLOPPY_DISK;

                        floppy_drives[i].number_of_heads = FLOPPY_160_NUMBER_OF_HEADS;
                        floppy_drives[i].number_of_sectors = FLOPPY_160_NUMBER_OF_SECTORS;
                        floppy_drives[i].number_of_cylinders = FLOPPY_160_NUMBER_OF_CYLINDERS;
                    }

#ifdef DEBUG
                    if (has_geometry(&floppy_drives[i])) {
                        printf("FOUND FLOPPY%d.TXT[c=%d,h=%d,s=%d]\r\n",
                                i,
                                floppy_drives[i].number_of_cylinders,
                                floppy_drives[i].number_of_heads,
                                floppy_drives[i].number_of_sectors
                                );
                    }
#endif                                                    
                }
            }

            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                if (has_geometry(&floppy_drives[i])) {
                    if (f_stat(floppy_image_file_names[i], &file_info) == FR_OK &&
                            file_info.fsize == disk_size_bytes(&floppy_drives[i])) {
                        if (
                                f_open(&floppy_drives[i].image_file, floppy_image_file_names[i], FA_READ | FA_WRITE) == FR_OK &&
                                setup_clmt(&floppy_drives[i].image_file, floppy_image_file_cltbl[i], FLOPPY_IMAGE_FILE_CLTBL_SIZE) == FR_OK &&
                                f_lseek(&floppy_drives[i].image_file, CREATE_LINKMAP) == FR_OK) {

                            if (f_stat(floppy_ro_file_names[i], &file_info) == FR_OK) {
                                floppy_drives[i].read_only = true;
                            }

                            floppy_drives[i].media_changed = true;

#ifdef DEBUG
                            if (has_image(&floppy_drives[i])) {
                                printf("MOUNTED FLOPPY%d.IMG[ro=%d,f=%ld]\r\n",
                                        i,
                                        floppy_drives[i].read_only,
                                        floppy_image_file_cltbl[i][0]);
                            }
#endif                            
                        } else {
#ifdef DEBUG
                            printf("TOO MANY FRAGMENTS FLOPPY%d.IMG[f=%ld]\r\n",
                                    i,
                                    floppy_image_file_cltbl[i][0]);
#endif                                

                            f_close(&floppy_drives[i].image_file);
                        }
                    } else {
#ifdef DEBUG
                        printf("NOT FOUND OR BAD SIZE FLOPPY%d.IMG\r\n", i);
#endif                                                    
                    }
                }
            }

            for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                if (f_stat(hard_image_file_names[i], &file_info) == FR_OK) {
                    FSIZE_t size = file_info.fsize;

                    if (size > HARD_CYLINDER_SIZE_BYTES) {
                        if (
                                f_open(&hard_drives[i].image_file, hard_image_file_names[i], FA_READ | FA_WRITE) == FR_OK &&
                                setup_clmt(&hard_drives[i].image_file, hard_image_file_cltbl[i], HARD_IMAGE_FILE_CLTBL_SIZE) == FR_OK &&
                                f_lseek(&hard_drives[i].image_file, CREATE_LINKMAP) == FR_OK) {
                            hard_drives[i].drive_type_fun8h = 0;
                            hard_drives[i].drive_type_fun15h = FUN15H_DRIVE_TYPE_HARD_DISK;

                            hard_drives[i].number_of_heads = HARD_NUMBER_OF_HEADS;
                            hard_drives[i].number_of_sectors = HARD_NUMBER_OF_SECTORS;
                            hard_drives[i].number_of_cylinders = size / HARD_CYLINDER_SIZE_BYTES;

                            if (hard_drives[i].number_of_cylinders > HARD_MAX_NUMBER_OF_CYLINDERS)
                                hard_drives[i].number_of_cylinders = HARD_MAX_NUMBER_OF_CYLINDERS;

#ifdef DEBUG
                            printf("MOUNTED HARD%d.IMG[c=%d,h=%d,s=%d,f=%ld]\r\n",
                                    i,
                                    hard_drives[i].number_of_cylinders,
                                    hard_drives[i].number_of_heads,
                                    hard_drives[i].number_of_sectors,
                                    hard_image_file_cltbl[i][0]);
#endif                                
                        } else {
#ifdef DEBUG
                            printf("TOO MANY FRAGMENTS HARD%d.IMG[f=%ld]\r\n",
                                    i,
                                    hard_image_file_cltbl[i][0]);
#endif                                
                            f_close(&hard_drives[i].image_file);
                        }
                    } else {
#ifdef DEBUG
                        printf("BAD SIZE HARD%d.IMG\r\n", i);
#endif                                                    
                    }

                } else {
#ifdef DEBUG
                    printf("NOT FOUND HARD%d.IMG\r\n", i);
#endif                                                    
                }
            }

            while (SD_SPI_IsMediaPresent()) {
                if (ctrl->request != CTRL_REQUEST_DONE) {
                    LED_SetLow();

                    struct Drive* drive = NULL;
                    uint32_t lba = 0;

                    switch (ctrl->request) {
                        case CTRL_REQUEST_CHECK:
#ifdef DEBUG
                            printf("CHECK\r\n");
#endif
                            invert_data_buffer();
                            break;

                        case CTRL_REQUEST_SCAN:
#ifdef DEBUG
                            printf("SCAN\r\n");
#endif
                            ctrl->req.scan_req.number_of_floppy_drives = 0;

                            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                                if (has_geometry(&floppy_drives[i])) {
                                    ctrl->req.scan_req.number_of_floppy_drives++;
                                }
                            }

                            ctrl->req.scan_req.number_of_hard_drives = 0;

                            for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                                if (has_geometry(&hard_drives[i])) {
                                    ctrl->req.scan_req.number_of_hard_drives++;
                                }
                            }

                            break;

                        case CTRL_REQUEST_RESET:
#ifdef DEBUG
                            printf("RESET[d=%d]\r\n", ctrl->req.drive_req.drive_number);
#endif
                            ctrl->req.drive_req.status = 0;

                            break;

                        case CTRL_REQUEST_READ:
                            drive = find_drive(ctrl->req.drive_req.drive_number);

#ifdef DEBUG
                            printf(
                                    "READ[d=%d,lc=%d,h=%d,shc=%d]",
                                    ctrl->req.drive_req.drive_number,
                                    ctrl->req.drive_req.low_cylinder_number,
                                    ctrl->req.drive_req.head_number,
                                    ctrl->req.drive_req.sector_and_high_cylinder_numbers
                                    );
#endif

                            if (drive && has_image(drive) && chs_to_lba(drive, &ctrl->req.drive_req, &lba)) {
#ifdef DEBUG
                                printf("[lba=%lu]\r\n", lba);
#endif
                                DWORD sector = 0;
                                ctrl->req.drive_req.status = (
                                        offset2sector(&drive->image_file, (FSIZE_t) DATA_BUFFER_SIZE * lba, &sector) == FR_OK &&
                                        SD_SPI_SectorRead(sector, data_buffer, 1)) ? 0 : STATUS_BAD_SECTOR;

                                next_drive_req(drive, &ctrl->req.drive_req);
                            } else {
#ifdef DEBUG
                                printf("[bad sector]\r\n");
#endif
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }

                            break;

                        case CTRL_REQUEST_WRITE:
                            drive = find_drive(ctrl->req.drive_req.drive_number);
#ifdef DEBUG
                            printf(
                                    "WRITE[d=%d,lc=%d,h=%d,shc=%d]",
                                    ctrl->req.drive_req.drive_number,
                                    ctrl->req.drive_req.low_cylinder_number,
                                    ctrl->req.drive_req.head_number,
                                    ctrl->req.drive_req.sector_and_high_cylinder_numbers
                                    );
#endif

                            if (drive && has_image(drive) && chs_to_lba(drive, &ctrl->req.drive_req, &lba)) {
                                if (drive->read_only) {
#ifdef DEBUG
                                    printf("[write protected]\r\n");
#endif
                                    ctrl->req.drive_req.status = STATUS_WRITE_PROTECTED;
                                } else {
#ifdef DEBUG
                                    printf("[lba=%lu]\r\n", lba);
#endif

                                    DWORD sector = 0;
                                    ctrl->req.drive_req.status = (
                                            offset2sector(&drive->image_file, (FSIZE_t) DATA_BUFFER_SIZE * lba, &sector) == FR_OK &&
                                            SD_SPI_SectorWrite(sector, data_buffer, 1)) ? 0 : STATUS_BAD_SECTOR;

                                    next_drive_req(drive, &ctrl->req.drive_req);
                                }
                            } else {
#ifdef DEBUG
                                printf("[bad sector]\r\n");
#endif
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }

                            break;

                        case CTRL_REQUEST_VERIFY:
                            drive = find_drive(ctrl->req.drive_req.drive_number);
#ifdef DEBUG
                            printf(
                                    "VERIFY[d=%d,lc=%d,h=%d,shc=%d]",
                                    ctrl->req.drive_req.drive_number,
                                    ctrl->req.drive_req.low_cylinder_number,
                                    ctrl->req.drive_req.head_number,
                                    ctrl->req.drive_req.sector_and_high_cylinder_numbers
                                    );
#endif

                            if (drive && has_image(drive) && chs_to_lba(drive, &ctrl->req.drive_req, &lba)) {
#ifdef DEBUG
                                printf("[lba=%lu]\r\n", lba);
#endif
                                ctrl->req.drive_req.status = 0;
                                next_drive_req(drive, &ctrl->req.drive_req);
                            } else {
#ifdef DEBUG
                                printf("[bad sector]\r\n");
#endif
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }

                            break;

                        case CTRL_REQUEST_READ_PARAMS_FUN8H:
                            ctrl->req.read_params_fun8h_req.number_of_drives = 0;

                            if (is_hard_drive(ctrl->req.read_params_fun8h_req.drive_number)) {
                                for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                                    if (has_geometry(&hard_drives[i])) {
                                        ctrl->req.read_params_fun8h_req.number_of_drives++;
                                    }
                                }
                            } else {
                                for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                                    if (has_geometry(&floppy_drives[i])) {
                                        ctrl->req.read_params_fun8h_req.number_of_drives++;
                                    }
                                }
                            }

                            drive = find_drive(ctrl->req.read_params_fun8h_req.drive_number);

#ifdef DEBUG
                            printf(
                                    "READ_PARAMS_FUN8H[d=%d]",
                                    ctrl->req.read_params_fun8h_req.drive_number
                                    );
#endif                                

                            if (drive && has_geometry(drive)) {
                                set_params_fun8h(drive, &ctrl->req.read_params_fun8h_req);
#ifdef DEBUG
                                printf(
                                        "[mlc=%d,mh=%d,mshc=%d]\r\n",
                                        ctrl->req.read_params_fun8h_req.max_low_cylinder_number,
                                        ctrl->req.read_params_fun8h_req.max_head_number,
                                        ctrl->req.read_params_fun8h_req.max_sector_and_high_cylinder_numbers
                                        );
#endif                                
                            } else {
#ifdef DEBUG
                                printf("[no geometry]\r\n");
#endif                                
                                ctrl->req.read_params_fun8h_req.success = 0;
                                ctrl->req.read_params_fun8h_req.drive_type = 0;
                                ctrl->req.read_params_fun8h_req.max_low_cylinder_number = 0;
                                ctrl->req.read_params_fun8h_req.max_head_number = 0;
                                ctrl->req.read_params_fun8h_req.max_sector_and_high_cylinder_numbers = 0;
                            }

                            break;

                        case CTRL_REQUEST_READ_PARAMS_FUN15H:
                            drive = find_drive(ctrl->req.read_params_fun15h_req.drive_number);
#ifdef DEBUG
                            printf(
                                    "READ_PARAMS_FUN15H[d=%d]",
                                    ctrl->req.read_params_fun15h_req.drive_number
                                    );
#endif

                            if (drive && has_geometry(drive)) {
#ifdef DEBUG
                                printf("[t=%d]\r\n", drive->drive_type_fun15h);
#endif

                                ctrl->req.read_params_fun15h_req.success = 1;
                                ctrl->req.read_params_fun15h_req.drive_type = drive->drive_type_fun15h;
                            } else {
#ifdef DEBUG
                                printf("[no geometry]\r\n");
#endif
                                ctrl->req.read_params_fun15h_req.success = 0;
                                ctrl->req.read_params_fun15h_req.drive_type = 0;
                            }

                            break;

                        case CTRL_REQUEST_DETECT_MEDIA_CHANGE:
                            drive = find_drive(ctrl->req.detect_media_change.drive_number);
#ifdef DEBUG
                            printf(
                                    "DETECT_MEDIA_CHANGE[d=%d",
                                    ctrl->req.detect_media_change.drive_number
                                    );
#endif

                            if (drive && has_image(drive)) {
                                if (drive->media_changed) {
                                    drive->media_changed = false;
#ifdef DEBUG
                                    printf("[media changed]\r\n");
#endif
                                    ctrl->req.detect_media_change.status = STATUS_DISK_CHANGED;
                                } else {
#ifdef DEBUG
                                    printf("[media not changed]\r\n");
#endif
                                    ctrl->req.detect_media_change.status = 0;
                                }
                            } else {
                                ctrl->req.drive_req.status = STATUS_BAD_SECTOR;
                            }
                            break;

                        default:
#ifdef DEBUG
                            printf("UNKNOWN REQUEST %d\r\n", ctrl->request);
#endif
                            break;
                    }

                    ctrl->request = CTRL_REQUEST_DONE;
                    LED_SetHigh();
                }
            }

            for (size_t i = 0; i < MAX_NUMBER_FLOPPY_DRIVES; i++) {
                if (has_image(&floppy_drives[i])) {
                    f_close(&floppy_drives[i].image_file);
                }
            }

            for (size_t i = 0; i < MAX_NUMBER_HARD_DRIVES; i++) {
                if (has_image(&hard_drives[i])) {
                    f_close(&hard_drives[i].image_file);
                }
            }

            f_mount(0, "0:", 0);

#ifdef DEBUG
            printf("UNMOUNTED\r\n");
#endif            
        } else {
            if (ctrl->request != CTRL_REQUEST_DONE) {
                LED_SetLow();
                switch (ctrl->request) {
                    case CTRL_REQUEST_CHECK:
                        invert_data_buffer();
                        break;

                    case CTRL_REQUEST_SCAN:
                        ctrl->req.scan_req.number_of_floppy_drives = 0;
                        ctrl->req.scan_req.number_of_hard_drives = 0;
                        break;

                    case CTRL_REQUEST_RESET:
                    case CTRL_REQUEST_READ:
                    case CTRL_REQUEST_WRITE:
                    case CTRL_REQUEST_VERIFY:
                        ctrl->req.drive_req.status = STATUS_CONTROLLER_FAILED;
                        break;

                    case CTRL_REQUEST_READ_PARAMS_FUN8H:
                        ctrl->req.read_params_fun8h_req.number_of_drives = 0;
                        ctrl->req.read_params_fun8h_req.success = 0;
                        break;

                    case CTRL_REQUEST_READ_PARAMS_FUN15H:
                        ctrl->req.read_params_fun15h_req.success = 0;
                        break;

                    default:
                        break;

                }
                ctrl->request = CTRL_REQUEST_DONE;
                LED_SetHigh();
            }

        }
    }
}
/**
 End of File
 */