#ifndef TCRT_H
#define TCRT_H

#include "mbed.h"
#include <cstdint>
#include "CommonDefs.h"

// TCRT class. Creates an object for individual sensors on the sensor array with rolling average (size 5) built in.
// Provide the Pin (ADC) for the sensor and the voltage levels expected (if you want to scale to voltage instead of from 0.0 - 1.0)
// Use getSensorVoltage() to get value of voltage (rolled average).
class TCRT
{
private:

    AnalogIn sensorPin;
    DigitalOut DarlingtonPin;
    double VDD, senseNorm;
    double senseNormRolled[SENSOR_BUFFER];
    uint32_t rIndex;
    static TCRT *sensors[SENSOR_AMOUNT]; // operating with one ticker, so we must have a pointer to all the TCRT objects (to be used for function callback)
    static int sensorCount;              // to keep count of the sensors

public:

    /**
    Constructor.

    This class creates individual objects for the sensors instead of having one object control all sensors. 
    This is done to facilitate easier sensor debugging and interaction between classes of the program. 
    The specific implementation also allows for great flexibility with sensor voltages (dependent on vertical position), sensor amount, and processing limits for the STM32.

    Sensor polling rate is defined by SENSOR_POLL_FREQ, and sensor buffer depth is hardcoded to be 10. 
    Each sensor is put through the buffer and then averaged over ten to give a rolling average.
    This is done to reduce noise, but also requires that the sensors be polled at 10 times the rate of the main loop. 

    @param Pin This pin is the analogue input for the sensor. Do not exceed 5 V or 500 kHz as per ADC limits.
    @param DarlingPin Pin for the darlington array specific to the sensor.
    @param v Expected maximum voltage for the sensor. Use TCRT_MAX_VDD if unsure.
    **/
    TCRT(PinName Pin, PinName DarlingPin, double v);

    //Sets the Darlington Pin output to 1 to enable the sensor.
    void turnSensorOn(void);

    //Sets the Darlington Pin output to 0 to disable the sensor.
    void turnSensorOff(void);

    /**
    Resource heavy function.

    Reads sensor voltage into senseNormRolled[] array cyclically with modulo. 
    Sums all 10 elements of the array and divides by 10 to give the rolling average of the sensor voltage (data specific to sensor object).
    Never called explicitly, but is called through pollSensors().
    **/
    void rollingPollAverage(void);

    /**
    Resource heavy function.

    Static function used across all sensors to call rollingPollAverage().
    Cycles through pointer array of sensor objects and calls rollingPollAverage() on each one.
    Use with care. Execution time increases linearly with number of sensors and sensor buffer depth.
    **/
    static void pollSensors(void);

    /**
    Gets current sensor voltage rolling average.

    @param Volt TRUE for average sensor voltage scaled to maximum sensor voltage. FALSE for ADC reading voltage without average.
    **/
    double getSensorVoltage(bool Volt = false);
    
};

#endif