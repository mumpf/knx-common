#include <Wire.h>
#include "Hardware.h"
#include "Helper.h"
#include "EepromManager.h"

void savePower() {
    printDebug("savePower: Switching off 5V rail...\n");
    // turn off 5V rail (CO2-Sensor, Buzzer, RGB-LED-Driver, 1-Wire-Busmaster)
    uint8_t lBuffer[] = {U_INT_REG_WR_REQ_ACR0, ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT };
    // get rid of knx reference
    Serial1.write(lBuffer, 2);
    // Turn off on-board leds
    digitalWrite(PROG_LED_PIN, LOW);
    digitalWrite(LED_YELLOW_PIN, LOW);
}

void restorePower(){
    printDebug("restorePower: Switching on 5V rail...\n");
    // turn off 5V rail (CO2-Sensor & Buzzer)
    uint8_t lBuffer[] = {U_INT_REG_WR_REQ_ACR0, ACR0_FLAG_DC2EN | ACR0_FLAG_XCLKEN | ACR0_FLAG_V20VCLIMIT};
    // get rid of knx reference
    Serial1.write(lBuffer, 2);
    // give all sensors some time to init
    delay(100);
}

void fatalError(uint8_t iErrorCode, const char* iErrorText) {
    const uint16_t lDelay = 100;
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    for (;;)
    {
        // we repeat the message on serial bus, so we can get it even 
        // if we connect USB later
        printDebug("FatalError %d: %s\n", iErrorCode, iErrorText);
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

bool boardCheck()
{
    bool lResult = false;

#ifdef I2C_EEPROM_DEVICE_ADDRESSS
    // we check herer Hardware we rely on
    printDebug("Checking EEPROM existence... ");
    // ceck for I2C ack
    Wire.beginTransmission(I2C_EEPROM_DEVICE_ADDRESSS);
    lResult = (Wire.endTransmission() == 0);
    printResult(lResult);
#endif

#ifdef I2C_1WIRE_DEVICE_ADDRESSS
    printDebug("Checking 1-Wire existence... ");
    // ceck for I2C ack
    Wire.beginTransmission(I2C_1WIRE_DEVICE_ADDRESSS);
    lResult = (Wire.endTransmission() == 0);
    printResult(lResult);
#endif

#ifdef I2C_RGBLED_DEVICE_ADDRESS
    printDebug("Checking LED driver existence... ");
    // ceck for I2C ack
    Wire.beginTransmission(I2C_RGBLED_DEVICE_ADDRESS);
    lResult = (Wire.endTransmission() == 0);
    printResult(lResult);
#endif

    printDebug("Checking NCN5130 existence... ");
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
