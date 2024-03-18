#include "SpeedRegulator.h"
#include "CommonDefs.h"

speedRegulator::speedRegulator(Encoder *LWC, Encoder *RWC) : leftWheelEncoder(LWC), rightWheelEncoder(RWC)
{
    currentLeftPWM = currentRightPWM = DEFAULT_PWM;
    currentLeftSpeed = currentRightSpeed = 0.0f;
    targetLeftSpeed = targetRightSpeed = 0.0f;
};

/*
Set target speed for left wheel and right wheel.
Preceded by PID calculation and succeeded by PWM Generation. Ran in main loop.
Will call adjustPWMOutputOnSpeed() and has dependencies in Encoder.h/cpp and CommonDefs.h
*/
void speedRegulator::updateTargetSpeed(float leftSpeed, float rightSpeed)
{
    targetLeftSpeed = leftSpeed;
    targetRightSpeed = rightSpeed;
    adjustPWMOutputOnSpeed();
};

/*
Can only be entered through calling updateTargetSpeed().
Adjusts PWM based on the speed difference between measured speed and target speed.
The rate of easing is dependent on SYS_OUTPUT_RATE and EASING_FACTOR, found in CommonDefs.h
*/
void speedRegulator::adjustPWMOutputOnSpeed()
{
    leftWheelEncoder->updateValues();                                       //forces encoder update. Be careful of aliasing at low speed if high main loop frequency
    rightWheelEncoder->updateValues();                                      //""
    currentLeftSpeed = leftWheelEncoder->getSpeed();                        //gets speed from encoder. 
    currentRightSpeed = rightWheelEncoder->getSpeed();                      //""
    float leftSpeedDiff = targetLeftSpeed - currentLeftSpeed;               //calculates left speed difference
    float rightSpeedDiff = targetRightSpeed - currentRightSpeed;            //calculates right speed difference

    currentLeftPWM += leftSpeedDiff * EASING_FACTOR / SYS_OUTPUT_RATE;      //adjusts PWM output based on speed difference and EASING_FACTOR (higher ease is faster adjustment)
    currentRightPWM += rightSpeedDiff * EASING_FACTOR / SYS_OUTPUT_RATE;    //""

    //PWM clamping; PWM is expected to exceed 1.0f and 0.0f with uncapped PID output. 
    //Clamping is already done by motor drive board, but clamping here prevents overshoot and extra delay.
    // currentLeftPWM = (currentLeftPWM < 0.00f) ? 0.0f : ((currentLeftPWM > 1.0f) ? 1.0f : currentLeftPWM);
    // currentRightPWM = (currentRightPWM < 0.00f) ? 0.0f : ((currentRightPWM > 1.0f) ? 1.0f : currentRightPWM);
};

/*
Get current LEFT PWM from within adjustPWMOutputOnSpeed()
*/
float speedRegulator::getCurrentLeftPWM(void) { return currentLeftPWM; };

/*
Get current RIGHT PWM from within adjustPWMOutputOnSpeed()
*/
float speedRegulator::getCurrentRightPWM(void) { return currentRightPWM; };
