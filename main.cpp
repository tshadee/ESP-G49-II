// external libraries
#include "mbed.h"
#include "C12832.h"

// Our own libraries
#include "CommonDefs.h"
#include "TCRT.h"
#include "encoder.h"
#include "PWMGen.h"
#include "PIDSys.h"
#include "SpeedRegulator.h"
#include "ExternalStimulus.h"
#include "LCDManager.h"

pstate ProgramState = init;

int main(void)
{
    QEI leftEnc(PB_3, PB_5, NC, CPR, QEI::X2_ENCODING);   // left encoder left channel, right channel
    QEI rightEnc(PB_10, PB_4, NC, CPR, QEI::X2_ENCODING); // right encoder left channel, right channel
    ExternalStim ExStim(PA_11, PA_12);                    // RXD -> TX (PIN), TXD -> RX (PIN)
    TCRT S1(PA_0, PC_9, TCRT_MAX_VDD);                    // Left GUARD2 sensor     (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S2(PA_1, PB_8, TCRT_MAX_VDD);                    // Left GUARD1 sensor     (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S3(PA_4, PB_9, TCRT_MAX_VDD);                    // Left EDGE sensor       (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S4(PB_0, PC_6, TCRT_MAX_VDD);                    // Right EDGE sensor      (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S5(PC_1, PC_8, TCRT_MAX_VDD);                    // Right GUARD1 sensor    (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S6(PC_0, PC_5, TCRT_MAX_VDD);                    // Right GUARD2 sensor    (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    PWMGen toMDB(PA_15, PB_7, PA_14, PC_2, PC_3);         // pwm1, pwm2, mdbe, be1, be2               
    C12832 lcd(D11, D13, D12, D7, D10);                   // LCD screen arduino pins    
/* .-------------------------------------Underneath are subsystems. IO config should be done up here.------------------------------------- */

    Encoder leftWheel(&leftEnc);                                            // from QEI above
    Encoder rightWheel(&rightEnc);                                          // from QEI above
    LCDManager LCD(&lcd);                                                   // from C12832 above
    speedRegulator speedReg(&leftWheel, &rightWheel);                       // from Encoder class above
    PIDSys PID(&S1, &S2, &S3, &S4, &S5, &S6, &leftWheel, &rightWheel);      // from sensor array above

    S1.turnSensorOn();
    S2.turnSensorOn();
    S3.turnSensorOn();
    S4.turnSensorOn();
    S5.turnSensorOn();
    S6.turnSensorOn();

    Ticker sensorPollTicker;
    double sensorPollRate = 1.0 / SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors), sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    Timer BLEtimer;
    BLEtimer.start();
    int timedelay = (static_cast<int>(1000 / SYS_OUTPUT_RATE)); // in ms
    int BLEdelay = (static_cast<int>(1000 / SYS_OUTPUT_RATE)); //in ms

    toMDB.begin();
    ExStim.serialConfigReady();

    volatile int RCstate = 0;
    volatile int LCDstepdown = 0;
    bool lineFollowingMode = false;
    bool RCmode = true;
    bool turnAroundEnter = false;
    bool lineFound = false;

    while (true)
    {
        if(BLEtimer.read_ms() >= BLEdelay)
        {
            BLEtimer.reset();
            if(ExStim.pullHM10())
            {
                RCstate = ExStim.getIntRC();
                switch(RCstate)
                {
                    case(1): {RCmode = true;lineFollowingMode = false;turnAroundEnter = false;ProgramState = RCstop;}   break;
                    case(2): {RCmode = true;lineFollowingMode = false;ProgramState = RCforward;}                        break;
                    case(3): {RCmode = true;lineFollowingMode = false;ProgramState = RCbackwards;}                      break;
                    case(4): {RCmode = true;lineFollowingMode = false;ProgramState = RCturnleft;}                       break;
                    case(5): {RCmode = true;lineFollowingMode = false;ProgramState = RCturnright;}                      break;
                    case(6): {RCmode = true;lineFollowingMode = false;ProgramState = RCturbo;}                          break;
                    case(7): {RCmode = true;lineFollowingMode = false;ProgramState = turnAround;}                       break;
                    case(8): {RCmode = false;lineFollowingMode = true;turnAroundEnter = false;}                         break;
                    default:                                                                                            break;
                };
            };
        };

        //TDB MODE
        if (lineFollowingMode)

        {
            if(outputUpdateTimer.read_ms() >= timedelay)
            {
                outputUpdateTimer.reset();

                PID.calculatePID();
                speedReg.updateTargetSpeed(PID.getLeftSpeed(), PID.getRightSpeed());
                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
            };
        }

        //RC MODE
        else if (RCmode)   

        {
            switch(ProgramState)
            {
                case(RCforward):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(1.0f,1.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCturbo):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(1.8f,1.8f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCbackwards):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(-1.0f,-1.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCturnleft):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(-0.5f,0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCturnright):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(0.5f,-0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                    break;
                };

                case(RCstop):
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(0.0f,0.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                    };
                    break;
                };

                case(turnAround):
                {
                    if(!turnAroundEnter)
                    {
                        leftWheel.resetDistance();
                        rightWheel.resetDistance();
                        turnAroundEnter = true;
                    };

                    if(outputUpdateTimer.read_ms() >= timedelay)
                    {
                        outputUpdateTimer.reset();
                        if((leftWheel.getDist() > -0.1) && (rightWheel.getDist() < 0.1))
                        {
                            speedReg.updateTargetSpeed(-0.2f,0.2f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());   
                        } 
                        else 
                        {  
                            if((S3.getSensorVoltage(true) + S4.getSensorVoltage(true)) < 3.0f)
                            {
                                speedReg.updateTargetSpeed(-0.1f, 0.1f);
                                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                            } 
                            else
                            {
                                PID.calculatePID();
                                speedReg.updateTargetSpeed((PID.getLeftSpeed() - BASE_SPEED)*0.2, (PID.getRightSpeed() - BASE_SPEED)*0.2);
                                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                            };
                        };
                    };
                    break;
                };

                default:
                {
                    if(outputUpdateTimer.read_ms() >= timedelay)
                    { 
                        outputUpdateTimer.reset();
                        speedReg.updateTargetSpeed(0.0f,0.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                };
            };
        };
    };
};
