#pragma once
// #include <knx/bits.h>
#include "Helper.h"

#define SENSOR_COUNT 2

#define BIT_1WIRE 1
#define BIT_Temp 2
#define BIT_Hum 4
#define BIT_Pre 8
#define BIT_Voc 16
#define BIT_Co2 32
#define BIT_RESERVE 64
#define BIT_LOGIC 128
#define BIT_LUX 256
#define BIT_TOF 512

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
    Accuracy = 128,
    Lux = 256,
    Tof = 512
};

class Sensor
{
  private:
    static Sensor* sSensors[SENSOR_COUNT];
    static uint8_t sNumSensors;
    uint16_t gMeasureTypes;

  protected:
    // Sensor();
    uint8_t gAddress;
    SensorState gSensorState = Wakeup;
    uint32_t gSensorStateDelay = 0;

    bool checkSensorConnection();
    virtual float measureValue(MeasureType iMeasureType) = 0; //pure
    virtual void sensorLoopInternal();
    virtual void sensorSaveState();
    // non blocking restart approach for a sensor
    void restartSensor();

  public:
    Sensor(uint16_t iMeasureTypes, uint8_t iAddress);
    virtual ~Sensor() {}

    // static 
    static void sensorLoop();
    static bool measureValue(MeasureType iMeasureType, float& eValue);
    static uint8_t getError();
    static void saveState();
    static void restartSensors();
    static void changeSensorOrder(Sensor *iSensor, uint8_t iPosition);

    virtual bool begin(); // first initialization, may be blocking, should be called druing setup(), not during loop()
};
