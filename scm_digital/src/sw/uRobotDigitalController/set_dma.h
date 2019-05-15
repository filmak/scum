#include "Memory_Map.h"
#include <stdio.h>

#define DMA_CONTROL_INIT    0x00000003	

//SET FUNCTIONS
void dma_set_control(unsigned int DMA_CONTROL) {
	*(unsigned int*)(AHB_DMA_BASE + 0x00) = DMA_CONTROL;
}	

void dma_set_rfstartaddr(unsigned int DMA_RFSTART) {
	*(unsigned int*)(AHB_DMA_BASE + 0x04) = DMA_RFSTART;
}	

void dma_set_rfendaddr(unsigned int DMA_RFEND) {
	*(unsigned int*)(AHB_DMA_BASE + 0x08) = DMA_RFEND;
}

void dma_set_adcstartaddr(unsigned int DMA_ADCSTART) {
	*(unsigned int*)(AHB_DMA_BASE + 0x0C) = DMA_ADCSTART;
}

void dma_set_adcendaddr(unsigned int DMA_ADCEND) {
	*(unsigned int*)(AHB_DMA_BASE + 0x10) = DMA_ADCEND;
}


//GET FUNCTIONS

unsigned int dma_get_control() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x00);
}

unsigned int dma_get_rfstartaddr() {
	return *(unsigned int*)(AHB_DMA_BASE+0x04);
}

unsigned int dma_get_rfendaddr() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x08);
}

unsigned int dma_get_adcstartaddr() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x0C);
}

unsigned int dma_get_adcendaddr() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x10);
}

unsigned int dma_get_rfcurrentaddr() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x14);
}

unsigned int dma_get_adccurraddr() {
	return *(unsigned int*)(AHB_DMA_BASE + 0x18);
}



void dma_init() {	
	//write control register
	dma_set_rfstartaddr(0x02000000);
	dma_set_rfendaddr(0x02000200);
	dma_set_adcstartaddr(0x02000400);
	dma_set_adcendaddr(0x02000200);
	*(unsigned int*)(AHB_DMA_BASE + 0x1C) = 0x0400000C;
	*(unsigned int*)(AHB_DMA_BASE + 0x20) = 0x04000004;
	*(unsigned int*)(AHB_DMA_BASE + 0x24) = 0x04000008;
	dma_set_control(DMA_CONTROL_INIT);	
	printf("DMA succesfully initialized\n");
}




