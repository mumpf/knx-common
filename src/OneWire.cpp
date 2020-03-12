#include "OneWire.h"

OneWire::OneWire(OneWireDS2482 *iBM, uint8_t *iId)
{
    pId = iId;
    pBM = iBM;
};

OneWire::~OneWire(){};

