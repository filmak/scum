#include <stdio.h>
#include <stdlib.h>
#include "Memory_Map.h"

extern unsigned int ASC[38]; // initialize the scan chain in memory

const unsigned char BPREAMBLE = 0x55;
const unsigned char BACCESS_ADDRESS1 = 0x6B; // need to split BACCESS_ADDRESS into bytes to avoid big-/little-endianness issue
const unsigned char BACCESS_ADDRESS2 = 0x7D;
const unsigned char BACCESS_ADDRESS3 = 0x91;
const unsigned char BACCESS_ADDRESS4 = 0x71;

const unsigned char PDU_HEADER1 = 0x40;
const unsigned char PDU_HEADER2 = 0x60;

const unsigned int PACKET_LENGTH = 16;
const unsigned int PDU_LENGTH = 8; // pdu is 8 bytes long
const unsigned int ADVA_LENGTH = 6; // adv address is 6 bytes long
const unsigned int CRC_LENGTH = 3; // crc is 3 bytes long

// -----------------------------------------------
// scan chain initialization and programming functions
// -----------------------------------------------

unsigned int flip_lsb8(unsigned int in){
    int out = 0;
    
    out |= (0x01 & in) << 7;
    out |= (0x02 & in) << 5;
    out |= (0x04 & in) << 3;
    out |= (0x08 & in) << 1;
    
    out |= (0x10 & in) >> 1;
    out |= (0x20 & in) >> 3;
    out |= (0x40 & in) >> 5;
    out |= (0x80 & in) >> 7;    
    
    return out;
}

void analog_scan_chain_write(unsigned int* scan_bits) {
	int i = 0;
	int j = 0;
	unsigned int asc_reg;
	
	for (i=37; i>=0; i--) {
		for (j=0; j<32; j++) {
			// Set scan_in (should be inverted)
			if((scan_bits[i] & (0x00000001 << j)) == 0)
				asc_reg = 0x21;
			else
				asc_reg = 0x20;
			
			// Write asc_reg to analog_cfg
			ANALOG_CFG_REG__22 = asc_reg;
			
			// Lower phi1
			asc_reg &= ~(0x2);
			ANALOG_CFG_REG__22 = asc_reg;
			
			// Toggle phi2
			asc_reg |= 0x4;
			ANALOG_CFG_REG__22 = asc_reg;
			asc_reg &= ~(0x4);
			ANALOG_CFG_REG__22 = asc_reg;
			
			// Raise phi1
			asc_reg |= 0x2;
			ANALOG_CFG_REG__22 = asc_reg;
			
		}
	}
}

void analog_scan_chain_load() {
	
	// Assert load signal (and cfg<357>)
	ANALOG_CFG_REG__22 = 0x0028;
	
	// Lower load signal
	ANALOG_CFG_REG__22 = 0x0020;
	
}

void initialize_ASC() {
	ASC[0] = 0xFF8002FF;   //0-31
	ASC[1] = 0xFFE03020;   //32-63
	ASC[2] = 0x20202000;   //64-95
	ASC[3] = 0x00160006;   //96-127
	ASC[4] = 0x48000000;   //128-159
	ASC[5] = 0x00000000;   //160-191
	ASC[6] = 0x00003000;   //192-223
	ASC[7] = 0x000342B3;   //224-255
	ASC[8] = 0x3000FFFF;   //256-287
	ASC[9] = 0xE0423100;   //288-319
	ASC[10] = 0x000A0060;   //320-351
	ASC[11] = 0x000C0912;   //352-383
	ASC[12] = 0x4A7F9F06;   //384-415
	ASC[13] = 0x076082C4;   //416-447
	ASC[14] = 0x607D0007;   //448-479
	ASC[15] = 0xFFE00801;   //480-511
	ASC[16] = 0xCFFFFF00;   //512-543
	ASC[17] = 0x22300080;   //544-575
	ASC[18] = 0x010205F9;   //576-607
	ASC[19] = 0x0506E7E8;   //608-639
	ASC[20] = 0x10200040;   //640-671
	ASC[21] = 0x03111810;   //672-703
	ASC[22] = 0x2FFFFC00;   //704-735
	ASC[23] = 0x20000000;   //736-767
	ASC[24] = 0x00007CFC;   //768-799
	ASC[25] = 0x20000018;   //800-831
	ASC[26] = 0x02800000;   //832-863
	ASC[27] = 0x00000000;   //864-895
	ASC[28] = 0x00000808;   //896-927
	ASC[29] = 0x00780072;   //928-959
	ASC[30] = 0xA0000807;   //960-991
	ASC[31] = 0xF7000805;   //992-1023
	ASC[32] = 0x70603C7F;   //1024-1055
	ASC[33] = 0x09E4B084;   //1056-1087
	ASC[34] = 0x7FE3798A;   //1088-1119
	ASC[35] = 0xB54AB540;   //1120-1151
	ASC[36] = 0x00000000;   //1152-1183
	ASC[37] = 0x00000000;
}


// -----------------------------------------------
// scan chain setting modifications
// -----------------------------------------------

unsigned char flipChar(unsigned char b) {
	b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
	b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
	b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
	return b;
}

void GPO_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4) {
	
	//         													CAUTION!!!         												//
	// 										THIS FUNCTION IS NOT FULLY VERIFIED.										//
	// 		USE AT YOUR OWN RISK (and feel free to fix it if it isn't working)			//
	
	// Initialize variables
	unsigned int r1;
	unsigned int r2;
	unsigned int r3_a;
	unsigned int r3_b;
	unsigned int r4;
	
	// Mask user inputs to make sure they didn't mess anything up and move them for flipping
	row1 &= (0x0F)<<4;
	row2 &= (0x0F)<<4;
	row3 &= (0x0F)<<4;
	row4 &= (0x0F)<<4;
	
	// Flip, recast as ints, and mask to ensure that no goop got in
	r1 = ((((unsigned int)flipChar(row1)))&(0x0000000F))<<7;
	r2 = ((((unsigned int)flipChar(row2)))&(0x0000000F))<<3;
	r3_a = ((((unsigned int)flipChar(row3)))&(0x0000000E));
	r3_b = ((((unsigned int)flipChar(row3)))&(0x00000001))<<31;
	r4 = ((((unsigned int)flipChar(row4)))&(0x0000000F))<<27;
		
	// Mask the relevant ASC sections
	ASC[7] &= 0xFFFFF800;
	ASC[8] &= 0x07FFFFFF;
	
	// set the bits in the ASC. Hopefully this works?
	ASC[7] |= r1;
	ASC[7] |= r2;
	ASC[7] |= r3_a;
	ASC[8] |= r3_b;
	ASC[8] |= r4;
	
	//         													CAUTION!!!         												//
	// 										THIS FUNCTION IS NOT FULLY VERIFIED.										//
	// 		USE AT YOUR OWN RISK (and feel free to fix it if it isn't working)			//
		
}

void GPI_control(char row1, char row2, char row3, char row4) {
	
	//         													CAUTION!!!         												//
	// 										THIS FUNCTION IS NOT FULLY VERIFIED.										//
	// 		USE AT YOUR OWN RISK (and feel free to fix it if it isn't working)			//	
	
	// Initialize Variables
	unsigned int r1;
	unsigned int r2;
	unsigned int r3;
	unsigned int r4;

	// Mask user inputs to make sure they didn't mess anything up
	row1 &= (0x03)<<4;
	row2 &= (0x03)<<4;
	row3 &= (0x03)<<4;
	row4 &= (0x03)<<4;
	
	// Flip, recast as ints, and mask to ensure that no goop got in
	r1 = (((unsigned int)flipChar(row1))&(0x0000000F))<<25;
	r2 = (((unsigned int)flipChar(row2))&(0x0000000F))<<23;
	r3 = (((unsigned int)flipChar(row3))&(0x0000000F))<<21;
	r4 = (((unsigned int)flipChar(row4))&(0x0000000F))<<19;
	
	// Mask the relevant ASC sections
	ASC[8] &= 0xF807FFFF;
	
	// set the bits in the ASC. Hopefully this works?
	ASC[8] |= r1;
	ASC[8] |= r2;
	ASC[8] |= r3;
	ASC[8] |= r4;
	
}
void disable_PA_ASC() {
	ASC[15] &= 0xFFFFFEFF;
	ASC[16] &= 0x7FFFFFFF;
}

void enable_PA_ASC() {
	ASC[15] |= 0x00000100;
	ASC[16] |= 0x80000000;
}

void disable_polyphase_ASC() {
	ASC[30] &= 0xFFEFFFFF;
}

void enable_polyphase_ASC() {
	ASC[30] |= 0x00100000;
}

void disable_div_power_ASC() {
	ASC[16] &= 0xB7FFFFFF;
}
void enable_div_power_ASC() {
	ASC[16] |= 0x48000000;
}

void ext_clk_ble_ASC() {
	ASC[32] |= 0x00080000;
}
void int_clk_ble_ASC() {
	ASC[32] &= 0xFFF7FFFF;
}


void enable_1mhz_ble_ASC() {
	ASC[32] &= 0xFFF9FFFF;
}

void disable_1mhz_ble_ASC() {
	ASC[32] |= 0x00060000;
}

void set_LC_current(unsigned int current) {
	unsigned int current_msb = (current & 0x000000F0) >> 4;
	unsigned int current_lsb = (current & 0x0000000F) << 28;
	
	ASC[30] &= 0xFFFFFFF0;
	ASC[30] |= current_msb;
	
	ASC[31] &= 0x0FFFFFFF;
	ASC[31] |= current_lsb;
}
void set_PA_supply(unsigned int code) {
	// 7-bit setting (between 0 and 127)
	// MSB is a "panic" bit that engages the high-voltage settings
	unsigned int code_ASC = ((~code)&0x0000007F) << 13;
	ASC[30] &= 0xFFF01FFF;
	ASC[30] |= code_ASC;
	
}
void set_LO_supply(unsigned int code, unsigned char panic) {
	// 7-bit setting (between 0 and 127)
	// MSB is a "panic" bit that engages the high-voltage settings
	unsigned int code_ASC = ((~code)&0x0000007F) << 5;
	ASC[30] &= 0xFFFFF017;
	ASC[30] |= code_ASC;
}
void set_DIV_supply(unsigned int code, unsigned char panic) {
	// 7-bit setting (between 0 and 127)
	// MSB is a "panic" bit that engages the high-voltage settings
	unsigned int code_ASC = ((~code)&0x0000007F) << 5;
	ASC[30] &= 0xFFF01FFF;
	ASC[30] |= code_ASC;
}

void prescaler(int code) {
	// code is a number between 0 and 5
	// 0 -> disable pre-scaler entirely
	// 1 -> enable div-by-5 back-up pre-scaler
	// 2 -> enable div-by-2 back-up pre-scaler
	// 3 -> enable dynamic pre-scaler version 1 (div-by-5, strong)
	// 4 -> enable dynamic pre-scaler version 2 (div-by-2, strong)
	// 5 -> enable dynamic pre-scaler version 3 (div-by-5, weak)
	
	if (code == 0) {
		// disable div-by-5 backup, disable div-by-2 backup, disable dynamic pre-scaler
		ASC[31] |= 0x00000004;
		ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
		ASC[32] |= 0x80000000;
		ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
		ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
	}
	else if (code == 1) {
		// enable div-by-5 backup, disable div-by-2 backup, disable dynamic pre-scaler
		ASC[31] |= 0x00000002;
		ASC[31] &= 0xFFFFFFFB; // enable div-by-5 backup
		ASC[32] |= 0x80000000;
		ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
		ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
	}
	else if (code == 2) {
		// disable div-by-5 backup, enable div-by-2 backup, disable dynamic pre-scaler
		ASC[31] |= 0x00000004;
		ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
		ASC[32] &= 0x7FFFFFFF;
		ASC[31] |= 0x00000001; // enable div-by-2 backup
		ASC[32] |= 0x70000000; // disable all of the dynamic pre-scalers
	}
	else if (code == 3) {
		// disable div-by-5 backup, disable div-by-2 backup, enable setting #1 of dynamic pre-scaler
		ASC[31] |= 0x00000004;
		ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
		ASC[32] |= 0x80000000;
		ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
		ASC[32] &= 0xBFFFFFFF; // enable first bit of pre-scaler
	}
	else if (code == 4) {
		// disable div-by-5 backup, disable div-by-2 backup, enable setting #2 of dynamic pre-scaler
		ASC[31] |= 0x00000004;
		ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
		ASC[32] |= 0x80000000;
		ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
		ASC[32] &= 0xDFFFFFFF; // enable second bit of pre-scaler
	}
	else if (code == 5) {
		// disable div-by-5 backup, disable div-by-2 backup, enable setting #3 of dynamic pre-scaler
		ASC[31] |= 0x00000004;
		ASC[31] &= 0xFFFFFFFD; // disable div-by-5 backup
		ASC[32] |= 0x80000000;
		ASC[31] &= 0xFFFFFFFE; // disable div-by-2 backup
		ASC[32] &= 0x9FFFFFFF; // enable third bit of pre-scaler
	}
}

void set_2M_RC_frequency(int freq) {
    
    unsigned int coarse1,coarse2,coarse3,fine,superfine;
    unsigned int newval;
    
    
    //ASC(1089:1093) = [1 1 1 1 1]; % coarse 1
    //ASC(1094:1098) = [1 1 1 1 1]; % coarse 2
    //ASC(1099:1103) = [1 1 1 1 1]; % coarse 3
    //ASC(1104:1108) = [1 1 1 1 1]; % fine
    //ASC(1109:1113) = [1 1 1 1 0]; % superfine
    //ASC(1114) = 1; % 2MHz RC enable
    
    // ASC[34] covers 1088:1119
    newval = ASC[34] & 0x8000001F;

    coarse1 = 31;
    coarse2 = 31;
    coarse3 = 31;
    fine = 31;
    superfine = 0x0000001F & freq;
    
    
    newval |= ((flip_lsb8(coarse1) >> 3) & 0x1F) << 26;
    newval |= ((flip_lsb8(coarse2) >> 3) & 0x1F) << 21;
    newval |= ((flip_lsb8(coarse3) >> 3) & 0x1F) << 16;
    newval |= ((flip_lsb8(fine) >> 3) & 0x1F) << 11;
    newval |= ((flip_lsb8(superfine) >> 3) & 0x1F) << 6;
    
    // Enable bit
    newval |= 0x1 << 5;
    
    ASC[34] = newval;
    
    //printf("newval=%X\n\n",newval);
    
}



void enable_32k() {
	ASC[19] |= 0x00010000;
}
void disable_32k() {
	ASC[19] &= 0xFFFEFFFF;
}

void tx_gpio_ctrl(char LO, char PA) {
	// if LO is high, oscillator power control is from GPIO, if LO is low, local oscillator power control is from scan chain
	// if PA is high, power amp power control is from GPIO, if PA is low, power amplifier power control is from scan chain
	char LO_ctrl = 0x01 & LO;
	char PA_ctrl = 0x01 & PA; // mask in case someone messes this whole thing up
	
	if (LO_ctrl == 1) {
		ASC[15] |= 0x00000021;
		ASC[15] &= 0xFFFFFDFF;
	}
	if (LO_ctrl == 0) {
		ASC[15] |= 0x00000201;
		ASC[15] &= 0xFFFFFFDF;
	}
	
	if (PA_ctrl == 1) {
		ASC[16] |= 0x80000000;
		ASC[15] |= 0x00000008;
		ASC[15] &= 0xFFFFFEFF;
	}
	if (PA_ctrl == 0) {
		ASC[16] |= 0x80000000;
		ASC[15] |= 0x00000100;
		ASC[15] &= 0xFFFFFFF7;
	}
	
}




// -----------------------------------------------
// assorted functions
// -----------------------------------------------

void LC_FREQCHANGE(int coarse, int mid, int fine){
	
	//	Inputs:
	//		coarse: 5-bit code (0-31) to control the ~15 MHz step frequency DAC
	//		mid: 5-bit code (0-31) to control the ~800 kHz step frequency DAC
	//		fine: 5-bit code (0-31) to control the ~100 kHz step frequency DAC
	//  Outputs:
	//		none, it programs the LC radio frequency immediately
    
  // mask to ensure that the coarse, mid, and fine are actually 5-bit
  char coarse_m = (char)(coarse & 0x1F);
  char mid_m = (char)(mid & 0x1F);
  char fine_m = (char)(fine & 0x1F);
	
	// flip the bit order to make it fit more easily into the ACFG registers
	unsigned int coarse_f = (unsigned int)(flipChar(coarse_m));
	unsigned int mid_f = (unsigned int)(flipChar(mid_m));
	unsigned int fine_f = (unsigned int)(flipChar(fine_m));

  // initialize registers
  unsigned int fcode = 0x00000000;   // contains everything but LSB of the fine DAC
  unsigned int fcode2 = 0x00000000;  // contains the LSB of the fine DAC
	
	fine_f &= 0x000000FF;
	mid_f &= 0x000000FF;
	coarse_f &= 0x000000FF;
	
	//printf("%d\n",fine_m);
	//printf("%d\n",mid_m);
	//printf("%d\n",coarse_m);
	    
  fcode |= (unsigned int)((fine_f & 0x78) << 9);
	fcode |= (unsigned int)(mid_f << 3);
  fcode |= (unsigned int)(coarse_f >> 3);
    
  fcode2 |= (unsigned int)((fine_f&0x80) >> 7);
	
	//printf("%X\n",fcode);
	//printf("%X\n",fcode2);
		
	// ACFG_LO_ADDR   = [ f1 | f2 | f3 | f4 | md | m0 | m1 | m2 | m3 | m4 | cd | c0 | c1 | c2 | c3 | c4 ]
	// ACFG_LO_ADDR_2 = [ xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | fd | f0 ]
	    
  // set the memory and prevent any overwriting of other analog config
  ACFG_LO__ADDR = fcode;
  ACFG_LO__ADDR_2 = fcode2;
		
}
void LC_monotonic(int LC_code, int mid_divs, int coarse_divs){
	
	int LC_code_temp;
	
	int mid1, mid2, mid3;
	int fine1, fine2, fine3;
	int coarse1, coarse2, coarse3;
	
	mid1=0; mid2=0; mid3=0; fine1=0; fine2=0; fine3=0; coarse1=0; coarse2=0; coarse3=0;
	
	//int coarse_divs = 131;
	//int mid_divs = 21;
	
	coarse1 = (((LC_code/coarse_divs + 19) & 0x000000FF));
	LC_code_temp = LC_code % coarse_divs;
	mid1 = ((((LC_code_temp/mid_divs)*3) & 0x000000FF));
	fine1 = (((LC_code_temp % mid_divs) & 0x000000FF));
	
	if (LC_code >= coarse_divs*4) {
		fine1 = 0; mid1 = 0; coarse1 = 19+4;
		LC_code = LC_code - coarse_divs*4;
		mid_divs++;
		coarse_divs += 6;
		coarse2 = (((LC_code/coarse_divs) & 0x000000FF));
		LC_code_temp = LC_code % coarse_divs;
		mid2 = ((((LC_code_temp/mid_divs)*3) & 0x000000FF));
		fine2 = (((LC_code_temp % mid_divs) & 0x000000FF));
		
		if (LC_code >= coarse_divs*4) {
			fine2 = 0; mid2 = 0; coarse2 = 4;
			LC_code = LC_code - coarse_divs*4;
			mid_divs++;
			coarse_divs+=6;
			coarse3 = (((LC_code/coarse_divs) & 0x000000FF));
			LC_code_temp = LC_code % coarse_divs;
			mid3 = ((((LC_code_temp/mid_divs)*3) & 0x000000FF));
			fine3 = (((LC_code_temp % mid_divs) & 0x000000FF));
		}
	}
	

	if (fine1 + fine2 + fine3 > 15){fine1++;};
	
	LC_FREQCHANGE(coarse1+coarse2+coarse3, mid1+mid2+mid3, fine1+fine2+fine3);
	
}
void divProgram(unsigned int div_ratio, unsigned int reset, unsigned int enable) {
	// Inputs: 
	//	div_ratio, a number between 1 and 65536 that determines the integer divide ratio of the static divider (after pre-scaler)
	//	reset, active low
	//	enable, active high
	// Outputs:
	//	none, it programs the divider immediately
	// Example:
	//	divProgram(480,1,1) will further divide the LC tank frequency by 480
	
	// For this function to work, the scan chain must have bitwise ASC[1081]=0, or ASC[33] &= 0xFFFFFFBF;
	// BIG BUG:::: odd divide ratios DO NOT WORK when the input to the divider is a high frequency (~1.2 GHz)
	
	// initialize the programming registers
	unsigned int div_code_1 = 0x00000000;
	unsigned int div_code_2 = 0x00000000;
	
	// 	The two analog config registers look like this:
	
	// ACFG_DIV_ADDR   = [ d11 | d10 | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx | xx  | xx  | xx  | xx  ]
	// ACFG_DIV_ADDR_2 = [ d3  | d2  | d1 | d0 | rb | en | d9 | d8 | d7 | d6 | d5 | d4 | d15 | d14 | d13 | d12 ]

	div_code_1  = ((div_ratio & 0x00000C00) << 4);
	div_code_2 |= ((div_ratio & 0x0000F000) >> 12);
	div_code_2 |= (div_ratio & 0x000003F0);
	div_code_2 |= (enable << 10);
	div_code_2 |= (reset << 11);
	div_code_2 |= ((div_ratio & 0x0000000F) << 12);
	
	// also every bit needs to be inverted, hooray
	ACFG_DIV__ADDR = ~div_code_1;
	ACFG_DIV__ADDR_2 = ~div_code_2;
	
}
void GPIO_test() {
	unsigned int gpio_status = GPIO_REG__INPUT;
	printf("%X\n", GPIO_REG__OUTPUT);
	
	GPIO_REG__OUTPUT = (gpio_status & 0x00000000);
	printf("%X\n", GPIO_REG__OUTPUT);
	
	GPIO_REG__OUTPUT = (gpio_status | 0xFFFFFFFF);
	printf("%X\n", GPIO_REG__OUTPUT);
}

void clk_128() {
	
	int i;
	for (i = 0; i<128; i++) {
		GPIO_REG__OUTPUT &= 0xFFFFFFFE;
		GPIO_REG__OUTPUT |= 0x00000001;
	}
}
void wait_128() {
	// approximately 128 cycle delay
	int i; int j;
	for (i=0;i<128;i++) {
		j++;
	}
}

void wait_16() {
	// approximately 16 cycle delay
	int i; int j;
	for (i=0;i<16;i++) {
		j++;
	}
}

void wait_alot() {
	int i;
	for (i=0;i<800;i++) {
		wait_128();
	}
}




void read_LC(unsigned int* count_LC){

	unsigned int rdata_lsb, rdata_msb;
	
	// Disable all counters
	ANALOG_CFG_REG__0 = 0x007F;
				
	// Read LC_div counter (via counter4)
	rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
	rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
	*count_LC = rdata_lsb + (rdata_msb << 16);
	
	// Reset all counters
	ANALOG_CFG_REG__0 = 0x0000;
	
	// Enable all counters
	ANALOG_CFG_REG__0 = 0x3FFF;

}

void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k){

	unsigned int rdata_lsb, rdata_msb;//, count_LC, count_32k;
	
	// Disable all counters
	ANALOG_CFG_REG__0 = 0x007F;
		
	// Read 2M counter
	rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x180000);
	rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x1C0000);
	*count_2M = rdata_lsb + (rdata_msb << 16);
		
	// Read LC_div counter (via counter4)
	rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x200000);
	rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x240000);
	*count_LC = rdata_lsb + (rdata_msb << 16);
		
	// Read 32k counter
	rdata_lsb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x000000);
	rdata_msb = *(unsigned int*)(APB_ANALOG_CFG_BASE + 0x040000);
	*count_32k = rdata_lsb + (rdata_msb << 16);
	
	// Reset all counters
	ANALOG_CFG_REG__0 = 0x0000;
	
	// Enable all counters
	ANALOG_CFG_REG__0 = 0x3FFF;
	
	//printf("LC_count=%X\n",count_LC);
	//printf("2M_count=%X\n",count_2M);
	//printf("32k_count=%X\n\n",count_32k);
	
	RFTIMER_REG__COMPARE0_CONTROL = 0x00;
	RFTIMER_REG__COMPARE1_CONTROL = 0x00;

}



// -----------------------------------------------
// some BLE packet assembly and debugging functions
// -----------------------------------------------

void resetBLE() {
	// function that hard resets the GFSK module and BLE asynchronous FIFO
	
	int i;
	
	// read initial GPIO status
	unsigned int gpio_status = GPIO_REG__INPUT;
	
	// enable divider to generate 1mhz pulses
	divProgram(480,1,1);
	
	// reset low, purge the buffer's contents
	ACFG_LO__ADDR_2 = 0x0000;
	
	// strobe GPIO 0 like, 7 times
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	
	// release the reset
	ACFG_LO__ADDR_2 = 0xFFFF;
	
	// strobe GPIO 0 like, 17 times
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	GPIO_REG__OUTPUT = 0x00000000;
	GPIO_REG__OUTPUT = 0x00000001;
	
	//while(1) {
	//	GPIO_REG__OUTPUT = 0x00000000;
	//	GPIO_REG__OUTPUT = 0x00000001;
	//}
	
}
void gen_ble_packet(unsigned char *packet, unsigned char *AdvA, unsigned char channel) {
	
	int byte1;
	int i;
	int bit2;
	unsigned char common;
	unsigned char crc3 = 0xAA;
	unsigned char crc2 = 0xAA;
	unsigned char crc1 = 0xAA;
	
	unsigned char pdu_crc[PDU_LENGTH + CRC_LENGTH];
	unsigned char *pdu_pointer = pdu_crc;
	unsigned char *crc_pointer = pdu_crc;
	unsigned char *byte_addr = pdu_crc;
	
	unsigned char lfsr = channel | 0x40; // [1 37]
		
	*packet = BPREAMBLE;
	packet++;
	
	*packet = BACCESS_ADDRESS1;
	packet++;
	*packet = BACCESS_ADDRESS2;
	packet++;
	*packet = BACCESS_ADDRESS3;
	packet++;
	*packet = BACCESS_ADDRESS4;
	packet++;
	
	*pdu_pointer = PDU_HEADER1;
	pdu_pointer++;
	*pdu_pointer = PDU_HEADER2;
	pdu_pointer++;
	
	for (byte1 = ADVA_LENGTH - 1; byte1 >= 0; byte1--) {
		*pdu_pointer = flipChar(AdvA[byte1]);
		pdu_pointer++;
	}
		
	crc_pointer = pdu_pointer;
	byte_addr = pdu_crc;
	
	for (i=0; i < PDU_LENGTH; byte_addr++, i++) {
		for (bit2 = 7; bit2 >= 0; bit2--) {
			common = (crc1 & 1) ^ ((*byte_addr & (1 << bit2)) >> bit2);
			crc1 = (crc1 >> 1) | ((crc2 & 1) << 7);
			crc2 = ((crc2 >> 1) | ((crc3 & 1) << 7)) ^ (common << 6) ^ (common << 5);
			crc3 = ((crc3 >> 1) | (common << 7)) ^ (common << 6) ^ (common << 4) ^ (common << 3) ^ (common << 1);
		}
	}
	
	crc1 = flipChar(crc1);
	crc2 = flipChar(crc2);
	crc3 = flipChar(crc3);
	
	*(crc_pointer) = crc1;
	*(crc_pointer + 1) = crc2;
	*(crc_pointer + 2) = crc3;
	
	byte_addr = pdu_crc;
	
	for (i = 0; i < PDU_LENGTH + CRC_LENGTH; byte_addr++, i++) {
		for (bit2 = 7; bit2 >= 0; bit2--) {
			*byte_addr = (*byte_addr & ~(1 << bit2)) | ((*byte_addr & (1 << bit2)) ^ ((lfsr & 1) << bit2));
			lfsr = ((lfsr >> 1) | ((lfsr & 1) << 6)) ^ ((lfsr & 1) << 2);
		}
	}

	byte_addr = pdu_crc;
	for (i = 0; i < PDU_LENGTH + CRC_LENGTH; byte_addr++, i++) {
		*packet = *byte_addr;
		packet++;
	}
}

void gen_test_ble_packet(unsigned char *packet) {
	*packet = 0x55;
	packet++;
	*packet = 0x6B;
	packet++;
	*packet = 0x7D;
	packet++;
	*packet = 0x91;
	packet++;
	*packet = 0x71;
	packet++;
	*packet = 0xFE;
	packet++;
	*packet = 0x2B;
	packet++;
	*packet = 0x89;
	packet++;
	*packet = 0x84;
	packet++;
	*packet = 0xF0;
	packet++;
	*packet = 0xAB;
	packet++;
	*packet = 0x26;
	packet++;
	*packet = 0x0D;
	packet++;
	*packet = 0x25;
	packet++;
	*packet = 0x1C;
	packet++;
	*packet = 0xD2;
}

void transmit_ble_packet(unsigned char *packet) {
	
	// initialize looping variables
	unsigned int gpio_status = GPIO_REG__INPUT;		// sample current GPIO status register
	unsigned int data_ready;      // used to check if the BLE module is ready for more data
	unsigned char current_byte;   // temporary variable used to get through each byte at a time
	unsigned int ble_status_reg = 0x0000FFFF;
	int i;												// used to loop through the bytes
	int j;												// used to loop through the 8 bits of each individual byte
	
	
	// THIS SECTION COMMENTED FOR DEBUG PURPOSES
	/*// Program the scan chain (1MHz on, 5MHz connected to "external")
	analog_scan_chain_write(&ASC[0]);
	analog_scan_chain_load();
	
	// reset the BLE module to flush the buffer
	resetBLE();
	
	// Program the scan chain (1MHz off, 5MHz connected to "external")
	ASC[32] = 0xF06E3C3E;
	analog_scan_chain_write(&ASC[0]);
	analog_scan_chain_load();*/
	// THIS SECTION COMMENTED FOR DEBUG PURPOSES
	
	// drop the first 128 bits (16 bytes) of the packet into the buffer (AND ON GPIO FOR TESTING)
	for (i = 0; i < 16; i++) {
		current_byte = *packet;
		for (j = 0; j < 8;) {
			//data_ready = ASYNC_FIFO__ADDR & 0x00000001;
			ACFG_LO__ADDR_2 = (unsigned int)((((~current_byte<<j)&0x80)>>3))|0xFFFFFFEF;
			//ACFG_LO__ADDR_2 = (unsigned int)((((0x00<<j)&0x80)>>3))|0xFFFFFFEF;
			//GPIO_REG__OUTPUT = (unsigned int)((((0x55<<j)&0x80)>>6))|0xFFFFFFFD;
			GPIO_REG__OUTPUT = (gpio_status & 0xFFFFFFFE);
			//printf("%X\n",((current_byte<<j)&0x80)>>3);
			GPIO_REG__OUTPUT = (gpio_status | 0x00000001);
			
			j++;
		}
		packet++;
	}
	
	// EVENTUALLY: will program the scan chain from here, for now I do it in matlab
	// FOR NOW: disable the data valid pin
	ACFG_LO__ADDR_2 = 0xFFFFFFF7;
	
	// THIS SECTION COMMENTED FOR DEBUG PURPOSES
	/*// And now, reprogram the scan chain (1MHz on, 5MHz connected to Cortex clock)
	ASC[32] = 0xF0603C3E;   //1024-1055
	ASC[33] = 0x58E4B0C4;
	analog_scan_chain_write(&ASC[0]);
	analog_scan_chain_load();*/
	
	//THIS SECTION COMMENTED FOR DEBUG PURPOSES
	
	
	// put the remaining bits in
	//for (i=16; i < PACKET_LENGTH; i++) {
	//	current_byte = *packet;
	//	for (j=0; j<7;) {
	//		data_ready = ASYNC_FIFO__ADDR & 0x00000001;
	//		if(data_ready == 1) {
	//			//ACFG_LO__ADDR_2 = (unsigned int)((((~current_byte<<j)&0x80)>>3))|0xFFFFFFF7;
	//			ACFG_LO__ADDR_2 = (unsigned int)((((0x55<<j)&0x80)>>3))|0xFFFFFFEF;
	//			GPIO_REG__OUTPUT = (unsigned int)((((0x55<<j)&0x80)>>6))|0xFFFFFFFD;
	//			j++;
	//			GPIO_REG__OUTPUT = (gpio_status & 0xFFFFFFFE);
	//			GPIO_REG__OUTPUT = (gpio_status | 0x00000001);
	//			//wait_alot();
	//		}
	//	}
	//	packet++;
	//}
		
}

void transmit_ble_packet_rachel(unsigned char *packet) {
	
	// GPIO0 - input clock (GPIO bit bang)
	// GPIO1 - input data (GPIO bit bang)
	// GPIO3 - data out ready
	// GPIO8 - data in valid
	// GPIO9 - choose_clk (0 for 1MHz/2MHz, 1 for 5 MHz)
	// GPIO10 - div_by_2_en
		
	// GPIO4 - 2 MHz clock out - means GPIO4-GPIO7 are not cortex controllable
	// Also GPIO2 has issues for some reason, unclear why
	
	// initialize looping variables
	unsigned int data_ready;      // used to check if the BLE module is ready for more data
	unsigned char current_byte;   // temporary variable used to get through each byte at a time
	unsigned char current_bit;		// temporary variable to put the current bit into the GPIO
	unsigned int ble_status_reg = 0x0000FFFF;
	int i;												// used to loop through the bytes
	int j;												// used to loop through the 8 bits of each individual byte
	
	// Set data in valid to 1, set data out ready to 0
	GPIO_REG__OUTPUT &= 0xFFFFFFF7; // de-activate data out ready
	GPIO_REG__OUTPUT |= 0x00000100; // activate data in valid
	
	// Set the clock selects
	GPIO_REG__OUTPUT |= 0x00000600; // activate div-by-2 and set clock to "5 MHz" (GPIO bit bang)
	
	// drop the first 128 bits (16 bytes) of the packet into the buffer (AND ON GPIO FOR TESTING)
	for (i = 0; i < 16; i++) {
		current_byte = *packet;
		for (j = 0; j < 8;) {
			current_bit = ((current_byte<<j)&0x80)>>7;
			if (current_bit == 0)
				GPIO_REG__OUTPUT &= 0xFFFFFFFD;
			else if (current_bit == 1)
				GPIO_REG__OUTPUT |= 0x00000002;
			
			// Pulse the clock
			GPIO_REG__OUTPUT &= 0xFFFFFFFE;
			GPIO_REG__OUTPUT |= 0x00000001;
			
			j++;
		}
		packet++;
	}
	
	for (i = 0; i < 32; i++) {
		current_byte = *packet;
		for (j = 0; j < 8;) {
			
			current_bit = ((0x00<<j)&0x80)>>7;
			if (current_bit == 0)
				GPIO_REG__OUTPUT &= 0xFFFFFFFD;
			else if (current_bit == 1)
				GPIO_REG__OUTPUT |= 0x00000002;
			
			// Pulse the clock
			GPIO_REG__OUTPUT &= 0xFFFFFFFE;
			GPIO_REG__OUTPUT |= 0x00000001;
			
			j++;
		}
		packet++;
	}
	
	// Set data in valid to 0, set data out ready to 1
	GPIO_REG__OUTPUT &= 0xFFFFFEFF;
	GPIO_REG__OUTPUT |= 0x00000008;
	
	// Switch clock from the "5MHz" clock to the 1/2 MHz clock
	GPIO_REG__OUTPUT &= 0xFFFFFDFF;
	
	// Wait outside of this function, then loop
		
}

void spec_test() {
	// initialize looping variables
	unsigned int gpio_status = GPIO_REG__INPUT;		// sample current GPIO status register
	unsigned int data_ready;      // used to check if the BLE module is ready for more data
	int j;
	
	
	while(1) {
		for (j = 0; j < 7;) {
			data_ready = ASYNC_FIFO__ADDR & 0x00000001;
			if (data_ready == 1) {
				//ACFG_LO__ADDR_2 = (unsigned int)((((~current_byte<<j)&0x80)>>3))|0xFFFFFFF7;
				ACFG_LO__ADDR_2 = (unsigned int)((((0x55<<j)&0x80)>>3))|0xFFFFFFEF;
				//GPIO_REG__OUTPUT = (unsigned int)((((current_byte<<j)&0x80)>>6))|0xFFFFFFFD;
				j++;
				GPIO_REG__OUTPUT = (gpio_status & 0xFFFFFFFE);
				GPIO_REG__OUTPUT = (gpio_status | 0x00000001);
				//wait_alot();
			}
		}
	}
}

int test_ble_packet() {
	unsigned char packet[PACKET_LENGTH];
	unsigned char *byte_addr = &packet[0];
	unsigned char AdvA[ADVA_LENGTH];
	int i;
	AdvA[0] = 0x00;
	AdvA[1] = 0x02;
	AdvA[2] = 0x72;
	AdvA[3] = 0x32;
	AdvA[4] = 0x80;
	AdvA[5] = 0xC6;
	gen_ble_packet(packet, AdvA, 37);
	//gen_test_ble_packet(packet);
	
	for (i=0; i < PACKET_LENGTH; byte_addr++, i++) {
  	printf("%02X ",*byte_addr);
	}
	printf("\n");

	transmit_ble_packet(packet);
	
	return 0;
	
}

void test_ble_packet_loop() {
	unsigned char packet[PACKET_LENGTH];
	unsigned char *byte_addr = &packet[0];
	unsigned char AdvA[ADVA_LENGTH];
	int i;
	int j;
	AdvA[0] = 0x00;
	AdvA[1] = 0x02;
	AdvA[2] = 0x72;
	AdvA[3] = 0x32;
	AdvA[4] = 0x80;
	AdvA[5] = 0xC6;
	gen_ble_packet(packet, AdvA, 37);
	//gen_test_ble_packet(packet);
	
	for (i=0; i < PACKET_LENGTH; byte_addr++, i++) {
  	printf("%02X ",*byte_addr);
	}
	printf("\n");
	
	// initialize scan chain
	analog_scan_chain_write(&ASC[0]);
	analog_scan_chain_load();
	
	// set correct channel frequency
	LC_monotonic(270,21,131);
	
	for (j=0; j<100000; j++) {
		
		// enable 1mhz, drive 5mhz from external
		enable_1mhz_ble_ASC();
		ext_clk_ble_ASC();
		analog_scan_chain_write(&ASC[0]);
		analog_scan_chain_load();
		
		// reset the module
		resetBLE();
		
		// disable clock out
		disable_1mhz_ble_ASC();
		analog_scan_chain_write(&ASC[0]);
		analog_scan_chain_load();
		
		// load bits into the buffer
		transmit_ble_packet(packet);
		
		// send it out!
		disable_1mhz_ble_ASC();
		int_clk_ble_ASC();
		analog_scan_chain_write(&ASC[0]);
		analog_scan_chain_load();
		
		wait_alot();
		wait_alot();
	}
	
}



void disable_ble_module() {
	ACFG_LO__ADDR_2 = 0x0000;
}


void enable_ble_module() {

	

	ACFG_LO__ADDR_2 = 0xFFFF;
}


void test_rachel_fifo() {
	
	unsigned char packet[PACKET_LENGTH];
	unsigned char *byte_addr = &packet[0];
	unsigned char AdvA[ADVA_LENGTH];
	int i;
	AdvA[0] = 0x00;
	AdvA[1] = 0x02;
	AdvA[2] = 0x72;
	AdvA[3] = 0x32;
	AdvA[4] = 0x80;
	AdvA[5] = 0xC6;
	gen_ble_packet(packet, AdvA, 37);
	
	for (i=0; i < PACKET_LENGTH; byte_addr++, i++) {
  	printf("%02X ",*byte_addr);
	}
	printf("\n");
	
	for (i=0; i<100; i++) {
		transmit_ble_packet_rachel(packet);
		wait_128();
		wait_128();
	}
}


void test_spi() {
	
	// Step 0: Pull CS and CLK high
	GPIO_REG__OUTPUT |= 0x00004000;
	GPIO_REG__OUTPUT &= 0xFFFFDFFF; // set chip select low
	//GPIO_REG__OUTPUT |= 0x00002000; // set chip select high

	
	// Step 1: bit bang in the test address
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one (for read)
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one
	GPIO_REG__OUTPUT |= 0x00004000;

	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT &= 0xFFFF7FFF; // zero
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT &= 0xFFFF7FFF; // zero
	GPIO_REG__OUTPUT |= 0x00004000;
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00008000; // one
	GPIO_REG__OUTPUT |= 0x00004000;
	
	// Step 2: bit bang out the response
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT &= 0xFFFFBFFF;
	GPIO_REG__OUTPUT |= 0x00004000;
	//printf("%X",GPIO_REG__OUTPUT);
	//printf("\n");
	
	GPIO_REG__OUTPUT |= 0x00002000; // set chip select high
	//GPIO_REG__OUTPUT &= 0xFFFFDFFF; // set chip select low

}

void spi_cs_low() {
	GPIO_REG__OUTPUT |= 0x00004000; // clock high
	GPIO_REG__OUTPUT &= 0xFFFFDFFF; // chip select low
}

void spi_cs_high() {
	GPIO_REG__OUTPUT |= 0x00002000;
}

void spi_write(unsigned char writeByte) {
	int j;
	for (j=7;j>=0;j--) {
		if ((writeByte&(0x01<<j)) != 0) {
			GPIO_REG__OUTPUT &= 0xFFFFBFFF; // clock low
			GPIO_REG__OUTPUT |= 0x00008000; // write a 1
			GPIO_REG__OUTPUT |= 0x00004000; // clock high
		}
		else {
			GPIO_REG__OUTPUT &= 0xFFFFBFFF; // clock low
			GPIO_REG__OUTPUT &= 0xFFFF7FFF; // write a 0
			GPIO_REG__OUTPUT |= 0x00004000; // clock high
		}
	}
}
unsigned char spi_read() {
	unsigned char readByte;
	int j;
	
	readByte=0;
	GPIO_REG__OUTPUT &= 0xFFFFBFFF; // clock low
	
	for (j=7;j>=0;j--) {		
		GPIO_REG__OUTPUT |= 0x00004000; // clock high
		readByte |= ((GPIO_REG__INPUT&0x00001000)>>12)<<j;
		GPIO_REG__OUTPUT &= 0xFFFFBFFF; // clock low
	}
	
	return readByte;
}
