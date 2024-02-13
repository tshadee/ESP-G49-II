#include "UserInputInterrupts.h"


//UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX) : centreJoy(centre), toggleState(false), HM10(TX,RX), PC(USBTX,USBRX)
UserInputInterrupts::UserInputInterrupts(PinName TX, PinName RX) : toggleState(false), HM10(TX,RX)
{
    HM10.baud(9600);
    //PC.baud(9600);
};

bool UserInputInterrupts::serialConfigReady()
{
    while(!HM10.writeable()){};
    return true;
};

void UserInputInterrupts::pullHM10()
{
    int i = 0;
    errorBLE = false;
    if(HM10.readable())
    {
        while(i < BLE_BUFFER_DEPTH){ s[i++] = HM10.getc(); }; 
        i = 0;
        if(strncmp(s,"491000")){intRC = 8;} else //F
        if(strncmp(s,"490100")){intRC = 4;} else //B
        if(strncmp(s,"490010")){intRC = 2;} else //L
        if(strncmp(s,"490001")){intRC = 1;};     //R
        memset(s,0,BLE_BUFFER_DEPTH);
    } else {
        errorBLE = true;
    };
};

int UserInputInterrupts::getIntRC(){return intRC;};
bool UserInputInterrupts::getErrorBLE(){return errorBLE;};

void UserInputInterrupts::centreISR() 
{
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
