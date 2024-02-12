#ifndef USER_INPUT_INTERRUPTS_H
#define USER_INPUT_INTERRUPTS_H

#include "mbed.h"

class UserInputInterrupts {
private:
    InterruptIn centreJoy;
    Timeout joyDebounce;
    bool toggleState;
    Serial HM10;
    //Serial PC;
    char s[4];
    bool up,down,left,right;

public:
    //UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX);
    UserInputInterrupts(PinName centre, PinName TX, PinName RX);
    void toggleInput();
    void timerDebounce();
    void centreISR();
    bool serialConfigReady();
    void pullHM10();
    bool getUp();
    bool getDown();
    bool getLeft();
    bool getRight();
};

#endif // USER_INPUT_INTERRUPTS_H
