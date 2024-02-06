### University of Manchester ESP Buggy Project Group 49 - Version 2

We aim to make a buggy that:
1. Works.
2. Wins (optional).

- Git and Mbed integration for [source control](https://os.mbed.com/docs/mbed-studio/current/source-control/index.html).
- Painful [Lower Layer Library and HAL documentation](https://www.st.com/en/embedded-software/stm32cubef4.html) by ST Electronics for those who want to change the Cortex M4's operation.
- Painful [STM32F401RE documentation](https://www.st.com/en/microcontrollers-microprocessors/stm32f401re.html) for interfacing with the STM32F401RE provided by ST Electronics.
- Outdated [Mbed Shield](https://os.mbed.com/components/mbed-Application-Shield/) that sits on top of the STM32.
- The Bluetooth module used ([HM-10](https://os.mbed.com/users/alexsaadfalcon/notebook/hm10-guide/)).
- Quadrature sensor used for monitoring speed ([AEAT-601B-F06](https://www.broadcom.com/products/motion-control-encoders/incremental-encoders/magnetic-encoders/aeat-601bf06)).
- Mbed Libraries: [QEI](https://os.mbed.com/users/aberk/code/QEI/), [C12832](https://os.mbed.com/teams/components/code/C12832/), [BLE API](https://os.mbed.com/teams/Bluetooth-Low-Energy/code/BLE_API/)

AS A CONSEQUENCE OF THE MOST TERRIBLE IMPLEMENTATION KNOWN TO MAN,
you are required to clone this project since Mbed OS 5 cannot handle the Libraries NECESSARY for the buggy to run. Mbed 2 is deprecated, and as far as I know, our QEI, BLE, and C12832 libraries are also FUCKED in the process.

Again, go through the procedure of linking your Keil Studio instance with the NEW GitHub repository on my account "tshadee/ESP-G49-II"; your collaborator permissions should already be configured. Test Git push-pull by commenting on the top of main.cpp to confirm that it is working!

Thank you, and fuck you Mbed.
