#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

// ========================== definition ======================================

#define TSSLOTDURATION      500000    // 500000 = 1000 ms@500kHz
#define TSTXPREPAREDATA      50000    //  50000 =  100 ms@500kHz
#define TSTXOFFSET          100000    // 100000 =  200 ms@500kHz

// uncomment to have the mote be the transmitter
//#define TXMOTE              1

// ========================== variables =======================================

char send_packet[127];
char recv_packet[130];

//=========================== main ============================================

int main(void) {
    
    printf("\r\nThis is a synchronization demo\r\n"); 
    
	  //=== initialize radio and rftimer
	  
    RFCONTROLLER_REG__TX_DATA_ADDR  = &send_packet[0];
    // enable all interrupts and pulses to radio timer
    RFCONTROLLER_REG__INT_CONFIG    = 0x3FF;   
    // enable all errors
    RFCONTROLLER_REG__ERROR_CONFIG  = 0x1F;
    RFTIMER_REG__MAX_COUNT          = TSSLOTDURATION;
    RFTIMER_REG__CONTROL            = 0x7;
    
	  //=== arm timer compare
	  
    // set timer for slot duration
    RFTIMER_REG__COMPARE0           = 0;
    RFTIMER_REG__COMPARE0_CONTROL   = 0x03; // enable compare 0 interrupt
#ifdef TXMOTE
    // set timer for loading packet
    RFTIMER_REG__COMPARE1           = TSTXPREPAREDATA;
    RFTIMER_REG__COMPARE1_CONTROL   = 0x05; // enable compare 1 interrupt
    // sent timer for sending packet
    RFTIMER_REG__COMPARE2           = TSTXOFFSET;
    RFTIMER_REG__COMPARE2_CONTROL   = 0x09; // enable compare 2 interrupt
#else
    RFTIMER_REG__CAPTURE0_CONTROL   = 0x20; // enable capturing when SFD received
    RFTIMER_REG__CAPTURE1_CONTROL   = 0x40; // enable capturing when full packet received
#endif
    // control led
    RFTIMER_REG__COMPARE3           = TSSLOTDURATION/10;
    RFTIMER_REG__COMPARE3_CONTROL   = 0x03; // enable compare 3 interrupt
    printf("Initialization complete\n");

    while(1)  {
			 // low power mode ?
    }
}

void UART_ISR(){
    int temp;
    temp = UART_REG__RX_DATA;
    UART_REG__TX_DATA = temp;
}

void ADC_ISR() {
    printf("Conversion complete: 0x%x\r\n", ADC_REG__DATA);
}

void RF_ISR() {
    
    int captureTime;
    int newSlotDuration;
    unsigned int interrupt = RFCONTROLLER_REG__INT;
    unsigned int error     = RFCONTROLLER_REG__ERROR;
    
    if (interrupt & 0x00000001) printf("TX LOAD DONE\r\n");
    if (interrupt & 0x00000002) printf("TX SFD DONE\r\n");
    if (interrupt & 0x00000004) printf("TX SEND DONE\r\n");
    if (interrupt & 0x00000008) {
        // RX DONE
        
        captureTime             = RFTIMER_REG__CAPTURE0;
        newSlotDuration         = TSSLOTDURATION + captureTime - TSTXOFFSET;
        RFTIMER_REG__MAX_COUNT  = newSlotDuration;
        printf("RX SFD DONE\r\n");
    }
    if (interrupt & 0x00000010) printf("RX DONE\r\n");
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
    
    if (interrupt & 0x00000001) {
        // COMPARE0: fires at start of slot
	    
#ifdef TXMOTE
        // TX
        
        RFCONTROLLER_REG__TX_DATA_ADDR  = &send_packet[0];
        sprintf(send_packet , "For synchronization!"); 
        RFCONTROLLER_REG__TX_PACK_LEN   = 22;
#else
        // RX
        
        DMA_REG__RF_RX_ADDR             = &recv_packet[0];
        RFCONTROLLER_REG__CONTROL       = 0x04;
#endif
        // (re)set duration of a slot
        RFTIMER_REG__MAX_COUNT          = TSSLOTDURATION;
        
        // log
        printf("Start of slot\r\n");
        GPIO_REG__OUTPUT                = 0x01; // turn on LED
    }
    if (interrupt & 0x00000008) {
        // COMPARE3
        
        // turn off LED
        GPIO_REG__OUTPUT = 0x00;
        printf("COMPARE3 MATCH\r\n");
    }
    if (interrupt & 0x00000010) printf("COMPARE4 MATCH\r\n");
    if (interrupt & 0x00000020) printf("COMPARE5 MATCH\r\n");
    if (interrupt & 0x00000040) printf("COMPARE6 MATCH\r\n");
    if (interrupt & 0x00000080) printf("COMPARE7 MATCH\r\n");
    if (interrupt & 0x00000100) printf("CAPTURE0 TRIGGERED AT: 0x%x\r\n", RFTIMER_REG__CAPTURE0);
    if (interrupt & 0x00000200) printf("CAPTURE1 TRIGGERED AT: 0x%x\r\n", RFTIMER_REG__CAPTURE1);
    if (interrupt & 0x00000400) printf("CAPTURE2 TRIGGERED AT: 0x%x\r\n", RFTIMER_REG__CAPTURE2);
    if (interrupt & 0x00000800) printf("CAPTURE3 TRIGGERED AT: 0x%x\r\n", RFTIMER_REG__CAPTURE3);
    if (interrupt & 0x00001000) printf("CAPTURE0 OVERFLOW AT: 0x%x\r\n", RFTIMER_REG__CAPTURE0);
    if (interrupt & 0x00002000) printf("CAPTURE1 OVERFLOW AT: 0x%x\r\n", RFTIMER_REG__CAPTURE1);
    if (interrupt & 0x00004000) printf("CAPTURE2 OVERFLOW AT: 0x%x\r\n", RFTIMER_REG__CAPTURE2);
    if (interrupt & 0x00008000) printf("CAPTURE3 OVERFLOW AT: 0x%x\r\n", RFTIMER_REG__CAPTURE3);
    
    RFTIMER_REG__INT_CLEAR = interrupt;
}
