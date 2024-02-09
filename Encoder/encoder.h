#ifndef ENCODER_H
#define ENCODER_H


#include "mbed.h"
#include "QEI.h"


/*
Records QEI inputs and returns speed and distance for wheel. Use getDist() and getSpeed().
*/
//class QEI;

class Encoder {
    private:
        QEI* encode;
        uint32_t count, countPrev;
        float distance, speed;
        int countBuffer;
    public:
        Encoder(PinName channel1, PinName channel2);
        void updateValues(void);
        void resetAllValues(void);
        float getDist(void);  //returns distance from last reset() call
        float getSpeed(void);    //returns speed in m/s
};

#endif