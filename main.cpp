#include "mbed.h"
#include "C12832.h"

typedef enum {starting,straightline,curve,stop,turnaround} pstate;
pstate ProgramState;

void toScreen(char* line1, char*  line2, char* line3,C12832* lcd){
    static char lastLine1[21] = "";
    static char lastLine2[21] = "";
    static char lastLine3[21] = "";
    if(strncmp(lastLine1,line1,20) != 0){
        strncpy(lastLine1,line1,20);
        lastLine1[20] = '\0';
        lcd->locate(0,0);
        lcd->printf(line1);
    };
    if(strncmp(lastLine2,line2,20) != 0){
        strncpy(lastLine2,line2,20);
        lastLine2[20] = '\0';
        lcd->locate(0,10);
        lcd->printf(line2);
    };
    if(strncmp(lastLine3,line3,20) != 0){
        strncpy(lastLine3,line3,20);
        lastLine3[20] = '\0';
        lcd->locate(0,20);
        lcd->printf(line3);
    };
};

int main (void)
{
    C12832 lcd(D11, D13, D12, D7, D10);
    while(1)
    {

    };
};