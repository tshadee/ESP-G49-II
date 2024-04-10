#include "PIDSys.h"

PIDSys::PIDSys(TCRT *s1, TCRT *s2, TCRT *s3, TCRT *s4, TCRT *s5, TCRT *s6, Encoder *LWC, Encoder *RWC)
    : S1(s1), S2(s2), S3(s3), S4(s4), S5(s5), S6(s6), output(0), leftSpeed(0.0f), rightSpeed(0.0f),leftWheelEncoder(LWC), rightWheelEncoder(RWC)
{
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

    S6V = S6->getSensorVoltage(true);
    S5V = S5->getSensorVoltage(true);
    S4V = S4->getSensorVoltage(true);
    S3V = S3->getSensorVoltage(true);
    S2V = S2->getSensorVoltage(true);
    S1V = S1->getSensorVoltage(true);

    error[0] = (S6V*S6_SCALE*GUARD2_SCALING +
                S5V*S5_SCALE*GUARD1_SCALING + 
                S4V*S4_SCALE*EDGE_SCALING - 
                S3V*S3_SCALE*EDGE_SCALING - 
                S2V*S2_SCALE*GUARD1_SCALING-
                S1V*S1_SCALE*GUARD2_SCALING);

    output = (((GAIN_PROPORTIONAL * error[0]) + 
               (GAIN_DERIVATIVE * (error[0] - error[1]) * SYS_OUTPUT_RATE) + 
               (GAIN_INTEGRAL * (error[0]/2 + error[1]/2) / SYS_OUTPUT_RATE)) 
               / GAIN_SCALE_DOWN);

    leftSpeed = BASE_SPEED + output;
    rightSpeed = BASE_SPEED - output;
};

double PIDSys::getLeftSpeed() const
{
    return leftSpeed;
};

double PIDSys::getRightSpeed() const
{
    return rightSpeed;
};

double PIDSys::getOutput() const
{
    return output;
};