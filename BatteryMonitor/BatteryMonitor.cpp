#include "BatteryMonitor.h"
#include "CommonDefs.h"
#include "ds2781.h"
#include "OneWire_Methods.h"

//use the one-wire-pin PC_12
BatteryMonitor::BatteryMonitor(PinName P1): one_wire_pin(P1),powerUsed(0.0f) 
{
    VoltageReading = ReadVoltage();
    CurrentReading = ReadCurrent();
    Voltage = VoltageReading*0.00967;
    Current = CurrentReading/6400.0;
    initCharge = Voltage*0.00967*2.8*3600;
};
void BatteryMonitor::pollBattery(void)
{
    VoltageReading = ReadVoltage();
    CurrentReading = ReadCurrent();
    Voltage = VoltageReading*0.00967;
    Current = CurrentReading/6400.0;
    powerUsed += Current*Voltage/SYS_OUTPUT_RATE;
    chargePercentLeft  = 100.0 - ((initCharge - powerUsed)/initCharge)*100;
};

float BatteryMonitor::getBatteryVoltage(void){return Voltage;};
float BatteryMonitor::getBatteryCurrent(void){return Current;};
float BatteryMonitor::getChargeLeft(void){return chargePercentLeft;};
