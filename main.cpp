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
#include "UserInputInterrupts.h"
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
    UserInputInterrupts UII     (PA_11,PA_12);                          //TX, RX
    C12832              lcd     (D11, D13, D12, D7, D10);               //LCD screen arduino pins
    PWMGen              toMDB   (PA_15,PB_7,PA_14,PC_2,PC_3);           //pwm1, pwm2, mdbe, be1, be2 
    BatteryMonitor      Battery (PC_12);                                //one wire pin, MUST BE PC_12
    TCRT                S1      (PA_0,TCRT_MAX_VDD);                    //Leftmost GUARD sensor
    TCRT                S2      (PA_1,TCRT_MAX_VDD);                    //Left-middle EDGE sensor
    TCRT                S3      (PA_4,TCRT_MAX_VDD);                    //CENTRE sensor
    TCRT                S4      (PB_0,TCRT_MAX_VDD);                    //Right-middle EDGE sensor
    TCRT                S5      (PC_1,TCRT_MAX_VDD);                    //Rightmost GUARD sensor
/* ---------------------------------------------------------------------------------------------------------------------------------------------- */
    LCDManager          LCD(&lcd);                                      //from above
    Encoder             leftWheel(&leftEnc);                            //from QEI above
    Encoder             rightWheel(&rightEnc);                          //from QEI above
    PIDSys              PID(&S1,&S2,&S4,&S5);                           //from sensor array above
    speedRegulator      speedReg(&leftWheel,&rightWheel);               //from Encoder class above
    
    /*
    
    timeout corner for stupid code
    
    */
    DigitalOut led(LED2);

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms

    bool straightLineStart = true;
    bool loop1enter = true;

    toMDB.begin();
    led = 0;
    int RCstate = 0;

    while(1)
    {
        switch (ProgramState){
            case (starting):
            { 
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();

                    //PID.calculatePID(false);
                    //speedReg.updateTargetPWM(PID.getLeftPWM(), PID.getRightPWM());
                    //toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    
                    if(UII.serialConfigReady()){UII.pullHM10();
                        RCstate = UII.getIntRC();
                        switch(RCstate)
                        {
                            case(8):toMDB.setPWMDuty(1.0f, 1.0f);break;
                            case(4):toMDB.setPWMDuty(0.0f, 0.0f);break;
                            case(2):toMDB.setPWMDuty(0.0f, 1.0f);break;
                            case(1):toMDB.setPWMDuty(1.0f, 0.0f);break;
                            default:toMDB.setPWMDuty(0.5f, 0.5f);break;
                        };
                    } else { led = 1; };  
                    led = 0;   
                };
                break;
            };

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


                    LCD.toScreen("                    ", LCD.encoderOutputTest(&leftWheel, &rightWheel), "PWM Testing                ");
                };
            break;};

            case (stop):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    toMDB.setPWMDuty(0.5f,0.5f);

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
        
        /*
        toMDB.setPWMDuty(1.0f, 1.0f);
        wait(2.0f);
        toMDB.setPWMDuty(0.5f,0.5f);
        wait(2.0f);
        toMDB.setPWMDuty(0.0f, 0.0f);
        wait(2.0f);
        toMDB.setPWMDuty(0.5f,0.5f);
        wait(2.0f);
        */

    };
};
