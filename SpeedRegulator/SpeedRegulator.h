#ifndef SPEEDREGULATOR_H
#define SPEEDREGULATOR_H

#include "encoder.h"

// Speed regulator. Takes the speed output of the PIDSys class and eases actual PWM towards the target value (similar to PID). This is to prevent sudden changes to buggy trajectory.
class speedRegulator
{
private:
    float currentLeftPWM, currentRightPWM;
    float currentLeftSpeed, currentRightSpeed;
    float targetLeftSpeed, targetRightSpeed;
    Encoder *leftWheelEncoder;
    Encoder *rightWheelEncoder;

    /*
    Can only be entered through calling updateTargetSpeed().
    Adjusts PWM based on the speed difference between measured speed and target speed.
    The rate of easing is dependent on SYS_OUTPUT_RATE and EASING_FACTOR, found in CommonDefs.h
    */
    void adjustPWMOutputOnSpeed();

public:
    speedRegulator(Encoder *LWC, Encoder *RWC);
    /*
    Set target speed for left wheel and right wheel.
    Preceded by PID calculation and succeeded by PWM Generation. Ran in main loop.
    Will call adjustPWMOutputOnSpeed() and has dependencies in Encoder.h/cpp and CommonDefs.h
    */
    void updateTargetSpeed(float leftspeed, float rightspeed);

    //Get current LEFT PWM from within adjustPWMOutputOnSpeed()
    float getCurrentLeftPWM(void);

    //Get current RIGHT PWM from within adjustPWMOutputOnSpeed()
    float getCurrentRightPWM(void);
};

#endif
