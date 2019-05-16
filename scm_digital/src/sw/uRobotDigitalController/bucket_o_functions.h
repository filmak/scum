void analog_scan_chain_write(unsigned int* scan_bits);
void analog_scan_chain_load(void);
void initialize_ASC(void);
void disable_PA_ASC(void);
void enable_PA_ASC(void);
void enable_polyphase_ASC(void);
void disable_polyphase_ASC(void);
void disable_div_power_ASC(void);
void enable_div_power_ASC(void);
void ext_clk_ble_ASC(void);
void int_clk_ble_ASC(void);
void enable_1mhz_ble_ASC(void);
void disable_1mhz_ble_ASC(void);
void set_LC_current(unsigned int current);
void set_PA_supply(unsigned int code);
void set_LO_supply(unsigned int code);
void set_DIV_supply(unsigned int code);
void prescaler(int code);
void set_2M_RC_frequency(int freq);
void enable_32k(void);
void disable_32k(void);
void tx_gpio_ctrl(char LO, char PA);
void GPO_control(unsigned char row1, unsigned char row2, unsigned char row3, unsigned char row4);
void GPI_control(char row1, char row2, char row3, char row4);

void LC_FREQCHANGE(int coarse, int mid, int fine);
void LC_monotonic(int LCcode, int mid_divs, int coarse_divs);

void GPIO_test(void);
void clk_128(void);
void wait_128(void);
void wait_16(void);
void wait_alot(void);
void spec_test(void);

void read_LC(unsigned int* count_LC);
void read_counters(unsigned int* count_2M, unsigned int* count_LC, unsigned int* count_32k);

void divProgram(unsigned int div_ratio, unsigned int reset, unsigned int enable);

unsigned char flipChar(unsigned char b);
void gen_ble_packet(unsigned char *packet, unsigned char *AdvA, unsigned char channel);
void test_ble_packet(void);
void transmit_ble_packet(unsigned char *packet);
void resetBLE(void);
void enable_ble_module(void);
void disable_ble_module(void);
void gen_test_ble_packet(unsigned char *packet);
void transmit_ble_packet_rachel(unsigned char *packet);
void test_ble_packet_loop(void);
void test_rachel_fifo(void);

void test_spi(void);
void spi_cs_low(void);
void spi_cs_high(void);
void spi_write(unsigned char sendbyte);
unsigned char spi_read(void);
