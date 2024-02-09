#include "encoder.h"
#include "CommonDefs.h"




Encoder::Encoder(QEI* q): encode(q){ resetAllValues(); };

//Encoder::Encoder(PinName channel1, PinName channel2): encode(channel1,channel2,NC,CPR,QEI::X4_ENCODING), count(0), countPrev(0), distance(0.0f), speed(0.0f){ resetAllValues(); };
void Encoder::updateValues(void)
{
    countPrev = count;
    count += encode->getPulses();
    distance = encode->getRevolutions()*WHEEL_DIAMETER*PI;
    countBuffer = count - countPrev;
    speed = ((static_cast<float>(countBuffer)/CPR)*SYS_OUTPUT_RATE)*WHEEL_DIAMETER*PI*GEAR_RATIO;
};
void Encoder::resetAllValues(void)
{
    encode.reset();
    distance = speed = 0.0f;
    count = countPrev = 0;
};
float Encoder::getDist(void){return distance;};  //returns distance from last reset() call
float Encoder::getSpeed(void){return speed;};    //returns speed in m/s