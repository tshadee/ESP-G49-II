#ifndef EXTERNAL_STIMULUS_H
#define EXTERNAL_STIMULUS_H

#include "mbed.h"
#include "CommonDefs.h"

class ExternalStim {
private:
    Serial HM10;
    char bleBuffer[BLE_BUFFER_DEPTH];
    int intRC;

public:
    ExternalStim(PinName TX, PinName RX);
    void centreISR();
    bool serialConfigReady();
    void pullHM10();
    int getIntRC();
};

#endif // EXTERNAL_STIMULUS_H
