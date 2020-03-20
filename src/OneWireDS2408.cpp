#include <arduino.h>
#include <stdio.h>
#include <Wire.h>
#include "OneWire.h"
#include "OneWireDS2408.h"

OneWireDS2408::OneWireDS2408(OneWireDS2482 *iBM, tIdRef iId) : OneWire(iBM, iId){
};

void OneWireDS2408::init()
{
}

void OneWireDS2408::loop() {
  switch (mState) 
  {
  case Startup:
      mState = Idle;
      break;
  
  default:
    break;
  }
}

// bool OneWireDS2408::set_LED_DS2408(uint8_t led, bool state) {

//   byte temp = get_state();

//   if (state)
//     bitClear(temp, led);
//   else
//     bitSet(temp, led);

//    return set_state(temp);
// }

uint8_t OneWireDS2408::getRegister(uint16_t iRegister)
{
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2408_PIO_READ_CMD);
    pBM->wireWriteByte(REG_LO(iRegister));
    pBM->wireWriteByte(REG_HI(iRegister));
    return pBM->wireReadByte();
}

void OneWireDS2408::setRegister(uint16_t iRegister, uint8_t iValue)
{
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2408_SEARCH_CMD);
    pBM->wireWriteByte(REG_LO(iRegister));
    pBM->wireWriteByte(REG_HI(iRegister));
    pBM->wireWriteByte(iValue);
}

uint8_t OneWireDS2408::getState() {
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2408_CHANNEL_READ_CMD);
    return pBM->wireReadByte();
}

bool OneWireDS2408::setState(uint8_t iState) {
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2408_CHANNEL_WRITE_CMD);
    pBM->wireWriteByte(iState);
    pBM->wireWriteByte(~iState);
    if (pBM->wireReadByte() == 0xAA && pBM->wireReadByte() == iState)
        return true;
    return false;
}

void OneWireDS2408::setSearchMask(uint8_t iMask) {
    wireSelectThisDevice();
    setRegister(DS2408_SEARCH_MASK_REG, iMask);
}

void OneWireDS2408::setSearchPolarity(uint8_t iPolarity) {
    wireSelectThisDevice();
    setRegister(DS2408_SEARCH_SELECT_REG, iPolarity);
}

void OneWireDS2408::setMode(uint8_t iMode) {
    wireSelectThisDevice();
    setRegister(DS2408_CONTROL_STATUS_REG, iMode);
}

uint8_t OneWireDS2408::getMode() {
    wireSelectThisDevice();
    return getRegister(DS2408_CONTROL_STATUS_REG);
}

uint8_t OneWireDS2408::getCurrentState() {
    wireSelectThisDevice();
    return getRegister(DS2408_PIO_LOGIC_REG);
}

uint8_t OneWireDS2408::getLastState() {
    wireSelectThisDevice();
    return getRegister(DS2408_PIO_OUTPUT_REG);
}

uint8_t OneWireDS2408::getActivity() {
    wireSelectThisDevice();
    return getRegister(DS2408_PIO_ACTIVITY_REG);
}

bool OneWireDS2408::resetActivity() {
    wireSelectThisDevice();
    pBM->wireWriteByte(DS2408_RESET_CMD);
    return (pBM->wireReadByte() == 0xAA);
}

