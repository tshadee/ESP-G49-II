#include "SpeedRegulator.h"
#include "CommonDefs.h"

speedRegulator::speedRegulator(Encoder *LWC, Encoder *RWC) : leftWheelEncoder(LWC), rightWheelEncoder(RWC)
{
    currentLeftPWM = currentRightPWM = DEFAULT_PWM;
    currentLeftSpeed = currentRightSpeed = 0.0f;
    targetLeftSpeed = targetRightSpeed = 0.0f;
};

void speedRegulator::updateTargetSpeed(float leftSpeed, float rightSpeed)
{
    targetLeftSpeed = leftSpeed;
    targetRightSpeed = rightSpeed;
    adjustPWMOutputOnSpeed();
};

void speedRegulator::adjustPWMOutputOnSpeed()
{
    leftWheelEncoder->updateValues();                                       //forces encoder update. Be careful of aliasing at low speed if high main loop frequency
    rightWheelEncoder->updateValues();                                      //""
    currentLeftSpeed = leftWheelEncoder->getSpeed();                        //gets speed from encoder. 
    currentRightSpeed = rightWheelEncoder->getSpeed();                      //""
    float leftSpeedDiff = targetLeftSpeed - currentLeftSpeed;               //calculates left speed difference
    float rightSpeedDiff = targetRightSpeed - currentRightSpeed;            //calculates right speed difference

    errorArr[0][1] = errorArr[0][0];
    errorArr[1][1] = errorArr[0][1];
    errorArr[0][0] = leftSpeedDiff;
    errorArr[1][0] = rightSpeedDiff; 

    currentLeftPWM += (errorArr[0][0]*GAIN_P_WHEEL  + (errorArr[0][0]/2 + errorArr[1][1]/2)*GAIN_I_WHEEL/SYS_OUTPUT_RATE)/SYS_OUTPUT_RATE;
    currentRightPWM += (errorArr[1][0]*GAIN_P_WHEEL + (errorArr[1][0]/2 + errorArr[1][1]/2)*GAIN_I_WHEEL/SYS_OUTPUT_RATE)/SYS_OUTPUT_RATE;
};

float speedRegulator::getCurrentLeftPWM(void) { return currentLeftPWM; };

float speedRegulator::getCurrentRightPWM(void) { return currentRightPWM; };
