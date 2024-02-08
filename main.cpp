//external libraries
#include "mbed.h"
#include "C12832.h"

//Our own libraries
#include "CommonDefs.h"
#include "TCRT.h"
#include "encoder.h"
#include "PWMGen.h"
#include "BatteryMonitor.h"
#include "PIDSys.h"
#include "SpeedRegulator.h"
#include "UserInputInterrupts.h"

//serial 6 RX TX PA_11/12
//ANALOG IN PC_2/3/4/5 PB_1
//PWM PC_8/6
//encoder PB_15/14

//LCD display buffer. Pass string pointers to display in lines 1-3 on the LCD screen. Keep the strings under 23 bytes if possible
// call toScreen with the appropriate arguments to push anything to the LCD display - refresh rate sensitive (dont go above 15 Hz)
void toScreen(char* line1, char*  line2, char* line3,C12832* lcd){
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

char* batteryMonitorBuffer(BatteryMonitor* Batt){
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f V          ", Batt->getBatteryVoltage());
    return dspBuffer;
};

char* sensorVoltageBuffer(TCRT* S1,TCRT* S2){
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", S1->getSensorVoltage(true),S2->getSensorVoltage(true));
    return dspBuffer;
};

int main (void)
{
    UserInputInterrupts joy(D4);
    C12832 lcd(D11, D13, D12, D7, D10);
    PWMGen toMDB(PC_8,PC_6);            //PLACEHOLDER PINS
    BatteryMonitor Battery(PC_12); 
    Encoder leftWheel(PB_14,PB_15);     //PLACEHOLDER PINS
    Encoder rightWheel(PA_0,PA_1);      //PLACEHOLDER PINS
    TCRT S1(PA_0,TCRT_MAX_VDD) , S2(PA_1,TCRT_MAX_VDD) , S3(PA_4,TCRT_MAX_VDD) , S4(PB_0,TCRT_MAX_VDD), S5(PC_1,TCRT_MAX_VDD);  //PLACEHOLDER PINS
    PIDSys PID(&S1,&S2,&S4,&S5);
    speedRegulator speedReg(&leftWheel,&rightWheel);
    


    /*
    
    timeout corner for stupid code
    
    */


    Ticker sensorPollTicker;
    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms

    bool straightLineStart = true;

    while(1)
    {
        switch (ProgramState){
            case (starting):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();
                    PID.calculatePID(false);
                    speedReg.updateTargetPWM(PID.getLeftPWM(), PID.getRightPWM());
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    
                    /*
    
                    timeout corner for stupid code
                    
                    */

                    toScreen("START STATE        ", batteryMonitorBuffer(&Battery), "START STATE        ", &lcd);
                };
            break;};
            case (straightline):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();
                    

                    /*
                    
                    toMDB.setPWMDuty(1.0f, 1.0f); //test

                    
                    if(straightLineStart)
                    {
                        toMDB.begin();
                        straightLineStart = false;
                    };
                    */
                    toScreen("STRAIGHT LINE       ", sensorVoltageBuffer(&S3, &S4), sensorVoltageBuffer(&S1, &S5), &lcd);
                };
            break;};
            case (stop):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    /*
                    Battery.pollBattery();
                    

                    
                    if(!straightLineStart)
                    {
                        toMDB.reset();
                        straightLineStart = true;
                    };
                    */
                    toScreen("STOP!!!             ", batteryMonitorBuffer(&Battery), "                       ", &lcd);
                };
            break;};
            case (turnaround):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();

                    /*
    
                    timeout corner for stupid code
                    
                    */

                    toScreen("TURN!!!             ", batteryMonitorBuffer(&Battery), "                       ", &lcd);
                };
            break;};
            default: {
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();

                    toScreen("SOMETHING BROKE","                       ","                       ", &lcd);
                };
            break;};
        };  
    };
};
