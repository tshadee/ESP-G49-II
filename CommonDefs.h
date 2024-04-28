// CommonDefs.h
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#define SENSOR_AMOUNT 6         //Number of sensors
#define SENSOR_BUFFER 2         //Buffer depth for sensor rolling average buffer
#define SENSOR_POLL_FREQ 16000   //Hz , Ideally 5 times that of SYS_OUTPUT_RATE

#define GAIN_PROPORTIONAL 5.0f  //Proportionl gain of PID
#define GAIN_INTEGRAL 0.08f      //Integral gain of PID
#define GAIN_DERIVATIVE 0.2f   //Derivative gain of PID

#define GAIN_SCALE_DOWN 125.0f  //scales down every scaling value. Divided at the end of output calculation.
#define EDGE_SCALING 0.35f       //Overall scaling for EDGE sensors (S4,S3)
#define GUARD1_SCALING 5.5f     //Overall scaling for GUARD1 sensors (S5,S2)
#define GUARD2_SCALING 20.0f     //Overall scaling for GUARD2 sensors (S6,S1)

#define GAIN_P_WHEEL 60.0f
#define GAIN_I_WHEEL 0.05f

#define BASE_SPEED 1.0f         //m/s. Base speed at which the buggy runs. With current tuning, stable until ~1.1 m/s

#define EASING_FACTOR 20.0f     //PWM easing based on target speed. Higher values means more reactive PWM.

#define PWM_FREQUENCY 20000     //Hz , should be <50 Hz or >20000Hz (outside human hearing range).
#define SYS_OUTPUT_RATE 8000    //Hz , Ideally 1/5 that of SENSOR_POLL_FREQ and no greater than 1000 Hz.

#define WHEEL_DIAMETER 0.083    //meters
#define PI 3.1415
#define CPR 512                 // Counts per revolution for the encoder (512 for X2, 1024 for X4)
#define TCRT_MAX_VDD 5.0        // Max voltage for TCRT sensors
#define DEFAULT_PWM 0.5f        // Default PWM duty cycle

#define BLACK 0.62f

#define SYS_LCD_STEPDOWN 10

typedef enum
{
    turnAround,
    init,
    RCstop,
    displayingSensor
} pstate;
extern pstate ProgramState;

#endif // COMMON_DEFS_H
