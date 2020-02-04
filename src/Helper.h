#pragma once

#include <cstdint>
#include <stdio.h>
#include <stdarg.h>
#include <Arduino.h>

/*********************
 * Helper for any module
 * *******************/

// generic helper for formatted debug output
int printDebug(const char *format, ...);
void printHEX(const char* iPrefix, const uint8_t *iData, size_t iLength);
void printResult(bool iResult);

bool delayCheck(uint32_t iOldTimer, uint32_t iDuration);
