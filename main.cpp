// external libraries
#include "mbed.h"
// #include "C12832.h"

// Our own libraries
#include "CommonDefs.h"         //contains parameters for the buggy and the main state
#include "TCRT.h"
#include "encoder.h"
#include "PWMGen.h"
#include "PIDSys.h"
#include "SpeedRegulator.h"
#include "ExternalStimulus.h"
// #include "LCDManager.h"

pstate ProgramState = init;
// PB_5 PB_3 PA_10 PA_2 PA_3 for darlington output pin
int main(void)
{
    QEI leftEnc(PB_14, PB_15, NC, CPR, QEI::X4_ENCODING); // left encoder left channel, right channel
    QEI rightEnc(PA_8, PA_9, NC, CPR, QEI::X4_ENCODING);  // right encoder left channel, right channel
    ExternalStim ExStim(PA_11, PA_12);                    // RXD -> TX (PIN), TXD -> RX (PIN)
    DigitalInOut one_wire_pin(PC_12);                     // one wire pin, MUST BE PC_12
    TCRT S1(PA_0, PB_8, TCRT_MAX_VDD);                    // Left GUARD sensor   (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S2(PA_1, PC_6, TCRT_MAX_VDD);                    // Left EDGE sensor    (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S3(PA_4, PC_5, TCRT_MAX_VDD);                    // CENTRE sensor       (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S4(PB_0, PC_8, TCRT_MAX_VDD);                    // Right EDGE sensor   (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    TCRT S5(PC_1, PC_9, TCRT_MAX_VDD);                    // Right GUARD sensor  (INPUT PIN, DARLINGTON OUTPUT PIN, 3.3V)
    PWMGen toMDB(PA_15, PB_7, PA_14, PC_2, PC_3);         // pwm1, pwm2, mdbe, be1, be2 
    // C12832 lcd(D11, D13, D12, D7, D10);                   // LCD screen arduino pins                         
/* .-------------------------------------Underneath are subsystems. IO config should be done up here.------------------------------------- */

    Encoder leftWheel(&leftEnc);                        // from QEI above
    Encoder rightWheel(&rightEnc);                      // from QEI above
    // LCDManager LCD(&lcd);                               // from C12832 above
    speedRegulator speedReg(&leftWheel, &rightWheel);   // from Encoder class above
    PIDSys PID(&S1, &S2, &S4, &S5);                     // from sensor array above

    S1.turnSensorOff();
    S2.turnSensorOff();
    S3.turnSensorOff();
    S4.turnSensorOff();
    S5.turnSensorOff();

    Ticker sensorPollTicker;
    float sensorPollRate = 1.0 / SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors), sensorPollRate);

    Timer BLEtimer;
    BLEtimer.start();
    int BLEdelay = (static_cast<int>(1000 / SYS_OUTPUT_RATE)); //in ms

    toMDB.begin();
    ExStim.serialConfigReady();

    volatile int RCstate = 0;
    bool lineFollowingMode = false;
    bool RCmode = true;
    bool enterLineFollowing = false;
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

            //TDB MODE
            if (lineFollowingMode)

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
                
                PID.calculatePID();
                speedReg.updateTargetSpeed(PID.getLeftSpeed(), PID.getRightSpeed());
                toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());

                if(S5.getSensorVoltage(true) < 0.8f && S4.getSensorVoltage(true) < 0.8f && S3.getSensorVoltage(true) < 0.8f && S2.getSensorVoltage(true) < 0.8f && S1.getSensorVoltage(true) < 0.8f)
                {
                    speedReg.updateTargetSpeed(0.0f, 0.0f);
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                }
            }

            //RC MODE
            else if (RCmode)   

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
                        speedReg.updateTargetSpeed(1.0f,1.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        break;
                    };
                    case(RCturbo):
                    {
                        speedReg.updateTargetSpeed(1.8f,1.8f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        break;
                    };

                    case(RCbackwards):
                    {
                        speedReg.updateTargetSpeed(-1.0f,-1.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        break;
                    };

                    case(RCturnleft):
                    {
                        speedReg.updateTargetSpeed(-0.5f,0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        break;
                    };

                    case(RCturnright):
                    {
                        speedReg.updateTargetSpeed(0.5f,-0.5f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                        break;
                    };

                    case(RCstop):
                    {
                        speedReg.updateTargetSpeed(0.0f,0.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                        break;
                    };

                    case(turnAround):
                    {
                        if(!turnAroundEnter)
                        {
                            do {
                            speedReg.updateTargetSpeed(-0.5f, 0.5f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                            }while (S3.getSensorVoltage(true) > 1.0f);
                            turnAroundEnter = true;
                        }
                        else if(S3.getSensorVoltage(true) < 1.5f)
                        {
                            speedReg.updateTargetSpeed(-0.5f, 0.5f);
                            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());  
                        } 
                        else
                        {
                            PID.calculatePID();
                            speedReg.updateTargetSpeed(PID.getLeftSpeed() - BASE_SPEED, PID.getRightSpeed() - BASE_SPEED);
                        };
                        break;
                    }

                    default:
                    {
                        speedReg.updateTargetSpeed(0.0f,0.0f);
                        toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    };
                };
            }

            else 

            //no BLE stop
            {
                speedReg.updateTargetSpeed(0.0f, 0.0f);
            };
        };
    };
};
