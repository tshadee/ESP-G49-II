#ifndef EXTERNAL_STIMULUS_H
#define EXTERNAL_STIMULUS_H

#include "mbed.h"
#include "CommonDefs.h"

class ExternalStim
{
private:
    Serial HM10;
    char bleBuffer[BLE_BUFFER_DEPTH];
    char cache;
    int intRC, prevRC;
    uint8_t i;

public:
    ExternalStim(PinName TX, PinName RX);
    bool serialConfigReady();
    bool pullHM10();
    int getIntRC();
};

#endif // EXTERNAL_STIMULUS_H

/*
#include "mbed.h"

Serial hm10(PA_11, PA_12);
DigitalOut led(LED2);

char c;

int main() {
    hm10.baud(9600);
    while(1) {
        if(hm10.readable()){
        c = hm10.getc(); //read a single character
            if(c == 'A'){
                led = 1;
            }
            else if(c == 'B'){
                led = 0;
            };
        };
    };
};
*/
