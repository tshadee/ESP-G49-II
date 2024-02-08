#ifndef PWMGEN_H
#define PWMGEN_h

#include "mbed.h"

class PWMGen {
    private:
        PwmOut PWM_LEFT, PWM_RIGHT;
    public:
        PWMGen(PinName P1, PinName P2);
        void reset();
        void begin();
        void setPWMDuty(float leftPWM = 0.5f, float rightPWM = 0.5f);
};


#endif