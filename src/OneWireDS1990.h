#pragma once

#include <inttypes.h>
#include "OneWire.h"
#include "OneWireDS2482.h"

class OneWireDS1990 : public OneWire
{
  public:
    OneWireDS1990(OneWireDS2482 *iBusMaster, tIdRef iId);

    void loop() override;
    void setModeDisconnected(bool iForce = false) override;

  private:
};