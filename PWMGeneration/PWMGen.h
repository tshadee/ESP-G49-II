#ifndef PWMGEN_H
#define PWMGEN_h

#include "mbed.h"

class PWMGen
{
private:

    PwmOut PWM_LEFT, PWM_RIGHT;
    DigitalOut MDBEnable, BPE1, BPE2;

public:

    /**
    Constructor.

    Breaks down PWM input into Mbed write() and period() functions to be written to PwmOut pins, subsequently to the motor drive board.
    Configures motor drive board to work with bipolar (if Pins Bipolar1E and Bipolar2E are connected to MDB bipolar active pins).

    @param leftPWMPin Pin for outputting PWM to the right motor
    @param rightPWMPin Pin for outputting PWM to the right motor
    @param MDBE Motor Drive Board Enable. Set 1 to turn on MDB.
    @param Bipolar1E Bipolar 1 Enable. Set 1 to use Bipolar for left motor.
    @param Bipolar2E Bipolar 2 Enable. Set 1 to use Bipolar for right motor.
    **/
    PWMGen(PinName leftPWMPin, PinName rightPWMPin, PinName MDBE, PinName Bipolar1E, PinName Bipolar2E);

    //Resets PWM out to motor. Only use to force buggy stop.
    void reset();

    //Enables motor drive board, sets bipolar mode for both motors, and initiates PWM period. PWM duty cycle is set to 0.5f
    void begin();

    /**
    Sets the PWM duty cycle for motors. Does not interact with anything else.
    Remember to call begin() before calling this function to enable MDB and set bipolar mode.

    @param leftPWM set duty cycle of left motor
    @param rightPWM set duty cycle of right motor
    **/
    void setPWMDuty(float leftPWM, float rightPWM);
    
};

#endif
