#ifndef SPEEDREGULATOR_H
#define SPEEDREGULATOR_H

#include "encoder.h"

// Speed regulator. Takes the PWM output of the PIDSys class and eases actual PWM towards the target value (similar to PID). This is to prevent sudden changes to buggy trajectory.
class speedRegulator
{
private:
    float currentLeftPWM, currentRightPWM;
    float targetLeftPWM, targetRightPWM;
    float currentLeftSpeed, currentRightSpeed;
    float targetLeftSpeed, targetRightSpeed;
    Encoder *leftWheelEncoder;
    Encoder *rightWheelEncoder;

public:
    speedRegulator(Encoder *LWC, Encoder *RWC);
    void updateTargetPWM(float leftPWM, float rightPWM);
    void adjustPWMOutputOnSpeed();
    void easePWM();
    float getCurrentLeftPWM(void);
    float getCurrentRightPWM(void);
};

#endif
