#include "OneWireDS2438Fromula.h"

OneWireDS2438Fromula::OneWireDS2438Fromula(/* args */)
{
}

OneWireDS2438Fromula::~OneWireDS2438Fromula()
{
}

float (*OneWireDS2438Fromula::userFunction[30])(float, float, float, float){
    userFunction1,
    userFunction2,
    userFunction3,
    userFunction4,
    userFunction5,
    userFunction6,
    userFunction7,
    userFunction8,
    userFunction9,
    userFunction10,
    userFunction11,
    userFunction12,
    userFunction13,
    userFunction14,
    userFunction15,
    userFunction16,
    userFunction17,
    userFunction18,
    userFunction19,
    userFunction20,
    userFunction21,
    userFunction22,
    userFunction23,
    userFunction24,
    userFunction25,
    userFunction26,
    userFunction27,
    userFunction28,
    userFunction29,
    userFunction30
};

// native functions for value conversions (already implemented)
float OneWireDS2438Fromula::nativeTemperatureNTC(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}
float OneWireDS2438Fromula::nativeHumidity(float iTemp, float iVDD, float iVAD1, float iVAD2) {
    return ((iVAD1 / iVDD) - 0.16) / 0.0062 / (1.0546 - (0.00216 * iTemp));
}
float OneWireDS2438Fromula::nativeBrightness(float iTemp, float iVDD, float iVAD1, float iVAD2) {
    return NAN;
}

// user functions for value conversions (empty, implemented by user)
float OneWireDS2438Fromula::userFunction1(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction2(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction3(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction4(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction5(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction6(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction7(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction8(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction9(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction10(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction11(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction12(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction13(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction14(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction15(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction16(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction17(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction18(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction19(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction20(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction21(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction22(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction23(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction24(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction25(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction26(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction27(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction28(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction29(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}

float OneWireDS2438Fromula::userFunction30(float iTemp, float iVDD, float iVAD1, float iVAD2)
{
    return NAN;
}
