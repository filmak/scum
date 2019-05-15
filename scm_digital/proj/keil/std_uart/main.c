/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ definition =====================================

#define BUFFER_LENGTH 20

//============================== Variables ====================================

char buffer_tx[BUFFER_LENGTH] = "hello world\n";
char buffer_rx[BUFFER_LENGTH];
int  buffer_idx;

//============================ Main Function===================================

int main(void) {
    int i;
    int j;
    buffer_idx = 0;
    for (i=0; i < BUFFER_LENGTH; i++) {
        UART_REG__TX_DATA = buffer_tx[i];
    }
    
    while(1)  {
        
        for (i=0;i<1000;i++){
            for (j=0;j<1000;j++){
            }
        }
    }
}

void UART_ISR(){
    int i;
    char rxByte;
    
    rxByte = UART_REG__RX_DATA;
    
    if (rxByte == '\r'){
        GPIO_REG__OUTPUT = 0x0f;
        for (i=0; i < BUFFER_LENGTH; i++) {
            UART_REG__TX_DATA = buffer_rx[i];
        }
        buffer_idx = 0;
        GPIO_REG__OUTPUT = 0x00;
    } else {
        buffer_rx[buffer_idx++] = rxByte;
        GPIO_REG__OUTPUT ^= 0x01;
    }
}


