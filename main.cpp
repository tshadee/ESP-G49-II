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

pstate ProgramState = init;

int main(void)
{
/* .------------------------------------- IO Config ------------------------------------- */
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
/* .------------------------------------- Subsystems ------------------------------------ */

    Encoder leftWheel(&leftEnc);                                            // from QEI above
    Encoder rightWheel(&rightEnc);                                          // from QEI above
    speedRegulator speedReg(&leftWheel, &rightWheel);                       // from Encoder class above
    PIDSys PID(&S1, &S2, &S3, &S4, &S5, &S6, &leftWheel, &rightWheel);      // from sensor array above

    S1.turnSensorOn();
    S2.turnSensorOn();
    S3.turnSensorOn();
    S4.turnSensorOn();
    S5.turnSensorOn();
    S6.turnSensorOn();

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0 / SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors), sensorPollRate);

    Timer outputUpdateTimer;
    Timer BLEtimer;
    outputUpdateTimer.start();
    BLEtimer.start();

    float timedelay = (static_cast<float>(1000.f / SYS_OUTPUT_RATE)); // in ms
    float BLEdelay = (static_cast<float>(1000.f / SYS_OUTPUT_RATE));  //in ms

    toMDB.begin();
    ExStim.serialConfigReady();

    volatile int RCstate = 0;
    volatile int s = 0;
    volatile int turnDelay = 0;
    volatile int stopTick = 0;
    bool turnDone = false;
    bool turnAroundEnter = false;

/* .------------------------------------- Main Loop ------------------------------------- */

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
                    case(1): {s = 0;} break;
                    case(2): {s = 1;} break;
                    case(3): {s = 2;} break;
                    default:          break;
                };
            };
        };

        //TDB MODE
        if(outputUpdateTimer.read_ms() >= timedelay)
        {
            outputUpdateTimer.reset();

            switch(s)
            {
                case(0):
                {
                    speedReg.updateTargetSpeed(0.0f,0.0f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                    turnDelay = 0;
                    turnDone = false;
                }
                break;

                case(1):
                {
                    
                    PID.calculatePID();
                    speedReg.updateTargetSpeed(PID.getLeftSpeed(), PID.getRightSpeed());
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                }
                break;

                case(2):
                {
                    PID.calculatePID();
                    if(!turnAroundEnter)
                    {
                        leftWheel.resetDistance();
                        rightWheel.resetDistance();
                        turnAroundEnter = true;
                    };

                    if(!turnDone)
                    {
                        if((leftWheel.getDist() > -0.1) && (rightWheel.getDist() < 0.1))
                        {
                            speedReg.updateTargetSpeed(-0.5f,0.5f);
                        } 
                        else 
                        {  
                            if((S3.getSensorVoltage(true) + S4.getSensorVoltage(true) ) < 1.0f)
                            {
                                speedReg.updateTargetSpeed(-0.3f, 0.3f);
                            } 
                            else
                            {
                                speedReg.updateTargetSpeed((PID.getLeftSpeed() - BASE_SPEED)*0.2, (PID.getRightSpeed() - BASE_SPEED)*0.2);
                                turnDelay++;
                                if(turnDelay >= 200)
                                {
                                    turnDone = true;
                                };
                            };
                        };
                    }
                    else
                    {
                        if(S3.getSensorVoltage(true) > 1.5f || S4.getSensorVoltage(true) > 1.5f)
                        {
                                speedReg.updateTargetSpeed((PID.getLeftSpeed())+0.2f, (PID.getRightSpeed())+0.2f);
                                stopTick = 0;
                        }
                        else
                        {
                            if(stopTick >= 1000)
                            {
                                speedReg.updateTargetSpeed(0.0f, 0.0f);
                            } 
                            else 
                            {
                                stopTick++;
                                speedReg.updateTargetSpeed((PID.getLeftSpeed()), (PID.getRightSpeed()));
                            };
                        };
                    };
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                };
                break;

                default:
                {
                    speedReg.updateTargetSpeed(0.0f,0.0f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                }
                break;
            };
        };
    };
};
