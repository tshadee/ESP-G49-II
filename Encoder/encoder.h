#ifndef ENCODER_H
#define ENCODER_H

#define CPR 256
#define WHEEL_DIAMETER 0.08//m
#define GEAR_RATIO 1/12
#define PI 3.14
#define CPR 256
#define SYS_OUTPUT_RATE 50 //Hz

#include "mbed.h"
#include "QEI.h"


/*
Records QEI inputs and returns speed and distance for wheel. Use getDist() and getSpeed().
*/
class QEI;

class Encoder {
    private:
        QEI encode;
        uint32_t count, countPrev;
        float distance, speed;
    public:
        Encoder(PinName channel1, PinName channel2);
        void updateValues(void);
        void resetAllValues(void);
        float getDist(void);  //returns distance from last reset() call
        float getSpeed(void);    //returns speed in m/s
};

#endif