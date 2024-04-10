#ifndef ENCODER_H
#define ENCODER_H

#include "mbed.h"
#include "QEI.h"
#include <cstdint>

/*
Records QEI inputs and returns speed and distance for wheel. Use getDist() and getSpeed().
*/

// forward declaration of QEI class
class QEI;

class Encoder
{
private:
    QEI *encode;
    int count, countPrev;
    double distance;
    double speed;
    int countBuffer;

public:

    /**
    Constructor.

    Polls QEI output to update internal functions. Calculates speed and distance for each wheel from QEI count.
    Dependent on SYS_OUTPUT_RATE, CPR, and WHEEL_DIAMETER (CommonDefs.h)
    @param q QEI object pointer.

    **/
    Encoder(QEI *q);

    //Updates all values, calculates speed and distance, and resets QEI pulses count.
    void updateValues(void);

    //Forces reset all values
    void resetAllValues(void);

    // returns distance (m)
    double getDist(void);  

    //returns speed (m/s)
    double getSpeed(void); 

    //returns encoder count for debugging
    int getCount(void); 

    //resets distance only
    void resetDistance(void);
};

#endif