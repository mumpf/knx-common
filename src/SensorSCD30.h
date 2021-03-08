#pragma once
#include <SparkFun_SCD30_Arduino_Library.h>
#include "Sensor.h"

#define SCD30_I2C_ADDR 0x61

class SensorSCD30 : public Sensor, protected SCD30
{

protected:
    float measureValue(MeasureType iMeasureType) override;

public:
    SensorSCD30(uint16_t iMeasureTypes, uint8_t iAddress);
    virtual ~SensorSCD30() {}
    bool begin() override; 
    void sensorLoopInternal() override;
};
