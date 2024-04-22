#ifndef EXTERNAL_STIMULUS_H
#define EXTERNAL_STIMULUS_H

#include "mbed.h"
#include "CommonDefs.h"

/**
This class is for interacting with the HM10. Only basic functionality is needed to receive BLE data.
**/
class ExternalStim
{
private:

    Serial HM10;
    uint32_t intRC;

public:

    /**
    Constructor.

    Enables communication with BLE with the TX and RX pins of the STM and the RXD and TXD pins of the bluetooth module. Readable but not writeable.
    @param TX TX pin on the STM32
    @param RX RX pin on the STM32
    **/
    ExternalStim(PinName TX, PinName RX);

    /**
    Mandatory function. Unless called, BLE will not work.
    
    Sets up baud rate (9600) for serial communication with external source.
    **/
    bool serialConfigReady();

    //Pulls data from the HM10 in 8-bit packages. Stores said data into intRC.
    bool pullHM10();

    //Gets the data from pullHM10().
    int getIntRC();
};

#endif // EXTERNAL_STIMULUS_H
