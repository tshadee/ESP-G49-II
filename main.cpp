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
/* .-------------------------------------Underneath are subsystems. IO config should be done up here.------------------------------------- */

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
    bool turnDone = false;
    bool turnAroundEnter = false;

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
                            speedReg.updateTargetSpeed(-0.4f,0.4f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());   
                        } 
                        else 
                        {  
                            if((S3.getSensorVoltage(true) + S4.getSensorVoltage(true) + S5.getSensorVoltage(true)) < 1.0f)
                            {
                                speedReg.updateTargetSpeed(-0.3f, 0.3f);
                                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                            } 
                            else
                            {
                                PID.calculatePID();
                                speedReg.updateTargetSpeed((PID.getLeftSpeed() - BASE_SPEED)*0.2, (PID.getRightSpeed() - BASE_SPEED)*0.2);
                                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
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
                        if(S2.getSensorVoltage(true) > 1.5f || S3.getSensorVoltage(true) > 1.5f || S4.getSensorVoltage(true) > 1.5f || S5.getSensorVoltage(true) > 1.5f || S6.getSensorVoltage(true) > 1.5f)
                        {
                                PID.calculatePID();
                                speedReg.updateTargetSpeed((PID.getLeftSpeed()), (PID.getRightSpeed()));
                                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        }
                        else
                        {
                            speedReg.updateTargetSpeed(0.0f, 0.0f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                        };
                    };
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
