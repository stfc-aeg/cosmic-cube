#ifndef LED_CUBE
#define LED_CUBE

#include <Arduino.h>
//length of one edge of the cube, in LEDs. (aim is a 16x16x16 cube)
#define CUBE_SIZE 12
#define NUM_STRANDS 9
#define TOWER_SIZE 4
#define PIXELS_PER_TOWER (TOWER_SIZE*TOWER_SIZE*CUBE_SIZE)

//total number leds in the cube
#define TOTAL_LEDS (CUBE_SIZE*CUBE_SIZE*CUBE_SIZE)
#define LED_PER_STRAND (TOTAL_LEDS / NUM_STRANDS)

#define SERIAL_SIZE_RX 1024

#define MSG_END 0xA5A5
//Length of serial message to/from controller, in bytes
//2 bytes for msg length, 2 bytes per LED, 2 bytes for end of msg, 2 for checksum
#define MESSAGE_LEN (2 + (2*CUBE_SIZE) + 2 + 2)

#define BAUD_RATE 115200




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
#endif //LED_CUBE