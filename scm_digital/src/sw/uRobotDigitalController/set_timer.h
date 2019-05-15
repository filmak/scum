#include "Memory_Map.h"
#include <stdio.h>

//Start-up initialization values
#define TIMER_LOAD_INIT			0x00004C4B	//0x00004C4B   	//(19531 * 256)/5M = 1s
#define	TIMER_CONTROL_INIT		0x0000000F	//prescaler=256, mode periodic, enable=1		


//SET FUNCTIONS
void timer_set_load(unsigned int TIMER_BASE, unsigned int TIMER_LOAD) {
	*(unsigned int*)(TIMER_BASE + 0x0) = TIMER_LOAD;
}	


void timer_set_control(unsigned int TIMER_BASE, unsigned int TIMER_CONTROL) {
	*(unsigned int*)(TIMER_BASE + 0x8) = TIMER_CONTROL;
}


void timer_set_clear(unsigned int TIMER_BASE, unsigned int CLEAR) {
	*(unsigned int*)(TIMER_BASE + 0xC) = CLEAR;		//if CLEAR = 1 interrupt is cleared
}



//GET FUNCTIONS
unsigned int timer_get_load(unsigned int TIMER_BASE) {
	return *(unsigned int*)(TIMER_BASE + 0x0);
}

unsigned int timer_get_value(unsigned int TIMER_BASE) {
	return *(unsigned int*)(TIMER_BASE + 0x4);
}

unsigned int timer_get_control(unsigned int TIMER_BASE) {
	return *(unsigned int*)(TIMER_BASE + 0x8);
}

unsigned int timer_read_clear(unsigned int TIMER_BASE) {
	return *(unsigned int*)(TIMER_BASE + 0xC);
}


void timer_init() {
	timer_set_load(AHB_TIMER_BASE, TIMER_LOAD_INIT);
	timer_set_control(AHB_TIMER_BASE, TIMER_CONTROL_INIT);
	
	//printf("Timer initialized to 1s period\n"); <-- NO!! CAUSE HARD FAULT AND LOCK UP!!
}

void ttimer_init() {
	*(unsigned int*)(APB_TTIMER_BASE + 0x40000) = 0x00FF;		//short timer prescaler
	*(unsigned int*)(APB_TTIMER_BASE + 0x80000) = 0x4C98;		//generate 3 int every s every 10s
	*(unsigned int*)(APB_TTIMER_BASE + 0xC0000) = 0x0003;
	*(unsigned int*)(APB_TTIMER_BASE + 0x100000)= 0xFFFF;	
	*(unsigned int*)(APB_TTIMER_BASE + 0x140000)= 0x02FB;
	*(unsigned int*)(APB_TTIMER_BASE + 0x0000)  = 0x00FF;
	
	*(unsigned int*)(AHB_RF_BASE)  = 101;
	//printf("TTimer initialized \n"); 
}
