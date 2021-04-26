#include "Sensor.h"
#include "Hardware.h"
#include "SensorDummy.h"
#include "SensorSHT3x.h"
#include "SensorBME280.h"
#include "SensorBME680.h"
#include "SensorSCD30.h"
#include "SensorSCD40.h"
#include "SensorIAQCore.h"
#include "SensorSGP30.h"
#include "SensorOPT300x.h"
#include "SensorVL53L1X.h"

Sensor* newSensor(uint8_t iSensorClass, MeasureType iMeasureType) {
    Sensor* lSensor = nullptr;
    switch (iSensorClass)
    {
        case SENS_SHT3X:
            lSensor = new SensorSHT3x(iMeasureType);
            break;

        case SENS_BME280:
            lSensor = new SensorBME280(iMeasureType);
            break;

        case SENS_BME680:
            lSensor = new SensorBME680(iMeasureType);
            break;

        case SENS_SCD30:
            lSensor = new SensorSCD30(iMeasureType);
            break;

        case SENS_SCD41:
            lSensor = new SensorSCD40(iMeasureType);
            break;

        case SENS_IAQCORE:
            lSensor = new SensorIAQCore(iMeasureType);
            break;

        case SENS_OPT300X:
            lSensor = new SensorOPT300x(iMeasureType);
            break;

        case SENS_VL53L1X:
            lSensor = new SensorVL53L1X(iMeasureType);
            break;

        case SENS_SGP30:
            lSensor = new SensorSGP30(iMeasureType);
            break;

        default:
            lSensor = nullptr;
            break;
    }
    if (lSensor == nullptr) {
        lSensor = new SensorDummy(iMeasureType);
    }
    if (lSensor == nullptr) {
        fatalError(FATAL_SENS_UNKNOWN, "Unknown Sensor or sensor creation error!");
    }
    return lSensor;
}
