#pragma once
#include <Helper.h>
#include "OneWireDS2482.h"

class OneWire {
  public:
    OneWire(OneWireDS2482 *iBM, uint8_t *iId);
    ~OneWire();
    // class members
    // instance members
    virtual void loop() = 0;
    uint8_t Id() { return *pId; }

  protected:
    uint8_t *pId;
    OneWireDS2482 *pBM; //ref to BusMaster
    uint32_t pDelay = 0;
};
