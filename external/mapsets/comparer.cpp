#include "common.h"
#include "comparer.h"

const double Tolerance = 1e-8;

bool Close(double a, double b) 
{
    return Abs(a - b) <= Tolerance;
}

int Compare(double numberA, double numberB) 
{
    double c = numberA - numberB;
    // The <= and >= here complement the < and > in Close(double, double).
    if (c <= -Tolerance)
        return -1;
    if (c >= Tolerance)
        return 1;
    return 0;
}

bool GreaterOrEqual(double numberA, double numberB) 
{
    return Compare(numberA, numberB) >= 0;
}

bool LessOrEqual(double numberA, double numberB) 
{
    return Compare(numberA, numberB) <= 0;
}
