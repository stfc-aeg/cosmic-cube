#include "teensy_cube.h"

void setup() {
    Serial.begin(BAUD_RATE); //usb cable, for debug
    Serial1.begin(BAUD_RATE); // connection to esp32?
    pinMode(LED_BUILTIN, OUTPUT);

    //setup for neopixels
    leds.begin();
    leds.show();
}

void loop() 
{ 
    // fade_whole_cube();
    if (Serial1.available() > 0)
    {
        uint16_t incoming_message[MESSAGE_LEN] = {0};
        if(!read_serial_message(incoming_message))
        {   
            Serial.println("CHECKSUM INVALID");
            return; //TODO if checksum invalid, skip the rest of the loop I guess?
        }
        // coord new_leds[incoming_message[0]+1];
        // Serial.println("Adding New Vals to ")
        for(int i = 1; i< incoming_message[0]+1; i++)
        {
            coord coord  = get_coord_from_msg(incoming_message[i]);

            // new_leds[i-1] = get_coord_from_msg(incoming_message[i]);
            int tower_pos = coord_to_tower_pos(coord);
            char buff[256];
            sprintf(buff, "Coord (%d,%d,%d) to position: %d", coord.x,coord.y,coord.z, tower_pos);
            Serial.println(buff);
            queue_append(tower_pos);
        }
    }
    update_cube();
    delay(100);
}

bool read_serial_message(uint16_t *msg_buff)
{
    digitalWrite(LED_BUILTIN, HIGH);
    uint16_t checksum = 0;
    for(int i = 0; i<MESSAGE_LEN; i++)
    {
        uint8_t first = Serial1.read();
        uint8_t second = Serial1.read();
        msg_buff[i] = (second << 8) | first;
        if(msg_buff[i] == MSG_END)
        {
            break;
        }
    }

    Serial.print(String("BOARD " + String(BOARD_NUM) + " Received: "));
    for(int i = 0; i<MESSAGE_LEN/2; i++){

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
    int next_led = queue_pop();
    if(next_led != -1)
    {
        leds.setPixel(next_led, set_color*(BRIGHTNESS/1024));
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
//this is all assuming a cube of 16x16x16
int coord_to_tower_pos(coord led)
{
    if(!IS_OUR_LED(led)) return -1;
#if BOARD_NUM == 1
    led.y -= 8; //offset for being the other board
#endif
    uint8_t tower_num = (led.x/4) + ((led.y/4)*4);
    led.x %= 4;
    led.y %= 4;
    //if x odd, z must be inverted
    if(led.x % 2) led.z = 15 - led.z;
    //if y odd, x must invert
    if(led.y % 2) led.x =  3 - led.x;

    return (PIXELS_PER_TOWER*tower_num) + led.z + (led.x * 16) + (led.y*64);
}


int queue_pop()
{
    if(queue_front == queue_end)
    {
        //buffer empty
        return -1;
    }
    int val = led_queue[queue_front];
    queue_front = (queue_front + 1) % queue_size;
    return val;
}

int queue_append(int val)
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