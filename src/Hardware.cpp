#include <Wire.h>
#include "Hardware.h"
#include "Helper.h"
#include "EepromManager.h"
#ifdef WATCHDOG
#include <Adafruit_SleepyDog.h>
#endif

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
    const uint16_t lDelay = 200;
#ifdef WATCHDOG
    Watchdog.disable();
#endif
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

// call this BEFORE Wire.begin()
// it clears I2C Bus, calls Wire.begin() and checks which board hardware is available
bool boardCheck()
{
    bool lResult = false;

    // first we clear I2C-Bus
    Wire.end(); // in case, Wire.begin() was called before
    uint8_t lI2c = 0;
    lI2c = clearI2cBus(); // clear the I2C bus first before calling Wire.begin()
    switch (lI2c)
    {
    case 1:
        printDebug("SCL clock line held low\n");
        break;
    case 2:
        printDebug("SCL clock line held low by slave clock stretch\n");
        break;
    case 3:
        printDebug("SDA data line held low\n");
        break;
    default:
        printDebug("I2C bus cleared successfully\n");
        Wire.begin();
        lResult = true;
        break;
    }

    if (!lResult) {
        fatalError(5, "Failed to initialize I2C-Bus");
    }
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

/**
 * This routine turns off the I2C bus and clears it
 * on return SCA and SCL pins are tri-state inputs.
 * You need to call Wire.begin() after this to re-enable I2C
 * This routine does NOT use the Wire library at all.
 *
 * returns 0 if bus cleared
 *         1 if SCL held low.
 *         2 if SDA held low by slave clock stretch for > 2sec
 *         3 if SDA held low after 20 clocks.
 */
uint8_t clearI2cBus()
{
#if defined(TWCR) && defined(TWEN)
    TWCR &= ~(_BV(TWEN)); //Disable the Atmel 2-Wire interface so we can control the SDA and SCL pins directly
#endif
    pinMode(SDA, INPUT_PULLUP); // Make SDA (data) and SCL (clock) pins Inputs with pullup.
    pinMode(SCL, INPUT_PULLUP);

    // delay(2500); // Wait 2.5 secs. This is strictly only necessary on the first power
    // up of the DS3231 module to allow it to initialize properly,
    // but is also assists in reliable programming of FioV3 boards as it gives the
    // IDE a chance to start uploaded the program
    // before existing sketch confuses the IDE by sending Serial data.

    boolean SCL_LOW = (digitalRead(SCL) == LOW); // Check is SCL is Low.
    if (SCL_LOW)
    {             //If it is held low Arduno cannot become the I2C master.
        return 1; //I2C bus error. Could not clear SCL clock line held low
    }

    boolean SDA_LOW = (digitalRead(SDA) == LOW); // vi. Check SDA input.
    int clockCount = 20;                         // > 2x9 clock

    while (SDA_LOW && (clockCount > 0))
    { //  vii. If SDA is Low,
        clockCount--;
        // Note: I2C bus is open collector so do NOT drive SCL or SDA high.
        pinMode(SCL, INPUT);        // release SCL pullup so that when made output it will be LOW
        pinMode(SCL, OUTPUT);       // then clock SCL Low
        delayMicroseconds(10);      //  for >5uS
        pinMode(SCL, INPUT);        // release SCL LOW
        pinMode(SCL, INPUT_PULLUP); // turn on pullup resistors again
        // do not force high as slave may be holding it low for clock stretching.
        delayMicroseconds(10); //  for >5uS
        // The >5uS is so that even the slowest I2C devices are handled.
        SCL_LOW = (digitalRead(SCL) == LOW); // Check if SCL is Low.
        int counter = 20;
        while (SCL_LOW && (counter > 0))
        { //  loop waiting for SCL to become High only wait 2sec.
            counter--;
            delay(100);
            SCL_LOW = (digitalRead(SCL) == LOW);
        }
        if (SCL_LOW)
        {             // still low after 2 sec error
            return 2; // I2C bus error. Could not clear. SCL clock line held low by slave clock stretch for >2sec
        }
        SDA_LOW = (digitalRead(SDA) == LOW); //   and check SDA input again and loop
    }
    if (SDA_LOW)
    {             // still low
        return 3; // I2C bus error. Could not clear. SDA data line held low
    }

    // else pull SDA line low for Start or Repeated Start
    pinMode(SDA, INPUT);  // remove pullup.
    pinMode(SDA, OUTPUT); // and then make it LOW i.e. send an I2C Start or Repeated start control.
    // When there is only one I2C master a Start or Repeat Start has the same function as a Stop and clears the bus.
    /// A Repeat Start is a Start occurring after a Start with no intervening Stop.
    delayMicroseconds(10);      // wait >5uS
    pinMode(SDA, INPUT);        // remove output low
    pinMode(SDA, INPUT_PULLUP); // and make SDA high i.e. send I2C STOP control.
    delayMicroseconds(10);      // x. wait >5uS
    pinMode(SDA, INPUT);        // and reset pins as tri-state inputs which is the default state on reset
    pinMode(SCL, INPUT);
    return 0; // all ok
}
