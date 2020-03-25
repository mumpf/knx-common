#include "OneWireSearch.h"
#include "OneWireDS2482.h"

// #define DebugInfoSearch

OneWireSearch::OneWireSearch(OneWireDS2482* iBM)
{
    mBM = iBM;
    mStateSearch = SearchNew;
}

OneWireSearch::StateSearch OneWireSearch::state()
{
    return mStateSearch;
}

#ifdef DebugInfoSearch
uint32_t gDuration = 0;
uint32_t gDurationOld = 0;
uint32_t gDelay = 0;
uint32_t gMaxDelay = 0;
#endif

void OneWireSearch::reset(){
    mStateSearch = SearchNew;
}

// async search, max blocking time 4 ms
bool OneWireSearch::loop()
{
    uint8_t lStatus = 0;
    bool lExitLoop = false;
#ifdef DebugInfoSearch
    uint32_t lDuration = 0;
    gDelay = millis();
#endif
    switch (mStateSearch)
    {
		case SearchNew:
#ifdef DebugInfoSearch
			// DEBUG: Time measurement
			gDuration = millis();
            gMaxDelay = 0;
#endif
            wireSearchNew();
            mStateSearch = SearchReset;
            break;
        case SearchReset:
            wireSearchReset();
            mStateSearch = SearchStart;
            mDelay = millis();
            break;
        case SearchStart:
            lStatus = mBM->readStatus();//(false);
            if (!(lStatus & DS2482_STATUS_BUSY))
            {
                mStateSearch = wireSearchStart(lStatus) ? SearchStep : SearchError;
                mDelay = millis();
            }
            else if (delayCheck(mDelay, 1000))
            {
                mStateSearch = SearchError;
            }
            break;
        case SearchStep:
            if (wireSearchStep(mSearchStep))
            {
                mSearchStep += 1;
                if (mSearchStep == 64)
                    mStateSearch = SearchEnd;
            }
            else
            {
                mStateSearch = SearchError;
            };
            mDelay = millis();
            break;
        case SearchEnd:
            // we do CRC check first
            if (mSearchResultId[7] == mBM->crc8(mSearchResultId, 7)) {
                mBM->CreateOneWire(mSearchResultId);
                mStateSearch = wireSearchEnd() ? SearchFinished : SearchReset;
            } else {
                mStateSearch = SearchError;
            }
            mDelay = millis();
            break;
        case SearchFinished:
            lExitLoop = true;
            wireSearchFinished(false);
#ifdef DebugInfoSearch
            gMaxDelay = max(gMaxDelay, millis() - gDelay);
            lDuration = millis() - gDuration;
			if (abs(gDurationOld - lDuration) > 5) { 
				// print only if there is a big difference between cycles
                printDebug("Finished search, took %d ms, max blocking was %d ms\n", lDuration, gMaxDelay);
                gDurationOld = lDuration;
            }
#endif
            break;
        default:
            lExitLoop = true;
            wireSearchFinished(true);
            mStateSearch = SearchError;
            break;
    }
#ifdef DebugInfoSearch
    gMaxDelay = max(gMaxDelay, millis() - gDelay);
#endif
    return lExitLoop;
}

