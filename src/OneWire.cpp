#include "OneWire.h"
#include "OneWireDS2482.h"

bool equalId(const tIdRef iId1, const tIdRef iId2)
{
    for (uint8_t i = 1; i < 7; i++)
        if (iId1[i] != iId2[i])
            return false;
    return (iId1[0] == iId2[0]);
}

bool copyId(tIdRef iIdLeft, const tIdRef iIdRight)
{
    for (uint8_t i = 0; i < 7; i++)
        iIdLeft[i] = iIdRight[i];
}

OneWire::OneWire(OneWireDS2482* iBM, tIdRef iId)
{
    pBM = iBM;
    copyId(pId, iId);
};

OneWire::~OneWire(){};

OneWire::SensorMode OneWire::Mode() {
    return mMode;
}

void OneWire::setModeKnown() {
    mMode = Known;
}

void OneWire::wireSelectThisDevice() {
    pBM->wireReset();
    pBM->wireSelect(pId);
}
