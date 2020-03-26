#include "SensorSCD30.h"

SensorSCD30::SensorSCD30(uint8_t iMeasureTypes, uint8_t iAddress)
    : Sensor(iMeasureTypes, iAddress), SCD30() {};

float SensorSCD30::measureValue(MeasureType iMeasureType) {
    switch (iMeasureType)
    {
    case Temperature:
        // hardware calibration
        return getTemperature() - 3.0f;
        break;
    case Humidity:
        return getHumidity();
        break;
    case Co2:
        return getCO2();
        break;
    default:
        break;
    }
    return -1000.0f;
}

bool SensorSCD30::begin() {
    printDebug("Starting sensor SCD30... ");
    bool lResult = SCD30::begin();
    if (lResult) lResult = Sensor::begin();
    printResult(lResult);
    return lResult;
}

void SensorSCD30::sensorLoopInternal() {
    switch (gSensorState)
    {
    case Wakeup:
        Sensor::sensorLoopInternal();
        break;
    case Calibrate:
        Sensor::sensorLoopInternal();
        break;
    case Finalize:
        // we ask for Temperature until we get a valid value
        if (delayCheck(gSensorStateDelay, 200)) {
            if (getTemperature() > 0) gSensorState = Running;
            gSensorStateDelay = millis();
        }
        break;
    default:
        Sensor::sensorLoopInternal();
        break;
    }
}

