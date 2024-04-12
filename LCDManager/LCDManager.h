#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include "mbed.h"
#include "C12832.h"
#include "TCRT.h"
#include "encoder.h"

/** 
LCD display buffer class. Pass string pointers to display in lines 1-3 on the LCD screen only if strings update (saves processing). 
Passed strings MUST be smaller than 23 bytes, else overflow occurs. Call toScreen() to push to screen. Refresh rate must be no greater than 20 Hz for stable operation.
This class should ONLY be used for debugging and displaying values. It should NOT be used actively when the buggy is running as it takes considerable resource to run.
**/
class LCDManager
{

private:

    C12832 *lcd;

public:

    /**
    Constructor.

    Handles all the buffering and display updating for the screen. Restrictions mentioned above.
    @param c Pointer to C12832 class object.
    **/
    LCDManager(C12832 *c); 

    /**
    !USE ONLY FOR DEBUGGING!
    !RESOURCE HEAVY FUNCTION!

    Prints three lines to the LCD screen. Saves passed string pointers into a comparison buffer to see if update is needed.
    Speed limit is 20 Hz if the microcontroller is running at 16 MHz and 80 Hz if 84 MHz (PLL).
    Size of passed string pointers MUST NOT exceed 23 bytes, SPACES INCLUDED.

    @param line1 First line of the LCD screen
    @param line2 Second line of the LCD screen
    @param line3 Third line of the LCD screen
    **/
    void toScreen(const char *line1, const char *line2, const char *line3);

    /**
    Below are functions that can be changed to prepare string pointers to be passed to toScreen() function.
    Can be changed accordingly (but FloatOutput and FloatOutput2 are convenient for displaying general information).

    Functions use static buffers to store strings (under 23 bytes). Not advised to update multiple lines with one function, better to have two with separate static buffers.
    **/

    char *encoderOutputTest(Encoder *leftWheel, Encoder *rightWheel);
    char *SVB1(TCRT *S6, TCRT *S5, TCRT *S4);
    char *SVB2(TCRT *S1, TCRT *S2, TCRT *S3);
    char *FloatOutput(float one, float two, float three);
    char *FloatOutput2(float one, float two, float three);

};

#endif // LCD_MANAGER_H