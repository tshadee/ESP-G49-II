#include "ExternalStimulus.h"

//UserInputInterrupts::UserInputInterrupts(PinName centre, PinName TX, PinName RX, PinName USBTX, PinName USBRX) : centreJoy(centre), toggleState(false), HM10(TX,RX), PC(USBTX,USBRX)
ExternalStim::ExternalStim(PinName TX, PinName RX) : HM10(TX,RX), intRC(0) { HM10.baud(9600); };

bool ExternalStim::serialConfigReady()
{
    while(!HM10.writeable()){};
    return true;
};

void ExternalStim::pullHM10()
{
    uint8_t i = 0;
    if(HM10.readable())
    {
        while(i < BLE_BUFFER_DEPTH){ bleBuffer[i++] = HM10.getc(); }; 
        i = 0;
        if(strncmp(bleBuffer,"491000",6) == 0){intRC = 8;memset(bleBuffer,0,BLE_BUFFER_DEPTH);} else //F
        if(strncmp(bleBuffer,"490100",6) == 0){intRC = 4;memset(bleBuffer,0,BLE_BUFFER_DEPTH);} else //B
        if(strncmp(bleBuffer,"490010",6) == 0){intRC = 2;memset(bleBuffer,0,BLE_BUFFER_DEPTH);} else //L
        if(strncmp(bleBuffer,"490001",6) == 0){intRC = 1;memset(bleBuffer,0,BLE_BUFFER_DEPTH);} else //R
                                              {intRC = 0;memset(bleBuffer,0,BLE_BUFFER_DEPTH);};
    } else {
        intRC = 9;
    };
};

int ExternalStim::getIntRC(){return intRC;};

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
