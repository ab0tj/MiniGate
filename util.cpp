#include <math.h>

float fround(float f, uint digits)
{
    if (digits == 0) return round(f);
    return float(int(f * pow(10, digits) + 0.5)) / pow(10, digits);
}