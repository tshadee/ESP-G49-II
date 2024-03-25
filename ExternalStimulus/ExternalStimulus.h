#ifndef EXTERNAL_STIMULUS_H
#define EXTERNAL_STIMULUS_H

#include "mbed.h"
#include "CommonDefs.h"

class ExternalStim
{
private:
    Serial HM10;
    char dataBuffer;
    float KP;
    float KD;
    float KI;
    char bleToData;
    int index;
    uint32_t intRC;
    // Ticker blePollTicker;

public:
    ExternalStim(PinName TX, PinName RX);
    bool serialConfigReady();
    bool pullHM10();
    int getIntRC();
    float getKP();
    float getKD();
    float getKI();
};

#endif // EXTERNAL_STIMULUS_H
