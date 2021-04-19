#include "OneWireSearchFirst.h"
#include "OneWireDS2482.h"

OneWireSearchFirst::OneWireSearchFirst(OneWireDS2482* iBM)
    : OneWireSearch(iBM)
{
    mBM = iBM;
    gInstance = iBM->gInstance;
    mSearchState = SearchNew;
}

//  1-Wire reset search algorithm
void OneWireSearchFirst::wireSearchNew()
{
    // reset search fields
	mSearchLastDiscrepancy = -1;
	mSearchLastDeviceFlag = 0;
    mSearchLastFamilyDiscrepancy = 0;

    // clear search buffer
    for (uint8_t i = 0; i < 8; i++)
		mSearchResultId[i] = 0;

    // set all known devices in search mode incrementing their search count
    manageSearchCounter(SearchNew);
    
    if (mSearchMode == Family) {
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
    }

    if (!(iStatus & DS2482_STATUS_PPD)) return false;
    mBM->waitOnBusy(false);
    mBM->wireWriteByte(WIRE_COMMAND_SEARCH);
    mBM->waitOnBusy(false);
    return true;
}

bool OneWireSearchFirst::wireSearchStep(uint8_t iStep) {
    uint8_t lDirection;
    
	int lSearchByte = iStep / 8;
    int lSearchBit = 1 << iStep % 8;

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
        if (mSearchLastZero < 9)
            mSearchLastFamilyDiscrepancy = mSearchLastZero;
    }
    if (lDirection)
        mSearchResultId[lSearchByte] |= lSearchBit;
    else
        mSearchResultId[lSearchByte] &= ~lSearchBit;
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
uint8_t OneWireSearchFirst::wireSearchBlocking(tIdRef eAddress)
{
	uint8_t lDirection;
	int8_t lLastZero = -1;

	if (mSearchLastDeviceFlag)
		return 0;

    if (!mBM->wireReset())
        return 0;

    mBM->waitOnBusy();

    mBM->wireWriteByte(WIRE_COMMAND_SEARCH);

    mBM->waitOnBusy();
    for(uint8_t i = 0; i < 64; i++)
	{
		int lSearchByte = i / 8; 
		int lSearchBit = 1 << i % 8;

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

		if (lId && lCompId)
			return 0;
		else if (!lId && !lCompId && !lDirection)
			lLastZero = i;

		if (lDirection)
			mSearchResultId[lSearchByte] |= lSearchBit;
		else
			mSearchResultId[lSearchByte] &= ~lSearchBit;
	}
	mSearchLastDiscrepancy = lLastZero;
	if (lLastZero == -1)
		mSearchLastDeviceFlag = 1;
	// found id is just valid if crc8 check is ok
    if (mSearchResultId[7] != mBM->crc8(mSearchResultId, 7))
        return 0;
    for (uint8_t i = 0; i < 7; i++)
		eAddress[i] = mSearchResultId[i];
	return 1;
}

