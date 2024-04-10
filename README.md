### University of Manchester ESP Buggy Project Group 49 - Academic Year 2023-2024

We aim to make a buggy that:

1. Works.
2. Wins (optional).

This repository will be split into three branches:

#### master
This is the base template for barebones functionality used in other branches. 
This branch should not be used for any of the tasks as it is not specialised and can only provide basic functions.

#### heat_race
Code optimised specifically for the heat race, without any other functionality than is needed to complete the track as fast as possible. 
This is as efficient and minimal as needed, so beware when loading code from *heat_race* as there will be **no** control over the buggy when it starts.

#### TDB
Includes BLE for the turnaround and utilises states. This has the full functionality of the buggy, ranging from Remote Control to autonomous line following.
Less efficient than *heat_race* but offers more flexibility in debugging and fine-tuning. 

Required external libraries (import these into Keil Studio): [QEI](https://os.mbed.com/users/aberk/code/QEI/), [C12832](https://os.mbed.com/teams/components/code/C12832/)

Other Resources:

- [Mbed Keil source control](https://os.mbed.com/docs/mbed-studio/current/source-control/index.html).
- [Lower Layer Library and HAL documentation](https://www.st.com/en/embedded-software/stm32cubef4.html).
- [STM32F401RE documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32f401re.html).
- [Mbed Shield](https://os.mbed.com/components/mbed-Application-Shield/).
- [HM-10](https://os.mbed.com/users/alexsaadfalcon/notebook/hm10-guide/).
- [AEAT-601B-F06](https://www.broadcom.com/products/motion-control-encoders/incremental-encoders/magnetic-encoders/aeat-601bf06).

You are currently in the **master** branch.

### Master

This is the base template for barebones functionality used in other branches. 
This branch should not be used for any of the tasks as it is not specialised and can only provide basic functions.

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
- Normal polling (5 kHz)
- Normal update (1 kHz)
