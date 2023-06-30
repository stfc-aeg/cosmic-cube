#include "led_cube.h"

LEDLine::LEDLine(coord top, coord bot)
{
    line[0] = top;
    line[CUBE_SIZE - 1] = bot;

    for(uint8_t z=0; z < CUBE_SIZE; z++)
    {
        int t = z / (CUBE_SIZE - 1);
        coord new_led = {0, 0, z};
        new_led.x = round(bot.x + t * (top.x - bot.x));
        new_led.y = round(bot.y + t * (top.y - bot.y));
        line[z] = new_led;
    }
}


int LEDLine::calc_line(coord top, coord bot)
{
    for (uint8_t z=0; z < CUBE_SIZE; z ++)
    {
        int t = z / (CUBE_SIZE - 1);
        coord new_led = {0, 0, z};
        new_led.x = round(bot.x + t * (top.x - bot.x));
        new_led.y = round(bot.y + t * (top.y - bot.y));
        line[z] = new_led;
    }
    return CUBE_SIZE;
}

