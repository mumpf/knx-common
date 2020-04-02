#pragma once

#include <inttypes.h>
#include "OneWire.h"
#include "OneWireDS2482.h"

class OneWireDS2438 : public OneWire
{
  public:
    OneWireDS2438(OneWireDS2482 *iBusMaster, tIdRef iId);

    void loop() override;

  private:
};