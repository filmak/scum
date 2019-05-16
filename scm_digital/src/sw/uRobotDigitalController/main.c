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

unsigned int ASC[38] = {0};

double temp = 20;

void read() {
	int i;
	unsigned int count_2M, count_LC, count_32k;
	
	RFCONTROLLER_REG__INT_CONFIG = 0x3FF;   // Enable all interrupts and pulses to radio timer
	RFCONTROLLER_REG__ERROR_CONFIG = 0x1F;  // Enable all errors
		
	// Initialize scan chain to default
	initialize_ASC();
	
	// Read counters
	enable_32k();
	set_2M_RC_frequency(31);
	divProgram(20,1,1);
	tx_gpio_ctrl(0,0);
	// LC_monotonic(0,21,131);
	disable_PA_ASC();
	enable_polyphase_ASC();
	count_valid = 0;
	
	analog_scan_chain_write(&ASC[0]);
	analog_scan_chain_load();
	
	RFTIMER_REG__CONTROL = 0x7;
	RFTIMER_REG__MAX_COUNT = 0x0003D090;
	RFTIMER_REG__COMPARE4 = 0x0003D090;
	RFTIMER_REG__COMPARE4_CONTROL = 0x03;

	// Reset all counters
	ANALOG_CFG_REG__0 = 0x0000;

	// Enable all counters
	ANALOG_CFG_REG__0 = 0x3FFF;
}

//////////////////////////////////////////////////////////////////
// Main Function
//////////////////////////////////////////////////////////////////

int main(void) {
	uint32_t iters = 0; // Iterations
	
	double k_temp; // Temperature in Kelvin
	uint16_t temp_payload; // Floating point error
	
	uint32_t *data_address = (uint32_t *) &GPIO_REG__OUTPUT;
	uint32_t data_0 = 0x00000000;
	uint32_t data_1 = 0xFFFFFFFF;
	
	uint8_t packet[16];
	uint8_t *byte_addr = &packet[0];
	uint8_t AdvA[6];
		
	int i, j;
	uint32_t data[129];
			
	// AdvA = 00027232 + temp payload, LSB first
	AdvA[0] = 0x00;
	AdvA[1] = 0x02;
	AdvA[2] = 0x72;
	AdvA[3] = 0x32;
	
	gen_ble_packet(packet, AdvA, 37);
	
	for (i = 0; i < 16; ++i) {
		for (j = 0; j < 8; ++j) {
			data[8 * i + j] = ((packet[i] >> (7 - j)) & 1) ? data_1 : data_0;
		}
	}
		
	while (1) {
		// Only update temperature every few seconds or so
		if (iters % 3000 == 0) {
			read();
			iters = 0;
		}
		
		k_temp = temp + 273.15; // Temperature in Kelvin
		temp_payload = 100 * k_temp + 1; // Floating point error
		
		// Temperature
		AdvA[4] = (temp_payload & 0xFF00) >> 8;
		AdvA[5] = temp_payload & 0xFF;
		
		gen_ble_packet(packet, AdvA, 37);
	
		for (i = 0; i < 16; ++i) {
			for (j = 0; j < 8; ++j) {
				data[8 * i + j] = ((packet[i] >> (7 - j)) & 1) ? data_1 : data_0;
			}
		}
		
		/*
		data[0] = data_0;
		data[1] = data_1;
		data[2] = data_0;
		data[3] = data_1;
		data[4] = data_0;
		data[5] = data_1;
		data[6] = data_0;
		data[7] = data_1;
		data[8] = data_0;
		data[9] = data_1;
		data[10] = data_1;
		data[11] = data_0;
		data[12] = data_1;
		data[13] = data_0;
		data[14] = data_1;
		data[15] = data_1;
		data[16] = data_0;
		data[17] = data_1;
		data[18] = data_1;
		data[19] = data_1;
		data[20] = data_1;
		data[21] = data_1;
		data[22] = data_0;
		data[23] = data_1;
		data[24] = data_1;
		data[25] = data_0;
		data[26] = data_0;
		data[27] = data_1;
		data[28] = data_0;
		data[29] = data_0;
		data[30] = data_0;
		data[31] = data_1;
		data[32] = data_0;
		data[33] = data_1;
		data[34] = data_1;
		data[35] = data_1;
		data[36] = data_0;
		data[37] = data_0;
		data[38] = data_0;
		data[39] = data_1;
		data[40] = data_1;
		data[41] = data_1;
		data[42] = data_1;
		data[43] = data_1;
		data[44] = data_0;
		data[45] = data_0;
		data[46] = data_0;
		data[47] = data_1;
		data[48] = data_0;
		data[49] = data_0;
		data[50] = data_1;
		data[51] = data_0;
		data[52] = data_1;
		data[53] = data_0;
		data[54] = data_1;
		data[55] = data_1;
		data[56] = data_0;
		data[57] = data_0;
		data[58] = data_1;
		data[59] = data_0;
		data[60] = data_0;
		data[61] = data_0;
		data[62] = data_0;
		data[63] = data_0;
		data[64] = data_1;
		data[65] = data_1;
		data[66] = data_0;
		data[67] = data_1;
		data[68] = data_1;
		data[69] = data_0;
		data[70] = data_1;
		data[71] = data_1;
		data[72] = data_1;
		data[73] = data_1;
		data[74] = data_1;
		data[75] = data_1;
		data[76] = data_0;
		data[77] = data_0;
		data[78] = data_0;
		data[79] = data_0;
		data[80] = data_1;
		data[81] = data_0;
		data[82] = data_1;
		data[83] = data_0;
		data[84] = data_1;
		data[85] = data_0;
		data[86] = data_1;
		data[87] = data_1;
		data[88] = data_0;
		data[89] = data_0;
		data[90] = data_1;
		data[91] = data_0;
		data[92] = data_0;
		data[93] = data_1;
		data[94] = data_1;
		data[95] = data_0;
		data[96] = data_0;
		data[97] = data_0;
		data[98] = data_0;
		data[99] = data_0;
		data[100] = data_1;
		data[101] = data_1;
		data[102] = data_0;
		data[103] = data_1;
		data[104] = data_0;
		data[105] = data_1;
		data[106] = data_0;
		data[107] = data_1;
		data[108] = data_0;
		data[109] = data_0;
		data[110] = data_0;
		data[111] = data_1;
		data[112] = data_1;
		data[113] = data_1;
		data[114] = data_0;
		data[115] = data_0;
		data[116] = data_1;
		data[117] = data_0;
		data[118] = data_1;
		data[119] = data_0;
		data[120] = data_1;
		data[121] = data_0;
		data[122] = data_0;
		data[123] = data_1;
		data[124] = data_1;
		data[125] = data_1;
		data[126] = data_0;
		data[127] = data_1;
		data[128] = data_0;
		*/
		
		*data_address = data[0];
		*data_address = data[1];
		*data_address = data[2];
		*data_address = data[3];
		*data_address = data[4];
		*data_address = data[5];
		*data_address = data[6];
		*data_address = data[7];
		*data_address = data[8];
		*data_address = data[9];
		*data_address = data[10];
		*data_address = data[11];
		*data_address = data[12];
		*data_address = data[13];
		*data_address = data[14];
		*data_address = data[15];
		*data_address = data[16];
		*data_address = data[17];
		*data_address = data[18];
		*data_address = data[19];
		*data_address = data[20];
		*data_address = data[21];
		*data_address = data[22];
		*data_address = data[23];
		*data_address = data[24];
		*data_address = data[25];
		*data_address = data[26];
		*data_address = data[27];
		*data_address = data[28];
		*data_address = data[29];
		*data_address = data[30];
		*data_address = data[31];
		*data_address = data[32];
		*data_address = data[33];
		*data_address = data[34];
		*data_address = data[35];
		*data_address = data[36];
		*data_address = data[37];
		*data_address = data[38];
		*data_address = data[39];
		*data_address = data[40];
		*data_address = data[41];
		*data_address = data[42];
		*data_address = data[43];
		*data_address = data[44];
		*data_address = data[45];
		*data_address = data[46];
		*data_address = data[47];
		*data_address = data[48];
		*data_address = data[49];
		*data_address = data[50];
		*data_address = data[51];
		*data_address = data[52];
		*data_address = data[53];
		*data_address = data[54];
		*data_address = data[55];
		*data_address = data[56];
		*data_address = data[57];
		*data_address = data[58];
		*data_address = data[59];
		*data_address = data[60];
		*data_address = data[61];
		*data_address = data[62];
		*data_address = data[63];
		*data_address = data[64];
		*data_address = data[65];
		*data_address = data[66];
		*data_address = data[67];
		*data_address = data[68];
		*data_address = data[69];
		*data_address = data[70];
		*data_address = data[71];
		*data_address = data[72];
		*data_address = data[73];
		*data_address = data[74];
		*data_address = data[75];
		*data_address = data[76];
		*data_address = data[77];
		*data_address = data[78];
		*data_address = data[79];
		*data_address = data[80];
		*data_address = data[81];
		*data_address = data[82];
		*data_address = data[83];
		*data_address = data[84];
		*data_address = data[85];
		*data_address = data[86];
		*data_address = data[87];
		*data_address = data[88];
		*data_address = data[89];
		*data_address = data[90];
		*data_address = data[91];
		*data_address = data[92];
		*data_address = data[93];
		*data_address = data[94];
		*data_address = data[95];
		*data_address = data[96];
		*data_address = data[97];
		*data_address = data[98];
		*data_address = data[99];
		*data_address = data[100];
		*data_address = data[101];
		*data_address = data[102];
		*data_address = data[103];
		*data_address = data[104];
		*data_address = data[105];
		*data_address = data[106];
		*data_address = data[107];
		*data_address = data[108];
		*data_address = data[109];
		*data_address = data[110];
		*data_address = data[111];
		*data_address = data[112];
		*data_address = data[113];
		*data_address = data[114];
		*data_address = data[115];
		*data_address = data[116];
		*data_address = data[117];
		*data_address = data[118];
		*data_address = data[119];
		*data_address = data[120];
		*data_address = data[121];
		*data_address = data[122];
		*data_address = data[123];
		*data_address = data[124];
		*data_address = data[125];
		*data_address = data[126];
		*data_address = data[127];
		*data_address = data[128];
		
		iters++;
		
		/*
		*data_address = data_0;
		for (i = 0; i < 4280; ++i) {
			wait_128();
		}
		*data_address = data_1;
		for (i = 0; i < 4280; ++i) {
			wait_128();
		}
		*/
	}
	
	return 0;
}
