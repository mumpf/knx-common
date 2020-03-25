#pragma once

#include <inttypes.h>
#include "OneWire.h"
#include "OneWireDS2482.h"

// Model Commands
#define DS2413_CHANNEL_READ_CMD  0xF5
#define DS2413_CHANNEL_WRITE_CMD 0x5A

// Control/Status Bits
#define PIOA_STATE_INPUT_BIT  0x1
#define PIOA_STATE_OUTPUT_BIT 0x2
#define PIOB_STATE_INPUT_BIT  0x4
#define PIOB_STATE_OUTPUT_BIT 0x8

#define PIOA_WRITE_BIT        0x01
#define PIOB_WRITE_BIT        0x02
#define PIO_WRITE_MASK        0xFC

class OneWireDS2413 : public OneWire
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

    OneWireDS2413(OneWireDS2482 *iBusMaster, tIdRef iId);

    void init();
    void loop() override;

    uint8_t getState();
    bool setState(uint8_t iState);

  private:
    uint8_t mState = Startup;
};