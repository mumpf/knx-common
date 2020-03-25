#include <arduino.h>
#include <stdio.h>
#include <Wire.h>
#include "OneWire.h"
#include "OneWireDS1990.h"

OneWireDS1990::OneWireDS1990(OneWireDS2482 *iBM, tIdRef iId)
    : OneWire(iBM, iId){};

void OneWireDS1990::loop()
{
}

void OneWireDS1990::setModeDisconnected(bool iForce /* = false */)
{
    // most of the devices are stationary and do not allow any disconnect
    // override this method for mobile devices like iButton
    if (iForce || pSearchCount >= cMaxCount)
    {
        pMode = Disconnected;
        OneWire::setModeDisconnected(iForce);
    }
}
