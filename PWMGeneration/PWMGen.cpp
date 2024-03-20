#include "PWMGen.h"
#include "CommonDefs.h"

PWMGen::PWMGen(PinName leftPWMPin, PinName rightPWMPin, PinName MDBE, PinName Bipolar1E, PinName Bipolar2E) : PWM_LEFT(leftPWMPin), PWM_RIGHT(rightPWMPin), MDBEnable(MDBE), BPE1(Bipolar1E), BPE2(Bipolar2E) { reset(); };
void PWMGen::reset()
{
    PWM_LEFT.write(DEFAULT_PWM);
    PWM_RIGHT.write(DEFAULT_PWM);
};
void PWMGen::begin()
{
    PWM_LEFT.period(1.0 / PWM_FREQUENCY);       //sets PWM period for both PWM outputs
    PWM_RIGHT.period(1.0 / PWM_FREQUENCY);      //""
    MDBEnable.write(1);                         //enables motor drive board
    BPE1.write(1);                              //sets left motor to bipolar
    BPE2.write(1);                              //sets right motor to bipolar
};
void PWMGen::setPWMDuty(float leftPWM, float rightPWM)
{
    PWM_LEFT.write(leftPWM);
    PWM_RIGHT.write(rightPWM);
};
