#include "SpeedRegulator.h"
#include "CommonDefs.h"

speedRegulator::speedRegulator(Encoder *LWC, Encoder *RWC) : leftWheelEncoder(LWC), rightWheelEncoder(RWC)
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
    float targetDiff = targetLeftPWM - targetRightPWM;
    float speedDiff = currentLeftSpeed - currentRightSpeed;
    float absSpeedDiff = abs(currentLeftSpeed) - abs(currentRightSpeed);

    easePWM();

    if(abs(targetDiff) <= PWM_DIFFERENTIAL_FACTOR)
    {
        if((targetLeftPWM != 0.5f) && (targetRightPWM != 0.5f))
        {
            if (speedDiff > 0) //left wheel faster than right wheel
            {
                currentRightPWM += abs(speedDiff) * S_EASING_FACTOR;
                currentLeftPWM -= abs(speedDiff) * S_EASING_FACTOR;
            } 
            else if (speedDiff < 0 ) //right wheel faster than left wheel
            {
                currentRightPWM -= abs(speedDiff) * S_EASING_FACTOR;
                currentLeftPWM += abs(speedDiff) * S_EASING_FACTOR;
            } 
        }
        else 
        {
            if ((currentLeftSpeed != 0) && (currentRightSpeed != 0))
            {
                currentLeftPWM = abs(0.5-currentLeftSpeed * BRAKING_FACTOR);
                currentRightPWM = abs(0.5-currentRightSpeed * BRAKING_FACTOR);
            }
            else
            {
                currentLeftPWM = currentRightPWM = 0.5f;
            };
        };
    }
    else
    {
        if (targetDiff < 0) // turning LEFT
        {
            if (abs(currentLeftSpeed) < abs(currentRightSpeed)) // right faster
            {
                (currentRightPWM += S_EASING_FACTOR * absSpeedDiff); 
            }
            else // left faster
            { 
                (currentLeftPWM += S_EASING_FACTOR * absSpeedDiff);
            }
        }
        else if (targetDiff > 0) // turning RIGHT
        {
            if (abs(currentLeftSpeed) > abs(currentRightSpeed)) // left (forward) faster
            {
                (currentLeftPWM -= S_EASING_FACTOR * absSpeedDiff);
            }
            else // right (reverse) faster
            { 
                (currentRightPWM -= S_EASING_FACTOR * absSpeedDiff); 
            };
        };
    };
    
    //PWM clamping
    currentLeftPWM = (currentLeftPWM < 0.02f) ? 0.0f : ((currentLeftPWM > 0.98f) ? 1.0f : currentLeftPWM);
    currentRightPWM = (currentRightPWM < 0.02f) ? 0.0f : ((currentRightPWM > 0.98f) ? 1.0f : currentRightPWM);
};

void speedRegulator::easePWM()
{
    currentLeftPWM += (targetLeftPWM - currentLeftPWM) * EASING_FACTOR;
    currentRightPWM += (targetRightPWM - currentRightPWM) * EASING_FACTOR; // easing curve, never reaches target value
};

float speedRegulator::getCurrentLeftPWM(void) { return currentLeftPWM; };
float speedRegulator::getCurrentRightPWM(void) { return currentRightPWM; };
