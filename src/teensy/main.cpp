#include "teensy_cube.h"

void setup() {
    DEBUG_SERIAL.begin(9600); //usb cable, for debug
    
    Serial1.addMemoryForRead(rx_buffer, sizeof(rx_buffer)); //larger buffer in case of long messages
    Serial1.begin(BAUD_RATE); // connection to esp32
    pinMode(LED_BUILTIN, OUTPUT);
    
    cubeTimer.begin(update_cube, 10000); // update cube every 10 milliseconds
    
    //setup for neopixels
    leds.begin();
    leds.show();
}

void loop() 
{ 
    if(Serial1.available())
    {
        digitalWrite(LED_BUILTIN, HIGH);
        uint8_t msg_byte = Serial1.read();

        incoming_message[byte_count] = msg_byte;
        byte_count ++;

        if(msg_byte == 0xa5 && incoming_message[byte_count-2] == 0xa5)
        {
            DEBUG_SERIAL.print("\nFull message Read. Message Length: ");
            DEBUG_SERIAL.println(byte_count);

            uint16_t message[byte_count/2] = {0};
            for(int i = 0; i < byte_count/2; i++)
            {
                message[i] = (incoming_message[(i*2)+1] << 8) | incoming_message[i*2];
            }

            //recalculate checksum to ensure message came through correctly
            if(!check_serial_message(message, byte_count/2))
            {
                byte_count = 0;  // reset count
                return; // checksum didn't match, cancel
            }

            byte_count = 0;  // reset count

            uint32_t color = set_color[num_traces % 3];
            num_traces ++;

            for(int j = message[0]; j>0; j--)
            {
            coord coord  = get_coord_from_msg(message[j]);

            int tower_pos = coord_to_tower_pos(coord);
#if DEBUG
            char buff[256];
            sprintf(buff, "Coord (%d,%d,%d) to position: %d", coord.x,coord.y,coord.z, tower_pos);
            DEBUG_SERIAL.println(buff);
#endif
            queue_object new_led = {tower_pos, color};
            queue_append(new_led);
            }
        }
        digitalWrite(LED_BUILTIN, LOW);
    }
}

bool check_serial_message(uint16_t *msg_buff, int count)
{
    digitalWrite(LED_BUILTIN, HIGH);
    uint16_t checksum = 0;

    for(int i = 0; i<count - 2; i++)
    {
        checksum ^= msg_buff[i];
    }
    DEBUG_SERIAL.print("CHECKSUM CALC: ");
    DEBUG_SERIAL.println(checksum, HEX);
    DEBUG_SERIAL.print("CHECKSUM MSG : ");
    DEBUG_SERIAL.println(msg_buff[count-2], HEX);
    digitalWrite(LED_BUILTIN, LOW);

    return checksum == msg_buff[count - 2];
}

coord get_coord_from_msg(uint16_t msg)
{
    coord coord;
    coord.x = msg & 0x00F;
    coord.y = (msg & 0x0F0) >> 4;
    coord.z = (msg & 0xF00) >> 8;

    return coord;

}

void update_cube()
{
    fade_whole_cube();
    queue_object next_led = queue_pop();
    if(next_led.led != -1)
    {
        leds.setPixel(next_led.led, next_led.color*(BRIGHTNESS/1024));
    }
    leds.show();
}

void fade_whole_cube()
{
    //loop through the entire set of LEDs, dimming each so it fades over time
    for(int i = 0; i < (LED_PER_STRAND*num_pins); i++)
    {
        int cur_pixel = leds.getPixel(i);
        uint8_t red = ((cur_pixel & 0xFF0000) >> 16) * DIM_PER_FRAME;
        uint8_t green = ((cur_pixel & 0x00FF00) >> 8) * DIM_PER_FRAME;
        uint8_t blue = (cur_pixel & 0x0000FF) * DIM_PER_FRAME;

        cur_pixel = (red << 16) | (green << 8) | blue;
        // cur_pixel /= DIM_PER_FRAME;
        leds.setPixel(i, cur_pixel);
    }
}

//returns position of led in the strands, or a negative number if the LED is not
//controlled by this teensy.
int coord_to_tower_pos(coord led)
{
    if(!IS_OUR_LED(led)) return -1;
#if BOARD_NUM == 1
    led.y -= TOWER_SIZE; //offset for being the other board
#endif
    uint8_t tower_num = (led.x/TOWER_SIZE) + ((led.y/TOWER_SIZE)*(CUBE_SIZE/TOWER_SIZE));
    // modulo x and y so we can work out specific position relative to tower
    led.x %= TOWER_SIZE;
    led.y %= TOWER_SIZE;
    
    //if z is odd, flip x and y, and invert
    if(led.z % 2)
    {
        int c = led.x;
        led.x = (TOWER_SIZE-1) - led.y;
        led.y = (TOWER_SIZE-1) - c;
    }

    if(led.y % 2) led.x = (TOWER_SIZE-1) - led.x; //if y is odd, invert x


    return (PIXELS_PER_TOWER*tower_num) + led.x + (led.y*TOWER_SIZE) + (led.z*TOWER_SIZE*TOWER_SIZE);
}

queue_object queue_pop()
{
    if(queue_front == queue_end)
    {
        //buffer empty
        queue_object ret = {-1, 0};
        return ret;
    }
    queue_object val = led_queue[queue_front];
    queue_front = (queue_front + 1) % queue_size;
    return val;
}

int queue_append(queue_object val)
{
    if((queue_end + 1) % queue_size == queue_front)
    {
        //buffer full
        return 0;
    }
    led_queue[queue_end] = val;
    queue_end = (queue_end + 1) % queue_size;
    return 1;
}