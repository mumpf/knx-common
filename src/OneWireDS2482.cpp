#include <Wire.h>
#include <Arduino.h>
#include "Helper.h"
#include "Hardware.h"
#include "OneWireDS2482.h"
#include "OneWireDS18B20.h"
#include "OneWireDS1990.h"
#include "OneWireDS2408.h"
#include "OneWireDS2413.h"

#ifndef I2C_1WIRE_DEVICE_ADDRESSS
#define I2C_1WIRE_DEVICE_ADDRESSS 0
#endif

// #define DebugInfoBM

// Constructor with no parameters for compatability with OneWire lib
OneWireDS2482::OneWireDS2482(foundNewId iNewIdCallback)
{
    mI2cAddress = I2C_1WIRE_DEVICE_ADDRESSS;
    fNewIdCallback = iNewIdCallback;
    mError = 0;
}

OneWireDS2482::OneWireDS2482(uint8_t iI2cAddressOffset, foundNewId iNewIdCallback)
{
	// Address is determined by two pins on the DS2482 AD1/AD0
	// Pass 0b00, 0b01, 0b10 or 0b11
    mI2cAddress = I2C_1WIRE_DEVICE_ADDRESSS | iI2cAddressOffset;
    fNewIdCallback = iNewIdCallback;
    mError = 0;
}

void OneWireDS2482::setup()
{
    deviceReset();
    setActivePullup();
    setStrongPullup();

    mState = Init;
}

void OneWireDS2482::loop()
{
    switch (mState)
	{
        case Init:
            mStateSearch = SearchNew;
            mState = Startup;
            break;
        case Startup:
			// at startup we do an initial search to find all supported sensors
            if (wireSearchLoop())
            {
                // search is finished, we have to find out if successful
                mState = (mStateSearch == SearchFinished) ? PriorityBusUse : Error;
            }
            mDelay = millis();
            break;
		case Search:
            if (wireSearchLoop()) {
				// search is finished, we have to find out if successful
                mState = (mStateSearch == SearchFinished) ? PriorityBusUse : Error;
            }
            mDelay = millis();
            break;
        case PriorityBusUse:
            mState = NormalBusUse;
            break;
        case NormalBusUse:
			if (ProcessNormalBusUse()) {
            	mState = Idle;
            } 
            break;
        case Idle:
            mState = Search;
            mStateSearch = SearchNew;
            mDelay = millis();
            break;
        default:
		    // we wait a Moment and afterwards start over
            mState = delayCheck(mDelay, 100) ? Idle : Error;
            break;
	}
}

// returns false while the method iterates thrugh sensor list, true if finished the list
bool OneWireDS2482::ProcessNormalBusUse()
{
    static int8_t sSensorIndex = 0;
	if (sSensorIndex >= mSensorCount)
        return true; // no sensors available or wrong sensor index
    OneWire *lSensor = mSensor[sSensorIndex++];
    if (lSensor->Family() == MODEL_DS18B20  && lSensor->Mode() == OneWire::Connected) {
        lSensor->loop();
    }
	if (sSensorIndex >= mSensorCount)
        sSensorIndex = 0;
    return (sSensorIndex == 0);
}

// Factory method, creates OneWireSensors just in case, they do not exist
// for new OneWireSensors also a callback is issued to the host
OneWire *OneWireDS2482::CreateOneWire(tIdRef iId) {
    OneWire *lSensor = NULL;

    // check if Sensor exists
	for (uint8_t i = 0; i < mSensorCount; i++)
	{
		if (equalId(mSensor[i]->Id(), iId)) {
            lSensor = mSensor[i];
            lSensor->setModeConnected(); // happens only, if it is not in state New
            break;
        }
	}
	if (lSensor == NULL) {
		// this is a new sensor
		switch (iId[0]) 
		{
		case MODEL_DS18B20:
        case MODEL_DS18S20:
            lSensor = (new OneWireDS18B20(this, iId));
            break;
        case MODEL_DS1990:
            lSensor = (new OneWireDS1990(this, iId));
            break;
        case MODEL_DS2408:
            lSensor = (new OneWireDS2408(this, iId));
            break;
        case MODEL_DS2413:
            lSensor = (new OneWireDS2413(this, iId));
            break;
        case MODEL_DS2436:
            lSensor = (new OneWireDS18B20(this, iId));
            break;
        default:
            printHEX("Unsupported family found: ", iId, 7);
            // sensor family not supported
            break;
        }
		if (lSensor != NULL) {
			if (fNewIdCallback != 0)
				fNewIdCallback(lSensor);
			mSensor[mSensorCount++] = lSensor;
			if (mSensorCount == 30)
				mSensorCount--;
        }
	}
    return lSensor;
}

OneWire *OneWireDS2482::Sensor(uint8_t iIndex){
    if (iIndex < mSensorCount)
        return mSensor[iIndex];
	else
        return NULL;
}

uint8_t OneWireDS2482::SensorCount() {
    return mSensorCount;
}

// Just for the DS2482-800
bool OneWireDS2482::selectChannel(uint8_t channel)
{
	uint8_t ch, ch_read;

	switch (channel)
	{
		case 0:
		default:
			ch = 0xf0;
			ch_read = 0xb8;
			break;
		case 1:
			ch = 0xe1;
			ch_read = 0xb1;
			break;
		case 2:
			ch = 0xd2;
			ch_read = 0xaa;
			break;
		case 3:
			ch = 0xc3;
			ch_read = 0xa3;
			break;
		case 4:
			ch = 0xb4;
			ch_read = 0x9c;
			break;
		case 5:
			ch = 0xa5;
			ch_read = 0x95;
			break;
		case 6:
			ch = 0x96;
			ch_read = 0x8e;
			break;
		case 7:
			ch = 0x87;
			ch_read = 0x87;
			break;
	};


	begin();
	Wire.write(0xc3);
	Wire.write(ch);
	end();
	

	uint8_t check = readByte();

	return check == ch_read;
}

uint8_t OneWireDS2482::getI2cAddress()
{
    return mI2cAddress;
}

uint8_t OneWireDS2482::getError()
{
	return mError;
}

// Helper functions to make dealing with I2C side easier
void OneWireDS2482::begin()
{
	Wire.beginTransmission(mI2cAddress);
}

uint8_t OneWireDS2482::end()
{
	return Wire.endTransmission();
}

void OneWireDS2482::writeByte(uint8_t data)
{
	Wire.write(data); 
}

uint8_t OneWireDS2482::readByte()
{
	Wire.requestFrom(mI2cAddress,1u);
	return Wire.read();
}

// Simply starts and ends an Wire transmission
// If no devices are present, this returns false
uint8_t OneWireDS2482::checkI2cPresence()
{
	begin();
	return !end() ? true : false;
}

// Performs a global reset of device state machine logic. Terminates any ongoing 1-Wire communication.
void OneWireDS2482::deviceReset()
{
	begin();
	wireWriteByte(DS2482_COMMAND_RESET);
	end();
}

// Sets the read pointer to the specified register. Overwrites the read pointer position of any 1-Wire communication command in progress.
void OneWireDS2482::setReadPointer(uint8_t readPointer)
{
	begin();
	writeByte(DS2482_COMMAND_SRP);
	writeByte(readPointer);
	end();
}

// Read the status register
uint8_t OneWireDS2482::readStatus(bool iSetReadPointer)
{
	if (iSetReadPointer) setReadPointer(DS2482_POINTER_STATUS);
	return readByte();
}


bool OneWireDS2482::readStatusShortDet()
{
  //uint8_t statusBits = 
  return   bitRead(readStatus(), 2);
}


// Read the data register
uint8_t OneWireDS2482::readData()
{
	setReadPointer(DS2482_POINTER_DATA);
	return readByte();
}

// Read the config register
uint8_t OneWireDS2482::readConfig()
{
	setReadPointer(DS2482_POINTER_CONFIG);
	return readByte();
}

void OneWireDS2482::setStrongPullup()
{
	writeConfig(readConfig() | DS2482_CONFIG_SPU);
}

void OneWireDS2482::clearStrongPullup()
{
	writeConfig(readConfig() & !DS2482_CONFIG_SPU);
}

void OneWireDS2482::setActivePullup()
{
  writeConfig(readConfig() | DS2482_CONFIG_APU);
}

void OneWireDS2482::clearActivePullup()
{
  writeConfig(readConfig() | !DS2482_CONFIG_APU);
}

// Churn until the busy bit in the status register is clear
uint8_t OneWireDS2482::waitOnBusy(bool iSetReadPointer)
{
	uint8_t status;

	for(int i=1000; i>0; i--)
	{
		status = readStatus(iSetReadPointer);;
		if (!(status & DS2482_STATUS_BUSY))
			break;
		delayMicroseconds(20);
	}

	// if we have reached this point and we are still busy, there is an error
	if (status & DS2482_STATUS_BUSY)
		mError = DS2482_ERROR_TIMEOUT;

	// Return the status so we don't need to explicitly do it again
	return status;
}

// Write to the config register
void OneWireDS2482::writeConfig(uint8_t config)
{
	waitOnBusy();
	begin();
	writeByte(DS2482_COMMAND_WRITECONFIG);
	// Write the 4 bits and the complement 4 bits
	writeByte(config | (~config)<<4);
	end();
	
	// This should return the config bits without the complement
	if (readByte() != config)
		mError = DS2482_ERROR_CONFIG;
}

// Generates a 1-Wire reset/presence-detect cycle (Figure 4) at the 1-Wire line. The state
// of the 1-Wire line is sampled at tSI and tMSP and the result is reported to the host 
// processor through the Status Register, bits PPD and SD.
uint8_t OneWireDS2482::wireReset()
{
	waitOnBusy();
	// Datasheet warns that reset with SPU set can exceed max ratings
	clearStrongPullup();

	waitOnBusy(false);

	begin();
	writeByte(DS2482_COMMAND_RESETWIRE);
	end();

	uint8_t status = waitOnBusy();

	if (status & DS2482_STATUS_SD)
	{
		mError = DS2482_ERROR_SHORT;
	} else {
        setActivePullup();
    }

	return (status & DS2482_STATUS_PPD) ? true : false;
}

// Writes a single data byte to the 1-Wire line.
void OneWireDS2482::wireWriteByte(uint8_t data, uint8_t power)
{
	waitOnBusy();
	if (power)
		setStrongPullup();
    // setActivePullup();
	begin();
	writeByte(DS2482_COMMAND_WRITEBYTE);
	writeByte(data);
	end();
}

// Generates eight read-data time slots on the 1-Wire line and stores result in the Read Data Register.
uint8_t OneWireDS2482::wireReadByte()
{
	waitOnBusy();
	begin();
	writeByte(DS2482_COMMAND_READBYTE);
	end();
	waitOnBusy(false);
	return readData();
}

// Generates a single 1-Wire time slot with a bit value “V” as specified by the bit byte at the 1-Wire line
// (see Table 2). A V value of 0b generates a write-zero time slot (Figure 5); a V value of 1b generates a 
// write-one time slot, which also functions as a read-data time slot (Figure 6). In either case, the logic
// level at the 1-Wire line is tested at tMSR and SBR is updated.
void OneWireDS2482::wireWriteBit(uint8_t data, uint8_t power)
{
	waitOnBusy();
	if (power)
		setStrongPullup();
    // setActivePullup();
	begin();
	writeByte(DS2482_COMMAND_SINGLEBIT);
	writeByte(data ? 0x80 : 0x00);
	end();
}

// As wireWriteBit
uint8_t OneWireDS2482::wireReadBit()
{
	wireWriteBit(1);
	uint8_t status = waitOnBusy();
	return status & DS2482_STATUS_SBR ? 1 : 0;
}

// 1-Wire skip
void OneWireDS2482::wireSkip()
{
	wireWriteByte(WIRE_COMMAND_SKIP);
}

void OneWireDS2482::wireSelect(const tIdRef iId)
{
	wireWriteByte(WIRE_COMMAND_SELECT);
	for (int i=0;i<7;i++)
		wireWriteByte(iId[i]);
    wireWriteByte(crc8(iId, 7));
}

uint32_t gDuration = 0;
uint32_t gDurationOld = 0;
uint32_t gDelay = 0;
uint32_t gMaxDelay = 0;

// async search, max blocking time 4 ms
bool OneWireDS2482::wireSearchLoop()
{
    uint8_t lStatus = 0;
    bool lExitLoop = false;
#ifdef DebugInfoBM
    uint32_t lDuration = 0;
#endif
    gDelay = millis();
    switch (mStateSearch)
    {
		case SearchNew:
			// DEBUG: Time measurement
			gDuration = millis();
            gMaxDelay = 0;
            wireSearchNew();
            mStateSearch = SearchReset;
            break;
        case SearchReset:
            wireSearchReset();
            mStateSearch = SearchStart;
            break;
        case SearchStart:
            lStatus = readStatus(false);
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
            if (mSearchResultId[7] == crc8(mSearchResultId, 7)) {
                CreateOneWire(mSearchResultId);
                mStateSearch = wireSearchEnd() ? SearchFinished : SearchReset;
            } else {
                mStateSearch = SearchError;
            }
            mDelay = millis();
            break;
        case SearchFinished:
            lExitLoop = true;
            wireSearchFinished(false);
            gMaxDelay = max(gMaxDelay, millis() - gDelay);
#ifdef DebugInfoBM
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
    gMaxDelay = max(gMaxDelay, millis() - gDelay);
    return lExitLoop;
}


//  1-Wire reset search algorithm
void OneWireDS2482::wireSearchNew(uint8_t iFamilyCode /* = 0 */ )
{
    // reset search fields
	mSearchLastDiscrepancy = 0;
	mSearchLastDeviceFlag = 0;
    mSearchLastFamilyDiscrepancy = 0;

    // clear search buffer
    for (uint8_t i = 0; i < 8; i++)
		mSearchResultId[i] = 0;

    // set all known devices in search mode incrementing their search count
    for (uint8_t i = 0; i < mSensorCount; i++)
    {
        mSensor[i]->incrementSearchCount();
    }
    
    if (iFamilyCode) {
        mSearchResultId[0] = iFamilyCode;
        mSearchLastDiscrepancy = 64;
    }
    // reset bus (see wireReset) in non blocking way
    waitOnBusy();
    // Datasheet warns that reset with SPU set can exceed max ratings
    clearStrongPullup();
}

void OneWireDS2482::wireSearchReset() {
    waitOnBusy();
    begin();
    writeByte(DS2482_COMMAND_RESETWIRE);
    end();
}

bool OneWireDS2482::wireSearchStart(uint8_t iStatus)
{
    mSearchStep = 0;
    mSearchLastZero = 0;
    if (mSearchLastDeviceFlag)
        return false;
    // if (!wireReset())
    //     return false;
    if (iStatus & DS2482_STATUS_SD)
    {
        mError = DS2482_ERROR_SHORT;
    }

    if (!(iStatus & DS2482_STATUS_PPD)) return false;
    waitOnBusy(false);
    wireWriteByte(WIRE_COMMAND_SEARCH);
    waitOnBusy(false);
	return true;
}

bool OneWireDS2482::wireSearchStep(uint8_t iStep) {
    uint8_t lDirection;
    
	int lSearchByte = iStep / 8;
    int lSearchBit = 1 << iStep % 8;

    if (iStep < mSearchLastDiscrepancy)
        lDirection = mSearchResultId[lSearchByte] & lSearchBit;
    else
        lDirection = (iStep == mSearchLastDiscrepancy);

    begin();
    writeByte(DS2482_COMMAND_TRIPLET);
    writeByte(lDirection ? 0x80 : 0x00);
    end();

    uint8_t lStatus = waitOnBusy(false);
    uint8_t lId = lStatus & DS2482_STATUS_SBR;
    uint8_t lCompId = lStatus & DS2482_STATUS_TSB;
    lDirection = lStatus & DS2482_STATUS_DIR;

    if (lId && lCompId)
        return false;
    else if (!lId && !lCompId && !lDirection)
        mSearchLastZero = iStep;

    if (lDirection)
        mSearchResultId[lSearchByte] |= lSearchBit;
    else
        mSearchResultId[lSearchByte] &= ~lSearchBit;
    return true;
}

bool OneWireDS2482::wireSearchEnd()
{
    mSearchLastDiscrepancy = mSearchLastZero;
    if (!mSearchLastZero)
        mSearchLastDeviceFlag = 1;
    return mSearchLastDeviceFlag;
}

bool OneWireDS2482::wireSearchFinished(bool iIsError) {
    // we set all remaining sensors in disconnected state
    // or delete their seach count in case of search error
    for (uint8_t i = 0; i < mSensorCount; i++)
    {
        if (iIsError)
            mSensor[i]->clearSearchCount();
        else
            mSensor[i]->setModeDisconnected();
    }
    return !iIsError;
}

// Sync search, takes 91 ms per sensor!!!!
// Perform a search of the 1-Wire bus
uint8_t OneWireDS2482::wireSearch(tIdRef eAddress)
{
	uint8_t lDirection;
	uint8_t lLastZero=0;

	if (mSearchLastDeviceFlag)
		return 0;

	if (!wireReset())
		return 0;

	waitOnBusy();

	wireWriteByte(WIRE_COMMAND_SEARCH);

	waitOnBusy();
	for(uint8_t i = 0; i < 64; i++)
	{
		int lSearchByte = i / 8; 
		int lSearchBit = 1 << i % 8;

		if (i < mSearchLastDiscrepancy)
			lDirection = mSearchResultId[lSearchByte] & lSearchBit;
		else
			lDirection = (i == mSearchLastDiscrepancy);

		begin();
		writeByte(DS2482_COMMAND_TRIPLET);
		writeByte(lDirection ? 0x80 : 0x00);
		end();

		uint8_t lStatus = waitOnBusy(false);
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
	if (!lLastZero)
		mSearchLastDeviceFlag = 1;
	// found id is just valid if crc8 check is ok
	if (mSearchResultId[7] != crc8(mSearchResultId, 7))
        return 0;
    for (uint8_t i = 0; i < 7; i++)
		eAddress[i] = mSearchResultId[i];
	return 1;
}

#if ONEWIRE_CRC8_TABLE
// Dow-CRC using polynomial X^8 + X^5 + X^4 + X^0
// Tiny 2x16 entry CRC table created by Arjen Lentz
// See http://lentz.com.au/blog/calculating-crc-with-a-tiny-32-entry-lookup-table
static const uint8_t PROGMEM dscrc2x16_table[] = {
    0x00, 0x5E, 0xBC, 0xE2, 0x61, 0x3F, 0xDD, 0x83,
    0xC2, 0x9C, 0x7E, 0x20, 0xA3, 0xFD, 0x1F, 0x41,
    0x00, 0x9D, 0x23, 0xBE, 0x46, 0xDB, 0x65, 0xF8,
    0x8C, 0x11, 0xAF, 0x32, 0xCA, 0x57, 0xE9, 0x74};

// Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// and the registers.  (Use tiny 2x16 entry CRC table)
uint8_t OneWireDS2482::crc8(const uint8_t *addr, uint8_t len)
{
    uint8_t crc = 0;

    while (len--)
    {
        crc = *addr++ ^ crc; // just re-using crc as intermediate
        crc = pgm_read_byte(dscrc2x16_table + (crc & 0x0f)) ^
              pgm_read_byte(dscrc2x16_table + 16 + ((crc >> 4) & 0x0f));
    }

    return crc;
}
// // This table comes from Dallas sample code where it is freely reusable,
// // though Copyright (C) 2000 Dallas Semiconductor Corporation
// static const uint8_t PROGMEM dscrc_table[] = {
//       0, 94,188,226, 97, 63,221,131,194,156,126, 32,163,253, 31, 65,
//     157,195, 33,127,252,162, 64, 30, 95,  1,227,189, 62, 96,130,220,
//      35,125,159,193, 66, 28,254,160,225,191, 93,  3,128,222, 60, 98,
//     190,224,  2, 92,223,129, 99, 61,124, 34,192,158, 29, 67,161,255,
//      70, 24,250,164, 39,121,155,197,132,218, 56,102,229,187, 89,  7,
//     219,133,103, 57,186,228,  6, 88, 25, 71,165,251,120, 38,196,154,
//     101, 59,217,135,  4, 90,184,230,167,249, 27, 69,198,152,122, 36,
//     248,166, 68, 26,153,199, 37,123, 58,100,134,216, 91,  5,231,185,
//     140,210, 48,110,237,179, 81, 15, 78, 16,242,172, 47,113,147,205,
//      17, 79,173,243,112, 46,204,146,211,141,111, 49,178,236, 14, 80,
//     175,241, 19, 77,206,144,114, 44,109, 51,209,143, 12, 82,176,238,
//      50,108,142,208, 83, 13,239,177,240,174, 76, 18,145,207, 45,115,
//     202,148,118, 40,171,245, 23, 73,  8, 86,180,234,105, 55,213,139,
//      87,  9,235,181, 54,104,138,212,149,203, 41,119,244,170, 72, 22,
//     233,183, 85, 11,136,214, 52,106, 43,117,151,201, 74, 20,246,168,
//     116, 42,200,150, 21, 75,169,247,182,232, 10, 84,215,137,107, 53};

// //
// // Compute a Dallas Semiconductor 8 bit CRC. These show up in the ROM
// // and the registers.  (note: this might better be done without to
// // table, it would probably be smaller and certainly fast enough
// // compared to all those delayMicrosecond() calls.  But I got
// // confused, so I use this table from the examples.)
// //
// uint8_t OneWireDS2482::crc8(const uint8_t *addr, uint8_t len)
// {
// 	uint8_t crc = 0;

// 	while (len--) {
// 		crc = pgm_read_byte(dscrc_table + (crc ^ *addr++));
// 	}
// 	return crc;
// }
#else
//
// Compute a Dallas Semiconductor 8 bit CRC directly.
// this is much slower, but much smaller, than the lookup table.
//
uint8_t OneWireDS2482::crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;
	
	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}
#endif

