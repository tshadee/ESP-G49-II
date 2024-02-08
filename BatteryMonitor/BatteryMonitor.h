#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include "mbed.h"
//use the one-wire-pin PC_12
class BatteryMonitor {
    private:
        DigitalInOut one_wire_pin;
        int VoltageReading, CurrentReading;
        float Voltage, Current, currentAccum, powerUsed, initCharge,chargePercentLeft;
    public:
        BatteryMonitor(PinName P1);
        void pollBattery(void);
        float getBatteryVoltage(void);
        float getBatteryCurrent(void);
        float getChargeLeft(void);
};





#endif