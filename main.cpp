#pragma once
#include "mbed.h"
#include "C12832.h"
#include "ds2781.h"
#include "OneWire_Methods.h"
#include "QEI.h"
#include <cstdint>

#define SENSOR_AMOUNT 5
#define SENSOR_BUFFER 5 //Buffer Size
#define SENSOR_POLL_FREQ 1000 //Hz

#define GAIN_PROPORTIONAL 0.1
#define GAIN_INTEGRAL 0.1
#define GAIN_DERIVATIVE 0.1

#define SYS_OUTPUT_RATE 50 //Hz

#define WHEEL_DIAMETER 0.08//m
#define GEAR_RATIO 1/12
#define PI 3.14
#define CPR 256
#define TCRT_MAX_VDD 3.3 //V

//seerial 6 RX TX PA_11/12
//ANALOG IN PC_2/3/4/5 PB_1
//PWM PC_8/6
//encoder PB_15/14


typedef enum {starting,straightline,curve,stop,turnaround} pstate;
pstate ProgramState;

//TCRT class. Creates an object for individual sensors on the sensor array with rolling average (size 5) built in. 
//Provide the Pin (ADC) for the sensor and the voltage levels expected (if you want to scale to voltage instead of from 0.0 - 1.0)
//Use getSensorNorm() to get value of voltage (rolled average). 
class TCRT {
    private:
        AnalogIn sensorPin;
        float VDD,senseNorm;
        float senseNormRolled[SENSOR_BUFFER];
        int rIndex;
        static TCRT* sensors[SENSOR_BUFFER]; //operating with one ticker, so we must have a pointer to all the TCRT objects (to be used for function callback)
        static int sensorCount; //to keep count of the sensors
    public:
        TCRT(PinName Pin, float v): sensorPin(Pin), VDD(v)
        {
            rIndex = 0;
            senseNorm = 0;
            for(int i = 0;i<SENSOR_BUFFER;i++){ senseNormRolled[i] = 0; };
            if(sensorCount < SENSOR_AMOUNT){ sensors[sensorCount++] = this; };
        };
        void rollingPollAverage()
        {
            senseNormRolled[rIndex % SENSOR_BUFFER] = ampNorm();
            rIndex++;
            senseNorm = 0;
            for(int i = 0;i < SENSOR_BUFFER;i++){senseNorm += senseNormRolled[i];};
            senseNorm /= SENSOR_BUFFER;
        };
        //runs through all the polling once called. This is for synchronous polling between sensors since static is shared between all objects derived from TCRT
        static void pollSensors(void) { for(int i = 0; i < sensorCount; i++) {sensors[i]->rollingPollAverage(); }; }; 
        float ampNorm(void){return (sensorPin.read());};
        float getSensorNorm(bool Volt){return (Volt? senseNorm*VDD : senseNorm);}; 
};

TCRT* TCRT::sensors[SENSOR_AMOUNT]; //static member declaration (must be outside class)
int TCRT::sensorCount = 0;  //static member declaration (must be outside class)


//class is being revamped
/*
            angleVelocity = (rightSpeed - leftSpeed)/0.1864;
            transVelocity = (rightSpeed + leftSpeed)/2;
*/
class Encoder {
    private:
        QEI encode;
        uint32_t count, countPrev;
        float distance, speed;
    public:
        Encoder(PinName channel1, PinName channel2): encode(channel1,channel2,NC,CPR){ resetAllValues(); };
        void updateValues(void)
        {
            countPrev = count;
            count += encode.getPulses();
            distance = encode.getRevolutions()*WHEEL_DIAMETER*PI;
            speed = (((count - countPrev)/CPR)*SYS_OUTPUT_RATE)*WHEEL_DIAMETER*PI*GEAR_RATIO;
        };
        void resetAllValues(void)
        {
            encode.reset();
            distance = 0;
            speed = 0;
            count = 0;
            countPrev = 0;
        };
        float getDist(void){return distance;};  //returns distance from last reset() call
        float getSpeed(void){return speed;};    //returns speed in m/s
};

//THIS CLASS INCLUDES PID AND PWM OUTPUT. Output rate will be tied to another Ticker. DO NOT MODIFY THIS YET.
class PWMGen {
    private:
        float PWM_Internal_1, PWM_Internal_2;
        DigitalOut PWM_OUT_CHANNEL_1, PWM_OUT_CHANNEL_2;
        float IntegralBuffer[2], DerivativeBuffer[2];
    public:
        PWMGen(PinName P1, PinName P2): PWM_OUT_CHANNEL_1(P1),PWM_OUT_CHANNEL_2(P2){};
};

//use the one-wire-pin PC_12
class BatteryMonitor {
    private:
        DigitalInOut one_wire_pin;
        int VoltageReading, CurrentReading;
        float Voltage, Current;
    public:
        BatteryMonitor(PinName P1): one_wire_pin(P1) {
            VoltageReading = 0;
            CurrentReading = 0;
            Voltage = 0.0;
            Current = 0.0;
        };
        void pollBattery(void){
            VoltageReading = ReadVoltage();
            Voltage = VoltageReading*0.00967;
            CurrentReading = ReadCurrent();
            Current = CurrentReading/6400.0;
        };

        float getBatteryVoltage(void){return Voltage;};
        float getBatteryCurrent(void){return Current;};
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

char* screenLine2Buffer(BatteryMonitor* Batt){
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f           ", Batt->getBatteryVoltage());
    return dspBuffer;
};

char* sensorVoltageTopBuffer(TCRT* S1,TCRT* S5)
{
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", S1->getSensorNorm(true),S5->getSensorNorm(true));
};

char* sensorVoltageMiddleBuffer(TCRT* S2,TCRT* S4)
{
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f      %.02f     ", S2->getSensorNorm(true),S4->getSensorNorm(true));
};

char* sensorVoltageLowerBuffer(TCRT* S3)
{
    static char dspBuffer[20];
    sprintf(dspBuffer, "%.02f               ", S3->getSensorNorm(true));
};

void SubSystemPoll(void){
};

int main (void)
{
    C12832 lcd(D11, D13, D12, D7, D10);
    BatteryMonitor Battery(PC_12);
    Ticker sensorPollTicker;
    Encoder leftWheel(PB_14,PB_15);
    TCRT S1(PC_2,TCRT_MAX_VDD) , S2(PC_3,TCRT_MAX_VDD) , S3(PC_4,TCRT_MAX_VDD) , S4(PC_5,TCRT_MAX_VDD), S5(PB_1,TCRT_MAX_VDD);

    float sensorPollRate = 1.0/SENSOR_POLL_FREQ;
    sensorPollTicker.attach(callback(&TCRT::pollSensors),sensorPollRate);
    
    Timer outputUpdateTimer;
    outputUpdateTimer.start();
    int timedelay = (static_cast<int>(1000/SYS_OUTPUT_RATE)); //in ms
    while(1)
    {
        switch (ProgramState){
            case (starting):{
                if(outputUpdateTimer.read_ms() >= timedelay){
                    outputUpdateTimer.reset();

                    toScreen("Battery Voltage:       ", screenLine2Buffer(&Battery), "                       ", &lcd);
                };
            };
            case (straightline):{
                
            };
            case (curve):{
                
            };
            case (stop):{
                
            };
            case (turnaround):{
                
            };


        
        };
    };
};
