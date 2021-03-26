#include "Sensor.h"
#include "SensorSHT3x.h"
#include "SensorBME280.h"
#include "SensorBME680.h"
#include "SensorSCD30.h"
#include "SensorIAQCore.h"
#include "SensorSGP30.h"
#include "SensorOPT300x.h"
#include "SensorVL53L1X.h"

Sensor* newSensor(uint8_t iSensorClass, MeasureType iMeasureType) {
    switch (iSensorClass)
    {
        case SENS_SHT3X:
            return new SensorSHT3x(iMeasureType);
            break;

        case SENS_BME280:
            return new SensorBME280(iMeasureType);
            break;

        case SENS_BME680:
            return new SensorBME680(iMeasureType);
            break;

        case SENS_SCD30:
            return new SensorSCD30(iMeasureType);
            break;

        case SENS_IAQCORE:
            return new SensorIAQCore(iMeasureType);
            break;

        case SENS_OPT300X:
            return new SensorOPT300x(iMeasureType);
            break;

        case SENS_VL53L1X:
            return new SensorVL53L1X(iMeasureType);
            break;

        case SENS_SGP30:
            return new SensorSGP30(iMeasureType);
            break;

        default:
            return nullptr;
            break;
    }
}
