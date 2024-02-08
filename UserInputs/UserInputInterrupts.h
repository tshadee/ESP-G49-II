#ifndef USER_INPUT_INTERRUPTS_H
#define USER_INPUT_INTERRUPTS_H

#include "mbed.h"

class UserInputInterrupts {
private:
    InterruptIn centreJoy;
    Timeout joyDebounce;
    bool toggleState;

public:
    UserInputInterrupts(PinName centre);
    void toggleInput();
    void timerDebounce();
    void centreISR();
};

#endif // USER_INPUT_INTERRUPTS_H
