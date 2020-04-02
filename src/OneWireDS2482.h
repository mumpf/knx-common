#pragma once
#include <inttypes.h>
#include "OneWire.h"
#include "OneWireSearchFirst.h"

// Chose between a table based CRC (flash expensive, fast)
// or a computed CRC (smaller, slow)
#define ONEWIRE_CRC8_TABLE 1

#define DS2482_COMMAND_RESET 0xF0 // Device reset

#define DS2482_COMMAND_SRP 0xE1 // Set read pointer
#define DS2482_POINTER_STATUS 0xF0
#define DS2482_STATUS_BUSY 0x01
#define DS2482_STATUS_PPD 0x02
#define DS2482_STATUS_SD 0x04
#define DS2482_STATUS_LL 0x08
#define DS2482_STATUS_RST 0x10
#define DS2482_STATUS_SBR 0x20
#define DS2482_STATUS_TSB 0x40
#define DS2482_STATUS_DIR 0x80
#define DS2482_POINTER_DATA 0xE1
#define DS2482_POINTER_CONFIG 0xC3
#define DS2482_CONFIG_APU 0x01
#define DS2482_CONFIG_SPU 0x04
#define DS2482_CONFIG_1WS 0x08

#define DS2482_COMMAND_WRITECONFIG 0xD2
#define DS2482_COMMAND_RESETWIRE 0xB4
#define DS2482_COMMAND_WRITEBYTE 0xA5
#define DS2482_COMMAND_READBYTE 0x96
#define DS2482_COMMAND_SINGLEBIT 0x87
#define DS2482_COMMAND_TRIPLET 0x78

#define WIRE_COMMAND_SKIP 0xCC
#define WIRE_COMMAND_SELECT 0x55
#define WIRE_COMMAND_SEARCH 0xF0

#define DS2482_ERROR_TIMEOUT 0x01
#define DS2482_ERROR_SHORT 0x02
#define DS2482_ERROR_CONFIG 0x04

class OneWireDS2482
{
  public:
    enum StateBM
    {
        Init,
        Startup,
        SearchIButton,
        ProcessIO,
        ProcessSensors,
        SearchNewDevices,
        Idle,
        Error
    };

    // callback definition, called if new id is found, with reference to newly created sensor
    typedef bool (*foundNewId)(OneWire *);

    OneWireDS2482(foundNewId iNewIdCallback);
    OneWireDS2482(uint8_t iI2cAddressOffset, foundNewId iNewIdCallback);

    void setup();
    void loop();

    uint8_t getI2cAddress();
    uint8_t getError();
    uint8_t checkI2cPresence();

    OneWire *Sensor(uint8_t iIndex);
    uint8_t SensorCount();

    //DS2482-800 only
    bool selectChannel(uint8_t channel);

    void deviceReset();
    void setReadPointer(uint8_t readPointer);
    uint8_t readStatus(bool iSetReadPoitner = true);
    bool readStatusShortDet();
    uint8_t readData();
    uint8_t waitOnBusy(bool iSetReadPointer = true);
    uint8_t readConfig();
    void writeConfig(uint8_t config);
    uint8_t crc8(const uint8_t *addr, uint8_t len);
    void setStrongPullup();
    void clearStrongPullup();
    void setActivePullup();
    void clearActivePullup();
    uint8_t wireReset();
    void wireWriteByte(uint8_t data, uint8_t power = 0);
    uint8_t wireReadByte();
    void wireWriteBit(uint8_t data, uint8_t power = 0);
    uint8_t wireReadBit();
    void wireSkip();
    void wireSelect(const tIdRef iId);
    uint8_t wireSearch(uint8_t *eAddress);
    OneWire *CreateOneWire(tIdRef iId);
    void begin();
    uint8_t end();
    void writeByte(uint8_t);
    uint8_t readByte();

  private:
    bool ProcessPriorityBusUse();
    bool ProcessNormalBusUse();

    OneWireSearch *mSearchPrio = NULL;
    OneWireSearch *mSearchNormal = NULL;

    uint8_t mI2cAddress;
    uint8_t mError;
    foundNewId fNewIdCallback = 0;

    StateBM mState = Init;
    uint32_t mDelay = 0;

    OneWire *mSensor[30];
    uint8_t mSensorCount = 0;
};
