#include "led_cube.h"


// LED Control Library
// #include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>


#define BOARD_NUM 1

#define DIM_PIN 3
#define BRIGHTNESS 1024
#define DIM_PER_FRAME 0.9
const uint32_t set_color = 0xFFFFFFFF; //RGB

const int PIXELS_PER_TOWER = (4*4*16);

// LED Control Setup
#if BOARD_NUM == 0
const int num_pins = 8;
byte led_pins[num_pins] = {14, 15, 16, 17, 18, 19, 20, 21};
#define IS_OUR_LED(coord) (coord.y < CUBE_SIZE/2)
#elif BOARD_NUM == 1
const int num_pins = 8;
byte led_pins[num_pins] = {14, 15, 16, 17, 18, 19, 20, 21};
const uint16_t msg_slice_start = MESSAGE_LEN / 2;
const uint16_t msg_slice_end = MESSAGE_LEN;
#define IS_OUR_LED(coord) (coord.y >= CUBE_SIZE/2)
#endif

// These buffers need to be large enough for all the pixels.
// The total number of pixels is "ledsPerStrip * numPins".
// Each pixel needs 3 bytes, so multiply by 3.  An "int" is
// 4 bytes, so divide by 4.  The array is created using "int"
// so the compiler will align it to 32 bit memory.
const int bytesPerLED = 3;
DMAMEM int displayMemory[LED_PER_STRAND * num_pins * bytesPerLED / 4];
int drawingMemory[LED_PER_STRAND * num_pins * bytesPerLED / 4];

const int config = WS2811_GRB | WS2811_800kHz;

OctoWS2811 leds(LED_PER_STRAND, displayMemory, drawingMemory, config, num_pins, led_pins);

int coord_to_tower_pos(coord led);
bool read_serial_message(uint16_t *msg_buff);
coord get_coord_from_msg(uint16_t msg);
void fade_whole_cube();
void update_cube();

float brightness;

const int queue_size = LED_PER_STRAND * num_pins;
int led_queue[queue_size];
int queue_front = 0;
int queue_end = 0;

int queue_pop();
int queue_append(int val);
