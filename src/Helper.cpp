#include "Helper.h"

// generic helper for formatted debug output
int printDebug(const char *format, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    int lResult = vsnprintf(buffer, 256, format, args);
    va_end(args);
    Serial.print(buffer);
    return lResult;
}

void printHEX(const char* iPrefix, const uint8_t *iData, size_t iLength)
{
    Serial.print(iPrefix);
    for (size_t i = 0; i < iLength; i++) {
        if (iData[i] < 0x10) { Serial.print("0"); }
        Serial.print(iData[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

void printResult(bool iResult)
{
    Serial.println(iResult ? "OK" : "FAIL");
}

// ensure correct time delta check
// cannot be used in interrupt handler
bool delayCheck(uint32_t iOldTimer, uint32_t iDuration)
{
    return millis() - iOldTimer >= iDuration;
}

// check if a float is a number (false if Not-a-number)
bool isNum(float iNumber) {
    return (iNumber + 10.0) > NO_NUM;
}