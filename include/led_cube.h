//length of one edge of the cube, in LEDs. (aim is a 16x16x16 cube)
#define CUBE_SIZE 16
#define NUM_STRANDS 16

//total number leds in the cube
#define TOTAL_LEDS (CUBE_SIZE*CUBE_SIZE*CUBE_SIZE)

//Length of serial message to/from controller, in bytes
#define MESSAGE_LEN TOTAL_LEDS / 8

#define BAUD_RATE 115200

#include <Arduino.h>

#ifdef ESP32
#pragma message "BUILDING FOR ESP32 in led_cube.h"
#else
#pragma message "BUILDING FOR TEENSY in led_cube.h"
#endif

struct coord {
    uint8_t x;
    uint8_t y;
    uint8_t z;
};

class LEDLine
{
    private:
        coord line[CUBE_SIZE];
        int calc_line(coord top, coord bot);

    public:
        LEDLine(coord top, coord bot);
        int set_top_coord(coord);
        int set_bottom_coord(coord);
        int get_line_coords(coord * line_buffer);
};