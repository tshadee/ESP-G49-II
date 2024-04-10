#ifndef EXTERNAL_STIMULUS_H
#define EXTERNAL_STIMULUS_H

#include "mbed.h"
#include "CommonDefs.h"

class ExternalStim
{
private:
    Serial HM10;
    uint32_t intRC;

public:
    ExternalStim(PinName TX, PinName RX);
    bool serialConfigReady();
    bool pullHM10();
    int getIntRC();
};

#endif // EXTERNAL_STIMULUS_H
