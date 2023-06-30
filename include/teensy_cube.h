#include "led_cube.h"


// LED Control Library
// #include <Adafruit_NeoPixel.h>
#include <OctoWS2811.h>


#define BOARD_NUM 1


#if BOARD_NUM == 0
int led_pins[8] = {1,2,3,4,5,6,7,8};

#elif BOARD_NUM == 1
int led_pins[8] = {1,2,3,4,5,6,7,8};
#endif