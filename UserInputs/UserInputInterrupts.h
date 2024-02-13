#ifndef USER_INPUT_INTERRUPTS_H
#define USER_INPUT_INTERRUPTS_H

#include "mbed.h"
#include "CommonDefs.h"

class UserInputInterrupts {
private:
    InterruptIn centreJoy;
    Timeout joyDebounce;
    bool toggleState;
    Serial HM10;
    //Serial PC;
    char s[BLE_BUFFER_DEPTH];
    bool errorBLE;
    int intRC;

public:
    //UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX);
    UserInputInterrupts(PinName TX, PinName RX);
    void toggleInput();
    void timerDebounce();
    void centreISR();
    bool serialConfigReady();
    void pullHM10();
    bool getErrorBLE();
    int getIntRC();
};

#endif // USER_INPUT_INTERRUPTS_H
