#include "encoder.h"
#include "CommonDefs.h"

Encoder::Encoder(QEI *q) : encode(q) { resetAllValues(); };

void Encoder::updateValues(void)
{
    countPrev = count;
    count -= encode->getPulses();
    distance = count * WHEEL_DIAMETER * PI / CPR;
    countBuffer = count - countPrev; // difference in count
    speed = ((static_cast<double>(countBuffer) / CPR) * SYS_OUTPUT_RATE) * WHEEL_DIAMETER * PI;
    encode->reset();
};

void Encoder::resetAllValues(void)
{
    encode->reset();
    distance = speed = 0.0f;
    count = countPrev = 0;
};

void Encoder::resetDistance(void)
{
    encode->reset();
    distance = count = 0.0f;
};

double Encoder::getDist(void) { return distance; }; // returns distance from last reset() call
double Encoder::getSpeed(void) { return speed; };   // returns speed in m/s
int Encoder::getCount(void) { return count; };
