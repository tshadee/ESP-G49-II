#include "SpeedRegulator.h"
#include "CommonDefs.h"

speedRegulator::speedRegulator(Encoder* LWC, Encoder* RWC): leftWheelEncoder(LWC),rightWheelEncoder(RWC)
{
    currentLeftPWM = currentRightPWM = DEFAULT_PWM;
    currentLeftSpeed = currentRightSpeed = 0.0f;
    targetLeftPWM = targetRightPWM = DEFAULT_PWM;
    targetLeftSpeed = targetRightSpeed = 0.0f;
};

void speedRegulator::updateTargetPWM(float leftPWM, float rightPWM) 
{
    targetLeftPWM = leftPWM;
    targetRightPWM = rightPWM;
    adjustPWMOutputOnSpeed();
};

void speedRegulator::adjustPWMOutputOnSpeed() 
{
    leftWheelEncoder->updateValues();
    rightWheelEncoder->updateValues();
    currentLeftSpeed = leftWheelEncoder->getSpeed();
    currentRightSpeed = rightWheelEncoder->getSpeed();
    easePWM();
    
    if(abs(targetLeftPWM - targetRightPWM) <= PWM_DIFFERENTIAL_FACTOR && targetLeftPWM > 0.5f && targetRightPWM > 0.5f) //GOING STRAIGHT
    {
        if(currentLeftSpeed > currentRightSpeed)
        {
            (currentRightPWM += (currentLeftSpeed - currentRightSpeed)*S_EASING_FACTOR);
            (currentLeftPWM -= (currentLeftSpeed - currentRightSpeed)*S_EASING_FACTOR);
        } else 
        {
            (currentLeftPWM += (currentRightSpeed - currentLeftSpeed)*S_EASING_FACTOR);
            (currentRightPWM -= (currentRightSpeed - currentLeftSpeed)*S_EASING_FACTOR);
        }
    } else if (abs(targetLeftPWM - targetRightPWM) <= PWM_DIFFERENTIAL_FACTOR && targetLeftPWM < 0.5f && targetRightPWM > 0.5f) //turning LEFT
    {
        abs(currentLeftPWM);
        abs(currentRightPWM);
        (currentRightPWM += (currentLeftSpeed - currentRightSpeed)*S_EASING_FACTOR);
        (currentLeftPWM -= (currentLeftSpeed - currentRightSpeed)*S_EASING_FACTOR);
        currentLeftPWM = -1*currentLeftPWM;
    };

    
    currentLeftPWM = (currentLeftPWM < 0.0f)? 0.0f : ((currentLeftPWM > 1.0f)? 1.0f : currentLeftPWM);
    currentRightPWM = (currentRightPWM < 0.0f)? 0.0f : ((currentRightPWM > 1.0f)? 1.0f : currentRightPWM);
};

void speedRegulator::easePWM()
{
    currentLeftPWM += (targetLeftPWM - currentLeftPWM)*EASING_FACTOR;
    currentRightPWM += (targetRightPWM - currentRightPWM)*EASING_FACTOR;
};

float speedRegulator::getCurrentLeftPWM(void){return currentLeftPWM;};
float speedRegulator::getCurrentRightPWM(void){return currentRightPWM;};
