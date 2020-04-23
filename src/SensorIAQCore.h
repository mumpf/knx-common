#pragma once
#include <IAQCore.h>
#include "Sensor.h"

class SensorIAQCore : public Sensor
{
  private:
    uint8_t mBuffer[IAQCORE_READ_ALL];
    bool getSensorData();

  protected:
    float mCo2 = NAN;
    float mVoc = NAN;
    
    void sensorLoopInternal() override;
    float measureValue(MeasureType iMeasureType) override;

  public:
    SensorIAQCore(uint8_t iMeasureTypes, uint8_t iAddress);
    virtual ~SensorIAQCore() {}
    bool begin() override;
};
