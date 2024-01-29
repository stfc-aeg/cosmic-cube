//TEST defines if we want to generate fake SPIM data rather than read it in
// #define TEST

// #define TEST_LAYERS  //light up one layer at a time, in order, to debug
// #define TEST_SPARKLES //light up random individual LEDs


#include "led_cube.h"
#include "lookup_table.h"

#define DEBUG false
#define DEBUG_SERIAL if(DEBUG)Serial  //leaving debug on while no serial monitor is connected is wasteful

#define PIN_EVENT_BUF 15  //Pin driven high when Ivan's electronics detect a conincidence event
#define PIN_ADC_CLOCK 32  //Pin to clock out a high/low pattern to read in ADC values
#define PIN_EVENT_CLR 33  // thought to be required to clear the ADC values once read, but not currently used

#define PIN_ADC_TN 26 //top north adc
#define PIN_ADC_TE 25 //top east
#define PIN_ADC_TS 34 //top south
#define PIN_ADC_TW 39 //top west
#define PIN_ADC_BN 36 //bottom north
#define PIN_ADC_BE 4  //bottom east
#define PIN_ADC_BS 5  //bottom south
#define PIN_ADC_BW 19 //bottom west

#define ADC_BITS 14 //only 12 are used but we need to read all 14 bits

//value used for simplistic base subtraction. Likely inaccurate but good 
//enough to spread the signal around the cube more. Without this, most 
//traces are drawn in the middle of the cube
#define BASELINE 500


int num_traces = 0;  //used in the layer test mode

//functions
int get_spim_data(uint16_t * dest_buffer);
LEDLine get_line_from_data_interpolation(uint16_t *spim_data);
int create_serial_message(uint16_t * dest_buffer, LEDLine led_line);
