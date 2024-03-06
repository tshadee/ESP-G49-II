// external libraries
#include "mbed.h"
#include "C12832.h"

// Our own libraries
#include "CommonDefs.h" //contains parameters for the buggy and the main state
#include "TCRT.h"
#include "encoder.h"
#include "PWMGen.h"
#include "BatteryMonitor.h"
#include "PIDSys.h"
#include "SpeedRegulator.h"
#include "ExternalStimulus.h"
#include "LCDManager.h"

pstate ProgramState = init;

int main(void)
{
    QEI leftEnc(PB_14, PB_15, NC, CPR, QEI::X2_ENCODING); // left encoder left channel, right channel
    QEI rightEnc(PB_1, PB_2, NC, CPR, QEI::X2_ENCODING);  // right encoder left channel, right channel
    ExternalStim ExStim(PA_11, PA_12);                    // RXD -> TX (PIN), TXD -> RX (PIN)
    DigitalInOut one_wire_pin(PC_12);                     // one wire pin, MUST BE PC_12
    TCRT S1(PA_0, PB_8, TCRT_MAX_VDD);                    // Left GUARD sensor   (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S2(PA_1, PC_6, TCRT_MAX_VDD);                    // Left EDGE sensor    (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S3(PA_4, PC_5, TCRT_MAX_VDD);                    // CENTRE sensor       (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S4(PB_0, PC_8, TCRT_MAX_VDD);                    // Right EDGE sensor   (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S5(PC_1, PC_9, TCRT_MAX_VDD);                    // Right GUARD sensor  (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    PWMGen toMDB(PA_15, PB_7, PA_14, PC_2, PC_3);         // pwm1, pwm2, mdbe, be1, be2 
    C12832 lcd(D11, D13, D12, D7, D10);                   // LCD screen arduino pins

/* ----------------------Underneath are subsystems. IO config should be done up here.------------------------------------- */

    Encoder leftWheel(&leftEnc);                        // from QEI above
    Encoder rightWheel(&rightEnc);                      // from QEI above
    LCDManager LCD(&lcd);                               // from C12832 above
    speedRegulator speedReg(&leftWheel, &rightWheel);   // from Encoder class above
    BatteryMonitor Battery(&one_wire_pin);              // from one_wire_pin above
    PIDSys PID(&S1, &S2, &S4, &S5);                     // from sensor array above

    S1.turnSensorOff();
    S2.turnSensorOff();
    S3.turnSensorOff();
    S4.turnSensorOff();
    S5.turnSensorOff();

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0 / SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors), sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000 / SYS_OUTPUT_RATE)); // in ms

    toMDB.begin();
    volatile int RCstate = 0;
    ExStim.serialConfigReady();

    volatile int count50 = 0;
    volatile int countTurn = 0;
    pstate prevState;
    bool lineFollowingMode = false;
    bool RCmode = false;
    bool enterLineFollowing = false;

    while (true)
    {
        //uncomment this if you want to be switchable between BLE and auto (will use more memory)
        if(ExStim.pullHM10())
        {
            RCstate = ExStim.getIntRC();
            if     (RCstate == 1){RCmode = true;lineFollowingMode = false;ProgramState = RCstop;}
            else if(RCstate == 2){RCmode = true;lineFollowingMode = false;ProgramState = RCforward;}
            else if(RCstate == 3){RCmode = true;lineFollowingMode = false;ProgramState = RCbackwards;}
            else if(RCstate == 4){RCmode = true;lineFollowingMode = false;ProgramState = RCturnleft;}
            else if(RCstate == 5){RCmode = true;lineFollowingMode = false;ProgramState = RCturnright;}
            else if(RCstate == 8){RCmode = true;lineFollowingMode = false;ProgramState = RCturbo;}
            else if(RCstate == 6) //this turns on the TDB code
            {
                RCmode = false;
                lineFollowingMode = true;
            };
        };

        //RC MODE
        if (RCmode)   

        {
            if(enterLineFollowing == true)
            {
                enterLineFollowing = false;
                S1.turnSensorOff();
                S2.turnSensorOff();
                S3.turnSensorOff();
                S4.turnSensorOff();
                S5.turnSensorOff();
            };
            
            switch(ProgramState)
            {
                case(RCforward):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.75f,0.75f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };
                case(RCturbo):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(1.0f,1.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCbackwards):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.25f,0.25f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCturnleft):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.2f,0.8f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCturnright):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.8f,0.2f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCstop):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.5f,0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                        LCD.toScreen("Stop         ","                  ",LCD.batteryMonitorBuffer(&Battery));   
                    };
                    break;
                };

                default:
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    { 
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.5f,0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        LCD.toScreen("BLE crash         ","                  ",LCD.batteryMonitorBuffer(&Battery));    
                    };
                };
            };
        }

        //TDB MODE
        else if (lineFollowingMode)

        {
            if(enterLineFollowing == false)
            {
                enterLineFollowing = true;
                S1.turnSensorOn();
                S2.turnSensorOn();
                S3.turnSensorOn();
                S4.turnSensorOn();
                S5.turnSensorOn();
            };
            
            if(outputUpdateTimer.read_ms() >= timedelay)
            {
                outputUpdateTimer.reset();
                if(S5.getSensorVoltage(true) > 2.5f && S1.getSensorVoltage(true) < 2.5f)
                {
                    speedReg.updateTargetPWM(0.8f, 0.7f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                } else if (S1.getSensorVoltage(true) > 2.5f && S5.getSensorVoltage(true) < 2.5f)
                {
                    speedReg.updateTargetPWM(0.7f, 0.8f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                } else if (S5.getSensorVoltage(true) < 2.5f && S1.getSensorVoltage(true) < 2.5f)
                {
                    speedReg.updateTargetPWM(0.8f, 0.8f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                };

                ; 
                LCD.toScreen(LCD.PIDoutput(speedReg.getCurrentLeftPWM(),speedReg.getCurrentRightPWM(),0.0f), "                  ", LCD.batteryMonitorBuffer(&Battery));
            };
        }

        else 

        {
            LCD.toScreen("E404              ", "                  ", LCD.batteryMonitorBuffer(&Battery));
        };
    };
};
