#pragma once
#include <Helper.h>

/**
 * OneWire Commands
 **/
#define ONEWIRE_READ_CMD 0x33
#define ONEWIRE_MATCH_CMD 0x55
#define ONEWIRE_SEARCH_CMD 0xF0
#define ONEWIRE_SKIP_CMD 0xCC
#define ONEWIRE_CONDITIONAL_SEARCH_CMD 0xEC
#define ONEWIRE_OVERRIDE_MATCH_CMD 0x69
#define ONEWIRE_OVERRIDE_SKIP_CMD 0x3C
#define ONEWIRE_RESUME_CMD 0xA5

// Model IDs
#define MODEL_DS18S20 0x10  // Temp-Sensor
#define MODEL_DS18B20 0x28
#define MODEL_DS1822 0x22
#define MODEL_DS1825 0x3B
#define MODEL_DS2436 0x26   // Battery meter
#define MODEL_DS2413 0x3A   // 2-Channel IO
#define MODEL_DS2408 0x29   // 8-Channel IO
#define MODEL_DS1990 0x01   // iButton

#define ModelFunction_Default 0
#define ModelFunction_IoBit0  1
#define ModelFunction_IoBit1  2
#define ModelFunction_IoBit2  3
#define ModelFunction_IoBit3  4
#define ModelFunction_IoBit4  5
#define ModelFunction_IoBit5  6
#define ModelFunction_IoBit6  7
#define ModelFunction_IoBit7  8
#define ModelFunction_IoByte  9
#define ModelFunction_TemperatureOnChip 10
#define ModelFunction_TemperatureNTC    11
#define ModelFunction_Humidity 12
#define ModelFunction_Brightness 13
#define ModelFunction_CustomMin 50
#define ModelFunction_CustomMax 79

// we store only 7 Bytes for Id, because byte 8 is CRC and just important for bus data transfer
typedef uint8_t tId[7];
typedef uint8_t *tIdRef;

bool equalId(const tIdRef iId1, const tIdRef iId2);
bool equalId(const tIdRef iId1, const int32_t* iId2);
bool copyId(tIdRef iIdLeft, const tIdRef iIdRight);

// forward declaration
class OneWireDS2482;

class OneWire {
  public:
    enum SensorMode
    {
        New,    // Sensor on 1W bus, but not known by application
        Connected,  // Sensor on 1W bus and known by application
        Disconnected // Sensor removed from 1W bus, but known by appliation
    };

    enum ModelParameter
    {
      	IoMask,
        IoInvertMask,
        MeasureResolution
    };

    enum SensorPriority
    {
        PrioLow,     // sensors like Temp, Hum etc
        PrioNormal,  // IO sensors
        PrioHigh     // iButtons
    };

    OneWire(OneWireDS2482 *iBM, tIdRef iId);
    ~OneWire();
    // class members
    // instance members
    virtual void loop() = 0;
    tIdRef Id() { return pId; }
    uint8_t Family() { return pId[0]; }
    SensorPriority Prio() { return pPrio; };
    SensorMode Mode();
    void setModeConnected(bool iForce = false);
    void setModeDisconnected(bool iForce = false);
    void incrementSearchCount();
    void clearSearchCount();
    virtual bool getValue(float &eValue, uint8_t iModelFunction);
    virtual bool getValue(uint8_t &eValue, uint8_t iModelFunction);
    virtual bool setValue(uint8_t iValue, uint8_t iModelFunction);
    virtual bool setParameter(ModelParameter iModelParameter, uint8_t iValue, uint8_t iModelFunction);

  protected:
    void wireSelectThisDevice();

    tId pId = { 0 };
    OneWireDS2482 *pBM; //ref to BusMaster
    uint32_t pDelay = 0;
    bool pValid = false; // is Value really valid?

    // just test
    const int cMaxCount = 2;

    SensorMode pMode = New;
    SensorPriority pPrio = PrioLow;
    uint8_t pSearchCount = 0;
};
