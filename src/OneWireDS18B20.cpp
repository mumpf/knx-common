#include <Wire.h>
#include <arduino.h>
#include <math.h>

#include "OneWireDS2482.h"
#include "OneWireDS18B20.h"

#define DebugInfoTemp

OneWireDS18B20::OneWireDS18B20(OneWireDS2482* iBM, uint8_t* iId)
    : OneWire(iBM, iId)
{}

void OneWireDS18B20::loop() {

    switch (mState)
    {
    case Startup:
        init(true);
        mState = Idle;
        break;
    case StartMeasurement:
        mState = startConversionTemp() ? GetMeasure : Error;
        pDelay = millis();
        break;
    case GetMeasure:
        if (delayCheck(pDelay, 750)) {
            mState = updateTemp() ? Idle : Error;
            pDelay = millis();
        }
        break;
    case Idle:
        if (delayCheck(pDelay, 2000)) {
            mState = StartMeasurement;
            pDelay = millis();
        }
        break;
    default:
        // error case
        mState = Error;
        break;
    }
}

void OneWireDS18B20::init(bool iIsactive)
{
    mIsActive = iIsactive;

    if (!mParasite && readPowerSupply())
        mParasite = true;
    mBitResolution = max(mBitResolution, resolution());
}

void OneWireDS18B20::isActive(bool iState)
{
    mIsActive = iState;
}

bool OneWireDS18B20::isActive()
{
    return mIsActive;
}

float OneWireDS18B20::getTemp()
{
    return mTemp;
}

bool OneWireDS18B20::startConversionTemp()
{
    pBM->wireReset();
    pBM->wireSelect(pId);
    pBM->wireWriteByte(STARTCONVO, 1); // Wandlung mit aktivierter parasitärer Versorgung starten
    return true;
}

bool OneWireDS18B20::updateTemp()
{
//     pBM->wireReset();
//     pBM->wireSelect(pId);
//     pBM->wireWriteByte(0xBE); // Scratchpad auslesen
// #ifdef DebugInfoTemp
//     printDebug(F("lScratchPad = %d"), present);
//     printDebugln(" ");
// #endif
//     ScratchPad lScratchPad;
//     for (uint8_t i = 0; i < 9; i++)
//     { // Wir brauchen 9 Byte
//         lScratchPad[i] = pBM->wireReadByte();
// #ifdef DebugInfoTemp
//         printDebug(F("%02x "), lScratchPad[i]);
// #endif
//     }
// #ifdef DebugInfoTemp
//     printDebugln(" ");
//     printDebug("CRC = ");
//     printDebugln(F("%02x"), DS2482_OneWire::crc8(lScratchPad, 8));
// #endif
//     byte lConfig = (lScratchPad[4] & 0x60);
// #ifdef DebugInfoTemp
//     printDebug("lConfig = ");
//     printDebugln(F("%02x"), lConfig);
// #endif
    uint8_t lResolution = resolution();
    uint16_t lTempRaw = (mScratchPad[1] << 8) | mScratchPad[0];

    if (lResolution) {
        uint16_t lShift = 0xFF << abs(lResolution - 12);
        lTempRaw &= lShift;
        mTemp = (float)lTempRaw / 16.0;
    #ifdef DebugInfoTemp
        printDebug("Temp = %0.1f°C\n", mTemp);
    #endif
        return true;
    } else {
        return false;
    }
}

// reads the device's power requirements
bool OneWireDS18B20::readPowerSupply()
{
    pBM->wireReset();
    bool lResult = false;
    pBM->wireSelect(pId);
    pBM->wireWriteByte(READPOWERSUPPLY);
    if (pBM->wireReadBit() == 0)
        lResult = true;
    pBM->wireReset();
    return lResult;
}

// returns true if the bus requires parasite power
bool OneWireDS18B20::isParasitePowerMode()
{
    return mParasite;
}

// attempt to determine if the device at the given address is connected to the bus
bool OneWireDS18B20::isConnected()
{
    readScratchPad();
    return (pBM->crc8(mScratchPad, 8) == mScratchPad[SCRATCHPAD_CRC]);
}

// returns the global resolution
uint8_t OneWireDS18B20::resolution()
{
    // this model has a fixed resolution of 9 bits but getTemp calculates
    // a full 12 bits resolution and we need 750ms convert time
    if (isConnected())
    {
        // returned values are 9-12
        return (pId[0] == DS18S20MODEL) ? 9 : ((mScratchPad[CONFIGURATION] >> 5) & 3) + 9;
    }
    return 0;
}

// set resolution of all devices to 9, 10, 11, or 12 bits
// if new resolution is out of range, it is constrained.
bool OneWireDS18B20::resolution(uint8_t iResolution)
{
    mBitResolution = constrain(iResolution, 9, 12);

    if (isConnected())
    {
        // DS18S20 has a fixed 9-bit resolution
        if (pId[0] != DS18S20MODEL)
        {
            mScratchPad[CONFIGURATION] = ((mBitResolution - 9) << 5) | 0x0F;
            writeScratchPad();
        }
        return true; // new value set
    }
    return false;
}

// writes device's scratch pad
void OneWireDS18B20::writeScratchPad()
{
    pBM->wireReset();
    pBM->wireSelect(pId);
    pBM->wireWriteByte(WRITESCRATCH);
    pBM->wireWriteByte(mScratchPad[HIGH_ALARM_TEMP]); // high alarm temp
    pBM->wireWriteByte(mScratchPad[LOW_ALARM_TEMP]);  // low alarm temp
    // DS18S20 does not use the configuration register
    if (pId[0] != DS18S20MODEL)
        pBM->wireWriteByte(mScratchPad[CONFIGURATION]); // configuration
    pBM->wireReset();
    // save the newly written values to eeprom
    pBM->wireWriteByte(COPYSCRATCH, mParasite);
    if (mParasite)
        delay(10); // 10ms delay
    pBM->wireReset();
}

// read device's scratch pad
void OneWireDS18B20::readScratchPad()
{
    // send the command
    pBM->wireReset();
    pBM->wireSelect(pId);
    pBM->wireWriteByte(READSCRATCH); // 0xBE  // Read EEPROM

    // TODO => collect all comments &  use simple loop
    // byte 0: temperature LSB
    // byte 1: temperature MSB
    // byte 2: high alarm temp
    // byte 3: low alarm temp
    // byte 4: DS18S20: store for crc
    //         DS18B20 & DS1822: configuration register
    // byte 5: internal use & crc
    // byte 6: DS18S20: COUNT_REMAIN
    //         DS18B20 & DS1822: store for crc
    // byte 7: DS18S20: COUNT_PER_C
    //         DS18B20 & DS1822: store for crc
    // byte 8: SCRATCHPAD_CRC
    //
    for(uint8_t i=0; i<9; i++)
    {
        mScratchPad[i] = pBM->wireReadByte();
    }
    pBM->wireReset();
}
