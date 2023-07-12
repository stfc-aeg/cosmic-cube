//TEST defines if we want to generate fake SPIM data rather than read it in
#define TEST

#include "led_cube.h"
#include "lookup_table.h"

#define DEBUG true
#define DEBUG_SERIAL if(DEBUG)Serial

#define SERIAL_SIZE_RX 1024

#define data_trigger_pin 21
#define data_read_clock_pin 0
const int data_pins[8] = {0, 1, 2, 3, 4, 5, 6, 7};

bool data_ready_flag = 0;

//functions


int get_spim_data(uint16_t * dest_buffer);
LEDLine get_line_from_data(uint16_t *spim_data);
coord get_coord_from_data(uint16_t val);
coord average_coords(coord * coords, int num);

int create_serial_message(uint16_t * dest_buffer, LEDLine led_line);
int send_message(uint16_t * message_buffer);
void debug_print_coord(coord coord);
