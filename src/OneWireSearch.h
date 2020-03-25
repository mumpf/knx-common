#pragma once
#include <inttypes.h>
#include <Arduino.h>
#include "OneWire.h"
// #include <OneWireDS2482.h>

// forward declaration
class OneWireDS2482;

// base class for different search implementations
class OneWireSearch
{
  public:
    enum StateSearch
    {
        SearchNew,
        SearchReset,
        SearchStart,
        SearchStep,
        SearchEnd,
        SearchError,
        SearchFinished
    };

    OneWireSearch(OneWireDS2482 *iBM);

    bool loop();
    void reset();
    StateSearch state();

  protected:
    OneWireDS2482 *mBM = NULL;
    uint32_t mDelay = 0;
    
    StateSearch mStateSearch = SearchNew;

    // search buffer has to be 8 Byte, 
    // part of search result is crc byte
    uint8_t mSearchResultId[8]; 
    uint8_t mSearchLastDiscrepancy;
    uint8_t mSearchLastFamilyDiscrepancy;
    uint8_t mSearchLastDeviceFlag;
    uint8_t mSearchLastZero = 0;
    uint8_t mSearchStep = 0;

    virtual void wireSearchNew(uint8_t iFamilyCode = 0) = 0;
    virtual void wireSearchReset() = 0;
    virtual bool wireSearchStart(uint8_t iStatus) = 0;
    virtual bool wireSearchStep(uint8_t iStep) = 0;
    virtual bool wireSearchEnd() = 0;
    virtual bool wireSearchFinished(bool iIsError) = 0;
    virtual uint8_t wireSearchBlocking(tIdRef eAddress) = 0;
};
