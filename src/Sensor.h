#pragma once
// #include <knx/bits.h>
#include "Helper.h"

enum SensorState {
    Off,
    Wakeup,
    Calibrate,
    Finalize,
    Running
};

enum MeasureType {
    OneWire = 1,
    Temperature = 2,
    Humidity = 4,
    Pressure = 8,
    Voc = 16,
    Co2 = 32,
    Co2Calc = 64, // calculated Co2 from VOC
    Accuracy = 128
};

class Sensor
{
  private:
    static Sensor* sSensors[2];
    static uint8_t sNumSensors;
    uint8_t gMeasureTypes;

  protected:
    // Sensor();
    uint8_t gAddress;
    SensorState gSensorState = Off;
    uint32_t gSensorStateDelay = 0;

    bool checkSensorConnection();
    virtual double measureValue(MeasureType iMeasureType) = 0; //pure
    virtual void sensorLoopInternal();
    virtual void sensorSaveState();
    // non blocking restart approach for a sensor
    void restartSensor();

  public:
    Sensor(uint8_t iMeasureTypes, uint8_t iAddress);
    virtual ~Sensor() {}

    // static 
    static void sensorLoop();
    static bool measureValue(MeasureType iMeasureType, double& eValue);
    static uint8_t getError();
    static void saveState();
    static void restartSensors();
    static void changeSensorOrder(Sensor *iSensor, uint8_t iPosition);

    virtual bool begin(); // first initialization, may be blocking, should be called druing setup(), not during loop()
};
