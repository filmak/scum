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
#define RFTIMER_CAPTURE0_INT        0x00001000
#define RFTIMER_CAPTURE1_INT        0x00002000

#define TIMER_MAX_COUNTER           1000000
#define TIMER_TX_LOAD_COUNTER        250000  // 0.5 second at 500KHz
#define TIMER_TX_SEND_COUNTER        500000  //   1 second at 500KHz

#define PACKET_LENGTH 128

enum radio_status_t{
    STATUS_IDLE             = 0x00,
    STATUS_TX_LOAD_DONE     = 0x01,
    STATUS_TX_SEND_DONE     = 0x02,
    STATUS_RX_SFD_DONE      = 0x03,
    STATUS_RX_RECEIVE_DONE  = 0x04,
};

//============================== Variables ====================================

enum radio_status_t status;

//============================ Main Function===================================

int main(void) {
    
    int i;
    int j;
    
    char send_packet[PACKET_LENGTH];
    
    RFCONTROLLER_REG__INT_CONFIG    = 0x3FF;    // Enable all interrupts and pulses to radio timer
    RFCONTROLLER_REG__ERROR_CONFIG  = 0x1F;     // Enable all errors
    RFTIMER_REG__MAX_COUNT          = TIMER_MAX_COUNTER;
    RFTIMER_REG__CONTROL            = 0x7;
    
    RFCONTROLLER_REG__TX_DATA_ADDR  = &send_packet[0];
    sprintf(send_packet , "Magic packet delivery!"); 
    RFCONTROLLER_REG__TX_PACK_LEN = 22;
    
    // set compare couter
    RFTIMER_REG__COMPARE0           = TIMER_TX_LOAD_COUNTER;
    // enable compare module and tx load interrup
    RFTIMER_REG__COMPARE0_CONTROL   = 0x05;
    
    // set compare couter
    RFTIMER_REG__COMPARE1           = TIMER_TX_SEND_COUNTER;
    // enable compare module  and tx send interrup
    RFTIMER_REG__COMPARE1_CONTROL   = 0x09;
    
    while(1){
        status = STATUS_IDLE;
        
        for (i=0;i<1000;i++){
            for (j=0;j<1000;j++){
            }
        }
    }
    
}

void RF_ISR() {
    
    unsigned int interrupt = RFCONTROLLER_REG__INT;
    unsigned int error     = RFCONTROLLER_REG__ERROR;
    
  if (interrupt & 0x00000001) printf("TX LOAD DONE\r\n");
    if (interrupt & 0x00000002) printf("TX SFD DONE\r\n");
    if (interrupt & 0x00000004) printf("TX SEND DONE\r\n");
    
    if (error == 0) {
        if (error & 0x00000001) printf("TX OVERFLOW ERROR\r\n");
        if (error & 0x00000002) printf("TX CUTOFF ERROR\r\n");
        if (error & 0x00000004) printf("RX OVERFLOW ERROR\r\n");
        if (error & 0x00000008) printf("RX CRC ERROR\r\n");
        if (error & 0x00000010) printf("RX CUTOFF ERROR\r\n");
        RFCONTROLLER_REG__ERROR_CLEAR = error;
    }
    RFCONTROLLER_REG__INT_CLEAR = interrupt;
}

void RFTIMER_ISR() {
  
    unsigned int interrupt = RFTIMER_REG__INT;
    
    if (interrupt & RFTIMER_COMPARE0_INT){

    }
    
    if (interrupt & RFTIMER_COMPARE1_INT){
        
    }
    
    if (interrupt & RFTIMER_CAPTURE0_INT){
        
    }
    
    if (interrupt & RFTIMER_CAPTURE1_INT){
        
    }
    
    RFTIMER_REG__INT_CLEAR = interrupt;
}

