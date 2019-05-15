#include "Memory_Map.h"
#include <stdio.h>


#define ADC_CONTROL_INIT		0x3// mode SINGLE, enable


void adc_init() {	
	*(unsigned int*)(APB_ADC_BASE+0x000000) = ADC_CONTROL_INIT;
	*(unsigned int*)(APB_ADC_BASE+0x100000) = 0x1F4;
	printf("ADC initialized\n");
}

