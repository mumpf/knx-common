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
  switch (mState) 
  {
  case Startup:
      mState = Idle;
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
    if (pBM->wireReadByte() == 0xAA && pBM->wireReadByte() == iState)
        return true;
    return false;
}
