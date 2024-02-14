//external libraries
#include "mbed.h"
#include "C12832.h"

//Our own libraries
#include "CommonDefs.h" //contains parameters for the buggy and the main state 
#include "TCRT.h"
#include "encoder.h"
#include "PWMGen.h"
#include "BatteryMonitor.h"
#include "PIDSys.h"
#include "SpeedRegulator.h"
#include "ExternalStimulus.h"
#include "LCDManager.h"

//UNIVERSITY LIBRARIES
//#include "ds2781.h"

pstate ProgramState = starting; //yippee!!! test 

int main (void)
{
    QEI                 leftEnc (PB_14,PB_15,NC,CPR,QEI::X2_ENCODING);  //left encoder left channel, right channel
    QEI                 rightEnc(PB_1,PB_2,NC,CPR,QEI::X2_ENCODING);    //right encoder left channel, right channel
    ExternalStim        ExStim  (PA_11,PA_12);                          //RXD -> TX (PIN), TXD -> RX (PIN)
    DigitalInOut        one_wire_pin(PC_12);                            //one wire pin, MUST BE PC_12
    TCRT                S1      (PA_0,TCRT_MAX_VDD);                    //Leftmost GUARD sensor
    TCRT                S2      (PA_1,TCRT_MAX_VDD);                    //Left-middle EDGE sensor
    TCRT                S3      (PA_4,TCRT_MAX_VDD);                    //CENTRE sensor
    TCRT                S4      (PB_0,TCRT_MAX_VDD);                    //Right-middle EDGE sensor
    TCRT                S5      (PC_1,TCRT_MAX_VDD);                    //Rightmost GUARD sensor
    PWMGen              toMDB   (PA_15,PB_7,PA_14,PC_2,PC_3);           //pwm1, pwm2, mdbe, be1, be2 
    C12832              lcd     (D11, D13, D12, D7, D10);               //LCD screen arduino pins


 /* ---------------------------------------------------------------------------------------------------------------------------------------------- */
    
    Encoder             leftWheel(&leftEnc);                            //from QEI above
    Encoder             rightWheel(&rightEnc);                          //from QEI above
    LCDManager          LCD(&lcd);                                      //from above
    speedRegulator      speedReg(&leftWheel,&rightWheel);               //from Encoder class above
    BatteryMonitor      Battery (&one_wire_pin);                        //from above
    PIDSys              PID(&S1,&S2,&S4,&S5);                           //from sensor array above
    
    /*
    
    timeout corner

    */

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);
    
    bool straightLineStart = true;
    bool loop1enter = true;
    //bool straightLineStart = true;
    //bool loop1enter = true;

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms

    toMDB.begin();
    volatile int RCstate = 0;
    ExStim.serialConfigReady();

    volatile int count50 = 0;
    pstate prevState;

    while(1)
    {
        switch (ProgramState){
            case (starting):{ //THIS IS THE STRAIGHT LINE STATE

                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    speedReg.updateTargetPWM(1.0f,1.0f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    LCD.toScreen("SS   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
                };

                if((leftWheel.getDist() < 1.0) && (rightWheel.getDist() < 1.0))
                { prevState = ProgramState; } else {
                    ProgramState = stop;
                };
            break;};

            case (straightline):{  //THIS IS TURN LEFT STATE
                if((leftWheel.getDist() > -0.15) && (rightWheel.getDist() < 0.215))
                {
                    if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.5f, 0.7f); //turn right
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        LCD.toScreen("TL   ", LCD.encoderOutputTest(&leftWheel, &rightWheel),"");
                    };
                } else {
                    prevState = ProgramState;
                    ProgramState = stop;
                };
                
            break;};

            case (stop):{ //THIS STOPS THE BUGGY (HOPEFULLY)

                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    speedReg.updateTargetPWM(0.5f, 0.5f); //turn left
                    leftWheel.resetDistance();
                    rightWheel.resetDistance();
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    LCD.toScreen("STOP ", LCD.encoderOutputTest(&leftWheel, &rightWheel),"");

                    if(count50 >= 100)
                    {
                        count50 = 0;
                        if(prevState == starting)
                        {
                            ProgramState = straightline;
                        } else {
                            ProgramState = starting;
                        };
                    } else {
                        count50++;
                    };
                };
            break;};
        
        };
    };

        /*
        switch (ProgramState){
            case (starting):
            { 
                
            
            
            break;};

            
            

            case (turnaround):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();

                    //LCD.toScreen("TURN!!!             ", LCD.batteryMonitorBuffer(&Battery), "                       ");
                };
            break;};

            default: {
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();

                    //LCD.toScreen("SOMETHING BROKE","                       ","                       ");
                };
            break;};

            
        }; 
        */
};