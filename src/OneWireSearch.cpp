#include "OneWireSearch.h"
#include "OneWireDS2482.h"

// #ifdef ONEWIRE_TRACE_SEARCH
// onewire debug output
int OneWireSearch::searchDebug(const char* iFormat, ...)
{
    char lBuffer[256];
    uint8_t lBufferPos = gInstance * 2;
    memset(lBuffer, ' ', lBufferPos + 1);
    switch (mSearchMode)
    {
        case Family:
            lBuffer[lBufferPos] = 'F';
            break;
        case Id:
            lBuffer[lBufferPos] = 'I';
            break;
        default:
            lBuffer[lBufferPos] = 'A';
            break;
    }
    sprintf(lBuffer + lBufferPos + 1, "%02i-", gInstance + 1);
    va_list lArgs;
    va_start(lArgs, iFormat);
    int lResult = vsnprintf(lBuffer + lBufferPos + 4, 252 - lBufferPos, iFormat, lArgs);
    va_end(lArgs);
    SerialUSB.print(lBuffer);
    return lResult;
}
// #endif

OneWireSearch::OneWireSearch(OneWireDS2482* iBM)
{
    mBM = iBM;
    gInstance = iBM->gInstance;
    mSearchState = SearchNew;
}

OneWireSearch::SearchState OneWireSearch::state()
{
    return mSearchState;
}

void OneWireSearch::newSearchAll()
{
    gInstance = mBM->gInstance;
    mSearchState = SearchNew;
    mSearchMode = All;
    mSearchFamily = 0;
    mSearchLastDeviceFlag = 0;
    mSearchLastDiscrepancy = -1;
    mSearchLastFamilyDiscrepancy = 0;
    for (uint8_t i = 0; i < 8; i++)
        mSearchResultId[i] = 0;
#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("### Init search all ###\n");
#endif
}

void OneWireSearch::newSearchFamily(uint8_t iFamily)
{
    gInstance = mBM->gInstance;
    mSearchState = SearchNew;
    mSearchMode = Family;
    mSearchFamily = iFamily;
#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("### Init search Family %02x ###\n", iFamily);
#endif
}

void OneWireSearch::newSearchNoFamily(uint8_t iFamily)
{
    gInstance = mBM->gInstance;
    mSearchState = SearchNew;
    mSearchMode = NoFamily;
    mSearchFamily = iFamily;
#if ONEWIRE_TRACE_SEARCH == detail
    searchDebug("### Init search NoFamily %02x ###\n", iFamily);
#endif
}

void OneWireSearch::newSearchForId(tIdRef iId)
{
    gInstance = mBM->gInstance;
    mSearchState = SearchNew;
    mSearchMode = Id;
    mSearchFamily = iId[0];
    mSearchLastDeviceFlag = 0;
    mSearchLastDiscrepancy = 64;
    mSearchLastFamilyDiscrepancy = 0;
    copyId(mSearchResultId, iId);
#if ONEWIRE_TRACE_SEARCH == detail
    // searchDebug("### Init search for Id %02x %02x %02x %02x %02x %02x %02x ###\n", iId[0], iId[1], iId[2], iId[3], iId[4], iId[5], iId[6]);
#endif
}

bool OneWireSearch::MatchSearchMode(uint8_t iFamily)
{
    bool lResult = false;
    switch (mSearchMode)
    {
        case All:
            lResult = true;
            break;
        case Family:
            lResult = (iFamily == mSearchFamily);
            break;
        case NoFamily:
            lResult = (iFamily != mSearchFamily);
            break;
        case Id:
            lResult = false; //todo
            break;
        default:
            break;
    }
    return lResult;
}

/****************************************
 * Due to the fact, that 1W-Devices might disappear shortly from bus
 * because of a short on the line or signal errors, we count such 
 * events and do a state change only if the event lasts for a specific
 * number of times (see cDisappearCount)
 * **************************************/
void OneWireSearch::manageSearchCounter(OneWireSearch::SearchState iState) {
    for (uint8_t i = 0; i < mBM->DeviceCount(); i++)
    {
        if (MatchSearchMode(mBM->Sensor(i)->Family()))
        {
            switch (iState)
            {
                case SearchNew:
                    mBM->Sensor(i)->incrementSearchCount();
                    break;
                case SearchFinished:
                    // All sensors, which are not found, are disconnected
                    mBM->Sensor(i)->setModeDisconnected();
                    break;
                case SearchError:
                    mBM->Sensor(i)->clearSearchCount();
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }
}

// async search, max blocking time 4 ms
OneWireSearch::SearchState OneWireSearch::loop()
{
    uint8_t lStatus = 0;
    OneWireSearch::SearchState lExitState = mSearchState;
#ifdef DebugInfoSearch
    uint32_t lDuration = 0;
    mCurrDelay = millis();
#endif
    switch (mSearchState)
    {
		case SearchNew:
#ifdef DebugInfoSearch
			// DEBUG: Time measurement
			mDuration = millis();
            mMaxDelay = 0;
#endif
            wireSearchNew();
            mSearchState = SearchNext;
            break;
        case SearchNext:
            wireSearchNext();
            mSearchState = SearchStart;
            mDelay = millis();
            break;
        case SearchStart:
            lStatus = mBM->readStatus();//(false);
            if (!(lStatus & DS2482_STATUS_BUSY))
            {
                mSearchState = wireSearchStart(lStatus) ? SearchStep : SearchError;
                mDelay = millis();
            }
            else if (delayCheck(mDelay, 1000))
            {
                mSearchState = SearchError;
            }
            break;
        case SearchStep:
            if (wireSearchStep(mSearchStep))
            {
                mSearchStep += 1;
                // with family search we can stop as soon as an other family is found
                if (mSearchMode == Family && mSearchFamily != mSearchResultId[0])
                    mSearchState = SearchFinished;
                if (mSearchStep == 64)
                    mSearchState = SearchEnd;
            }
            else
            {
                mSearchState = SearchError;
            };
            mDelay = millis();
            break;
        case SearchEnd:
            // we do CRC check first
            if (mSearchResultId[7] == mBM->crc8(mSearchResultId, 7)) {
                if (MatchSearchMode(mSearchResultId[0])) mBM->CreateOneWire(mSearchResultId);
                mSearchState = wireSearchEnd() ? SearchFinished : SearchNext;
            } else {
                mSearchState = SearchError;
            }
            mDelay = millis();
            break;
        case SearchFinished:
            wireSearchFinished(false);
            mSearchState = SearchNew;
#ifdef DebugInfoSearch
            mMaxDelay = max(mMaxDelay, millis() - mCurrDelay);
            lDuration = millis() - mDuration;
			if (abs(mDurationOld - lDuration) > 5) { 
				// print only if there is a big difference between cycles
                printDebug("(0x%X) Finished search %s (%s), took %d ms, max blocking was %d ms\n", mBM->mI2cAddress, mSearchMode == Family ? "Family" : "NoFamily", mSearchResultId, lDuration, mMaxDelay);
                mDurationOld = lDuration;
            }
#endif
            break;
        default:
            wireSearchFinished(true);
            mSearchState = SearchNew;
            break;
    }
#ifdef DebugInfoSearch
    mMaxDelay = max(mMaxDelay, millis() - mCurrDelay);
#endif
    return lExitState;
}

