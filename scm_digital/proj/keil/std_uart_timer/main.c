/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ definition =====================================

//#define LENGTH                      20
//#define LENGTH                      25
//#define LENGTH                      29
//#define LENGTH                      30
#define LENGTH                      50

// rf timer interruption bit
#define RFTIMER_COMPARE0_INT        0x00000001

#define TIMER_MAX_COUNTER           7487
#define TIMER_COMPARE0_INCREASEMENT 0  // 1 second at 500KHz

struct radiotimer_vars_t{
    unsigned int flag;
};

//============================== Variables ====================================

struct radiotimer_vars_t radiotimer_vars;

//============================ Main Function===================================

int main(void) {
    
    // set max counter
    RFTIMER_REG__MAX_COUNT          = TIMER_MAX_COUNTER;
    // enable the timer, global interrupt and clear when count reset
    RFTIMER_REG__CONTROL            = 0x07;
    
    // set compare couter
    RFTIMER_REG__COMPARE0           = TIMER_COMPARE0_INCREASEMENT;
    // enable compare module and interrup    
    RFTIMER_REG__COMPARE0_CONTROL   = 0x03;
    
    while(1){
    }
}

void RFTIMER_ISR() {
    unsigned int clear;
    unsigned int temp = 0;
    unsigned int interrupt      = RFTIMER_REG__INT;
    if (interrupt & RFTIMER_COMPARE0_INT){
        GPIO_REG__OUTPUT ^= 0x01;
        RFTIMER_REG__INT_CLEAR          = RFTIMER_COMPARE0_INT;
        GPIO_REG__OUTPUT ^= 0x04;
        while(temp<LENGTH){
            temp += 1;
            UART_REG__TX_DATA = temp;
        }
        GPIO_REG__OUTPUT ^= 0x04;
    }
}

void UART_ISR(){
    int rxByte;
    rxByte = UART_REG__RX_DATA;
//    UART_REG__TX_DATA = rxByte;
}


