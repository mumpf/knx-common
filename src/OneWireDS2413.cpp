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
    bool lSuccess = false;
    switch (mState) {
        case Startup:
            mValue = getState();
            mLastValue = mValue;
            mState = Idle;
            break;
        case SendOutput:
            // first find out, if we have to write
            if (mValue != mLastValue && mIoMask < 3) {
                // mValue was changed by external call, we have to write this to the 1W bus
                lSuccess = setState(mValue);
                // status is automatically fetched after a write command, so mValue is now correct
            }
            mState = lSuccess ? SendOutput : GetInput;
            break;
        case GetStatus:
            // after an Ouput the current state shoud be read
            mValue = getState();
            mLastValue = mValue;
            mState = SendOutput;
        case GetInput:
            // normal read happens just if the IO is configured for Input 
            if (mIoMask) {
                mValue = getState();
                mLastValue = mValue;
            }
            mState = SendOutput;
            break;
        case Idle:
            // first find out, if we have to write
            if ((mIoMask < 3) && (mValue != mLastValue))
            {
                // mValue was changed by external call, we have to write this to the 1W bus
                setState(mValue);
                // afterwards mValue and mLastvalue are equal and mValue contains the state info
                // so we need not to read anymore from 1W-Bus
            }
            else if (mIoMask)
            {
                // we do not have to write, so we will read inputs from 1W bus if there are input pins
                mValue = getState();
                mLastValue = mValue;
            }
            break;
        default:
            break;
  }
}

bool OneWireDS2413::setParameter(OneWire::ModelParameter iModelParameter, uint8_t iValue)
{
    // default implementation for devices without parameters
    if (iModelParameter == IoMask) {
        // if an IOMast is set, we use it here, 1 is Input, 0 is Ouput;
        mIoMask = iValue & ~PIO_WRITE_MASK;
        return true;
    }
    return false;
}

// bool OneWireDS2413::set_LED_DS2413(uint8_t led, bool state) {

//   byte temp = get_state();

//   if (state)
//     bitClear(temp, led);
//   else
//     bitSet(temp, led);

//    return set_state(temp);
// }

uint8_t OneWireDS2413::convertStateToValue(uint8_t iValue){
    uint8_t lResult = ((iValue & PIOB_STATE_INPUT_BIT) >> 1) | (iValue & PIOA_STATE_INPUT_BIT);
    return lResult;
}

uint8_t OneWireDS2413::getState()
{
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2413_CHANNEL_READ_CMD);
    return convertStateToValue(pBM->wireReadByte());
}

bool OneWireDS2413::setState(uint8_t iState) {
    wireSelectThisDevice();
    iState |= PIO_WRITE_MASK | mIoMask;
    pBM->wireWriteByte(DS2413_CHANNEL_WRITE_CMD);
    pBM->wireWriteByte(iState);
    pBM->wireWriteByte(~iState);
    uint8_t lByte1 = pBM->wireReadByte();
    // here the check for state gets complicated
    // the last fetched byte is not the sent byte, 
    // but the state byte (as in getState)
    uint8_t lByte2 = convertStateToValue(pBM->wireReadByte());
    if (lByte1 == 0xAA && lByte2 == (iState & ~PIO_WRITE_MASK))
    {
        mValue = lByte2;
        mLastValue = mValue;
        return true;
    }
    return false;
}

bool OneWireDS2413::setValue(uint8_t iValue, ModelFunction iModelFunction)
{
    // we set here the output according to given model funtion as byte or bit
    bool lResult = (Mode() == Connected);
    if (lResult) {
        if (iModelFunction == IoByte || iModelFunction == Default) {
            mValue = iValue & ~PIO_WRITE_MASK;
        } else if (iModelFunction < IoByte && iModelFunction <= IoBit1) {
            if (iValue) {
                mValue |= (1 << (iModelFunction - 1));
            } else {
                mValue &= ~(1 << (iModelFunction - 1));
            }
        } else {
            lResult = false;
        }
    }
    // ensure, that state machine is set to write this value next
    // if (lResult)
    //     mState = SendOutput;
    return lResult;
}

bool OneWireDS2413::getValue(uint8_t &eValue, ModelFunction iModelFunction)
{
    // we get here the input according to given model funtion as byte or bit
    bool lResult = (Mode() == Connected);
    if (lResult)
    {
        if (iModelFunction == IoByte || iModelFunction == Default)
        {
            eValue = mValue;
        }
        else if (iModelFunction < IoByte && iModelFunction <= IoBit1)
        {
            eValue = (mValue & (1 << (iModelFunction - 1))) ? 1 : 0;
        }
        else
        {
            lResult = false;
        }
    }
    return lResult;
}