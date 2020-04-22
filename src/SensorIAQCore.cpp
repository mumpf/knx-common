#include <Wire.h>
#include "SensorIAQCore.h"

SensorIAQCore::SensorIAQCore(uint8_t iMeasureTypes, uint8_t iAddress)
    : Sensor(iMeasureTypes, iAddress){};

void SensorIAQCore::sensorLoopInternal()
{
    switch (gSensorState)
    {
        case Wakeup:
            Sensor::sensorLoopInternal();
            break;
        case Calibrate:
            Sensor::sensorLoopInternal();
            break;
        case Finalize:
            if (delayCheck(gSensorStateDelay, 1000)) {
                // start first measurement
                if (getSensorData()) 
                    gSensorState = Running;
                gSensorStateDelay = millis();
            }
            break;
        case Running:
            if (delayCheck(gSensorStateDelay, 2000)) {
                getSensorData();
                gSensorStateDelay = millis();
            }
            break;
        default:
            gSensorStateDelay = millis();
            break;
    }
}

float SensorIAQCore::measureValue(MeasureType iMeasureType)
{
    switch (iMeasureType)
    {
    case Voc:
        return mVoc;
        break;
    case Co2Calc:
        return mCo2;
        break;
    case Accuracy:
        if (mAccuracy < 100.0 && delayCheck(mAccuracyDelay, 5*60*1000)) 
            mAccuracy = 100.0;
        return mAccuracy;
    default:
        break;
    }
    return NAN;
}

bool SensorIAQCore::begin()
{
    bool lResult = false;
    printDebug("Starting sensor IAQCore... ");
    lResult = Sensor::begin();
    printResult(lResult);
    mAccuracyDelay = millis();
    return lResult;
}

void SensorIAQCore::startReadingData() {
    Wire.beginTransmission(gAddress);
    Wire.write(IAQCORE_START_READING);
    Wire.endTransmission();
}

bool SensorIAQCore::getSensorData()
{
    uint8_t lBuffer[9];

    // clear read buffer
    memset(lBuffer, 0, sizeof(lBuffer));
    // start reading data
    startReadingData();
    // request sensor data
    Wire.requestFrom(gAddress, 9, true);
    if (Wire.available() != 9)
        return false;
    for (uint8_t i = 0; i < 9; i++)
    {
        lBuffer[i] = Wire.read();
    }
    if (lBuffer[IAQCORE_STATUS_OFFSET] != IAQCORE_STATUS_OK)
        return false;

    mCo2 = (float)(((uint16_t)lBuffer[IAQCORE_CO2_PREDICTION_MSB_OFFSET] << 8) | lBuffer[IAQCORE_CO2_PREDICTION_LSB_OFFSET]);
    mVoc = (float)(((uint16_t)lBuffer[IAQCORE_TVOC_PREDICTION_MSB_OFFSET] << 8) | lBuffer[IAQCORE_TVOC_PREDICTION_LSB_OFFSET]);

    return true;
}
