/*
**  standlone project: std_leds Firmware
*/

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"

//============================ definition =====================================

#define RADIO_INTERRUP_RX_SFD_DONE      0x00000008
#define RADIO_INTERRUP_RX_RECEIVE_DONE  0x00000010

// we need space for 127 bytes of PHY payload, 2 bytes of CRC, 1 byte ??? (at least 130 bytes says the user guide)
// note: this might be a BUG in the design, as the CRC is part of the 127B
#define PACKET_LENGTH 130

enum radio_status_t{
    STATUS_IDLE             = 0x00,
    STATUS_TX_LOAD_DONE     = 0x01,
    STATUS_TX_SEND_DONE     = 0x02,
    STATUS_RX_SFD_DONE      = 0x03,
    STATUS_RX_RECEIVE_DONE  = 0x04,
};

//============================== Variables ====================================

enum radio_status_t status;
int pkt_counter;

char recv_packet[PACKET_LENGTH];

//============================ Main Function===================================

int main(void) {
    
    // configure rx interrupt of radio; RX_DONE and RX_SFD_DONE 
    RFCONTROLLER_REG__INT_CONFIG    = 0x018;
    RFCONTROLLER_REG__ERROR_CONFIG  = 0x01F;
    
    DMA_REG__RF_RX_ADDR             = &recv_packet[0];
    RFCONTROLLER_REG__CONTROL       = 0x04; // start receiving
    pkt_counter                     = 0;
    while(1){
        // low power mode?
    }
    
}

void RF_ISR() {
    
    unsigned int interrupt = RFCONTROLLER_REG__INT;
    unsigned int error     = RFCONTROLLER_REG__ERROR;
    
    int i;
    int packet_length;
    
    if (interrupt & RADIO_INTERRUP_RX_SFD_DONE) {
        // I just received the SFD, still receiving bytes
        
        status  = STATUS_RX_SFD_DONE;
        printf("start to receive\r\n");
    }
    if (interrupt & RADIO_INTERRUP_RX_RECEIVE_DONE) {
        // I'm just done receiving the full packet
        
        status  = STATUS_RX_RECEIVE_DONE;
        packet_length = recv_packet[0];
        printf("packet %d",pkt_counter++);
        for (i=0;i<packet_length;i++){
            printf("%c",recv_packet[i+1]);
        }
        printf(" received\r\n");
        DMA_REG__RF_RX_ADDR             = &recv_packet[0];
        RFCONTROLLER_REG__CONTROL       = 0x04; // start receiving
    }
    if (error == 0) {
        if (error & 0x00000004) printf("RX OVERFLOW ERROR\n");
        if (error & 0x00000008) printf("RX CRC ERROR\n");
        if (error & 0x00000010) printf("RX CUTOFF ERROR\n");
        RFCONTROLLER_REG__ERROR_CLEAR = error;
    }
    RFCONTROLLER_REG__INT_CLEAR = interrupt;
}
