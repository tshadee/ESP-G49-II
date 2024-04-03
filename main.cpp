// external libraries
#include "mbed.h"

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
    double sensorPollRate = 1.0 / SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors), sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000 / SYS_OUTPUT_RATE)); // in ms

    toMDB.begin();

    while (true)
    {
        if(outputUpdateTimer.read_ms() >= timedelay)
        {
            outputUpdateTimer.reset();

            PID.calculatePID();
            speedReg.updateTargetSpeed(PID.getLeftSpeed(), PID.getRightSpeed());
            toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
        };
    };
};
