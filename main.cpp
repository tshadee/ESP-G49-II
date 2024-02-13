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

pstate ProgramState = starting; //yippee!!!

// Battery One-Wire PC_12
// EncoderLeft PB_14, PB_15
// EncoderRight PB_1, PB_2
// TCRT PA_0,PA_1,PA_4,PB_0,PC_1,PC_0

int main (void)
{
    QEI                 leftEnc (PB_14,PB_15,NC,CPR,QEI::X4_ENCODING);  //left encoder left channel, right channel
    QEI                 rightEnc(PB_1,PB_2,NC,CPR,QEI::X4_ENCODING);    //right encoder left channel, right channel
    ExternalStim        ExStim  (PA_11,PA_12);                          //TX, RX
    
    DigitalOut          led(LED2);
    PWMGen              toMDB   (PA_15,PB_7,PA_14,PC_2,PC_3);           //pwm1, pwm2, mdbe, be1, be2 
    BatteryMonitor      Battery (PC_12);                                //one wire pin, MUST BE PC_12
    
/* ---------------------------------------------------------------------------------------------------------------------------------------------- */
    
    Encoder             leftWheel(&leftEnc);                            //from QEI above
    Encoder             rightWheel(&rightEnc);                          //from QEI above
    
    speedRegulator      speedReg(&leftWheel,&rightWheel);               //from Encoder class above
    
    /*
    
    timeout corner for stupid code

    PIDSys              PID(&S1,&S2,&S4,&S5);                           //from sensor array above
    LCDManager          LCD(&lcd);                                      //from above
    C12832              lcd     (D11, D13, D12, D7, D10);               //LCD screen arduino pins
    TCRT                S1      (PA_0,TCRT_MAX_VDD);                    //Leftmost GUARD sensor
    TCRT                S2      (PA_1,TCRT_MAX_VDD);                    //Left-middle EDGE sensor
    TCRT                S3      (PA_4,TCRT_MAX_VDD);                    //CENTRE sensor
    TCRT                S4      (PB_0,TCRT_MAX_VDD);                    //Right-middle EDGE sensor
    TCRT                S5      (PC_1,TCRT_MAX_VDD);                    //Rightmost GUARD sensor

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);
    

    bool straightLineStart = true;
    bool loop1enter = true;

    */

    

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms

    toMDB.begin();
    volatile int RCstate = 0;
    led = 0;

    while(1)
    {
        switch (ProgramState){
            case (starting):
            { 
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    //Battery.pollBattery();

                    //PID.calculatePID(false);
                    //speedReg.updateTargetPWM(PID.getLeftPWM(), PID.getRightPWM());
                    //toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    
                    ExStim.pullHM10();
                    RCstate = ExStim.getIntRC();
                    switch(RCstate)
                    {
                        case(9): led = 1;break;
                        case(8): led = 0;break;
                        case(4): led = 0;break;
                        case(2): led = 0;break;
                        case(1): led = 0;break;
                        case(0): led = 0;break;
                        default: led = 0;break;
                    };
                };
                break;
            };

            /*
            case (straightline):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    if(straightLineStart)
                    {
                        leftWheel.resetAllValues();
                        rightWheel.resetAllValues();
                        straightLineStart = false;
                    };
                    toMDB.setPWMDuty(1.0f,1.0f);
                    leftWheel.updateValues();
                    rightWheel.updateValues();


                    //LCD.toScreen("                    ", LCD.encoderOutputTest(&leftWheel, &rightWheel), "PWM Testing                ");
                };
            break;};

            case (stop):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    toMDB.setPWMDuty(0.5f,0.5f);

                    //LCD.toScreen("STOP!!!             ", LCD.batteryMonitorBuffer(&Battery), "                       ");
                };
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

            */
        }; 
    };
};
