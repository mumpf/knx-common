#include "OneWireSearchFirst.h"
#include "OneWireDS2482.h"

OneWireSearchFirst::OneWireSearchFirst(OneWireDS2482* iBM)
    : OneWireSearch(iBM)
{
}

//  1-Wire reset search algorithm
void OneWireSearchFirst::wireSearchNew()
{
    // reset search fields
	mSearchLastDiscrepancy = -1;
	mSearchLastDeviceFlag = 0;
    mSearchLastFamilyDiscrepancy = 0;

    // clear search buffer^
    if (mSearchMode != Id) {
    }
    // set all known devices in search mode incrementing their search count
    manageSearchCounter(SearchNew);
    
    if (mSearchMode == Family || mSearchMode == Id) {
        mSearchResultId[0] = mSearchFamily;
        mSearchLastDiscrepancy = 64;
    }
    // reset bus (see wireReset) in non blocking way
    mBM->waitOnBusy();
    // Datasheet warns that reset with SPU set can exceed max ratings
    mBM->clearStrongPullup();
}

void OneWireSearchFirst::wireSearchNext() {
    mBM->waitOnBusy();
    mBM->begin();
    mBM->writeByte(DS2482_COMMAND_RESETWIRE);
    mBM->end();
#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("### Reset sent ###\n");
#endif
}

bool OneWireSearchFirst::wireSearchStart(uint8_t iStatus)
{
    mSearchStep = 0;
    mSearchLastZero = -1;
    if (mSearchLastDeviceFlag)
        return false;
    // if (!wireReset())
    //     return false;
    if (iStatus & DS2482_STATUS_SD)
    {
        // mError = DS2482_ERROR_SHORT;
        return false;
    } else {
        mBM->setActivePullup();
    }

    if (!(iStatus & DS2482_STATUS_PPD)) return false;
    mBM->waitOnBusy(false);
    mBM->wireWriteByte(WIRE_COMMAND_SEARCH);
    mBM->waitOnBusy(false);
#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("### Search command sent ###\n");
#endif
    return true;
}

bool OneWireSearchFirst::wireSearchStep(uint8_t iStep) {
    uint8_t lDirection;
    
	int lSearchByte = iStep / 8;
    int lSearchBit = 1 << iStep % 8;

#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("Step %02d: ", iStep);
#endif

    if (iStep < mSearchLastDiscrepancy)
        lDirection = mSearchResultId[lSearchByte] & lSearchBit;
    else
        lDirection = (iStep == mSearchLastDiscrepancy);

    mBM->begin();
    mBM->writeByte(DS2482_COMMAND_TRIPLET);
    mBM->writeByte(lDirection ? 0x80 : 0x00);
    mBM->end();

    uint8_t lStatus = mBM->waitOnBusy(false);
    uint8_t lId = lStatus & DS2482_STATUS_SBR;
    uint8_t lCompId = lStatus & DS2482_STATUS_TSB;
    lDirection = lStatus & DS2482_STATUS_DIR;

    if (lId && lCompId) {
        return false;
    } else if (!lId && !lCompId && !lDirection) {
        mSearchLastZero = iStep;
        // check for Last discrepancy in family
        if (mSearchLastZero < 8)
            mSearchLastFamilyDiscrepancy = mSearchLastZero;
    }
    if (lDirection)
        mSearchResultId[lSearchByte] |= lSearchBit;
    else
        mSearchResultId[lSearchByte] &= ~lSearchBit;

#if ONEWIRE_TRACE_SEARCH == detail
    for (uint8_t i = 0; i <= lSearchByte; i++)
    {
        printDebug(" %02X", mSearchResultId[i]);
    }
    printDebug(" - found: %d\n", lDirection);
#endif
    return true;
}

bool OneWireSearchFirst::wireSearchEnd()
{
    mSearchLastDiscrepancy = mSearchLastZero;
    if (mSearchLastZero == -1)
        mSearchLastDeviceFlag = 1;
    return mSearchLastDeviceFlag;
}

bool OneWireSearchFirst::wireSearchFinished(bool iIsError) {
    // we set all remaining sensors in disconnected state
    // or delete their seach count in case of search error
    manageSearchCounter(iIsError ? SearchError : SearchFinished);
    return !iIsError;
}

// blocking search, takes 91 ms per sensor!!!!
// Perform a search of the 1-Wire bus
bool OneWireSearchFirst::wireSearchBlocking()
{
	uint8_t lDirection;
	int8_t lLastZero = -1;
    bool lResult = true;
    uint8_t lCurrentByte = 0;

#if ONEWIRE_TRACE_SEARCH == detail
    // searchDebug("### start Blocking search ###\n");
#endif
    uint32_t lDelay = millis();

    if (mSearchLastDeviceFlag)
        lResult = false;

    if (!mBM->wireReset())
        lResult = false;

    if (lResult) {
        mBM->waitOnBusy();

        mBM->wireWriteByte(WIRE_COMMAND_SEARCH);

        mBM->waitOnBusy();
        for(uint8_t i = 0; i < 64; i++)
        {
            int lSearchByte = i / 8; 
            int lSearchBit = 1 << i % 8;

            if ((i % 4) == 0) {
                // callback main routine each byte found
                mBM->searchLoopCallback();
            }
            if (i < mSearchLastDiscrepancy)
                lDirection = mSearchResultId[lSearchByte] & lSearchBit;
            else
                lDirection = (i == mSearchLastDiscrepancy);

            mBM->begin();
            mBM->writeByte(DS2482_COMMAND_TRIPLET);
            mBM->writeByte(lDirection ? 0x80 : 0x00);
            mBM->end();

            uint8_t lStatus = mBM->waitOnBusy(); //(false);
            uint8_t lId = lStatus & DS2482_STATUS_SBR;
            uint8_t lCompId = lStatus & DS2482_STATUS_TSB;
            lDirection = lStatus & DS2482_STATUS_DIR;

            if (lId && lCompId) {
                lResult = false;
                break;
            } else if (!lId && !lCompId && !lDirection) {
                lLastZero = i;
                // check for Last discrepancy in family
                if (lLastZero < 8)
                    mSearchLastFamilyDiscrepancy = lLastZero;
            }
            lCurrentByte = mSearchResultId[lSearchByte];
            if (lDirection)
                lCurrentByte |= lSearchBit;
            else
                lCurrentByte &= ~lSearchBit;
            // in case of search for Id we just compare if
            // found bit is the same as the one in searched Id
            if (mSearchMode == Id && lCurrentByte != mSearchResultId[lSearchByte] && lSearchByte < 7) {
                lResult = false;
                break;
            }
            // in case of Family search, we can stop as soon as
            // an other family is found
            if (mSearchMode == Family && mSearchFamily != mSearchResultId[0]) {
                lResult = false;
                break;
            }
            mSearchResultId[lSearchByte] = lCurrentByte;
        }
        if (lResult) {
            mSearchLastDiscrepancy = lLastZero;
            if (lLastZero == -1)
                mSearchLastDeviceFlag = 1;
            mBM->searchLoopCallback();
            // found id is just valid if crc8 check is ok
            if (mSearchResultId[7] != mBM->crc8(mSearchResultId, 7))
                lResult = false;
            // for (uint8_t i = 0; i < 7; i++)
            // 	eAddress[i] = mSearchResultId[i];
        }
    }
#if ONEWIRE_TRACE_SEARCH == detail
    // searchDebug("### End blocking search after %ld ms\n", millis() - lDelay);
#endif 
    return lResult;
}

bool OneWireSearchFirst::wireSearchNewDevices() {
    if (wireSearchBlocking()) {
        // found a device
        if (MatchSearchMode(mSearchResultId[0]))
            mBM->CreateOneWire(mSearchResultId);
        if (mSearchLastDeviceFlag) {
            newSearchAll();
        }
    } else {
        // if there was an error, we start over
        newSearchAll();
    }
}
