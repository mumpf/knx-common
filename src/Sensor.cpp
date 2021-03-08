#include <Arduino.h>
#include <Wire.h>
#include "Sensor.h"

// static
uint8_t Sensor::sNumSensors = 0;
Sensor* Sensor::sSensors[SENSOR_COUNT];

Sensor::Sensor(uint16_t iMeasureTypes, uint8_t iAddress)
{
    if (sNumSensors >= SENSOR_COUNT)
    {
        // println("Sensor::Sensor() - Currently only 2 (Hardware)Sensors are supported");
        //fatal error handling
        return;
    }
    gMeasureTypes = iMeasureTypes;
    gAddress = iAddress;
    sSensors[sNumSensors++] = this;
};

// static
void Sensor::sensorLoop() {
    for (uint8_t lCounter = 0; lCounter < sNumSensors; lCounter++)
        sSensors[lCounter]->sensorLoopInternal();
    }

// static
void Sensor::restartSensors() {
    for (uint8_t lCounter = 0; lCounter < sNumSensors; lCounter++)
        sSensors[lCounter]->restartSensor();
}

void Sensor::restartSensor() {
    // gSensorStateDelay = 0;
    gSensorState = Wakeup;
}

void Sensor::changeSensorOrder(Sensor *iSensor, uint8_t iPosition){
    // first check, if the sensor is already at his position
    if (sSensors[iPosition] == iSensor) return;
    // as long as we have just 2 Sensors, new position is a simple exchange
    int8_t lNewPosition = abs(iPosition - 1);
    sSensors[lNewPosition] = sSensors[iPosition];
    sSensors[iPosition] = iSensor;
}

bool Sensor::checkSensorConnection()
{
    bool lResult = false;
    // if (gSensorState == Running) {
        // ceck for I2C ack
        Wire.beginTransmission(gAddress);
        lResult = (Wire.endTransmission() == 0);
        if (!lResult)
            restartSensor();
    // }
    return lResult;
}

void Sensor::sensorLoopInternal() {
    switch (gSensorState)
    {
    case Wakeup:
        // try immediately to start the sensor, then every second
        if (gSensorStateDelay == 0 || delayCheck(gSensorStateDelay, 1000)) {
            if (begin()) gSensorState = Calibrate;
            gSensorStateDelay = millis();
        }
        break;
    case Calibrate:
        // no calibration necessary
        gSensorState = Finalize;
        break;
    case Finalize:
        // give the sensor 100 ms before querying starts
        if (delayCheck(gSensorStateDelay, 100)) gSensorState = Running;
        break;
    default:
        gSensorStateDelay = 0;
        break;
    }
}

bool Sensor::begin() {
    // gSensorState = Running;
    return checkSensorConnection();
}

// should be overridden, if there is a state to save before power failure
void Sensor::sensorSaveState() {};

// static
void Sensor::saveState() {
    // dispatch the call to all sensors
    for (uint8_t lCounter = 0; lCounter < sNumSensors; lCounter++)
        sSensors[lCounter]->sensorSaveState();
}

// static
bool Sensor::measureValue(MeasureType iMeasureType, float& eValue) {
    bool lResult = false;
    for (uint8_t lCounter = 0; lCounter < sNumSensors; lCounter++)
    {
        if (sSensors[lCounter]->gMeasureTypes & iMeasureType) {
            lResult = sSensors[lCounter]->checkSensorConnection();
            if (lResult)
                lResult = sSensors[lCounter]->gSensorState == Running;
            if (lResult) eValue = sSensors[lCounter]->measureValue(iMeasureType);
            break;
        }
    }
    return lResult;
}

//static
uint8_t Sensor::getError() {
    uint8_t lResult = 0;
    // for (uint8_t lCounter = 0; lCounter < sNumSensors; lCounter++)
    // {
    //     if (sSensors[lCounter]->gSensorState != Running) {
    //         lResult |= sSensors[lCounter]->gMeasureTypes;
    //     }
    // }
    return lResult;
}