//TEST defines if we want to generate fake SPIM data rather than read it in
#define TEST

#include "led_cube.h"
#include "lookup_table.h"

#define DEBUG true
#define DEBUG_SERIAL if(DEBUG)Serial

#define SERIAL_SIZE_RX 1024

#define PIN_EVENT_BUF 21
// #define PIN_ADC_CLOCK 20
// #define PIN_EVENT_CLR 22
#define SIGNAL_TIMER 500

const int data_pins[8] = {5,6,7,8,9,10,11,12};

#define PIN_ADC_TN 5 //top north adc
#define PIN_ADC_TE 6 //top east
#define PIN_ADC_TS 7 //top south
#define PIN_ADC_TW 8 //top west
#define PIN_ADC_BN 9 //bottom north
#define PIN_ADC_BE 10//bottom east
#define PIN_ADC_BS 11//bottom south
#define PIN_ADC_BW 12//bottom west

#define ADC_BITS 14 //only 12 are used but we might need to read all 14 bits?


bool data_ready_flag = 0;

uint32_t last_timestamp = 0;


//functions


int get_spim_data(uint16_t * dest_buffer);
LEDLine get_line_from_data(uint16_t *spim_data);
coord get_coord_from_data(uint16_t val);
coord average_coords(coord * coords, int num);

int create_serial_message(uint16_t * dest_buffer, LEDLine led_line);
int send_message(uint16_t * message_buffer);
void debug_print_coord(coord coord);
