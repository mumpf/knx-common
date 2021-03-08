#pragma once
#include <Adafruit_BME280.h>
#include "Sensor.h"

#define BME280_I2C_ADDR (0x76)

class SensorBME280 : public Sensor, protected Adafruit_BME280
{

protected:
    float measureValue(MeasureType iMeasureType) override;
    void sensorLoopInternal() override;
    bool initWakeup();
    bool initFinalize();

public:
    SensorBME280(uint16_t iMeasureTypes, uint8_t iAddress);
    virtual ~SensorBME280() {}
    bool begin() override; 
};
