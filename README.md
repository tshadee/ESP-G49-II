### heat_race

Code optimised specifically for the heat race, without any other functionality than is needed to complete the track as fast as possible. 
This is as efficient and minimal as needed, so beware when loading code from *heat_race* as there will be **no** control over the buggy when it starts.

Classes included:
- TCRT
- SpeedRegulator
- PWMGeneration
- PIDSys
- Encoder

Current Configuration:
- 6 sensors
- X2 encoding
- No LCD
- No BLE
- No states
- Fast polling (10 kHz)
- Fast update (2 kHz)

Required external libraries (import these into Keil Studio): [QEI](https://os.mbed.com/users/aberk/code/QEI/)
