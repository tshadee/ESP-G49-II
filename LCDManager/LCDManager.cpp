#include "LCDManager.h"

LCDManager::LCDManager(C12832* c):lcd(c){};

void LCDManager::toScreen(const char* line1, const char* line2, const char* line3) 
{
    static char lastLine1[21] = "";
    static char lastLine2[21] = "";
    static char lastLine3[21] = "";
    if(strncmp(lastLine1,line1,20) != 0){
        strncpy(lastLine1,line1,20);
        lastLine1[20] = '\0';
        lcd->locate(0,0);
        lcd->printf(line1);
    };
    if(strncmp(lastLine2,line2,20) != 0){
        strncpy(lastLine2,line2,20);
        lastLine2[20] = '\0';
        lcd->locate(0,10);
        lcd->printf(line2);
    };
    if(strncmp(lastLine3,line3,20) != 0){
        strncpy(lastLine3,line3,20);
        lastLine3[20] = '\0';
        lcd->locate(0,20);
        lcd->printf(line3);
    };
};

char* LCDManager::batteryMonitorBuffer(BatteryMonitor* Batt) 
{
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", Batt->getBatteryVoltage(), Batt->getChargeLeft());
    return dspBuffer;
};

char* LCDManager::sensorVoltageBuffer(TCRT* S1, TCRT* S2) 
{
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", S1->getSensorVoltage(true),S2->getSensorVoltage(true));
    return dspBuffer;
};

char* LCDManager::encoderOutputTest(Encoder *leftWheel, Encoder *rightWheel)
{
    static char dspBuffer[20];
    //sprintf(dspBuffer, "%.02f    %.02f   ", leftWheel->getSpeed(),rightWheel->getSpeed());
    sprintf(dspBuffer, "%.02f    %.02f   ", leftWheel->getDist(),rightWheel->getDist());
    //sprintf(dspBuffer, "%02d      %02d     ", leftWheel->getCount(),rightWheel->getCount());
    return dspBuffer;
};