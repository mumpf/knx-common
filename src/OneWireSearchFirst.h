#pragma once
#include <inttypes.h>
#include "OneWireSearch.h"
#include "OneWireDS2482.h"

class OneWireSearchFirst : public OneWireSearch
{
  public:
    OneWireSearchFirst(OneWireDS2482 *iBM);

  protected:
    void wireSearchNew() override;
    void wireSearchNext() override;
    bool wireSearchStart(uint8_t iStatus) override;
    bool wireSearchStep(uint8_t iStep) override;
    bool wireSearchEnd() override;
    bool wireSearchFinished(bool iIsError) override;
    uint8_t wireSearchBlocking(tIdRef eAddress) override;
};
