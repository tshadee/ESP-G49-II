#include "ExternalStimulus.h"

// UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX) : centreJoy(centre), toggleState(false), HM10(TX,RX), PC(USBTX,USBRX)
ExternalStim::ExternalStim(PinName TX, PinName RX) : HM10(TX, RX)
{
    intRC = prevRC = 0;
    i = 0;
};

bool ExternalStim::serialConfigReady()
{
    HM10.baud(9600);
    while (!HM10.writeable())
    {
    };

    return true;
};

bool ExternalStim::pullHM10()
{
    if (HM10.readable())
    {
        intRC = HM10.getc();
        return true;
    }
    else
    {
        return false;
    };
};

int ExternalStim::getIntRC()
{
    return intRC;
};

void ExternalStim::centreISR()
{
    extern pstate ProgramState; // accessing global variable
    switch (ProgramState)
    {
    case starting:
        ProgramState = straightline;
        break;
    case straightline:
        ProgramState = stop;
        break;
    case stop:
        ProgramState = turnaround;
        break;
    case turnaround:
        ProgramState = starting;
        break;
    default:
        break;
    };
};
