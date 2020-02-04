#pragma once
// #include <knx.h>

// Board specific implementations

// additional LED (yellow) as debug indicator
#define LED_YELLOW_PIN             38
#define PROG_LED_PIN 13
#define PROG_LED_PIN_ACTIVE_ON HIGH
// Buzzer
#define BUZZER_PIN                 9
#define BUZZER_FREQ                2400

#define I2C_1WIRE_DEVICE_ADDRESSS  0x18 // Address of DS2484 1-Wire-Busmaster chip

// // EEPROM Support
// #define I2C_EEPROM_DEVICE_ADDRESSS 0x50 // Address of 24LC256 eeprom chip
// #define SAVE_BUFFER_START_PAGE     0    // All stored KO data begin at this page and takes 37 pages, 
//                                         // allow 3 pages boundary, so next store should start at page 40

// NCN5130: internal commands
#define U_RESET_REQ          0x01
#define U_STATE_REQ          0x02
#define U_SET_BUSY_REQ       0x03
#define U_QUIT_BUSY_REQ      0x04
#define U_BUSMON_REQ         0x05
#define U_SET_ADDRESS_REQ    0xF1 // different on TP-UART
#define U_SET_REPETITION_REQ 0xF2
#define U_L_DATA_OFFSET_REQ  0x08 //-0x0C
#define U_SYSTEM_STATE       0x0D
#define U_STOP_MODE_REQ      0x0E
#define U_EXIT_STOP_MODE_REQ 0x0F
#define U_ACK_REQ            0x10 //-0x17
#define U_CONFIGURE_REQ      0x18
#define U_INT_REG_WR_REQ     0x28 //-0x2B
#define U_INT_REG_RD_REQ     0x38
#define U_POLLING_STATE_REQ  0xE0

// NCN5130: control services
#define U_RESET_IND           0x03
#define U_STATE_IND           0x07
#define SLAVE_COLLISION       0x80
#define RECEIVE_ERROR         0x40
#define TRANSMIT_ERROR        0x20
#define PROTOCOL_ERROR        0x10
#define TEMPERATURE_WARNING   0x08
#define U_FRAME_STATE_IND     0x13
#define U_FRAME_STATE_MASK    0x17
#define PARITY_BIT_ERROR      0x80
#define CHECKSUM_LENGTH_ERROR 0x40
#define TIMING_ERROR          0x20
#define U_CONFIGURE_IND       0x01
#define U_CONFIGURE_MASK      0x83
#define AUTO_ACKNOWLEDGE      0x20
#define AUTO_POLLING          0x10
#define CRC_CCITT             0x80
#define FRAME_END_WITH_MARKER 0x40
#define U_FRAME_END_IND       0xCB
#define U_STOP_MODE_IND       0x2B
#define U_SYSTEM_STAT_IND     0x4B

// NCN5130 internal registers
#define U_INT_REG_WR_REQ_WD        0x28
#define U_INT_REG_WR_REQ_ACR0      0x29
#define U_INT_REG_WR_REQ_ACR1      0x2A
#define U_INT_REG_WR_REQ_ASR0      0x2B

// NCN5130: Analog Control Register 0 - Bit values
#define ACR0_FLAG_V20VEN           0x40
#define ACR0_FLAG_DC2EN            0x20
#define ACR0_FLAG_XCLKEN           0x10
#define ACR0_FLAG_TRIGEN           0x08
#define ACR0_FLAG_V20VCLIMIT       0x04


// check board hardware availibility
bool boardCheck();
// Turn off 5V rail from NCN5130 to save power for EEPROM write during knx save operation
void savePower();
// Turn on 5V rail from NCN5130 in case SAVE-Interrupt was false positive
void restorePower();

void fatalError(uint8_t iErrorCode, const char *iErrorText = 0);
