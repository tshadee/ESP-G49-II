#include "mbed.h"
#include "C12832.h"
#include "ds2781.h"
#include "OneWire_Methods.h"
#include "TCRT.h"
#include "encoder.h"
#include <cstdint>

#define SENSOR_POLL_FREQ 1000 //Hz

//need tuning
#define GAIN_PROPORTIONAL 0.1 
#define GAIN_INTEGRAL 0.1
#define GAIN_DERIVATIVE 0.1
#define GAIN_AGGRESSIVE 1.2

#define BASE_DUTY 0.8f

#define EASING_FACTOR 1.0 //natural easing. For faster PID-to-PWM output, set > 1.0. For better smoothness and easing, set < 1.0.

#define SYS_OUTPUT_RATE 50 //Hz

#define WHEEL_DIAMETER 0.08//m
#define GEAR_RATIO 1/12
#define PI 3.14
#define CPR 256
#define TCRT_MAX_VDD 3.3 //V
#define DEFAULT_PWM 0.5f

//seerial 6 RX TX PA_11/12
//ANALOG IN PC_2/3/4/5 PB_1
//PWM PC_8/6
//encoder PB_15/14

typedef enum {starting,straightline,stop,turnaround} pstate;
pstate ProgramState;

class PWMGen {
    private:
        PwmOut PWM_LEFT, PWM_RIGHT;
    public:
        PWMGen(PinName P1, PinName P2): PWM_LEFT(P1),PWM_RIGHT(P2) { reset();};
        void reset()
        {
            //PWM_LEFT.period(0.0f);
            //PWM_RIGHT.period(0.0f);
            PWM_LEFT.write(DEFAULT_PWM);
            PWM_RIGHT.write(DEFAULT_PWM);
        };
        void begin()
        {
            PWM_LEFT.period(1.0/SYS_OUTPUT_RATE);
            PWM_RIGHT.period(1.0/SYS_OUTPUT_RATE);
        };
        void setPWMDuty(float leftPWM, float rightPWM)
        {
            PWM_LEFT.write(leftPWM);
            PWM_RIGHT.write(rightPWM);
        };
};

//use the one-wire-pin PC_12
class BatteryMonitor {
    private:
        DigitalInOut one_wire_pin;
        int VoltageReading, CurrentReading;
        float Voltage, Current, currentAccum, powerUsed, initCharge,chargePercentLeft;
    public:
        BatteryMonitor(PinName P1): one_wire_pin(P1) {
            VoltageReading = ReadVoltage();
            CurrentReading = ReadCurrent();
            Voltage = VoltageReading*0.00967;
            Current = CurrentReading/6400.0;
            powerUsed = 0.0;
            initCharge = Voltage*0.00967*2.8*3600;
        };
        void pollBattery(void){
            VoltageReading = ReadVoltage();
            CurrentReading = ReadCurrent();
            Voltage = VoltageReading*0.00967;
            Current = CurrentReading/6400.0;
            powerUsed += Current*Voltage/SYS_OUTPUT_RATE;
            chargePercentLeft  = 100.0 - ((initCharge - powerUsed)/initCharge)*100;
        };
        float getBatteryVoltage(void){return Voltage;};
        float getBatteryCurrent(void){return Current;};
        float getChargeLeft(void){return chargePercentLeft;};
};

/*
This class is for calculating PID error as two PWM outputs. This is unfiltered output, btw, so we will have to pass this through a speed regulator that takes into account current wheel speed and other buggy params.
The PID output value assume that the error is RIGHT - LEFT (looking from buggy top view), so a situation where right > left would be when the buggy is headed left and vice versa.
*/
class PIDSys {
    private:
        float error[3];
        float errorOuter[3];
        float output;
        float leftPWM, rightPWM;
        float A0,A1,A2;
        TCRT* S1;
        TCRT* S2;
        TCRT* S4;
        TCRT* S5;
    public:
        PIDSys(TCRT* s1, TCRT* s2, TCRT* s4, TCRT* s5): S1(s1),S2(s2),S4(s4),S5(s5)
        {
            A0 = GAIN_PROPORTIONAL + GAIN_INTEGRAL/SYS_OUTPUT_RATE + GAIN_DERIVATIVE*SYS_OUTPUT_RATE;
            A1 = -GAIN_PROPORTIONAL - 2*GAIN_DERIVATIVE*SYS_OUTPUT_RATE;
            A2 = GAIN_DERIVATIVE*SYS_OUTPUT_RATE;
            reset();
        };
        void reset()
        {
            error[2] = error[1] = error[0] = output = 0;
            leftPWM = DEFAULT_PWM;
            rightPWM = DEFAULT_PWM;
        };
        void calculatePID(bool toggleAggressive)
        {
            if(toggleAggressive){
                errorOuter[2] = errorOuter[1];
                errorOuter[1] = errorOuter[0];
                errorOuter[0] = (S5->getSensorVoltage(false) - S1->getSensorVoltage(false));
                output = (output + A0*errorOuter[0] + A1*errorOuter[1] + A2*errorOuter[2])*GAIN_AGGRESSIVE;
            } else {
                error[2] = error[1];
                error[1] = error[0];
                error[0] = (S4->getSensorVoltage(false) - S2->getSensorVoltage(false));
                output = output + A0*error[0] + A1*error[1] + A2*error[2];
            };
            outputPWM();
        };
        void outputPWM(void)
        {
            if(output > 0)
            {
                leftPWM = BASE_DUTY + output; 
                rightPWM = BASE_DUTY - output;
            } else if (output < 0) {
                leftPWM = BASE_DUTY - output;
                rightPWM = BASE_DUTY + output;
            } else {
                leftPWM = 1.0f;
                rightPWM = 1.0f;
            };
        };
        float getLeftPWM(void){return leftPWM;};
        float getRightPWM(void){return rightPWM;};
};

//Speed regulator. Takes the PWM output of the PIDSys class and eases actual PWM towards the target value (similar to PID). This is to prevent sudden changes to buggy trajectory.
class speedRegulator {
    private:
        float currentLeftPWM, currentRightPWM;
        float targetLeftPWM, targetRightPWM;
        Encoder* leftWheelEncoder;
        Encoder* rightWheelEncoder;
    public:
        speedRegulator(Encoder* LWC, Encoder* RWC): leftWheelEncoder(LWC),rightWheelEncoder(RWC){
            currentLeftPWM = currentRightPWM = DEFAULT_PWM;
        };
        void updateTargetPWM(float leftPWM, float rightPWM) {
            targetLeftPWM = leftPWM;
            targetRightPWM = rightPWM;
            easePWMOutput();
        };
        void easePWMOutput() {
            currentLeftPWM += (targetLeftPWM - currentLeftPWM)*EASING_FACTOR;
            currentRightPWM += (targetRightPWM - currentRightPWM)*EASING_FACTOR;
            currentLeftPWM = (currentLeftPWM < 0.0f)? 0.0f : ((currentLeftPWM > 1.0f)? 1.0f : currentLeftPWM);
            currentRightPWM = (currentRightPWM < 0.0f)? 0.0f : ((currentRightPWM > 1.0f)? 1.0f : currentRightPWM);
        };
        float getCurrentLeftPWM(void){return currentLeftPWM;};
        float getCurrentRightPWM(void){return currentRightPWM;};
};

//LCD display buffer. Pass string pointers to display in lines 1-3 on the LCD screen. Keep the strings under 23 bytes if possible
// call toScreen with the appropriate arguments to push anything to the LCD display - refresh rate sensitive (dont go above 15 Hz)
void toScreen(char* line1, char*  line2, char* line3,C12832* lcd){
    static char lastLine1[21] = "";
    static char lastLine2[21] = "";
    static char lastLine3[21] = "";
    if(strncmp(lastLine1,line1,20) != 0){
        strncpy(lastLine1,line1,20);
        lastLine1[20] = '\0';
        lcd->locate(0,0);
        lcd->printf(line1);
    };
    if(strncmp(lastLine2,line2,20) != 0){
        strncpy(lastLine2,line2,20);
        lastLine2[20] = '\0';
        lcd->locate(0,10);
        lcd->printf(line2);
    };
    if(strncmp(lastLine3,line3,20) != 0){
        strncpy(lastLine3,line3,20);
        lastLine3[20] = '\0';
        lcd->locate(0,20);
        lcd->printf(line3);
    };
};

char* batteryMonitorBuffer(BatteryMonitor* Batt){
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f V          ", Batt->getBatteryVoltage());
    return dspBuffer;
};

char* sensorVoltageBuffer(TCRT* S1,TCRT* S2){
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", S1->getSensorVoltage(true),S2->getSensorVoltage(true));
    return dspBuffer;
};

class JoystickISR {
    private:
        InterruptIn centreJoy;
        Timeout joyDebounce;
        bool toggleState;
    public:
    JoystickISR(PinName centre) : centreJoy(centre){
        toggleState = false;
        centreJoy.rise(callback(this,&JoystickISR::centreISR));
    };
    void toggleInput(){
        toggleState = !toggleState;
        if(toggleState){
            centreJoy.disable_irq();
        } else {
            centreJoy.enable_irq();
        };
    };
    void timerDebounce(){joyDebounce.attach(callback(this,&JoystickISR::toggleInput),0.3);};
    void centreISR(){
        toggleInput();
        timerDebounce();
        switch(ProgramState){
            case(starting)      : ProgramState = straightline;  break;
            case(straightline)  : ProgramState = stop;         break;
            case(stop)          : ProgramState = turnaround;    break;
            case(turnaround)    : ProgramState = starting;      break;
            default:break;
        };
    };
};

int main (void)
{
    JoystickISR joy(D4);
    C12832 lcd(D11, D13, D12, D7, D10);
    PWMGen toMDB(PC_8,PC_6);            //PLACEHOLDER PINS
    BatteryMonitor Battery(PC_12); 
    Encoder leftWheel(PB_14,PB_15);     //PLACEHOLDER PINS
    Encoder rightWheel(PA_0,PA_1);      //PLACEHOLDER PINS
    TCRT S1(PA_0,TCRT_MAX_VDD) , S2(PA_1,TCRT_MAX_VDD) , S3(PA_4,TCRT_MAX_VDD) , S4(PB_0,TCRT_MAX_VDD), S5(PC_1,TCRT_MAX_VDD);  //PLACEHOLDER PINS
    PIDSys PID(&S1,&S2,&S4,&S5);
    speedRegulator speedReg(&leftWheel,&rightWheel);
    


    /*
    
    timeout corner for stupid code
    
    */


    Ticker sensorPollTicker;
    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);

    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms

    bool straightLineStart = true;

    while(1)
    {
        switch (ProgramState){
            case (starting):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();
                    PID.calculatePID(false);
                    speedReg.updateTargetPWM(PID.getLeftPWM(), PID.getRightPWM());
                    toMDB.setPWMDuty(speedReg.getCurrentLeftPWM(), speedReg.getCurrentRightPWM());
                    
                    /*
    
                    timeout corner for stupid code
                    
                    */

                    toScreen("START STATE        ", batteryMonitorBuffer(&Battery), "START STATE        ", &lcd);
                };
            break;};
            case (straightline):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();
                    

                    /*
                    
                    toMDB.setPWMDuty(1.0f, 1.0f); //test

                    
                    if(straightLineStart)
                    {
                        toMDB.begin();
                        straightLineStart = false;
                    };
                    */
                    toScreen("STRAIGHT LINE       ", sensorVoltageBuffer(&S3, &S4), sensorVoltageBuffer(&S1, &S5), &lcd);
                };
            break;};
            case (stop):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    /*
                    Battery.pollBattery();
                    

                    
                    if(!straightLineStart)
                    {
                        toMDB.reset();
                        straightLineStart = true;
                    };
                    */
                    toScreen("STOP!!!             ", batteryMonitorBuffer(&Battery), "                       ", &lcd);
                };
            break;};
            case (turnaround):{
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();
                    Battery.pollBattery();

                    /*
    
                    timeout corner for stupid code
                    
                    */

                    toScreen("TURN!!!             ", batteryMonitorBuffer(&Battery), "                       ", &lcd);
                };
            break;};
            default: {
                if(outputUpdateTimer.read_ms() >= timedelay){outputUpdateTimer.reset();

                    toScreen("SOMETHING BROKE","                       ","                       ", &lcd);
                };
            break;};
        };  
    };
};
