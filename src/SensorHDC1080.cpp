#include "SensorHDC1080.h"

SensorHDC1080::SensorHDC1080(uint8_t iMeasureTypes, uint8_t iAddress)
    : Sensor(iMeasureTypes, iAddress), ClosedCube_HDC1080() {};

double SensorHDC1080::measureValue(MeasureType iMeasureType) {
    switch (iMeasureType)
    {
    case Temperature:
        // hardware calibration
        return readTemperature() - 2.0;
        break;
    case Humidity:
        return readHumidity();
        break;
    default:
        break;
    }
    return -1000.0;
}

bool SensorHDC1080::begin() {
    bool lResult = false;
    printDebug("Starting sensor HDC1080... ");
    ClosedCube_HDC1080::begin(gAddress);
    lResult = Sensor::begin();
    printResult(lResult);
    return lResult;
}

