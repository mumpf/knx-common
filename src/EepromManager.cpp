#include <Wire.h>
#include "EepromManager.h"
#include "Board.h"

EepromManager::EepromManager(uint16_t iStartPage, uint16_t iNumPages, uint8_t *iMagicWord)
{
    mStartPage = iStartPage;
    mNumPages = iNumPages;
    mMagicWord = iMagicWord;
}

EepromManager::~EepromManager()
{
}


uint8_t EepromManager::mFiller[] = {0, 0, 0, 0};

void EepromManager::beginPage(uint16_t iAddress) {
    if (!mIsTransmission)
    {
        Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
        Wire.write((uint8_t)((iAddress) >> 8)); // MSB
        Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
        mIsTransmission = true;
    }
}

bool EepromManager::endPage() {
    bool lResult = false;
    if (mIsTransmission)
    {
        mIsTransmission = false;
        lResult = Wire.endTransmission() == 0;
        delay(EEPROM_WRITE_DELAY);
    }
    return lResult;
}

void EepromManager::write4Bytes(uint8_t* iData, uint8_t iLen) {
    Wire.write(iData, iLen);
    if (iLen < 4)
        Wire.write(mFiller, 4 - iLen);
}

void EepromManager::prepareRead(uint16_t iAddress, uint8_t iLen) {
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    Wire.write((uint8_t)((iAddress) >> 8)); // MSB
    Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
    Wire.endTransmission();
    Wire.requestFrom(I2C_EEPROM_DEVICE_ADDRESSS, iLen);
}

bool EepromManager::checkMagicWord(uint16_t iAddress) {
    prepareRead(iAddress, 4);
    int lIndex = 0;
    bool lResult = true;
    while (lResult && lIndex < 4) 
        lResult = Wire.available() && (mMagicWord[lIndex++] == Wire.read());
    return lResult;
}

bool EepromManager::writeSession(bool iBegin) {
    uint16_t lAddress = mStartPage * 32 + 12;
    // first we delete magic word, it is rewritten at the end. This is the ack for the successful write.
    beginPage(lAddress);
    write4Bytes(iBegin ? mFiller : mMagicWord, 4);
    return endPage();
}

bool EepromManager::beginWriteSession() {
    return writeSession(true);
}

void EepromManager::endWriteSession() {
    // as a last step we write magic number back
    // this is also the ACK, that writing was successfull
    writeSession(false);
}


bool EepromManager::checkDataValid() {
    uint16_t lAddress = mStartPage * 32 + 12;
    mIsValidEEPROM = checkMagicWord(lAddress);
    mValidityChecked = true;
    return mIsValidEEPROM;
}

bool EepromManager::isValid() {
    if (!mValidityChecked) checkDataValid();
    return mIsValidEEPROM;
}

