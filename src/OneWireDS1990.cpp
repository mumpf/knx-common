#include <arduino.h>
#include <stdio.h>
#include <Wire.h>
#include "OneWire.h"
#include "OneWireDS1990.h"

OneWireDS1990::OneWireDS1990(OneWireDS2482 *iBM, tIdRef iId)
    : OneWire(iBM, iId) {
    pPrio = PrioHigh;
};

void OneWireDS1990::loop()
{
}

bool OneWireDS1990::getValue(uint8_t &eValue, uint8_t iModelFunction) {
    eValue = (pMode == Connected);
    return true;
}