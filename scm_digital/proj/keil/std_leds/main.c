/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ Main Function===================================

int main(void) {
    int i;
    int j;
    int d;

    while(1)  {
        //read the GPIO value
        d = GPIO_REG__OUTPUT;
        // generate some delay
        for(i=0;i<1000;i++){
            for(j=0;j<1000;j++){
                
            }
        }
        // increase value of GPIO OUTPUT by one
        GPIO_REG__OUTPUT = d+1;
    }
}


