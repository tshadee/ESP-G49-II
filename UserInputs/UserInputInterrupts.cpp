#include "UserInputInterrupts.h"
#include "CommonDefs.h"

UserInputInterrupts::UserInputInterrupts(PinName centre) : centreJoy(centre), toggleState(false) 
{
    centreJoy.rise(callback(this, &UserInputInterrupts::centreISR));
};

void UserInputInterrupts::toggleInput() 
{
    toggleState = !toggleState;
    if(toggleState) 
    {
        centreJoy.disable_irq();
    } else {
        centreJoy.enable_irq();
    };
};

void UserInputInterrupts::timerDebounce() 
{
    joyDebounce.attach(callback(this, &UserInputInterrupts::toggleInput), 0.3);
};

void UserInputInterrupts::centreISR() 
{
    toggleInput();
    timerDebounce();
    extern pstate ProgramState; //accessing global variable
    switch(ProgramState) 
    {
        case starting: ProgramState = straightline; break;
        case straightline: ProgramState = stop; break;
        case stop: ProgramState = turnaround; break;
        case turnaround: ProgramState = starting; break;
        default: break;
    };
};
