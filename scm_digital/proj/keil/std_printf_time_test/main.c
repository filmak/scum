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
    // enable the timer, global interrupt and clear when count reset
    RFTIMER_REG__CONTROL            = 0x07;
    
    // set compare couter
    RFTIMER_REG__COMPARE0           = TIMER_COMPARE_INCREASEMENT;
    // enable compare module and interrup    
    RFTIMER_REG__COMPARE0_CONTROL   = 0x03;
    while(1){
        if(radiotimer_vars.flag)  {
            temp                    = GPIO_REG__OUTPUT;
            GPIO_REG__OUTPUT        = temp + 1;
            radiotimer_vars.flag    = 0;
            /*   Printf DELAY TEST
            **   try the following printf command and see the delay in a serail port
            **   when you test each case, comment the other printf code.
            **   Or, comment them all to see the delay.
            */
            printf("hello ");
            printf("hello world ");
            printf("hello world %d", temp);
            printf("hello world %d", RFTIMER_REG__COUNTER);
            
            // print out the delay with uart
            temp = RFTIMER_REG__COUNTER%500000;
            UART_REG__TX_DATA = (char)((temp/1000)+48);
            temp %= 1000;
            UART_REG__TX_DATA = (char)((temp/100)+48);
            temp %= 100;
            UART_REG__TX_DATA = (char)((temp/10)+48);
            temp %= 10;
            UART_REG__TX_DATA = (char)((temp)+48);
            UART_REG__TX_DATA = '\r';
            UART_REG__TX_DATA = '\n';
        }
    }
}

void RFTIMER_ISR() {
    unsigned int compareCounter;
    unsigned int interrupt      = RFTIMER_REG__INT;
    
    if (interrupt & RFTIMER_COMPARE0_INT){
        compareCounter          = RFTIMER_REG__COMPARE0;
        RFTIMER_REG__COMPARE0   = compareCounter + TIMER_COMPARE_INCREASEMENT;
        radiotimer_vars.flag    = 0x01;
    }
    
    RFTIMER_REG__INT_CLEAR      = interrupt;
}

void UART_ISR(){
    int rxByte;
    rxByte = UART_REG__RX_DATA;
//    UART_REG__TX_DATA = rxByte;
}


