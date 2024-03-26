#include "PIDSys.h"

PIDSys::PIDSys(TCRT *s1, TCRT *s2, TCRT *s4, TCRT *s5, TCRT *s6, Encoder *LWC, Encoder *RWC)
    : S1(s1), S2(s2), S4(s4), S5(s5), S6(s6), output(0), leftSpeed(0.0f), rightSpeed(0.0f),leftWheelEncoder(LWC), rightWheelEncoder(RWC)
{
    reset();
};

void PIDSys::reset()
{
    error[1] = error[0] = output = 0;
    leftSpeed = 0.0f;
    rightSpeed = 0.0f;
};

void PIDSys::calculatePID()
{
    error[1] = error[0];

    error[0] = (S6->getSensorVoltage(true)*GUARD2_SCALING +
                S5->getSensorVoltage(true)*GUARD1_SCALING + 
                S4->getSensorVoltage(true)*EDGE_SCALING - 
                S3->getSensorVoltage(true)*EDGE_SCALING -
                S2->getSensorVoltage(true)*GUARD1_SCALING - 
                S1->getSensorVoltage(true)*GUARD2_SCALING);

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
