#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include "mbed.h"
#include "C12832.h"
#include "BatteryMonitor.h" 
#include "TCRT.h" 

//LCD display buffer. Pass string pointers to display in lines 1-3 on the LCD screen. Keep the strings under 23 bytes if possible
// call toScreen with the appropriate arguments to push anything to the LCD display - refresh rate sensitive (dont go above 15 Hz)
class LCDManager {
    C12832* lcd;
public:
    LCDManager(C12832* c); // Constructor might not be necessary if no initialization is needed
    void toScreen(const char* line1, const char* line2, const char* line3);
    char* batteryMonitorBuffer(BatteryMonitor* Batt);
    char* sensorVoltageBuffer(TCRT* S1, TCRT* S2);
};

#endif // LCD_MANAGER_H
