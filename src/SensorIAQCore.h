#pragma once
#include <IAQCore.h>
#include "Sensor.h"

class SensorIAQCore : public Sensor
{

  protected:
    float measureValue(MeasureType iMeasureType) override;

  public:
    SensorIAQCore(uint8_t iMeasureTypes, uint8_t iAddress);
    virtual ~SensorIAQCore() {}
    bool begin() override;
};
