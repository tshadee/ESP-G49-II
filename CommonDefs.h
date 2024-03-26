// CommonDefs.h
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#define SENSOR_AMOUNT 6
#define SENSOR_BUFFER 5
#define SENSOR_POLL_FREQ 5000 // Hz

#define GAIN_PROPORTIONAL 4.0f //scale in the single digits now
#define GAIN_INTEGRAL 0.22f
#define GAIN_DERIVATIVE 0.75f
#define GAIN_SCALE_DOWN 150.0f //used to scale down gain values 
#define EDGE_SCALING 1.20f
#define GUARD1_SCALING 2.70f
#define GUARD2_SCALING 5.00f

#define BASE_SPEED 0.7f
#define SYS_LCD_STEPDOWN 10

#define EASING_FACTOR 3.0            // For initial PWM ease between PID output and internal PWM output
#define S_EASING_FACTOR 0.05         // For secondary correction between internal PWM (takes into account wheel speed)
#define PWM_DIFFERENTIAL_FACTOR 0.05 // difference between the two PWM duty cycles (used for determining condition to enforce straight line logic)
#define BRAKING_FACTOR 0.035

#define PWM_FREQUENCY 20000 // Hz
#define SYS_OUTPUT_RATE 800  // Hz

#define WHEEL_DIAMETER 0.083     // meters
#define GEAR_RATIO (1.0 / 15.0)    // Gear ratio (GB1)
#define PI 3.1415
#define CPR 512         // Counts per revolution for the encoder
#define TCRT_MAX_VDD 5.0 // Max voltage for TCRT sensors
#define DEFAULT_PWM 0.5f // Default PWM duty cycle

typedef enum
{
    turnAround,
    init,
    RCforward,
    RCbackwards,
    RCturnleft,
    RCturnright,
    RCstop,
    RCturbo
} pstate;
extern pstate ProgramState;

#endif // COMMON_DEFS_H
