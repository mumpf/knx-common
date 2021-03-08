#include "SensorBME280.h"

SensorBME280::SensorBME280(uint16_t iMeasureTypes, uint8_t iAddress)
    : Sensor(iMeasureTypes, iAddress), Adafruit_BME280() {};

/*!
 * @brief implements a state engine to restart the sensor without delays
 */
void SensorBME280::sensorLoopInternal() {
    switch (gSensorState)
    {
    case Wakeup:
        if (gSensorStateDelay == 0 || delayCheck(gSensorStateDelay, 1000)) {
            if (initWakeup()) gSensorState = Calibrate;
            gSensorStateDelay = millis();
        }
        break;
    case Calibrate:
        if (delayCheck(gSensorStateDelay, 100))
        {
            if (!isReadingCalibration()) {
                initFinalize();
                gSensorState = Finalize;
            }
            gSensorStateDelay = millis();
        }
        break;
    case Finalize:
        Sensor::sensorLoopInternal();
        break;
    default:
        Sensor::sensorLoopInternal();
        break;
    }
}

/*!
 *   @brief  Initialise sensor with given parameters / settings, skips all delay functions, expects correct external call sequence
 *   @returns true on success, false otherwise
 */
bool SensorBME280::initWakeup()
{
    // init I2C interface
    _wire->begin();

    // check if sensor, i.e. the chip ID is correct
    _sensorID = read8(BME280_REGISTER_CHIPID);
    if (_sensorID != 0x60)
        return false;

    // reset the device using soft-reset
    // this makes sure the IIR is off, etc.
    write8(BME280_REGISTER_SOFTRESET, 0xB6);
    return true;
}

bool SensorBME280::initFinalize() {
    readCoefficients(); // read trimming parameters, see DS 4.2.2
    setSampling(); // use defaults
    return true;
}

float SensorBME280::measureValue(MeasureType iMeasureType) {
    switch (iMeasureType)
    {
    case Temperature:
        // hardware calibration
        return readTemperature() - 2.0f;
        break;
    case Humidity:
        return readHumidity();
        break;
    case Pressure:
        return readPressure();
    default:
        break;
    }
    return -1000.0;
}

bool SensorBME280::begin() {
    printDebug("Starting sensor BME280... ");
    bool lResult = Adafruit_BME280::begin(gAddress);
    if (lResult)
        lResult = Sensor::begin();
    printResult(lResult);
    return lResult;
}