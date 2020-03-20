#pragma once

#include <inttypes.h>
#include "OneWire.h"
#include "OneWireDS2482.h"

// Model Commands
#define DS2408_PIO_READ_CMD      0xF0
#define DS2408_CHANNEL_READ_CMD  0xF5
#define DS2408_CHANNEL_WRITE_CMD 0x5A
#define DS2408_SEARCH_CMD        0xCC
#define DS2408_RESET_CMD         0xC3

// Register Addresses
#define DS2408_PIO_LOGIC_REG       0x0088   // Current state
#define DS2408_PIO_OUTPUT_REG      0x0089   // Last write
#define DS2408_PIO_ACTIVITY_REG    0x008A   // State Change Activity
#define DS2408_SEARCH_MASK_REG     0x008B
#define DS2408_SEARCH_SELECT_REG   0x008C
#define DS2408_CONTROL_STATUS_REG  0x008D

#define DS2408_PLS_STATUS_MASK  0x0   // 
#define DS2408_CT_STATUS_MASK   0x1   // Conditional Search Term
#define DS2408_ROS_STATUS_MASK  0x2   // Reset state
#define DS2408_PORL_STATUS_MASK 0x3   // Power-On Reset status
#define DS2408_VCCP_STATUS_MASK 0x7   //

// Control/Status Bits
#define SEARCH_TRIGGER_BIT  0x0
#define SEARCH_TERM_BIT     0x1
#define RESET_PIN_BIT       0x2
#define POWERON_RESET_BIT   0x3
#define VCCC_POWER_BIT      0x7

// SEARCH_TRIGGER modes
#define PIN_TRIGGER         0x0
#define ACTIVITY_TRIGGER    0x1

// SEARCH_TERM modes
#define OR_TERM             0x0
#define AND_TERM            0x1

// RESET_PIN_MODE modes
#define RESET_MODE          0x0
#define STROBE_MODE         0x1

// POWERON_RESET_MODE modes
#define RESET_ON            0x0
#define RESET_OFF           0x1

// Control modes
#define SEARCH_TRIGGER(mode)      (mode<<SEARCH_TRIGGER_BIT)
#define SEARCH_TERM(mode)         (mode<<SEARCH_TERM_BIT)
#define RESET_PIN_MODE(mode)      (mode<<RESET_PIN_BIT)
#define POWERON_RESET_MODE(mode)  (mode<<POWERON_RESET_BIT)

#define REG_LO(addr) (addr&0xFF)
#define REG_HI(addr) (addr>>8)

class OneWireDS2408 : public OneWire
{
  public:
    enum StateSensorIO
    {
        Startup,
        GetInput,
        SendOutput,
        Idle,
        Error
    };

    OneWireDS2408(OneWireDS2482 *iBusMaster, tIdRef iId);

    void init();
    void loop() override;

    // bool set_LED_DS2408(uint8_t led, bool state);

    uint8_t getRegister(uint16_t iRegister);
    void setRegister(uint16_t iRegister, uint8_t iValue);

    uint8_t getState();
    bool setState(uint8_t iState);

    // Registers
    void setSearchMask(uint8_t iMask);
    void setSearchPolarity(uint8_t iPolarity);

    void setMode(uint8_t iMode);             // Control/Status Register
    uint8_t getMode();                         // Control/Status Register

    uint8_t getCurrentState();                // Logic State  Register
    uint8_t getLastState();                   // Output   Latch State Register
    uint8_t getActivity();                     // Activity Latch state Register
    bool resetActivity();

private:
  uint8_t mState = Startup;
};