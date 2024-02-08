#include "PWMGen.h"
#include "CommonDefs.h"


PWMGen::PWMGen(PinName P1, PinName P2): PWM_LEFT(P1),PWM_RIGHT(P2) { reset();};
void PWMGen::reset()
{
    //PWM_LEFT.period(0.0f);
    //PWM_RIGHT.period(0.0f);
    PWM_LEFT.write(DEFAULT_PWM);
    PWM_RIGHT.write(DEFAULT_PWM);
};
void PWMGen::begin()
{
    PWM_LEFT.period(1.0/SYS_OUTPUT_RATE);
    PWM_RIGHT.period(1.0/SYS_OUTPUT_RATE);
};
void PWMGen::setPWMDuty(float leftPWM, float rightPWM)
{
    PWM_LEFT.write(leftPWM);
    PWM_RIGHT.write(rightPWM);
};
