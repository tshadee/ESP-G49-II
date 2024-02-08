#include "SpeedRegulator.h"
#include "CommonDefs.h"


speedRegulator::speedRegulator(Encoder* LWC, Encoder* RWC): leftWheelEncoder(LWC),rightWheelEncoder(RWC)
{
            currentLeftPWM = currentRightPWM = DEFAULT_PWM;
};

void speedRegulator::updateTargetPWM(float leftPWM, float rightPWM) 
{
    targetLeftPWM = leftPWM;
    targetRightPWM = rightPWM;
    easePWMOutput();
};

void speedRegulator::easePWMOutput() 
{
    currentLeftPWM += (targetLeftPWM - currentLeftPWM)*EASING_FACTOR;
    currentRightPWM += (targetRightPWM - currentRightPWM)*EASING_FACTOR;
    currentLeftPWM = (currentLeftPWM < 0.0f)? 0.0f : ((currentLeftPWM > 1.0f)? 1.0f : currentLeftPWM);
    currentRightPWM = (currentRightPWM < 0.0f)? 0.0f : ((currentRightPWM > 1.0f)? 1.0f : currentRightPWM);
};

float speedRegulator::getCurrentLeftPWM(void){return currentLeftPWM;};

float speedRegulator::getCurrentRightPWM(void){return currentRightPWM;};
