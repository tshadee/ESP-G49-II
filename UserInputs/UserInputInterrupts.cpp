#include "UserInputInterrupts.h"
#include "CommonDefs.h"

//UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX) : centreJoy(centre), toggleState(false), HM10(TX,RX), PC(USBTX,USBRX)
UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX) : centreJoy(centre), toggleState(false), HM10(TX,RX)

{
    centreJoy.rise(callback(this, &UserInputInterrupts::centreISR));
    HM10.baud(9600);
    //PC.baud(9600);
    up = down = left = right = false;
};

bool UserInputInterrupts::serialConfigReady()
{
    while(!HM10.writeable()){};
    return true;
};

void UserInputInterrupts::pullHM10()
{
    if(HM10.readable())
    {
        s = HM10.getc();
        s[0] == 1 ? up = true       : up = false;
        s[1] == 1 ? down = true     : down = false;
        s[2] == 1 ? left = true     : left = false;
        s[3] == 1 ? right = true    : right = false;
    };
};

bool UserInputInterrupts::getUp(){return up;};
bool UserInputInterrupts::getDown(){return down;};
bool UserInputInterrupts::getLeft(){return left;};
bool UserInputInterrupts::getRight(){return right;};

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
