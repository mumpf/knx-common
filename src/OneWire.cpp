#include "OneWire.h"
#include "OneWireDS2482.h"

bool equalId(const tIdRef iId1, const tIdRef iId2)
{
    for (uint8_t i = 1; i < 7; i++)
        if (iId1[i] != iId2[i])
            return false;
    return (iId1[0] == iId2[0]);
}

bool equalId(const tIdRef iId1, const int32_t* iId2)
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
    return pMode;
}

void OneWire::setModeConnected(bool iForce /* = false */)
{
    if (iForce || pSearchCount >= cMaxCount) {
        pMode = Connected;
        pSearchCount = 0;
    }
}

void OneWire::setModeDisconnected(bool iForce /* = false */)
{
    if (iForce || pSearchCount >= cMaxCount)
    {
        pMode = Disconnected;
        pSearchCount = 0;
    }
}

void OneWire::clearSearchCount() {
    pSearchCount = 0;
}

void OneWire::incrementSearchCount() {
    if (pMode != New) pSearchCount++;
}

void OneWire::wireSelectThisDevice() {
    pBM->wireReset();
    pBM->wireSelect(pId);
}

bool OneWire::getValue(float& eValue)
{
    return false;
}

bool OneWire::getValue(uint8_t& eValue)
{
    return false;
}

bool OneWire::setValue(uint8_t iValue)
{
    // do nothing, should be overridden for devices with output capabilities
    return false;
}
