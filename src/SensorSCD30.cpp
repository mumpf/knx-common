#include "SensorSCD30.h"

SensorSCD30::SensorSCD30(uint8_t iMeasureTypes, uint8_t iAddress)
    : Sensor(iMeasureTypes, iAddress), SCD30() {};

double SensorSCD30::measureValue(MeasureType iMeasureType) {
    switch (iMeasureType)
    {
    case Temperature:
        return getTemperature();
        break;
    case Humidity:
        return getHumidity();
        break;
    case Co2:
        return getCO2();
        break;
    case Reserved:
        return getCO2();
        break;
    default:
        break;
    }
    return -1000.0;
}

bool SensorSCD30::begin() {
    printDebug("Starting sensor SCD30... ");
    bool lResult = SCD30::begin();
    if (lResult) lResult = Sensor::begin();
    printResult(lResult);
    return lResult;
}

