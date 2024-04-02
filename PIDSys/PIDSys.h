#ifndef PIDSYS_H
#define PIDSYS_H

#include "TCRT.h"
#include "CommonDefs.h"
#include "encoder.h"

/**
This class is for calculating PID error as two PWM outputs. This is unfiltered output, btw, so we will have to pass this through a speed regulator that takes into account current wheel speed and other buggy params.
The PID output value assume that the error is RIGHT - LEFT (looking from buggy top view), so a situation where right > left would be when the buggy is headed left and vice versa.
**/
class PIDSys
{
private:
    double error[3];
    double output;
    double leftSpeed, rightSpeed;
    double A0, A1, A2;
    double S6V,S5V,S4V,S3V,S2V,S1V;
    TCRT *S1;
    TCRT *S2;
    TCRT *S3;
    TCRT *S4;
    TCRT *S5;
    TCRT *S6;
    Encoder *leftWheelEncoder;
    Encoder *rightWheelEncoder;

public:
    /**
    Constructor.

    Calculates PID output between four sensors (middle sensor excluded). PID system is first-order, hence only uses a two elements of error array to store error history.
    Intermediate PID value is then converted into speed difference between two wheels, which is later fed into the SpeedRegulator Class.
    Sensors are given individual gain separate from PID gain values, both can be found in CommonDef.h
    No speed easing is done here.

    @param s1 Pointer to TCRT class sensor 1 (left-most sensor)
    @param s2 Pointer to TCRT class sensor 2 (left-middle sensor)
    @param s3 Pointer to TCRT class sensor 2 (left-middle sensor)
    @param s4 Pointer to TCRT class sensor 4 (right-middle sensor)
    @param s5 Pointer to TCRT class sensor 5 (right-most sensor)
    @param s6 Pointer to TCRT class sensor 5 (right-most sensor)
    **/
    PIDSys(TCRT *s1, TCRT *s2, TCRT *s3, TCRT *s4, TCRT *s5, TCRT *s6, Encoder *LWC, Encoder *RWC);

    //Use to forcefully reset error history and set speed to 0.0f
    void reset();

    /**
    Resource heavy function.

    Call to update PID output, leftSpeed, and rightSpeed based on current sensor values.
    **/
    void calculatePID();

    //Gets the left speed of PID output. Make sure to call calculatePID() before this.
    double getLeftSpeed() const;

    //Gets the right speed of PID output. Make sure to call calculatePID() before this.
    double getRightSpeed() const;

    //Gets intermediate PID output before conversion to left/right speed.
    double getOutput() const;
    
};

#endif // PIDSYS_H
