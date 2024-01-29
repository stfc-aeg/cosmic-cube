#include "led_cube.h"

LEDLine::LEDLine(coord top, coord bot)
{
    calc_line(top, bot);
}


int LEDLine::calc_line(coord top, coord bot)
{
    for (uint8_t z=0; z < CUBE_SIZE; z ++)
    {
        float t = (float)z / (float)(CUBE_SIZE - 1);
        coord new_led = {0, 0, z};
        new_led.x = round(bot.x + t * (top.x - bot.x));
        new_led.y = round(bot.y + t * (top.y - bot.y));
        line[z] = new_led;
    }
    return CUBE_SIZE;
}

int LEDLine::set_top_coord(coord top)
{
    calc_line(top, line[CUBE_SIZE - 1]);
    return CUBE_SIZE;
}

int LEDLine::set_bottom_coord(coord bot)
{
    calc_line(line[0], bot);
    return CUBE_SIZE;
}

int LEDLine::get_line_coords(coord * buffer)
{
    for(int i = 0; i < CUBE_SIZE; i++)
    {
        buffer[i] = line[i];
    }
    return CUBE_SIZE;
}
