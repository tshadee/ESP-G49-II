#include "PIDSys.h"

PIDSys::PIDSys(TCRT *s1, TCRT *s2, TCRT *s4, TCRT *s5, Encoder *LWC, Encoder *RWC)
    : S1(s1), S2(s2), S4(s4), S5(s5), output(0), leftSpeed(0.0f), rightSpeed(0.0f),leftWheelEncoder(LWC), rightWheelEncoder(RWC)
{
    // A0 = GAIN_PROPORTIONAL + GAIN_INTEGRAL / SYS_OUTPUT_RATE + GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    // A1 = -GAIN_PROPORTIONAL - 2 * GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    // A2 = GAIN_DERIVATIVE * SYS_OUTPUT_RATE;
    reset();
};

void PIDSys::reset()
{
    error[2] = error[1] = error[0] = output = 0;
    leftSpeed = 0.0f;
    rightSpeed = 0.0f;
};

void PIDSys::calculatePID()
{

    error[2] = error[1];
    error[1] = error[0];

    error[0] = (S5->getSensorVoltage(true)*GUARD_SCALING + 
                S4->getSensorVoltage(true)*EDGE_SCALING - 
                S2->getSensorVoltage(true)*EDGE_SCALING - 
                S1->getSensorVoltage(true)*GUARD_SCALING);

    output = (((GAIN_PROPORTIONAL * error[0]) + 
               (GAIN_DERIVATIVE * (error[0] - error[1]) * SYS_OUTPUT_RATE) + 
               (GAIN_INTEGRAL * (error[0]/2 + error[1]/2) / SYS_OUTPUT_RATE)) 
               / GAIN_SCALE_DOWN);
               
    leftSpeed = BASE_SPEED + output;
    rightSpeed = BASE_SPEED - output;
};

float PIDSys::getLeftSpeed() const
{
    return leftSpeed;
};

float PIDSys::getRightSpeed() const
{
    return rightSpeed;
};

float PIDSys::getOutput() const
{
    return output;
};
