#include "Helper.h"

// generic helper for formatted debug output
int printf(const char *format, bool iNewLine, ...)
{
    char buffer[256];
    va_list args;
    va_start(args, format);
    int lResult = vsnprintf(buffer, 256, format, args);
    va_end(args);
    if (iNewLine) 
        SerialUSB.println(buffer);
    else
        SerialUSB.print(buffer);
    return lResult;
}

void printHEX(const char* iPrefix, const uint8_t *iData, size_t iLength)
{
    SerialUSB.print(iPrefix);
    for (size_t i = 0; i < iLength; i++) {
        if (iData[i] < 0x10) { SerialUSB.print("0"); }
        SerialUSB.print(iData[i], HEX);
        SerialUSB.print(" ");
    }
    SerialUSB.println();
}

void printResult(bool iResult)
{
    SerialUSB.println(iResult ? "OK" : "FAIL");
}

// ensure correct time delta check
// cannot be used in interrupt handler
bool delayCheck(uint32_t iOldTimer, uint32_t iDuration)
{
    return millis() - iOldTimer >= iDuration;
}
