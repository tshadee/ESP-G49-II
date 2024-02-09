// CommonDefs.h
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H

#define SENSOR_AMOUNT 5
#define SENSOR_BUFFER 5
#define SENSOR_POLL_FREQ 1000 // Hz

#define GAIN_PROPORTIONAL 0.1
#define GAIN_INTEGRAL 0.1
#define GAIN_DERIVATIVE 0.1
#define GAIN_AGGRESSIVE 1.2

#define BASE_DUTY 0.8f

#define EASING_FACTOR 1.0 // For PID-to-PWM output smoothing

#define SYS_OUTPUT_RATE 50 // Hz

#define WHEEL_DIAMETER 0.08 // meters
#define GEAR_RATIO (1.0 / 12.0) // Gear ratio
#define PI 3.1415
#define CPR 256 // Counts per revolution for the encoder
#define TCRT_MAX_VDD 3.3 // Max voltage for TCRT sensors
#define DEFAULT_PWM 0.5f // Default PWM duty cycle

typedef enum {starting,straightline,stop,turnaround} pstate;
extern pstate ProgramState;

#endif // COMMON_DEFS_H
