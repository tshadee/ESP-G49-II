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
#include "LCDManager.h"

//serial 6 RX TX PA_11/12
//ANALOG IN PC_2/3/4/5 PB_1
//PWM PC_8/6
//encoder PB_15/14

pstate ProgramState = starting; //yippee!!!

/* 
TESTING OUT FUNCTION DECOMPOSITION

void handleStartingState(Timer &outputUpdateTimer, BatteryMonitor &Battery, PIDSys &PID, speedRegulator &speedReg,PWMGen &toMDB, LCDManager &LCD, int timedelay)
{
    if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
        Battery.pollBattery();
        PID.calculatePID(false);
        speedReg.updateTargetPWM(PID.getLeftPWM(), PID.getRightPWM());
        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    
        
    
        timeout corner for stupid code
                    
        

        LCD.toScreen("START STATE        ", LCD.batteryMonitorBuffer(&Battery), "START STATE        ");
    };
};

void handleStraightLineState(Timer &outputUpdateTimer, BatteryMonitor &Battery, PIDSys &PID, speedRegulator &speedReg,PWMGen &toMDB, LCDManager &LCD, int timedelay, TCRT &S1, TCRT &S2, TCRT &S3, TCRT &S4, TCRT &S5)
{
    if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
        Battery.pollBattery();

        LCD.toScreen("STRAIGHT LINE       ", LCD.sensorVoltageBuffer(&S3, &S4), LCD.sensorVoltageBuffer(&S1, &S5));
    };
};
*/

int main (void)
{
    UserInputInterrupts joy(D4);
    C12832 lcd(D11, D13, D12, D7, D10);
    LCDManager LCD(&lcd);
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
                                
                    

                    LCD.toScreen("START STATE        ", LCD.batteryMonitorBuffer(&Battery), "START STATE        ");
                }; 
                
            break;};

            case (straightline):{
                
            break;};
            case (stop):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();

                    LCD.toScreen("STOP!!!             ", LCD.batteryMonitorBuffer(&Battery), "                       ");
                };
            break;};
            case (turnaround):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();

                    LCD.toScreen("TURN!!!             ", LCD.batteryMonitorBuffer(&Battery), "                       ");
                };
            break;};
            default: {
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();

                    LCD.toScreen("SOMETHING BROKE","                       ","                       ");
                };
            break;};
        };  
    };
};
