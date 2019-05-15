/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ definition =====================================

#define PACKET_LENGTH 128

enum radio_status_t{
    STATUS_IDLE           = 0x00,
    STATUS_TX_LOAD_DONE   = 0x01,
    STATUS_TX_SEND_DONE   = 0x02,
};

//============================== Variables ====================================

enum radio_status_t status;

//============================ Main Function===================================

int main(void) {
    
    int i;
    int j;
    
    char send_packet[PACKET_LENGTH];
    char recv_packet[PACKET_LENGTH];
    
    // configure interrupt of radio
    RFCONTROLLER_REG__INT_CONFIG    = 0x01D;
    RFCONTROLLER_REG__ERROR_CONFIG  = 0x01F;
    
    while(1){
        status = STATUS_IDLE;
        
        RFCONTROLLER_REG__TX_DATA_ADDR  = &send_packet[0]; 
        DMA_REG__RF_RX_ADDR             = &recv_packet[0];

        // load packet
        sprintf(send_packet , "Magic packet delivery!"); 
        RFCONTROLLER_REG__TX_PACK_LEN   = 22;
        RFCONTROLLER_REG__CONTROL       = 0x01; // causes the radio to load the packet
        printf("Tx loading packet ");
        for (i=0;i<RFCONTROLLER_REG__TX_PACK_LEN;i++){
            printf("%c",send_packet[i]);
        }
        printf("\r\n");
        // wait until loading packet is done
        while(status != STATUS_TX_LOAD_DONE);
        printf("Tx load done\r\n");
        // send the packet
        RFCONTROLLER_REG__CONTROL       = 0x02; // causes the radio to send the packet
        // wait until senddone
        while(status != STATUS_TX_SEND_DONE);
        printf("Tx send done\r\n");
        // wait
        for (i=0;i<1000;i++){
            for (j=0;j<1000;j++){
            }
        }
    }
    
}

void RF_ISR() {
    
    unsigned int interrupt = RFCONTROLLER_REG__INT;
    unsigned int error     = RFCONTROLLER_REG__ERROR;
    
    if (interrupt & 0x00000001){
        // indicates radio done loading the packet
        
        status = STATUS_TX_LOAD_DONE;
    };

    if (interrupt & 0x00000004){
        // indicates radio done sending the packet
        
        status = STATUS_TX_SEND_DONE;
    };
    
    if (error == 0) {
        if (error & 0x00000001) printf("TX OVERFLOW ERROR\n");
        if (error & 0x00000002) printf("TX CUTOFF ERROR\n");
        RFCONTROLLER_REG__ERROR_CLEAR = error;
    }
    RFCONTROLLER_REG__INT_CLEAR = interrupt;
}
