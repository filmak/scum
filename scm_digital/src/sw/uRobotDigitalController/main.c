//------------------------------------------------------------------------------
// u-robot Digital Controller Firmware
//------------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include "Memory_map.h"
#include "Int_Handlers.h"
#include "rf_global_vars.h"
#include "bucket_o_functions.h"

extern char send_packet[127];

unsigned int ASC[38] = {0};
unsigned char ble_pkt[28] = {0};

//////////////////////////////////////////////////////////////////
// Main Function
//////////////////////////////////////////////////////////////////

int main(void) {
	
	int t;	
	int k;
	
	// TOGGLE GPO0
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	
	printf("\nWelcome to the uRobot Digital Controller\n\n"); 

	//SYSTEM INITIALIZATION
  RFCONTROLLER_REG__TX_DATA_ADDR = &send_packet[0];
	RFCONTROLLER_REG__INT_CONFIG = 0x3FF;   // Enable all interrupts and pulses to radio timer
	RFCONTROLLER_REG__ERROR_CONFIG = 0x1F;  // Enable all errors
	RFTIMER_REG__MAX_COUNT = 0xFFFFFFFF;
	RFTIMER_REG__CONTROL = 0x7;
	
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	
	//STUFF FOR RUNNING COUNTER
	// Reset and disable counters
	
  ANALOG_CFG_REG__0 = 0x0;
                            
  // Enable all counters
  ANALOG_CFG_REG__0 = 0x3FFF;
	
	// initialize scan chain to default:
	
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	
	initialize_ASC();
	disable_PA_ASC();
	enable_div_power_ASC();
	set_LC_current(127);
	set_LO_supply(0);
	set_DIV_supply(0);
	prescaler(1);
	tx_gpio_ctrl(0,1);
	
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	
	analog_scan_chain_write(&ASC[0]);
	//analog_scan_chain_load();
	
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;
	
	LC_FREQCHANGE(15, 15, 15);
	divProgram(240,1,1);
	
	//RFTIMER_REG__MAX_COUNT = 0x0003D090;
	//RFTIMER_REG__COMPARE0 = 0x1;
  //RFTIMER_REG__COMPARE0_CONTROL   = 0x03;

	printf("Initialization complete\n");

	while(1)  { //LOOP
		
	//for (k=0; k<40; k++) {
	//	send_packet[k] = k+0x30;
	//	
	//}
		
	GPIO_REG__OUTPUT = 0x0000;
	GPIO_REG__OUTPUT = 0xFFFF;	
	
	//RFCONTROLLER_REG__TX_PACK_LEN = 42;
	//RFCONTROLLER_REG__CONTROL = 0x1;
	//RFCONTROLLER_REG__CONTROL = 0x2;
	
	//j = GPIO_REG__INPUT;

	//for(t=0;t<1000000;t++);

	//GPIO_REG__OUTPUT = j;		
		
	}

}


