#include <arduino.h>
#include <stdio.h>
#include <Wire.h>
#include "OneWire.h"
#include "OneWireDS2413.h"

OneWireDS2413::OneWireDS2413(OneWireDS2482 *iBM, tIdRef iId) : OneWire(iBM, iId){
};

void OneWireDS2413::init()
{
}

void OneWireDS2413::loop() {
    switch (mState) {
        case Startup:
            mState = Idle;
            // somehow we have to set the IO bitmask...
            mLastValue = getState();
            break;
        case SendOutput:
            // open: do we need specific output handling
        case GetInput:
            // open: do we need specific input handling
        case Idle:
            // first find out, if we have to write
            if (mValue != mLastValue) {
                // mValue was changed by external call, we have to write this to the 1W bus
                setState(mValue);
                // afterwards mValue and mLastvalue are equal and mValue contains the state info
                // so we need not to read anymore from 1W-Bus
            } else {
                // we do not have to write, so we will read inputs from 1W bus
                mValue = getState();
            }
            mLastValue = mValue;
            break;

        default:
            break;
  }
}

// bool OneWireDS2413::set_LED_DS2413(uint8_t led, bool state) {

//   byte temp = get_state();

//   if (state)
//     bitClear(temp, led);
//   else
//     bitSet(temp, led);

//    return set_state(temp);
// }

uint8_t OneWireDS2413::getState() {
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2413_CHANNEL_READ_CMD);
    return pBM->wireReadByte();
}

bool OneWireDS2413::setState(uint8_t iState) {
    wireSelectThisDevice();
    iState |= PIO_WRITE_MASK;
    pBM->wireWriteByte(DS2413_CHANNEL_WRITE_CMD);
    pBM->wireWriteByte(iState);
    pBM->wireWriteByte(~iState);
    if (pBM->wireReadByte() == 0xAA && pBM->wireReadByte() == iState) {
        mValue = iState;
        return true;
    }
    return false;
}

bool OneWireDS2413::setValue(uint8_t iValue) {
    mValue = iValue;
    return Mode() == Connected;
}

bool OneWireDS2413::getValue(uint8_t &eValue) {
    eValue = mValue;
    return Mode() == Connected;
}