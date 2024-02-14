#include "ExternalStimulus.h"

//UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX) : centreJoy(centre), toggleState(false), HM10(TX,RX), PC(USBTX,USBRX)
ExternalStim::ExternalStim(PinName TX, PinName RX) : HM10(TX,RX)
{ 
    intRC = prevRC = 0;
    i = 0;
}; 

bool ExternalStim::serialConfigReady()
{
    HM10.baud(9600);
    while(!HM10.writeable()){};

    return true;
};

void ExternalStim::pullHM10()
{
    if(HM10.readable())
    {
        cache = HM10.getc();
        if(cache == 'A'){intRC = 8;} else
        if(cache == 'B'){intRC = 9;} else { intRC = 0; };

        /*
        char bleBuffer[] = {0};
        while(i < BLE_BUFFER_DEPTH - 1){ bleBuffer[i++] = HM10.getc(); }; 
        bleBuffer[i] = '\0';
        i = 0;
        intRC = 2;




        if(strncmp(bleBuffer,"49100",BLE_BUFFER_DEPTH) == 0){intRC = 8;} else
        if(strncmp(bleBuffer,"49010",BLE_BUFFER_DEPTH) == 0){intRC = 9;} else { intRC = 0; };
        memset(bleBuffer,0,BLE_BUFFER_DEPTH);
        */
    } 
    else 
    {
        intRC = 4; //error code
    };
};

int ExternalStim::getIntRC()
{
    return intRC;
};

void ExternalStim::centreISR() 
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
