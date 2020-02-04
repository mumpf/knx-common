#include <Wire.h>
#include "Board.h"
#include "Helper.h"
#include "EepromManager.h"

void savePower() {
    printf("savePower: Switching off 5V rail...", true);
    // turn off 5V rail (CO2-Sensor & Buzzer)
    uint8_t lBuffer[] = {U_INT_REG_WR_REQ_ACR0, ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT };
    // get rid of knx reference
    // knx.platform().writeUart(lBuffer, 2);
    Serial1.write(lBuffer, 2);
    // Turn off on board leds
    // digitalWrite(knx.ledPin(), HIGH - knx.ledPinActiveOn());
    digitalWrite(PROG_LED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
    // in future: Turn off i2c leds
    // in future: Turn off i2c 1Wire if possible
}

void restorePower(){
    printf("restorePower: Switching on 5V rail...", true);
    // turn off 5V rail (CO2-Sensor & Buzzer)
    uint8_t lBuffer[] = {U_INT_REG_WR_REQ_ACR0, ACR0_FLAG_DC2EN | ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT};
    // get rid of knx reference
    // knx.platform().writeUart(lBuffer, 2);
    Serial1.write(lBuffer, 2);
}

// const uint8_t gMagicWord[] = {0xAE, 0x49, 0xD2, 0x9F};
// const uint8_t gFiller[] = {0, 0, 0, 0};

// bool beginWriteKoEEPROM() {
//     uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
//     bool lResult = false;
//     // first we delete magic word, it is rewritten at the end. This is the ack for the successful write.
//     Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
//     Wire.write((uint8_t)((lAddress) >> 8)); // MSB
//     Wire.write((uint8_t)((lAddress)&0xFF)); // LSB
//     Wire.write(gFiller, 4);
//     lResult = Wire.endTransmission() == 0;
//     delay(EEPROM_WRITE_DELAY);
//     return lResult;
// }

// void writeMagicWordToEEPROM(uint16_t iAddress) {
//     Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
//     Wire.write((uint8_t)((iAddress) >> 8)); // MSB
//     Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
//     Wire.write(gMagicWord, 4);
//     Wire.endTransmission();
//     delay(EEPROM_WRITE_DELAY);
// }

// bool checkMagicWordInEEPROM(uint16_t iAddress) {
//     prepareReadEEPROM(iAddress, 4);
//     int lIndex = 0;
//     bool lResult = true;
//     while (lResult && lIndex < 4) 
//         lResult = Wire.available() && (gMagicWord[lIndex++] == Wire.read());
//     return lResult;
   
// }

// void endWriteKoEEPROM() {
//     // as a last step we write magic number back
//     // this is also the ACK, that writing was successfull
//     uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
//     writeMagicWordToEEPROM(lAddress);
// }

// bool gIsValidEEPROM = false;

// bool checkDataValidEEPROM() {
//     uint16_t lAddress = SAVE_BUFFER_START_PAGE * 32 + 12;
//     gIsValidEEPROM = checkMagicWordInEEPROM(lAddress);
//     return gIsValidEEPROM;
// }

// bool isValidEEPROM() {
//     return gIsValidEEPROM;
// }

// bool gIsTransmission = false;

// void beginPageEEPROM(uint16_t iAddress) {
//     if (!gIsTransmission)
//     {
//         Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
//         Wire.write((uint8_t)((iAddress) >> 8)); // MSB
//         Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
//         gIsTransmission = true;
//     }
// }

// void endPageEEPROM() {
//     if (gIsTransmission)
//     {
//         gIsTransmission = false;
//         Wire.endTransmission();
//         delay(EEPROM_WRITE_DELAY);
//     }
// }

// void write4BytesEEPROM(uint8_t* iData, uint8_t iLen) {
//     Wire.write(iData, iLen);
//     if (iLen < 4)
//         Wire.write(gFiller, 4 - iLen);
// }

// void prepareReadEEPROM(uint16_t iAddress, uint8_t iLen) {
//     Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
//     Wire.write((uint8_t)((iAddress) >> 8)); // MSB
//     Wire.write((uint8_t)((iAddress)&0xFF)); // LSB
//     Wire.endTransmission();
//     Wire.requestFrom(I2C_EEPROM_DEVICE_ADDRESSS, iLen);
// }

void fatalError(uint8_t iErrorCode, const char* iErrorText) {
    const uint16_t lDelay = 100;
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    for (;;)
    {
        // we repeat the message on serial bus, so we can get it even 
        // if we connect USB later
        printf("FatalError %d: %s", true, iErrorCode, iErrorText);
        digitalWrite(LED_YELLOW_PIN, HIGH);
        delay(lDelay);
        // number of red blinks during a yellow blink is the error code
        for (uint8_t i = 0; i < iErrorCode; i++)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(lDelay);
            digitalWrite(LED_BUILTIN, LOW);
            delay(lDelay);
        }
        digitalWrite(LED_YELLOW_PIN, LOW);
        delay(lDelay * 5);
    }
}

// bool delayCheck(uint32_t iOldTimer, uint32_t iDuration)
// {
//     return millis() - iOldTimer >= iDuration;
// }

bool boardCheck()
{
    bool lResult = false;
    // we check herer Hardware we rely on
    printf("Checking for EEPROM existence... ", false);
    // ceck for I2C ack
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    lResult = (Wire.endTransmission() == 0);
    printResult(lResult);

    printf("Checking for 1-Wire existence... ", false);
    // ceck for I2C ack
    Wire.beginTransmission(I2C_1WIRE_DEVICE_ADDRESSS);
    lResult = (Wire.endTransmission() == 0);
    printResult(lResult);

    printf("Checking NCN5130 existence... ", false);
    lResult = false;
    // send system state command and interpret answer
    uint8_t cmd = U_SYSTEM_STATE;
    // get rid of knx reference
    // knx.platform().setupUart();
    // knx.platform().writeUart(cmd);
    Serial1.begin(19200, SERIAL_8E1);
    while (!Serial1); 
    Serial1.write(cmd);

    uint32_t lUartResponseDelay = millis();
    while (!delayCheck(lUartResponseDelay, 100))
    {
        // int resp = knx.platform().readUart();
        int resp = Serial1.read();
        if (resp == U_SYSTEM_STAT_IND) {
            // resp = knx.platform().readUart();
            resp = Serial1.read();
            // "normal mode" answered
            lResult = (resp & 3) == 3;
            break;
        }
    }
    printResult(lResult);
    return lResult;
}
