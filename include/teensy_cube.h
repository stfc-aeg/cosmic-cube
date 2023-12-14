#include "led_cube.h"


// LED Control Library
// #include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>

#include <intervalTimer.h>

#define BOARD_NUM 1

#define DEBUG false
#define DEBUG_SERIAL if(DEBUG)Serial

#define DIM_PIN 3
#define BRIGHTNESS 1024
#define DIM_PER_FRAME 0.95
const uint32_t set_color[3] = {0x00FF0000, 0x0000FF00, 0x000000FF}; //RGB

// LED Control Setup
#if BOARD_NUM == 0
const int num_pins = 3;
byte led_pins[num_pins] = {14, 15, 16}; //, 15, 16, 17, 18, 19, 20, 21};
#define IS_OUR_LED(coord) (coord.y < TOWER_SIZE)
#elif BOARD_NUM == 1
const int num_pins = 6;
byte led_pins[num_pins] = {14, 15, 16, 17, 18, 19};
#define IS_OUR_LED(coord) (coord.y >= TOWER_SIZE)
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

int num_traces = 0;
uint8_t incoming_message[SERIAL_SIZE_RX] = {0};
uint8_t rx_buffer[SERIAL_SIZE_RX*2];
int byte_count = 0;

IntervalTimer cubeTimer;

OctoWS2811 leds(LED_PER_STRAND, displayMemory, drawingMemory, config, num_pins, led_pins);

int coord_to_tower_pos(coord led);
int coord_to_mini_cube_pos(coord led);
bool check_serial_message(uint16_t *msg_buff, int count);
coord get_coord_from_msg(uint16_t msg);
void fade_whole_cube();
void update_cube();

float brightness;

struct queue_object{
    int led;
    uint32_t color;
};

const int queue_size = CUBE_SIZE*CUBE_SIZE*CUBE_SIZE;
queue_object led_queue[queue_size];
int queue_front = 0;
int queue_end = 0;

queue_object queue_pop();
int queue_append(queue_object val);
