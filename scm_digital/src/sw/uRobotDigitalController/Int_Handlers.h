#include "Memory_Map.h"
#include "rf_global_vars.h"
#include <stdio.h>
#include "bucket_o_functions.h"
#include "fixed-point.h"

extern char send_packet[127];
extern char recv_packet[130];

extern double temp;
double temp_sum = 0;
int temp_counts = 0;
double ratio;
double LC_frequency;
double frequency_per_LSB;
double LSB_per_degree;
int increments;

int count_valid;
int previous_count;
int test_index;

int fine_code;
int mid_code;
int coarse_code;

unsigned char monotonic_loop_index;

//unsigned int count_LC;

extern unsigned int ASC[38];

unsigned int rollover[40];
unsigned int cnt_val[1600];
unsigned int cnt_val_prev[40];
int test_shit[40];

int end_val;
int end_test_process;

void UART_ISR(){	
	static char i=0;
	static char buff[4] = {0x0, 0x0, 0x0, 0x0};
	static char waiting_for_end_of_copy = 0;
	char inChar;
	
	inChar = UART_REG__RX_DATA;
	buff[3] = buff[2];
	buff[2] = buff[1];
	buff[1] = buff[0];
	buff[0] = inChar;
	
	// If we are still waiting for the end of a load command
	if (waiting_for_end_of_copy) {
		if (inChar=='\n'){
				int j=0;
				printf("copying string of size %u to send_packet: ", i);
				for (j=0; j < i; j++) {
					printf("%c",send_packet[j]);
				}
				printf("\n");
				RFCONTROLLER_REG__TX_PACK_LEN = i;
				i = 0;
				waiting_for_end_of_copy = 0;
		} else if (i < 127) {		
			send_packet[i] = inChar;			
			i++;
		}	else {
			printf("Input exceeds maximum packet size\n");
		}
	} else { //If waiting for a command
		// Copies string from UART to send_packet
		if ( (buff[3]=='c') && (buff[2]=='p') && (buff[1]=='y') && (buff[0]==' ') ) {
			waiting_for_end_of_copy = 1;
		// Sends TX_LOAD signal to radio controller
		} else if ( (buff[3]=='l') && (buff[2]=='o') && (buff[1]=='d') && (buff[0]=='\n') ) {
		  RFCONTROLLER_REG__CONTROL = 0x1;
			printf("TX LOAD\n");
		// Sends TX_SEND signal to radio controller
	  } else if ( (buff[3]=='s') && (buff[2]=='n') && (buff[1]=='d') && (buff[0]=='\n') ) {
		  RFCONTROLLER_REG__CONTROL = 0x2;
		  printf("TX SEND\n");
		// Sends RX_START signal to radio controller
		} else if ( (buff[3]=='r') && (buff[2]=='c') && (buff[1]=='v') && (buff[0]=='\n') ) {
			printf("Recieving\n");
	    DMA_REG__RF_RX_ADDR = &recv_packet[0];
		  RFCONTROLLER_REG__CONTROL = 0x4;
		// Sends RX_STOP signal to radio controller
	  } else if ( (buff[3]=='e') && (buff[2]=='n') && (buff[1]=='d') && (buff[0]=='\n') ) {
			RFCONTROLLER_REG__CONTROL = 0x8;
			printf("RX STOP\n");
		// Sends RF_RESET signal to radio controller
		} else if ( (buff[3]=='r') && (buff[2]=='s') && (buff[1]=='t') && (buff[0]=='\n') ) {
		  RFCONTROLLER_REG__CONTROL = 0x10;
			printf("RF RESET\n");	
		// Returns the status register of the radio controller
		} else if ( (buff[3]=='s') && (buff[2]=='t') && (buff[1]=='a') && (buff[0]=='\n') ) {
		  int status = RFCONTROLLER_REG__STATUS;
			printf("status register is 0x%x\n", status);	
		// Initiates an ADC conversion
	  } else if ( (buff[3]=='a') && (buff[2]=='d') && (buff[1]=='c') && (buff[0]=='\n') ) {
		  ADC_REG__START = 0x1;
		  printf("starting ADC conversion\n");
		// Uses the radio timer to send TX_LOAD in 0.5as, TX_SEND in 1s, capture when SFD is sent and capture when packet is sent
		} else if ( (buff[3]=='a') && (buff[2]=='t') && (buff[1]=='x') && (buff[0]=='\n') ) {
			unsigned int t = RFTIMER_REG__COUNTER + 0x3D090;
			RFTIMER_REG__COMPARE0 = t;
		  RFTIMER_REG__COMPARE1 = t + 0x3D090;
		  printf("%x\n", RFTIMER_REG__COMPARE0);
		  printf("%x\n", RFTIMER_REG__COMPARE1);
		  RFTIMER_REG__COMPARE0_CONTROL = 0x5;
		  RFTIMER_REG__COMPARE1_CONTROL = 0x9;
		  RFTIMER_REG__CAPTURE0_CONTROL = 0x9;
		  RFTIMER_REG__CAPTURE1_CONTROL = 0x11;
		  printf("Auto TX\n");
		// Uses the radio timer to send RX_START in 0.5s, capture when SFD is received and capture when packet is received
		} else if ( (buff[3]=='a') && (buff[2]=='r') && (buff[1]=='x') && (buff[0]=='\n') ) {
			RFTIMER_REG__COMPARE0 = RFTIMER_REG__COUNTER + 0x3D090;
		  RFTIMER_REG__COMPARE0_CONTROL = 0x11;
		  RFTIMER_REG__CAPTURE0_CONTROL = 0x21;
		  RFTIMER_REG__CAPTURE1_CONTROL = 0x41;
			DMA_REG__RF_RX_ADDR = &recv_packet[0];
			printf("Auto RX\n");
		// Reset the radio timer compare and capture units
		} else if ( (buff[3]=='r') && (buff[2]=='r') && (buff[1]=='t') && (buff[0]=='\n') ) {
			RFTIMER_REG__COMPARE0_CONTROL = 0x0;
			RFTIMER_REG__COMPARE1_CONTROL = 0x0;
			RFTIMER_REG__CAPTURE0_CONTROL = 0x0;
			RFTIMER_REG__CAPTURE1_CONTROL = 0x0;
		  printf("Radio timer reset\n");
		// Set the LC frequency to a value hard-coded in the void
		} else if ( (buff[3]=='d') && (buff[2]=='a') && (buff[1]=='d') && (buff[0]=='\n') ) {
			LC_monotonic(270,21,131);
		}
		else if ( (buff[3]=='b') && (buff[2]=='l') && (buff[1]=='e') && (buff[0]=='\n') ) {
			printf("BLE test packet:\n");
			LC_monotonic(459,21,121);
			divProgram(1200,1,1);
			set_DIV_supply(127);
			enable_PA_ASC();
			disable_polyphase_ASC();
			prescaler(4);
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			test_rachel_fifo();
		}
		else if ( (buff[3]=='a') && (buff[2]=='s') && (buff[1]=='y') && (buff[0]=='\n') ) {
			printf("BLE register: 0x%X\n",ASYNC_FIFO__ADDR);
		}
		else if ( (buff[3]=='r') && (buff[2]=='b') && (buff[1]=='t') && (buff[0]=='\n') ) {
			resetBLE();
		}
		else if ( (buff[3]=='p') && (buff[2]=='s') && (buff[1]=='c') && (buff[0]=='\n') ) {
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			printf("Scan chain programmed");
		}
		else if ( (buff[3]=='b') && (buff[2]=='t') && (buff[1]=='1') && (buff[0]=='\n') ) {
			// Program the scan chain (1MHz off, 5MHz connected to "external")
			disable_1mhz_ble_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
		}
		else if ( (buff[3]=='b') && (buff[2]=='t') && (buff[1]=='2') && (buff[0]=='\n') ) {
			// And now, reprogram the scan chain (1MHz on, 5MHz connected to Cortex clock)
			enable_1mhz_ble_ASC();
			int_clk_ble_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
		}
		else if ( (buff[3]=='e') && (buff[2]=='d') && (buff[1]=='v') && (buff[0]=='\n') ) {
			// And now, reprogram the scan chain (1MHz on, 5MHz connected to Cortex clock)
			enable_div_power_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
		
			printf("%d\n",((101250 - 101247)*4800*5)/3249);
		}
		else if ( (buff[3]=='d') && (buff[2]=='d') && (buff[1]=='v') && (buff[0]=='\n') ) {
			// And now, reprogram the scan chain (1MHz on, 5MHz connected to Cortex clock)
			disable_div_power_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
		}
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='t') && (buff[0]=='\n') ) {
			enable_32k();
			set_2M_RC_frequency(31);
			divProgram(20,1,1);
			tx_gpio_ctrl(0,0);
			LC_monotonic(0,21,131);
			enable_PA_ASC();
			disable_polyphase_ASC();
			//set_LO_supply(31); // used to make the supply voltages the same on every chip
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			count_valid = 0;
		
			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE0 = 0x0003D090;
			RFTIMER_REG__COMPARE0_CONTROL = 0x03;
		
			// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;
	
			// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
		
			printf("Count executed\n");
		}
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='u') && (buff[0]=='\n') ) {
			enable_32k();
			set_2M_RC_frequency(31);
			divProgram(20,1,1);
			tx_gpio_ctrl(0,0);
			LC_monotonic(0,21,131);
			disable_PA_ASC();
			enable_polyphase_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			count_valid = 0;
		
			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE0 = 0x0003D090;
			RFTIMER_REG__COMPARE0_CONTROL   = 0x03;
		
			// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;
	
			// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
		
			printf("Count executed\n");
		}
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='v') && (buff[0]=='\n') ) {
			enable_32k();
			set_2M_RC_frequency(31);
			divProgram(20,1,1);
			tx_gpio_ctrl(0,0);
			LC_monotonic(0,21,131);
			disable_PA_ASC();
			enable_polyphase_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			count_valid = 0;
		
			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE4 = 0x0003D090;
			RFTIMER_REG__COMPARE4_CONTROL   = 0x03;
		
			// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;
	
			// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
		
			printf("Count executed\n");
		}
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='s') && (buff[0]=='\n') ) {
			enable_32k();
			set_2M_RC_frequency(31);
			divProgram(480,1,1);
			tx_gpio_ctrl(0,0);
			LC_monotonic(440,21,131);
			disable_PA_ASC();
			enable_polyphase_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			count_valid = 0;
		
			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE3 = 0x0003D090;
			RFTIMER_REG__COMPARE3_CONTROL   = 0x03;
		
			// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;
	
			// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
		
			printf("Count executed\n");
		}
		
		else if ( (buff[3]=='p') && (buff[2]=='o') && (buff[1]=='1') && (buff[0]=='\n') ) {
			LC_monotonic(0,21,131);
			tx_gpio_ctrl(1, 1);
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			GPIO_REG__OUTPUT = 0xFFFF;
		}
		else if ( (buff[3]=='p') && (buff[2]=='o') && (buff[1]=='2') && (buff[0]=='\n') ) {
			LC_monotonic(0,21,131);
			tx_gpio_ctrl(1,0);
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			wait_128();
			GPIO_REG__OUTPUT = 0xFFFF;
		}
		else if ( (buff[3]=='p') && (buff[2]=='o') && (buff[1]=='3') && (buff[0]=='\n') ) {
			LC_monotonic(0,21,131);
			tx_gpio_ctrl(0,1);
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			wait_128();
			GPIO_REG__OUTPUT = 0xFFFF;
		}
		
		else if ( (buff[3]=='p') && (buff[2]=='o') && (buff[1]=='f') && (buff[0]=='\n') ) {
			unsigned int x_msb;
			unsigned int x_lsb;
		
			//test_spi();
			spi_cs_high();
			spi_cs_low();
			spi_write(0xF5);
			//printf("WHO AM I: %X\n",spi_read());
		  spi_cs_high();
		
			spi_cs_low();
			spi_write(0xF5);
			//printf("WHO AM I: %X\n",spi_read());
		  spi_cs_high();
		
			spi_cs_low();
			spi_write(0xC1);
			x_msb = (unsigned int)spi_read();
			spi_cs_high();
			wait_128();
			spi_cs_low();
			spi_write(0xC2);
			x_lsb = (unsigned int)spi_read();
			spi_cs_high();
			printf("Temperature reading: %d\n",((x_msb<<8)|(x_lsb)));
		
		}
		
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='w') && (buff[0]=='\n') ) {
			enable_32k();
			set_2M_RC_frequency(31);
			divProgram(480,1,1);
			tx_gpio_ctrl(0,0);
			enable_PA_ASC();
			disable_polyphase_ASC();
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			count_valid = 0;
		  test_index = 0;
			coarse_code = 19;
			mid_code = 1;
			fine_code = 24;
		
			LC_FREQCHANGE(coarse_code,mid_code,fine_code);
		
			//// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;

			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE4 = 0x0003D090;
			RFTIMER_REG__COMPARE4_CONTROL   = 0x03;
			
			//// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
		}
		
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='p') && (buff[0]=='\n') ) {
			int kk;
			for (kk=0; kk<15; kk+=1) {
				printf("%d\n",cnt_val[kk]);
				}
		}
		else if ( (buff[3]=='l') && (buff[2]=='c') && (buff[1]=='q') && (buff[0]=='\n') ) {
			printf("%d\n",cnt_val[0]);
			printf("%d\n",cnt_val[1]);
		}
		// Unknown command
		else if (inChar=='\n'){	
			printf("unknown command\n");
		}
	}
}

void ADC_ISR() {
	printf("Conversion complete: 0x%x\n", ADC_REG__DATA);
}

void RF_ISR() {
	unsigned int interrupt = RFCONTROLLER_REG__INT;
	unsigned int error     = RFCONTROLLER_REG__ERROR;
	
  if (interrupt & 0x00000001) printf("TX LOAD DONE\n");
	if (interrupt & 0x00000002) printf("TX SFD DONE\n");
	if (interrupt & 0x00000004) printf("TX SEND DONE\n");
	if (interrupt & 0x00000008) printf("RX SFD DONE\n");
	if (interrupt & 0x00000010) {
		int i;
		char num_bytes_rec = recv_packet[0];
		char *current_byte_rec = recv_packet+1;
		printf("RX DONE\n");
		printf("Received packet of size %d: ", num_bytes_rec);
		for (i=0; i < num_bytes_rec; i++) {
			printf("%c",current_byte_rec[i]);
		}
		printf("\n");
	}
	
	if (error == 0) {
		if (error & 0x00000001) printf("TX OVERFLOW ERROR\n");
		if (error & 0x00000002) printf("TX CUTOFF ERROR\n");
		if (error & 0x00000004) printf("RX OVERFLOW ERROR\n");
		if (error & 0x00000008) printf("RX CRC ERROR\n");
		if (error & 0x00000010) printf("RX CUTOFF ERROR\n");
		RFCONTROLLER_REG__ERROR_CLEAR = error;
	}
	RFCONTROLLER_REG__INT_CLEAR = interrupt;
}

void RFTIMER_ISR() {
	
	unsigned int count_LC, count_2M, count_32k;
	//unsigned int count_LC;
	unsigned int interrupt = RFTIMER_REG__INT;
	end_val = 15;
	end_test_process = 100;
			
	if (interrupt & 0x00000001) {
		// This code does the simple "monotonic-by-modulo" sweeps
		// It works ok. Not great, just ok.
    
    // Call counter functions
    read_LC(&count_LC);

		// Save counter value
		cnt_val[count_valid] = count_LC;
			
		if(count_valid >= end_val) {
			count_valid = end_val;
			printf("count_LC: %d\n", count_LC);
			printf("DONE\n");
			RFTIMER_REG__COMPARE0_CONTROL = 0x00;
		}
			
		if (count_valid < end_val) {
			//LC_monotonic(count_valid, 23, 144); // works well for ok1 in transmit; pretty well for ok1 in receive
			LC_monotonic(count_valid*100, 21, 130);
			count_valid++; // for all other sweeps
		}
		
	}
		
	if (interrupt & 0x00000002) {
		////// First determine all of the mid-overlapping codes

		// Read counter
		read_LC(&count_LC);
		
		// If I'm not done sweeping
		if (test_index == end_test_process) {
			printf("DONE\n");
			RFTIMER_REG__COMPARE1_CONTROL   = 0x00;
		}
		else if (test_index < end_test_process) {
			// Case 1: increment mid, make the measurement to compare against
			if (count_valid == 0) {
				if (test_index == 0) {
					test_index++;
				}
				else {
					previous_count = count_LC;
					mid_code += 2;
					fine_code = 10;
					count_valid = 1;
					monotonic_loop_index++;
					test_index++;
				}
			}
			// Case 2: start w/ fine = 8, iterate until I get the best value for fine, store it, and move on
			else {
				if (((count_LC-previous_count) > 1) && ((count_LC-previous_count) < 8)){
					//rollover[monotonic_loop_index] = fine_code;
					fine_code = 24;
					count_valid = 0;
				}
				
				else if (count_LC-previous_count >= 8) {
					fine_code--;
				}
				else {
					fine_code++;
				}
				
				test_index++;
			}
			
			// Program the oscillator frequency
			LC_FREQCHANGE(coarse_code, mid_code, fine_code);
						
			// Do the song and dance to come back to this ISR
			
			cnt_val[test_index] = count_LC;
			cnt_val_prev[test_index] = previous_count;
			test_shit[test_index] = 0;
			rollover[test_index] = fine_code;
			
		}
	}
		
	
	if (interrupt & 0x00000004) printf("COMPARE2 MATCH\n");
	if (interrupt & 0x00000008) {
		// This code does the simple "monotonic-by-modulo" sweeps
		// It works ok. Not great, just ok.
    
    // Call counter functions
    read_LC(&count_LC);

		// Save counter value
		cnt_val[count_valid] = count_LC;
		
		//if (count_valid == 100) {count_valid = 1500;}; // for doing low/high fine sweeps
			
		if(count_valid >= end_val) {
			//count_valid = end_val;
			printf("DONE\n");
			RFTIMER_REG__COMPARE0_CONTROL = 0x00;
		}
			
		if (count_valid < end_val) {
			//LC_monotonic(count_valid, 23, 144); // works well for ok1 in transmit; pretty well for ok1 in receive
			LC_monotonic(count_valid, 21, 130);
			count_valid++; // for all other sweeps
		}
	} //printf("COMPARE3 MATCH\n");
	if (interrupt & 0x00000010) {
				
		read_counters(&count_2M, &count_LC, &count_32k);
		cnt_val[0] = count_2M;
		cnt_val[1] = count_32k;
				
		ratio = fix_double(fix_div(fix_init(count_2M), fix_init(count_32k)));
		temp = -31.41 * ratio + 1893.87 - 20;
		
		// printf("count_2M: %d, count_32k: %d, temp: %lf\n", count_2M, count_32k, temp);
		printf("count_2M: %d\n", count_2M);
		printf("count_32k: %d\n", count_32k);
		printf("count_LC: %d\n", count_LC);
		// printf("temp: %.2f\n", temp);
		
		// LC_frequency = -196185.9212 * temp + 2352980673.4913;
		// printf("LC frequency: %lf\n", LC_frequency);
		
		temp_sum += temp;
		temp_counts++;
		
		printf("DONE LF CNT\n");
		RFTIMER_REG__COMPARE4_CONTROL = 0x00;
		
		printf("%d\n", temp_counts);
		
		if (temp_counts < 10) {

			count_valid = 0;
			
			RFTIMER_REG__CONTROL = 0x7;
			RFTIMER_REG__MAX_COUNT = 0x0003D090;
			RFTIMER_REG__COMPARE4 = 0x0003D090;
			RFTIMER_REG__COMPARE4_CONTROL = 0x03;

			// Reset all counters
			ANALOG_CFG_REG__0 = 0x0000;

			// Enable all counters
			ANALOG_CFG_REG__0 = 0x3FFF;
			
		} else if (temp_counts == 10) {
			
			// Reset scan chain for transmitting packet
			// Cortex tunes frequency
			ASC[0] = 0x7F8002FF;   //0-31
			ASC[1] = 0xFFE03020;   //32-63
			ASC[2] = 0x20202000;   //64-95
			ASC[3] = 0x00160006;   //96-127
			ASC[4] = 0x48000000;   //128-159
			ASC[5] = 0x00000000;   //160-191
			ASC[6] = 0x00003000;   //192-223
			ASC[7] = 0x00034333;   //224-255
			ASC[8] = 0x3060FFFF;   //256-287
			ASC[9] = 0xE3FFFFFF;   //288-319
			ASC[10] = 0xFF0A0060;   //320-351
			ASC[11] = 0x013F4081;   //352-383
			ASC[12] = 0x027F9F06;   //384-415
			ASC[13] = 0x07608044;   //416-447
			ASC[14] = 0x600280FF;   //448-479
			ASC[15] = 0xFBF01B01;   //480-511
			ASC[16] = 0xC8000000;   //512-543
			ASC[17] = 0x00000000;   //544-575
			ASC[18] = 0x00000000;   //576-607
			ASC[19] = 0x00000000;   //608-639
			ASC[20] = 0x00000000;   //640-671
			ASC[21] = 0x00000000;   //672-703
			ASC[22] = 0x00000000;   //704-735
			ASC[23] = 0x20000000;   //736-767
			ASC[24] = 0x00007CFC;   //768-799
			ASC[25] = 0x20000018;   //800-831
			ASC[26] = 0x02800000;   //832-863
			ASC[27] = 0x00000000;   //864-895
			ASC[28] = 0x00000800;   //896-927
			ASC[29] = 0x08002092;   //928-959
			ASC[30] = 0xA0000003;   //960-991
			ASC[31] = 0xF7000805;   //992-1023
			ASC[32] = 0x7060A07F;   //1024-1055
			ASC[33] = 0x09E4B0C4;   //1056-1087
			ASC[34] = 0x7FF70000;   //1088-1119
			ASC[35] = 0x00000000;   //1120-1151
			ASC[36] = 0x00000000;   //1152-1183
			ASC[37] = 0x00000000;

			// Scan chain tunes frequency
			/*ASC[0] = 0x7F8002FF;   //0-31
			ASC[1] = 0xFFE03020;   //32-63
			ASC[2] = 0x20202000;   //64-95
			ASC[3] = 0x00160006;   //96-127
			ASC[4] = 0x48000000;   //128-159
			ASC[5] = 0x00000000;   //160-191
			ASC[6] = 0x00003000;   //192-223
			ASC[7] = 0x00034333;   //224-255
			ASC[8] = 0x3060FFFF;   //256-287
			ASC[9] = 0xE3FFFFFF;   //288-319
			ASC[10] = 0xFF0A0060;   //320-351
			ASC[11] = 0x013F4081;   //352-383
			ASC[12] = 0x027F9F06;   //384-415
			ASC[13] = 0x07608044;   //416-447
			ASC[14] = 0x600280FF;   //448-479
			ASC[15] = 0xFBF01B01;   //480-511
			ASC[16] = 0xC8000000;   //512-543
			ASC[17] = 0x00000000;   //544-575
			ASC[18] = 0x00000000;   //576-607
			ASC[19] = 0x00000000;   //608-639
			ASC[20] = 0x00000000;   //640-671
			ASC[21] = 0x00000000;   //672-703
			ASC[22] = 0x00000000;   //704-735
			ASC[23] = 0x20000000;   //736-767
			ASC[24] = 0x00007CFC;   //768-799
			ASC[25] = 0x20000018;   //800-831
			ASC[26] = 0x02800000;   //832-863
			ASC[27] = 0x00000000;   //864-895
			ASC[28] = 0x00000800;   //896-927
			ASC[29] = 0x08002092;   //928-959
			ASC[30] = 0xA8000003;   //960-991
			ASC[31] = 0xF7000805;   //992-1023
			ASC[32] = 0x7060A07F;   //1024-1055
			ASC[33] = 0x09E4B0C4;   //1056-1087
			ASC[34] = 0x7FF70000;   //1088-1119
			ASC[35] = 0x00000000;   //1120-1151
			ASC[36] = 0x00000000;   //1152-1183
			ASC[37] = 0x00000000;*/
			
			/*
			increments = (2402000000.0 - LC_frequency) / (20 * temp + 88000);
			LC_monotonic(600, 21, 131);
			// LC_monotonic(increments, 21, 131);
			printf("Increments: %d\n", increments);
			*/
			
			/*
			 * Frequency per degree Celsius = -196185.9212
			 * Frequency per LSB = 130000
			 * LSB per degree Celsius = -1.5
			 * Current: LSB = 599, temp = 15.7
			 */ 
			// frequency_per_LSB = -150.1 * temp + 183865;
			// LSB_per_degree = fix_double(fix_div(fix_init(-196186), fix_init((int) frequency_per_LSB)));
			// increments = 599 + LSB_per_degree * (temp - 15.7);
			
			increments = -0.2519 * (temp_sum / temp_counts) + 338.980 - 2;
			LC_monotonic(increments, 21, 131);
			printf("Increments: %d\n", increments);
			
			analog_scan_chain_write(&ASC[0]);
			analog_scan_chain_load();
			
			temp_counts = 0;
			temp_sum = 0;
		}

	}	//printf("COMPARE4 MATCH\n");
	
	
	if (interrupt & 0x00000020) printf("COMPARE5 MATCH\n");
	if (interrupt & 0x00000040) printf("COMPARE6 MATCH\n");
	if (interrupt & 0x00000080) printf("COMPARE7 MATCH\n");
	if (interrupt & 0x00000100) printf("CAPTURE0 TRIGGERED AT: 0x%x\n", RFTIMER_REG__CAPTURE0);
	if (interrupt & 0x00000200) printf("CAPTURE1 TRIGGERED AT: 0x%x\n", RFTIMER_REG__CAPTURE1);
	if (interrupt & 0x00000400) printf("CAPTURE2 TRIGGERED AT: 0x%x\n", RFTIMER_REG__CAPTURE2);
	if (interrupt & 0x00000800) printf("CAPTURE3 TRIGGERED AT: 0x%x\n", RFTIMER_REG__CAPTURE3);
	if (interrupt & 0x00001000) printf("CAPTURE0 OVERFLOW AT: 0x%x\n", RFTIMER_REG__CAPTURE0);
	if (interrupt & 0x00002000) printf("CAPTURE1 OVERFLOW AT: 0x%x\n", RFTIMER_REG__CAPTURE1);
	if (interrupt & 0x00004000) printf("CAPTURE2 OVERFLOW AT: 0x%x\n", RFTIMER_REG__CAPTURE2);
	if (interrupt & 0x00008000) printf("CAPTURE3 OVERFLOW AT: 0x%x\n", RFTIMER_REG__CAPTURE3);
	
	RFTIMER_REG__INT_CLEAR = interrupt;
}
