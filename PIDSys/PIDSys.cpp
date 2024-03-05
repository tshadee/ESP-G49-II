#include "PIDSys.h"

PIDSys::PIDSys(TCRT *s1, TCRT *s2, TCRT *s4, TCRT *s5)
    : S1(s1), S2(s2), S4(s4), S5(s5), output(0), leftPWM(DEFAULT_PWM), rightPWM(DEFAULT_PWM)
{
    A0 = GAIN_PROPORTIONAL + GAIN_INTEGRAL / SYS_OUTPUT_RATE + GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    A1 = -GAIN_PROPORTIONAL - 2 * GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    A2 = GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    reset();
};

void PIDSys::reset()
{
    error[2] = error[1] = error[0] = output = 0;
    leftPWM = DEFAULT_PWM;
    rightPWM = DEFAULT_PWM;
};

void PIDSys::calculatePID(bool toggleAggressive)
{
    if (toggleAggressive)
    {
        errorOuter[2] = errorOuter[1];
        errorOuter[1] = errorOuter[0];
        errorOuter[0] = (S5->getSensorVoltage(true) - S1->getSensorVoltage(true));
        output = (output + A0 * errorOuter[0] + A1 * errorOuter[1] + A2 * errorOuter[2]) * GAIN_AGGRESSIVE;
    }
    else
    {
        error[2] = error[1];
        error[1] = error[0];
        error[0] = (S4->getSensorVoltage(true) - S2->getSensorVoltage(true));
        output = output + A0 * error[0] + A1 * error[1] + A2 * error[2];
    }
    outputPWM();
};

void PIDSys::outputPWM()
{
    if (output > 0)
    {
        leftPWM = BASE_DUTY + output;
        rightPWM = BASE_DUTY - output;
    }
    else if (output < 0)
    {
        leftPWM = BASE_DUTY - output;
        rightPWM = BASE_DUTY + output;
    }
    else
    {
        leftPWM = rightPWM = 0.5f; // stop in case anything goes wrong
    };
};

float PIDSys::getLeftPWM() const
{
    return leftPWM;
};

float PIDSys::getRightPWM() const
{
    return rightPWM;
};
