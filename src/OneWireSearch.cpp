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
    wireSearchNew();
}

void OneWireSearch::newSearchFamily(uint8_t iFamily)
{
    mSearchState = SearchNew;
    mSearchMode = Family;
    mSearchFamily = iFamily;
    wireSearchNew(iFamily);
}

void OneWireSearch::newSearchNoFamily(uint8_t iFamily)
{
    mSearchState = SearchNew;
    mSearchMode = NoFamily;
    mSearchFamily = 0;
    wireSearchNew(iFamily);
}

// async search, max blocking time 4 ms
bool OneWireSearch::loop()
{
    uint8_t lStatus = 0;
    bool lExitLoop = false;
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
            mSearchState = SearchReset;
            break;
        case SearchReset:
            wireSearchReset();
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
                mBM->CreateOneWire(mSearchResultId);
                mSearchState = wireSearchEnd() ? SearchFinished : SearchReset;
            } else {
                mSearchState = SearchError;
            }
            mDelay = millis();
            break;
        case SearchFinished:
            lExitLoop = true;
            wireSearchFinished(false);
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
            lExitLoop = true;
            wireSearchFinished(true);
            mSearchState = SearchError;
            break;
    }
#ifdef DebugInfoSearch
    mMaxDelay = max(mMaxDelay, millis() - mCurrDelay);
#endif
    return lExitLoop;
}

