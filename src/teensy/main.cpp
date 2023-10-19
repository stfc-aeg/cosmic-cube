#include "teensy_cube.h"

void setup() {
    // Serial.begin(9600); //usb cable, for debug
    Serial1.begin(BAUD_RATE); // connection to esp32?
    pinMode(LED_BUILTIN, OUTPUT);

    //setup for neopixels
    leds.begin();
    leds.show();
}

void loop() 
{ 
    // fade_whole_cube();
    if (Serial1.available() >= MESSAGE_LEN)
    {
        uint16_t incoming_message[MESSAGE_LEN] = {0};
        if(!read_serial_message(incoming_message))
        {   
            // Serial.println("CHECKSUM INVALID");
            return; //TODO if checksum invalid, skip the rest of the loop I guess?
        }
        uint32_t color = set_color[num_traces % 3];
        num_traces ++;
        // coord new_leds[incoming_message[0]+1];
        // Serial.println("Adding New Vals to ")

        //need to read this backwards in order to append to queue correctly

        for(int i = incoming_message[0]; i>0; i--)
        // for(int i = 1; i< incoming_message[0]+1; i++)
        {
            coord coord  = get_coord_from_msg(incoming_message[i]);

            // int tower_pos = coord_to_tower_pos(coord);
            int tower_pos = coord_to_mini_cube_pos(coord);
            char buff[256];
            sprintf(buff, "Coord (%d,%d,%d) to position: %d", coord.x,coord.y,coord.z, tower_pos);
            Serial.println(buff);
            queue_object new_led = {tower_pos, color};
            queue_append(new_led);
        }
    }
    update_cube();
    delay(10);
}

bool read_serial_message(uint16_t *msg_buff)
{
    digitalWrite(LED_BUILTIN, HIGH);
    uint16_t checksum = 0;
    int counter = 0;
    for(int i = 0; i<MESSAGE_LEN; i++)
    {
        uint8_t first = Serial1.read();
        uint8_t second = Serial1.read();
        msg_buff[i] = (second << 8) | first;
        counter ++;
        if(msg_buff[i] == MSG_END)
        {
            break;
        }
    }

    // Serial.print(String("BOARD " + String(BOARD_NUM) + " Received: " + counter + " bytes: "));
    for(int i = 0; i<counter; i++){

        Serial.print(msg_buff[i], HEX);
        Serial.print(" ");
    }
    Serial.println("");

    for(int i = 0; i<(MESSAGE_LEN/2) - 2; i++)
    {
        checksum ^= msg_buff[i];
    }
    Serial.print("CHECKSUM: ");
    Serial.println(checksum, HEX);
    digitalWrite(LED_BUILTIN, LOW);

    return checksum == msg_buff[(MESSAGE_LEN/2) - 2];
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
    led.y -= CUBE_SIZE/TOWER_SIZE; //offset for being the other board
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

//returns the position of an LED in 

//returns the position of an LED in the strand, if using the CUBE:BIT 5x5x5 cube
int coord_to_mini_cube_pos(coord led)
{
    /*examples: 
    (0,0,0) = 0
    (1,0,0) = 1
    (0,1,0) = 9
    (2,2,0) = 12
    (4,1,0) = 5

    */

    if(led.z % 2)
    {
        //if z is odd, flip along the xy diagonal. think this works?
        int c = led.x;
        led.x = (CUBE_SIZE-1) - led.y;
        led.y = (CUBE_SIZE-1) - c;
    }

    if(led.y % 2) led.x = 4 - led.x;

    return led.x + (led.y*CUBE_SIZE) + (led.z*CUBE_SIZE*CUBE_SIZE);
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