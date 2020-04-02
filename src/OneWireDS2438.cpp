#include <arduino.h>
#include <stdio.h>
#include <Wire.h>
#include "OneWire.h"
#include "OneWireDS2438.h"

OneWireDS2438::OneWireDS2438(OneWireDS2482 *iBM, tIdRef iId)
    : OneWire(iBM, iId){};

void OneWireDS2438::loop()
{
}

