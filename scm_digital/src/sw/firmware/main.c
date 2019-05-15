//------------------------------------------------------------------------------
// u-robot Digital Controller Firmware
//------------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <rt_misc.h>
#include <stdlib.h>
#include <limits.h>
#include "Memory_map.h"

//////////////////////////////////////////////////////////////////
// Main Function
//////////////////////////////////////////////////////////////////

int main(void) {
	unsigned long int i;
	//int bootload_data;	
	//int done;
	
	//printf("\nWelcome to the Bootloader\n\n"); 
  //done = 0;
	printf("Setting Analog CFG 0 to 1\n");
	*(unsigned int*) (APB_GPIO_BASE + 0x0000004) = 0xFF;
	*(unsigned int*) (APB_ANALOG_CFG_BASE + 0x00000000) = 0x1;
	
	while(done == 0)  { //LOOP
		bootload_data = *(unsigned int*)AHB_BOOTLOAD_BASE;
		if ( (bootload_data & 0x10) == 0x10 ) {
			done = 1;
		}
	}
	
	return 0;
}
