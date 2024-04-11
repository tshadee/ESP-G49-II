### TDB

Includes BLE for the turnaround and utilises states. This has the full functionality of the buggy, ranging from Remote Control to autonomous line following.
Less efficient than *heat_race* but offers more flexibility in debugging and fine-tuning. 

Classes included:
- TCRT
- SpeedRegulator
- PWMGeneration
- PIDSys
- Encoder
- LCDManager
- ExternalStimulus

Current Configuration:
- 6 sensors
- X2 encoding
- LCD 
- BLE 
- All states (All RC States, turnAround, lineFollowing, etc.)
- Normal polling (5 kHz)
- Normal update (1 kHz)

Required external libraries: [QEI](https://os.mbed.com/users/aberk/code/QEI/), [C12832](https://os.mbed.com/teams/components/code/C12832/)
