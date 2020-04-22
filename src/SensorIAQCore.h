#pragma once
#include <IAQCore.h>
#include "Sensor.h"

class SensorIAQCore : public Sensor
{
  private:
    void startReadingData();
    bool getSensorData();

  protected:
    float mCo2 = NAN;
    float mVoc = NAN;
    float mAccuracy = 0.0;
    uint32_t mAccuracyDelay = 0;
    
    void sensorLoopInternal() override;
    float measureValue(MeasureType iMeasureType) override;

  public:
    SensorIAQCore(uint8_t iMeasureTypes, uint8_t iAddress);
    virtual ~SensorIAQCore() {}
    bool begin() override;
};
