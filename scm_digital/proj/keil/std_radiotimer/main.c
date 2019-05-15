/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ definition =====================================

// rf timer interruption bit
#define RFTIMER_COMPARE0_INT        0x00000001
#define RFTIMER_COMPARE1_INT        0x00000010

#define TIMER_MAX_COUNTER           0xFFFFFFFF
#define TIMER_COMPARE_INCREASEMENT  500000  // 1 second at 500KHz

struct radiotimer_vars_t{
    unsigned int flag;
};

//============================== Variables ====================================

struct radiotimer_vars_t radiotimer_vars;

//============================ Main Function===================================

int main(void) {
    unsigned int temp;
    
    // set max counter
    RFTIMER_REG__MAX_COUNT          = TIMER_MAX_COUNTER;
    
    // enable the timer, global interrupt and clear when counter resets (i.e. starts)
    RFTIMER_REG__CONTROL            = 0x07;
    
    // set compare counter
    RFTIMER_REG__COMPARE0           = TIMER_COMPARE_INCREASEMENT;
    
    // enable compare module and enable timer compare interrupt
    RFTIMER_REG__COMPARE0_CONTROL   = 0x03;
    while(1){
        if(radiotimer_vars.flag)  {
            temp                    = GPIO_REG__OUTPUT;
            GPIO_REG__OUTPUT        = temp + 1;
            radiotimer_vars.flag    = 0;
        }
    }
}

void RFTIMER_ISR() {
    unsigned int compareCounter;
    unsigned int interrupt      = RFTIMER_REG__INT;
    
    if (interrupt & RFTIMER_COMPARE0_INT){
        compareCounter          = RFTIMER_REG__COMPARE0;
        RFTIMER_REG__COMPARE0   = compareCounter + TIMER_COMPARE_INCREASEMENT;
        printf("Hello world!\r\n");
        radiotimer_vars.flag    = 0x01;
    }
    
    RFTIMER_REG__INT_CLEAR      = interrupt;
}

void UART_ISR(){
    
    int rxByte;
    // bug? if we do not read the contents of UART_REG__RX_DATA in the ISR, execution hangs
    rxByte = UART_REG__RX_DATA;
    // UART_REG__TX_DATA = rxByte;
}


