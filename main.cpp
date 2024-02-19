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

pstate ProgramState = starting;

/*





*/

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

/* ---------------------------------------------------------------------------------------------------------------------------------------------- */

    Encoder leftWheel(&leftEnc);                      // from QEI above
    Encoder rightWheel(&rightEnc);                    // from QEI above
    LCDManager LCD(&lcd);                             // from above
    speedRegulator speedReg(&leftWheel, &rightWheel); // from Encoder class above
    BatteryMonitor Battery(&one_wire_pin);            // from above
    PIDSys PID(&S1, &S2, &S4, &S5);                   // from sensor array above


    // S1.turnSensorOn();
    // S2.turnSensorOn();
    // S3.turnSensorOn();
    // S4.turnSensorOn();
    // S5.turnSensorOn();

    // S1.turnSensorOff();
    // S2.turnSensorOff();
    // S3.turnSensorOff();
    // S4.turnSensorOff();
    // S5.turnSensorOff();

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
    pstate prevState;
    bool autoMode = false;
    bool lineFollowingMode = false;
    bool RCmode = false;

    while (true)
    {
        autoMode = false;
        RCmode = true;
        lineFollowingMode = false;
        //uncomment this if you want to be switchable between BLE and auto (will use more memory)
        if(ExStim.pullHM10())
        {
            RCstate = ExStim.getIntRC();
            if     (RCstate == 1){ProgramState = RCstop;}
            else if(RCstate == 2){ProgramState = RCforward;}
            else if(RCstate == 3){ProgramState = RCbackwards;}
            else if(RCstate == 4){ProgramState = RCturnleft;}
            else if(RCstate == 5){ProgramState = RCturnright;}
            else if(RCstate == 6) //this turns on the TDA code
            {
                autoMode = true;
                lineFollowingMode = false;
                RCmode = false;
            }
            else if(RCstate == 7) //this turns on the TDB code
            {
                autoMode = false;
                lineFollowingMode = true;
                RCmode = false;
            };
        };

        //TDA MODE
        if(autoMode == true && lineFollowingMode == false && RCmode == false)

        {
            switch (ProgramState)
            {
                case (starting):
                { // THIS IS THE STRAIGHT LINE STATE
                    if (outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        Battery.pollBattery();
                        speedReg.updateTargetPWM(0.75f, 0.75f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        LCD.toScreen("SS   ", LCD.encoderOutputTest(&leftWheel, &rightWheel), LCD.batteryMonitorBuffer(&Battery));
                        if ((leftWheel.getDist() < 1.0) && (rightWheel.getDist() < 1.0))
                        {
                            prevState = ProgramState;
                        }
                        else
                        {
                            ProgramState = stop;
                        };
                    };
                    break;
                };

                case (straightline):
                { // THIS IS TURN LEFT STATE
                    if ((leftWheel.getDist() > -0.15) && (rightWheel.getDist() < 0.15))
                    {
                        if (outputUpdateTimer.read_ms() >= timedelay)
                        {
                            outputUpdateTimer.reset();
                            Battery.pollBattery();
                            speedReg.updateTargetPWM(0.25f, 0.75f); // turn right
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                            LCD.toScreen("TL   ", LCD.encoderOutputTest(&leftWheel, &rightWheel), LCD.batteryMonitorBuffer(&Battery));
                        };
                    }
                    else
                    {
                        prevState = ProgramState;
                        ProgramState = stop;
                    };
                    break;
                };

                case (stop):
                { // THIS STOPS THE BUGGY (HOPEFULLY)

                    if (outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        Battery.pollBattery();
                        speedReg.updateTargetPWM(0.5f, 0.5f); 
                        leftWheel.resetDistance();
                        rightWheel.resetDistance();
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        LCD.toScreen("STOP ", LCD.encoderOutputTest(&leftWheel, &rightWheel), LCD.batteryMonitorBuffer(&Battery));

                        if (count50 >= 100)
                        {
                            count50 = 0;
                            if (prevState == starting)
                            {
                                ProgramState = straightline;
                            }
                            else
                            {
                                ProgramState = starting;
                            };
                        }
                        else
                        {
                            count50++;
                        };
                    };
                    break;
                };

                default:
                {
                    if (outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetPWM(0.5f, 0.5f);
                        leftWheel.resetDistance();
                        rightWheel.resetDistance();
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        LCD.toScreen("crash1 ", LCD.encoderOutputTest(&leftWheel, &rightWheel), LCD.batteryMonitorBuffer(&Battery));
                        
                    };
                    ProgramState = starting;
                    break;
                };
            };
        }

        //RC MODE
        else if (autoMode == false && lineFollowingMode == false && RCmode == true)   

        {
            switch(ProgramState)
            {
                case(RCforward):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                            speedReg.updateTargetPWM(1.0f,1.0f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                            LCD.toScreen("forward   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
                    };
                    break;
                };

                case(RCbackwards):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                            speedReg.updateTargetPWM(0.0f,0.0f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                            LCD.toScreen("back   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
                    };
                    break;
                };

                case(RCturnleft):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                            speedReg.updateTargetPWM(0.3f,0.7f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                            LCD.toScreen("left   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
                    };
                    break;
                };

                case(RCturnright):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                            speedReg.updateTargetPWM(0.7f,0.3f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                            LCD.toScreen("right   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
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
                        LCD.toScreen("stop   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
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
                        LCD.toScreen("crash2   ",LCD.encoderOutputTest(&leftWheel, &rightWheel),"");    
                    };
                };
            };
        }

        //TDB MODE
        else if (autoMode == false && lineFollowingMode == true && RCmode == false)

        {
        }

        //YOU HAVE DONE GOOFED
        else 

        {
            LCD.toScreen("E404              ", "                  ", "                  ");
        };
    };
};