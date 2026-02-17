#include "sd.h"

#include <stddef.h>
#include <string.h>

#include "spi.h"

#define SD_COMMAND_CODE_BIT_MASK (0b00111111)
#define SD_COMMAND_TRANSMIT_BIT_MASK (1<<6)

#define SD_WRITE_RESPONSE_TOKEN_MASK   0x1F

enum SDToken {
    SD_TOKEN_START = 0xFE,
    SD_TOKEN_START_MULTI_BLOCK = 0xFC,
    SD_TOKEN_STOP_TRANSMISSION = 0xFD,
    SD_TOKEN_DATA_ACCEPTED = 0x05,
    SD_TOKEN_FLOATING_BUS = 0xFF
};

enum SDCommandCode {
    // Reset the SD card
    SD_COMMAND_CODE_GO_IDLE_STATE = 0,
    // Initialize the SD card
    SD_COMMAND_CODE_SEND_OP_COND = 1,
    // Check for sector addressing
    SD_COMMAND_CODE_SEND_IF_COND = 8,
    // Get CSD (Card Specific Data)
    SD_COMMAND_CODE_SEND_CSD = 9,
    // Get CID (Card Information)
    SD_COMMAND_CODE_SEND_CID = 10,
    // Stop transmission during a multi-block read
    SD_COMMAND_CODE_STOP_TRANSMISSION = 12,
    // Get card ServiceStatus information
    SD_COMMAND_CODE_SEND_STATUS = 13,
    // Set block length of the card
    SD_COMMAND_CODE_SET_BLOCK_LENGTH = 16,
    // Read one block from the card
    SD_COMMAND_CODE_READ_SINGLE_BLOCK = 17,
    // Read multiple blocks from the card
    SD_COMMAND_CODE_READ_MULTI_BLOCK = 18,
    // Tell the media how many blocks to pre-erase for the subsequent WRITE_MULTI_BLOCK
    SD_COMMAND_CODE_SET_WRITE_BLOCK_ERASE_COUNT = 23,
    // Write one block to the card
    SD_COMMAND_CODE_WRITE_SINGLE_BLOCK = 24,
    // Write multiple blocks to the card
    SD_COMMAND_CODE_WRITE_MULTI_BLOCK = 25,
    // Set address of the start of an erase operation
    SD_COMMAND_CODE_TAG_SECTOR_START = 32,
    // Det address of the end of an erase operation
    SD_COMMAND_CODE_TAG_SECTOR_END = 33,
    // Erase all previously selected blocks
    SD_COMMAND_CODE_ERASE = 38,
    // Initialize an SD card and provide the CSD register value
    SD_COMMAND_CODE_SD_SEND_OP_COND = 41,
    // Begin application specific command
    SD_COMMAND_CODE_APP_CMD = 55,
    // Get OCR register information from the card
    SD_COMMAND_CODE_READ_OCR = 58,
    // Disable CRC checking
    SD_COMMAND_CODE_CRC_ON_OFF = 59
};

enum SDCommand
{
    SD_COMMAND_GO_IDLE_STATE = 0,
    SD_COMMAND_SEND_OP_COND,
    SD_COMMAND_SEND_IF_COND,
    SD_COMMAND_SEND_CSD,
    SD_COMMAND_SEND_CID,
    SD_COMMAND_STOP_TRANSMISSION,
    SD_COMMAND_SEND_ServiceStatus,
    SD_COMMAND_SET_BLOCK_LENGTH,
    SD_COMMAND_READ_SINGLE_BLOCK,
    SD_COMMAND_READ_MULTI_BLOCK,
    SD_COMMAND_SET_WRITE_BLOCK_ERASE_COUNT,
    SD_COMMAND_WRITE_SINGLE_BLOCK,
    SD_COMMAND_WRITE_MULTI_BLOCK,
    SD_COMMAND_TAG_SECTOR_START,
    SD_COMMAND_TAG_SECTOR_END,
    SD_COMMAND_ERASE,
    SD_COMMAND_SD_SEND_OP_COND,
    SD_COMMAND_APP_CMD,
    SD_COMMAND_READ_OCR,
    SD_COMMAND_CRC_ON_OFF
};

enum SDResponseType
{
    SD_RESPONSE_TYPE_R1,
    SD_RESPONSE_TYPE_R1B,
    SD_RESPONSE_TYPE_R2,
    SD_RESPONSE_TYPE_R3,
    SD_RESPONSE_TYPE_R7
};

struct SDCommandTableEntry
{
    enum SDCommandCode code;
    uint8_t crc;
    enum SDResponseType response_type;
    bool extra_data_expected;
};

static const struct SDCommandTableEntry sd_command_table[] =
{
    {SD_COMMAND_CODE_GO_IDLE_STATE,                  0x95,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_SEND_OP_COND,                   0xF9,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_SEND_IF_COND,                   0x87,   SD_RESPONSE_TYPE_R7,     false},
    {SD_COMMAND_CODE_SEND_CSD,                       0xAF,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_SEND_CID,                       0x1B,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_STOP_TRANSMISSION,              0xC3,   SD_RESPONSE_TYPE_R1B,    false},
    {SD_COMMAND_CODE_SEND_STATUS,                    0xAF,   SD_RESPONSE_TYPE_R2,     false},
    {SD_COMMAND_CODE_SET_BLOCK_LENGTH,               0xFF,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_READ_SINGLE_BLOCK,              0xFF,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_READ_MULTI_BLOCK,               0xFF,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_SET_WRITE_BLOCK_ERASE_COUNT,    0xFF,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_WRITE_SINGLE_BLOCK,             0xFF,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_WRITE_MULTI_BLOCK,              0xFF,   SD_RESPONSE_TYPE_R1,     true},
    {SD_COMMAND_CODE_TAG_SECTOR_START,               0xFF,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_TAG_SECTOR_END,                 0xFF,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_ERASE,                          0xDF,   SD_RESPONSE_TYPE_R1B,    false},
    {SD_COMMAND_CODE_SD_SEND_OP_COND,                0xFF,   SD_RESPONSE_TYPE_R3,     false},
    {SD_COMMAND_CODE_APP_CMD,                        0x73,   SD_RESPONSE_TYPE_R1,     false},
    {SD_COMMAND_CODE_READ_OCR,                       0x25,   SD_RESPONSE_TYPE_R7,     false},
    {SD_COMMAND_CODE_CRC_ON_OFF,                     0x25,   SD_RESPONSE_TYPE_R1,     false}
};

#define SD_NCR_TIMEOUT     (uint16_t)20          // SPI byte times before command response is expected (must be at least 8)
#define SD_NAC_TIMEOUT     (uint32_t)0x40000     // SPI byte times we should wait when performing read operations (should be at least 100ms for SD cards)
#define SD_WRITE_TIMEOUT   (uint32_t)0xA0000     // SPI byte times to wait before timing out when the media is performing a write operation (should be at least 250ms for SD cards).

union SDResponse1
{
    uint8_t _uint8;
    struct
    {
        unsigned IN_IDLE_STATE:1;
        unsigned ERASE_RESET:1;
        unsigned ILLEGAL_COMMAND:1;
        unsigned CRC_ERROR:1;
        unsigned ERASE_SEQ_ERROR:1;
        unsigned ADDRESS_ERROR:1;
        unsigned PARAM_ERROR:1;
        unsigned _bit7:1;
    };
} ;

union SDResponse2
{
    uint16_t _uint16;
    struct
    {
        uint8_t      _uint8[2];
    };
    struct
    {
        unsigned IN_IDLE_STATE:1;
        unsigned ERASE_RESET:1;
        unsigned ILLEGAL_COMMAND:1;
        unsigned CRC_ERROR:1;
        unsigned ERASE_SEQ_ERROR:1;
        unsigned ADDRESS_ERROR:1;
        unsigned PARAM_ERROR:1;
        unsigned _bit7:1;
        unsigned CARD_IS_LOCKED:1;
        unsigned WP_ERASE_SKIP_LK_FAIL:1;
        unsigned ERROR:1;
        unsigned CC_ERROR:1;
        unsigned CARD_ECC_FAIL:1;
        unsigned WP_VIOLATION:1;
        unsigned ERASE_PARAM:1;
        unsigned OUTRANGE_CSD_OVERWRITE:1;
    };
};

union SDResponse3_7
{
    struct
    {
        uint8_t _uint8;
        union
        {
            uint32_t return_val; // This is big endian format!
            struct
            {
                uint8_t _uint8[4];
            };    
        } argument;    
    } bytes;

    struct
    {
        struct
        {
            unsigned IN_IDLE_STATE:1;
            unsigned ERASE_RESET:1;
            unsigned ILLEGAL_COMMAND:1;
            unsigned CRC_ERROR:1;
            unsigned ERASE_SEQ_ERROR:1;
            unsigned ADDRESS_ERROR:1;
            unsigned PARAM_ERROR:1;
            unsigned _bit7:1;
        };

        uint32_t return_val;  // This is big endian format!
    } bits;
};

union SDResponse
{
    union SDResponse1  response_1;  
    union SDResponse2  response_2;
    union SDResponse3_7  response_3_7;
};

static void send_command(uint8_t command, uint32_t param, union SDResponse *response) {
    uint16_t timeout;
    uint32_t long_timeout;

    spi_chip_select();

    const struct SDCommandTableEntry* command_entry = &sd_command_table[command];

    (void) spi_exchange_byte((command_entry->code & SD_COMMAND_CODE_BIT_MASK) | SD_COMMAND_TRANSMIT_BIT_MASK);

    uint8_t *param_bytes = (uint8_t *) & param;

    (void) spi_exchange_byte(param_bytes[3]);
    (void) spi_exchange_byte(param_bytes[2]);
    (void) spi_exchange_byte(param_bytes[1]);
    (void) spi_exchange_byte(param_bytes[0]);

    (void) spi_exchange_byte(command_entry->crc);

    // Special case for STOP_TRANSMISSION: Ignore the first byte
    // post-command. It is frequently 'bogus' residual data and
    // should not be processed as the R1 response.

    if (command == SD_COMMAND_STOP_TRANSMISSION)
    {
        (void) spi_exchange_byte(0xFF); //Perform dummy read to fetch the residual non R1 byte
    } 

    // NCR delay to wait for media response. The initial byte of
    // any response (R1, R1B, R2, R3, or R7) is always equivalent
    // to an R1 byte.

    timeout = SD_NCR_TIMEOUT;
    do
    {
        response->response_1._uint8 = spi_exchange_byte(0xFF);
        timeout--;
    } while((response->response_1._uint8 == SD_TOKEN_FLOATING_BUS) && (timeout != 0));

    switch (command_entry->response_type) {
        case SD_RESPONSE_TYPE_R1:
            break;

        case SD_RESPONSE_TYPE_R2:
            response->response_2._uint8[1] = response->response_1._uint8;
            response->response_2._uint8[0] = spi_exchange_byte(0xFF);
            break;

        case SD_RESPONSE_TYPE_R1B:
            // Poll the media until the busy signal clears. While busy, the card
            // will continuously output 0x00 bytes; a non-zero value indicates the
            // card is ready for the next command. This R1B busy state typically
            // follows a STOP_TRANSMISSION, as the card requires time—often several
            // milliseconds—to commit its internal buffers to flash memory.

            long_timeout = SD_WRITE_TIMEOUT;
            do
            {
                response->response_1._uint8 = spi_exchange_byte(0xFF);
                long_timeout--;
            } while((response->response_1._uint8 == 0x00) && (long_timeout != 0));

            response->response_1._uint8 = 0x00;
            break;

        case SD_RESPONSE_TYPE_R7:
        case SD_RESPONSE_TYPE_R3:
            // Retrieve the remaining four bytes of the R3/R7 response.
            // Note that the SD card transmits its 32-bit argument field in big-endian
            // order. Since the PIC18 uses little-endian storage in RAM, ensure the
            // bytes are reordered correctly when writing to the return_val to maintain
            // numerical integrity.

            response->response_3_7.bytes.argument._uint8[3] = spi_exchange_byte(0xFF);
            response->response_3_7.bytes.argument._uint8[2] = spi_exchange_byte(0xFF);
            response->response_3_7.bytes.argument._uint8[1] = spi_exchange_byte(0xFF);
            response->response_3_7.bytes.argument._uint8[0] = spi_exchange_byte(0xFF);
            break;
    }

    // The device requires a minimum synchronization period of 8
    // clock pulses following the completion of a response before
    // it can accept a subsequent command.

    (void) spi_exchange_byte(0xFF);    

    if (command_entry->extra_data_expected == false)
    {
        spi_chip_deselect();
    }
}

// In SPI Slow Mode, a 400kHz clock provides 400 cycles per millisecond.
// Since each byte consists of 8 clock pulses, transmitting 50 dummy bytes
// ensures a minimum delay of 1ms.

#define SD_SLOW_CLOCK_DELAY_1MS_MIN 50

#define SD_SPI_COMMAND_WAIT_MS 1
#define SD_SPI_STARTUP_DELAY_MS 30

#define SD_MEDIA_BLOCK_SIZE 512

static void delay_ms(uint8_t ms) {
    uint16_t timeout = SD_SLOW_CLOCK_DELAY_1MS_MIN * ms;

    spi_chip_deselect();

    while(timeout--)
    {
        (void) spi_exchange_byte(0xFF);
    }
}

void sd_media_init(struct SDMediaInfo* media_info) {
    uint16_t timeout;
    union SDResponse response;

    media_info->error = SD_MEDIA_ERROR_NO_ERROR;
    media_info->sd_mode = SD_MODE_NORMAL;

    spi_chip_deselect();

    spi_enable_slow();

    // The media requires an initialization period defined by the greater of
    // three factors: the Vdd ramp time, a 1ms fixed delay, or a minimum of
    // 74 clock pulses. Per the SD specification, Chip Select (CS) must remain 
    // de-asserted (high) during these initial clock cycles. In practical
    // application, it is advisable to exceed the 1ms minimum significantly
    // to account for potential contact bounce or incomplete mechanical
    // insertion during the power-up phase.

    delay_ms(SD_SPI_STARTUP_DELAY_MS);

    // Issue the SD_COMMAND_GO_IDLE_STATE while Chip Select (CS) is de-asserted.
    // This sequence triggers a software reset of the media and transitions the
    // SD card from its default native bus mode into SPI mode.

    timeout = 100;
    do
    {
        // Cycle the Chip Select (CS) line to force the media to abort any pending
        // internal operations. Asserting CS low immediately before transmitting
        // SD_COMMAND_GO_IDLE_STATE ensures the command starts on a clean state
        // transition. This practice mitigates synchronization errors between the
        // SPI master and slave that may have been caused by signal noise on the SCK (clock) line.

        spi_chip_deselect();

        // Generate additional 'dummy' clock pulses to ensure the SPI state machine
        // is fully synchronized. This accounts for scenarios where a preceding
        // command may have been prematurely terminated, leaving the media card
        // without the trailing clock cycles required to finalize the internal transfer.

        (void) spi_exchange_byte(0xFF); 
        spi_chip_select();
        timeout--;

        send_command(SD_COMMAND_GO_IDLE_STATE, 0x0, &response);
    } while((response.response_1._uint8 != 0x01) && (timeout != 0));
    
    // Identify if all initialization attempts have timed out. This condition
    // typically arises when the SD card remains in a 'Busy' or 'Data Transfer'
    // state following an asynchronous reset of the microcontroller. If the MCU
    // was power-cycled while the card was mid-operation, the card may still be
    // attempting to fulfill a previous read/write request, rendering it unresponsive
    // to SD_COMMAND_GO_IDLE_STATE. In this scenario, issue a STOP_TRANSMISSION to
    // force the media to terminate its current state and regain command synchronization.

    if (timeout == 0)
    {
        spi_chip_deselect();

        // Generate additional 'dummy' clock pulses to ensure the SPI state machine
        // is fully synchronized. This accounts for scenarios where a preceding
        // command may have been prematurely terminated, leaving the media card
        // without the trailing clock cycles required to finalize the internal transfer.

        (void) spi_exchange_byte(0xFF);
        spi_chip_select();

        send_command(SD_COMMAND_STOP_TRANSMISSION, 0x0, &response);
        send_command(SD_COMMAND_GO_IDLE_STATE, 0x0, &response);

        if (response.response_1._uint8 != 0x01)
        {
            // The media has failed to process SD_COMMAND_GO_IDLE_STATE after multiple
            // recovery attempts. Ideally, a hardware power cycle would be initiated to
            // force a cold reset; however, the SD/MMC lacks software-controlled power
            // switching for the SD slot. As the hardware cannot programmatically resolve
            // this hang, the initialization process will terminate. A manual power cycle
            // of the media or the host board is required to restore functionality.

            media_info->error = SD_MEDIA_ERROR_CANNOT_INITIALIZE;
            
            spi_chip_deselect();
            spi_disable();
            return;
        }
    }

    // Issue SD_COMMAND_SEND_IF_COND to negotiate the interface operating conditions. The argument
    // 0x000001AA specifies a voltage supply (VHS) of 2.7V–3.6V and includes a recommended 0xAA 
    // check pattern. A Version 2.0+ SD card will acknowledge this by echoing the check pattern
    // within a 6-byte R7 response. If the card is an older Legacy (v1.x) or MMC device, it will
    // treat SEND_IF_COND as an invalid command, allowing the host to identify the card's generation.
    
    send_command(SD_COMMAND_SEND_IF_COND, 0x1AA, &response);

    if (((response.response_3_7.bytes.argument.return_val & 0xFFF) == 0x1AA) && (!response.response_3_7.bits.ILLEGAL_COMMAND))
    {
        // Successful completion of SD_COMMAND_SEND_IF_COND confirms the device
        // is compatible with the host’s voltage range and indicates an SD v2.0+
        // (Standard or High Capacity) card. Proceed by issuing SD_COMMAND_READ_OCR
        // to retrieve the Operating Conditions Register (OCR). This triggers an
        // R3 response totaling 5 bytes: a standard R1 ServiceStatus byte followed by the
        // 32-bit OCR payload.

        send_command(SD_COMMAND_READ_OCR, 0x0, &response);

        // With the OCR register retrieved, the host could programmatically evaluate
        // the card's supported voltage ranges. For systems with variable power rails,
        // this allows for optimizing Vdd to the card's minimum requirement to reduce
        // power consumption. Subsequently, enter a polling loop using the
        // SD_COMMAND_APP_CMD/SD_COMMAND_SD_SEND_OP_COND sequence to complete the
        // internal initialization. Per SD specifications, implement a timeout of
        // at least 1 second to allow the card sufficient time to transition from the
        // busy state.

        for (timeout = 0; timeout < 0xFFFF; timeout++)
        {				
            send_command(SD_COMMAND_APP_CMD, 0x00000000, &response);

            // Issue SD_COMMAND_SD_SEND_OP_COND to verify that the SD card has completed
            // its power-on sequence and is ready for high-frequency operation. This command
            // returns an R3 response (6 bytes total); while the leading byte is the standard
            // ServiceStatus, the middle four bytes provide the OCR register data. Crucially,
            // bit 30—the HCS (Host Capacity Support) bit—must be set to 1 in the command
            // argument to notify the card that the host is compatible with SDHC/SDXC high-capacity
            // media.

            send_command(SD_COMMAND_SD_SEND_OP_COND, 0x40000000, &response);

            // An R1 response of 0x00 confirms the card has transitioned from the 'Idle' state
            // (set by SD_COMMAND_GO_IDLE_STATE) into the 'Standby' or 'Ready' state. This
            // indicates that the internal power-up and initialization sequences are complete,
            // and the media is now prepared to accept data-transfer commands such as read and
            // write operations.

            if (response.response_1._uint8 == 0)
            {
                break;
            }				
        }
		
        if (timeout >= 0xFFFF)
        {
            media_info->error = SD_MEDIA_ERROR_CANNOT_INITIALIZE;
        }				

        // Issue SD_COMMAND_READ_OCR to read the Operating Conditions Register (OCR). This
        // register provides critical device metadata, specifically allowing the host to
        // differentiate between Standard Capacity (SDSC) and High Capacity (SDHC/SDXC) media.

        send_command(SD_COMMAND_READ_OCR, 0x0, &response);

        // Examine the CCS (Card Capacity ServiceStatus) bit, located at bit 30 of the OCR register.
        // This bit identifies whether the media is a High Capacity (SDHC/SDXC) or
        // Standard Capacity (SDSC) device. Note that the CCS value is only valid once the
        // 'Busy' bit (bit 31) transitions to a 'Ready' state, signaling that the initialization
        // process is complete.

        if (response.response_3_7.bytes.argument.return_val & 0x40000000)
        {
            media_info->sd_mode = SD_MODE_HC;
        }				
        else
        {
            media_info->sd_mode = SD_MODE_NORMAL;
        }
    }
    else
    {
        // The rejection of SD_COMMAND_SEND_IF_COND indicates that the media does not conform to
        // the SD v2.0+ specification. The device is categorized as a Legacy (v1.x) or MMC card,
        // which supports Standard Capacity only. Ensure a brief synchronization delay is observed
        // before issuing the next command to satisfy the device's timing requirements.

        delay_ms(SD_SPI_COMMAND_WAIT_MS);

        spi_chip_select();

        media_info->sd_mode = SD_MODE_NORMAL;
        timeout = 0x1FFF;
        do
        {
            // Issue SD_COMMAND_SEND_OP_COND to initiate the media's internal power-up sequence.
            // By providing an argument of 0x00000000, the host effectively queries the card's
            // supported operating voltage range—a requirement for identifying compatible MMC devices.

            send_command(SD_COMMAND_SEND_OP_COND, 0x00000000, &response);
            timeout--;
        } while((response.response_1._uint8 != 0x00) && (timeout != 0));

        if (timeout == 0)
        {
            media_info->error = SD_MEDIA_ERROR_CANNOT_INITIALIZE;
            spi_chip_deselect();
        }
    }

    spi_chip_deselect();
    spi_disable();
    spi_enable_fast();
    spi_chip_select();

    // Attempt to explicitly disable the CRC7 check using SD_COMMAND_CRC_ON_OFF.
    // While this command may be rejected by some legacy media, it is generally
    // considered redundant; according to the SD specification, cards default to
    // CRC-disabled mode upon entering SPI operation after a Power-On Reset (POR).

    send_command(SD_COMMAND_CRC_ON_OFF, 0x0, &response);

    // Configure the device block length to match the physical media sector size
    // using SD_COMMAND_SET_BLOCK_LENGTH. While the specification mandates this as
    // the default power-on state for most cards, explicitly issuing this command
    // ensures synchronization between the host’s expected buffer size and the card's
    // internal logic.

    send_command(SD_COMMAND_SET_BLOCK_LENGTH , SD_MEDIA_BLOCK_SIZE, &response);

    spi_chip_deselect();
}

static void start_transaction() {
    spi_enable_fast();
    spi_chip_select();
}

static void end_transaction() {
    spi_chip_deselect();
    (void) spi_exchange_byte(0xFF);

    spi_disable();
}

bool sd_start_read_blocks(uint32_t address) {
    union SDResponse response;

    start_transaction();
    send_command(SD_COMMAND_READ_MULTI_BLOCK, address, &response);
    return (response.response_1._uint8 == 0);
}

bool sd_start_write_blocks(uint32_t address) {
    union SDResponse response;

    start_transaction();
    send_command(SD_COMMAND_WRITE_MULTI_BLOCK, address, &response);
    return (response.response_1._uint8 == 0);
}

static uint8_t wait_for_token() {
    uint32_t long_timeout = SD_NAC_TIMEOUT;
    uint8_t response;
    do {
        response = spi_exchange_byte(0xFF);
        long_timeout--;
    } while ((response == SD_TOKEN_FLOATING_BUS) && (long_timeout != 0));
    return response;
}

bool sd_read_next_block(void *buffer) {
    if (wait_for_token() == SD_TOKEN_START) {
        spi_read_block(buffer, SD_MEDIA_BLOCK_SIZE);

        // CRC
        spi_exchange_byte(0xFF);
        spi_exchange_byte(0xFF);

        return true;
    }

    return false;
}

static void wait_busy() {
    uint32_t long_timeout = SD_WRITE_TIMEOUT;
    uint8_t response;
    do {
        response = spi_exchange_byte(0xFF);
        long_timeout--;
    } while ((response == 0x00) && (long_timeout != 0));
}

bool sd_write_next_block(void *buffer) {
    spi_exchange_byte(SD_TOKEN_START_MULTI_BLOCK);

    spi_write_block(buffer, SD_MEDIA_BLOCK_SIZE);

    // CRC
    spi_exchange_byte(0xFF);
    spi_exchange_byte(0xFF);

    uint8_t response = spi_exchange_byte(0xFF);

    spi_exchange_byte(0xFF);

    wait_busy();

    return (response & SD_WRITE_RESPONSE_TOKEN_MASK) == SD_TOKEN_DATA_ACCEPTED;
}

void sd_stop_read_blocks(void) {
    union SDResponse response;

    send_command(SD_COMMAND_STOP_TRANSMISSION, 0, &response);

    end_transaction();
}

void sd_stop_write_blocks(void) {
    spi_exchange_byte(SD_TOKEN_STOP_TRANSMISSION);

    spi_exchange_byte(0xFF);

    wait_busy();

    end_transaction();
}
