// CommonDefs.h
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#define SENSOR_AMOUNT 6
#define SENSOR_BUFFER 5
#define SENSOR_POLL_FREQ 5000 // Hz
#define BLE_BYTE_LENGTH 16    //Bytes being sent to the HM10 (4-byte int, 4-byte PID KP, 4-byte PID KI, 4-byte PID KD)
#define BLE_POLL_RATE 20      // Hz

#define GAIN_PROPORTIONAL 4.0f //scale in the single digits now
#define GAIN_INTEGRAL 0.30f
#define GAIN_DERIVATIVE 1.15f
#define GAIN_SCALE_DOWN 140.0f //used to scale down gain values 
#define EDGE_SCALING 1.2f
#define GUARD1_SCALING 3.0f
#define GUARD2_SCALING 5.0f

#define BASE_SPEED 1.00f
#define SYS_LCD_STEPDOWN 10

#define EASING_FACTOR 10.0f          // For initial PWM ease between PID output and internal PWM output
#define S_EASING_FACTOR 0.05         // For secondary correction between internal PWM (takes into account wheel speed)

#define PWM_FREQUENCY 20000   // Hz
#define SYS_OUTPUT_RATE 1000  // Hz

#define WHEEL_DIAMETER 0.083     // meters
#define PI 3.1415
#define CPR 512          // Counts per revolution for the encoder
#define TCRT_MAX_VDD 5.0 // Max voltage for TCRT sensors
#define DEFAULT_PWM 0.5f // Default PWM duty cycle

typedef enum
{
    turnAround,
    init,
    RCstop,
} pstate;
extern pstate ProgramState;

#endif // COMMON_DEFS_H
