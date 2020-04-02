#include "OneWireSearch.h"
#include "OneWireDS2482.h"

OneWireSearch::OneWireSearch(OneWireDS2482* iBM)
{
    mBM = iBM;
    mSearchState = SearchNew;
}

OneWireSearch::SearchState OneWireSearch::state()
{
    return mSearchState;
}

void OneWireSearch::newSearchAll()
{
    mSearchState = SearchNew;
    mSearchMode = All;
    mSearchFamily = 0;
}

void OneWireSearch::newSearchFamily(uint8_t iFamily)
{
    mSearchState = SearchNew;
    mSearchMode = Family;
    mSearchFamily = iFamily;
}

void OneWireSearch::newSearchNoFamily(uint8_t iFamily)
{
    mSearchState = SearchNew;
    mSearchMode = NoFamily;
    mSearchFamily = iFamily;
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
 * number of times (see cMaxCount)
 * **************************************/
void OneWireSearch::manageSearchCounter(OneWireSearch::SearchState iState) {
    for (uint8_t i = 0; i < mBM->SensorCount(); i++)
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
                printDebug("Finished search %s, took %d ms, max blocking was %d ms\n", mSearchMode == Family ? "Family" : "NoFamily", lDuration, mMaxDelay);
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

