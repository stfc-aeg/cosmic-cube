#include "esp32_cube.h"

void setup() {
  Serial1.setRxBufferSize(SERIAL_SIZE_RX);
  Serial1.setTxBufferSize(SERIAL_SIZE_RX);
  DEBUG_SERIAL.begin(9600); //usb cable, for debug

  Serial1.begin(BAUD_RATE); // connection to teensys?
  pinMode(PIN_EVENT_BUF, INPUT);

  pinMode(PIN_ADC_CLOCK, OUTPUT);
  pinMode(PIN_EVENT_CLR, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
// #ifndef TEST
  pinMode(PIN_ADC_TN, INPUT_PULLUP);
  pinMode(PIN_ADC_TE, INPUT_PULLUP);
  pinMode(PIN_ADC_TS, INPUT_PULLUP);
  pinMode(PIN_ADC_TW, INPUT_PULLUP);
  pinMode(PIN_ADC_BN, INPUT_PULLUP);
  pinMode(PIN_ADC_BE, INPUT_PULLUP);
  pinMode(PIN_ADC_BS, INPUT_PULLUP);
  pinMode(PIN_ADC_BW, INPUT_PULLUP);
// #endif

}

void loop(){
#ifdef TEST_SPARKLES //random lights, one at a time. An Easter Egg, no real function
  const int msg_size = 4;
  uint16_t msg[msg_size];
  coord pixel;
  pixel.x = random(CUBE_SIZE);
  pixel.y = random(CUBE_SIZE);
  pixel.z = random(CUBE_SIZE);

  uint16_t coord_bytes = pixel.x | (pixel.y <<4) | (pixel.z << 8);
  msg[0] = 1; //single pixel
  uint16_t checksum = 1;
  checksum ^= coord_bytes;
  msg[1] = coord_bytes;
  msg[2] = checksum;
  msg[3] = MSG_END;
  Serial1.write((uint8_t *)msg, msg_size*2);
  delay(20);
#else
#ifdef TEST_LAYERS 
  // test to make sure the towers are aligned properly,
  // by drawing a layer at a time 
  DEBUG_SERIAL.println("Creating Layer");
  digitalWrite(LED_BUILTIN, HIGH);
  const int msg_size = 1 + (CUBE_SIZE*CUBE_SIZE) + 2;
  uint16_t msg[msg_size];
  coord plane[CUBE_SIZE*CUBE_SIZE];
  for(int i = 0; i<(CUBE_SIZE*CUBE_SIZE); i++)
  {
    plane[i].z = num_traces % CUBE_SIZE;
    
    plane[i].x = i % CUBE_SIZE;
    plane[i].y = i / CUBE_SIZE;
  }

  num_traces ++;

  msg[0] = CUBE_SIZE*CUBE_SIZE;
  uint16_t checksum = msg[0];
  for(int j = 0; j<CUBE_SIZE*CUBE_SIZE; j++)
  {
    uint16_t coord_bytes = plane[j].x | (plane[j].y << 4) | (plane[j].z << 8);
    msg[j+1] = coord_bytes;
    checksum ^= coord_bytes;
  }

  msg[msg_size - 2] = checksum;
  msg[msg_size - 1] = MSG_END;

  Serial1.write((uint8_t *)msg, msg_size*2);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
#else
  if(digitalRead(PIN_EVENT_BUF))
  {
    DEBUG_SERIAL.print(millis());
    DEBUG_SERIAL.print("\t");
    while(digitalRead(PIN_EVENT_BUF)); //wait for the event buf to go low again
#ifdef TEST
    //generate a fake trace, rather than basing it off the actual signals from the SiPM
    digitalWrite(PIN_EVENT_CLR, HIGH);
    long xtop = random(CUBE_SIZE);
    long ytop = random(CUBE_SIZE);
    long xbot = random(CUBE_SIZE);
    long ybot = random(CUBE_SIZE);

    DEBUG_SERIAL.println("GENERATED COORD:");
    DEBUG_SERIAL.print("Top: ");
    DEBUG_SERIAL.print(xtop);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(ytop);
    DEBUG_SERIAL.print(": ");
    DEBUG_SERIAL.println(lookup_table[ytop][xtop]);
    DEBUG_SERIAL.print("bot: ");
    DEBUG_SERIAL.print(xbot);
    DEBUG_SERIAL.print(", ");
    DEBUG_SERIAL.print(ybot);
    DEBUG_SERIAL.print(": ");
    DEBUG_SERIAL.println(lookup_table[ybot][xbot]);

    coord top;
    coord bottom;
    top.x = xtop;
    top.y = ytop;
    bottom.x = xbot;
    bottom.y = ybot;
    LEDLine line = LEDLine(top, bottom);
    digitalWrite(PIN_EVENT_CLR, LOW);
#else
    //reading data from the SPiM ADCs
    uint16_t data_buff[8] = {};
    get_spim_data(data_buff);
    LEDLine line = get_line_from_data_interpolation(data_buff);
#endif //TEST
    coord line_coords[CUBE_SIZE] = {0};
    line.get_line_coords(line_coords);

    uint16_t msg[MESSAGE_LEN/2];
    create_serial_message(msg, line);
    // digitalWrite(LED_BUILTIN, HIGH);
    Serial1.write((uint8_t *)msg, MESSAGE_LEN);

  }
  
#endif //DEF TEST_LAYERS
#endif //DEF TEST SPARKLES
}

#ifndef TEST
int get_spim_data(uint16_t * dest_buffer)
{
  //manually control clock pin:
  //read bits in from 8 ADCs on 8 pins (14 bits)
  //save to buffer (8 values)

  for(int i=0; i<ADC_BITS; i++)
  {
    //set clock high, read in bit, set clock low
    
    digitalWrite(PIN_ADC_CLOCK, HIGH);
    dest_buffer[0] = (dest_buffer[0] << 1) | digitalRead(PIN_ADC_TN);
    dest_buffer[1] = (dest_buffer[1] << 1) | digitalRead(PIN_ADC_TE);
    dest_buffer[2] = (dest_buffer[2] << 1) | digitalRead(PIN_ADC_TS);
    dest_buffer[3] = (dest_buffer[3] << 1) | digitalRead(PIN_ADC_TW);

    dest_buffer[4] = (dest_buffer[4] << 1) | digitalRead(PIN_ADC_BN);
    dest_buffer[5] = (dest_buffer[5] << 1) | digitalRead(PIN_ADC_BE);
    dest_buffer[6] = (dest_buffer[6] << 1) | digitalRead(PIN_ADC_BS);
    dest_buffer[7] = (dest_buffer[7] << 1) | digitalRead(PIN_ADC_BW);
    digitalWrite(PIN_ADC_CLOCK, LOW);
  }

#if DEBUG
  // DEBUG_SERIAL.print("VALUES READ:");
  // DEBUG_SERIAL.print("TN: ");
  DEBUG_SERIAL.print(dest_buffer[0], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("TE: ");
  DEBUG_SERIAL.print(dest_buffer[1], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("TS: ");
  DEBUG_SERIAL.print(dest_buffer[2], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("TW: ");
  DEBUG_SERIAL.print(dest_buffer[3], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("BN: ");
  DEBUG_SERIAL.print(dest_buffer[4], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("BE: ");
  DEBUG_SERIAL.print(dest_buffer[5], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("BS: ");
  DEBUG_SERIAL.print(dest_buffer[6], DEC);
  DEBUG_SERIAL.print("\t");
  // DEBUG_SERIAL.print("BW: ");
  DEBUG_SERIAL.print(dest_buffer[7], DEC);
  DEBUG_SERIAL.print("\t");
#endif // DEBUG
  
  return 0;
}
#endif //IF NOT DEF TEST

LEDLine get_line_from_data_interpolation(uint16_t *spim_data)
{
  //ASSUMPTIONS AND SIMPLIFICATIONS: 
  //  assume SPIMs are in center of square side
  //  assume SPIMS all have same calibration?
  //  assume linear falloff of signal is SPIM based on
  for(int i = 0; i < 8; i++)
  {
    if(spim_data[i] < BASELINE)
    {
      spim_data[i] = 1; //not setting it to 0 to avoid the slim chance of a div/0 error
    }
    else
    {
      spim_data[i] -= BASELINE;
    }
  }
  //this needs to be done for top, and for bottom. spim_data[0-3] is top (NESW)
  uint16_t top_data[4] = {spim_data[0], spim_data[1], spim_data[2], spim_data[3]};
  uint16_t bot_data[4] = {spim_data[4], spim_data[5], spim_data[6], spim_data[7]};

  // char str_buf_top[100];
  // sprintf(str_buf_top, "Top Data: %d, %d, %d, %d", top_data[0], top_data[1], top_data[2], top_data[3]);
  // DEBUG_SERIAL.println(str_buf_top);

  //get ratios

  double top_ns_ratio = (float)top_data[0] / (float)(top_data[0] + top_data[2]);
  double top_ew_ratio = (float)top_data[1] / (float)(top_data[1] + top_data[3]);

  double bot_ns_ratio = (float)bot_data[0] / (float)(bot_data[0] + bot_data[2]);
  double bot_ew_ratio = (float)bot_data[1] / (float)(bot_data[1] + bot_data[3]);

  

  //position between N and S sensor is the ratio of N with the total?
  coord top = coord();
  coord bot = coord();

  top.x = round((float)(CUBE_SIZE-1) * top_ns_ratio);
  top.y = round((float)(CUBE_SIZE-1) * top_ew_ratio);
  top.z = 11;

  bot.x = round((float)(CUBE_SIZE-1) * bot_ns_ratio);
  bot.y = round((float)(CUBE_SIZE-1) * bot_ew_ratio);
  bot.z = 0;

  //lets faff with the numbers a lil more to spread them out on the cube a bit

  // DEBUG_SERIAL.print("Top: ");
  DEBUG_SERIAL.print(top.x);
  DEBUG_SERIAL.print("\t");
  DEBUG_SERIAL.print(top.y);
  DEBUG_SERIAL.print("\t");

  // DEBUG_SERIAL.print("Bot: ");
  DEBUG_SERIAL.print(bot.x);
  DEBUG_SERIAL.print("\t");
  DEBUG_SERIAL.println(bot.y);

  return LEDLine(top, bot);



}

int create_serial_message(uint16_t * msg, LEDLine led_line)
{
  /*msg layout:
   0x1000 num points, 2 bytes. max 4096 as thats the total number of LEDs
   0x0ZYX 2 bytes, point coords (each coord takes half a byte, pad with 0s to avoid byte boundry weirdness)
   0x0ZYX
   ....
   0xA5A5 END of message bytes
  */

  // uint16_t msg[MESSAGE_LEN/2] = {};
  coord line[CUBE_SIZE];
  uint16_t checksum = CUBE_SIZE;
  led_line.get_line_coords(line);
  msg[0] = CUBE_SIZE;
  for(int i = 0; i<CUBE_SIZE; i++)
  {
    uint16_t coord_bytes = line[i].x | (line[i].y << 4) | (line[i].z << 8);
    msg[i + 1] = coord_bytes;
    checksum ^= coord_bytes;
  }
  
  msg[(MESSAGE_LEN/2) - 2] = checksum;
  msg[(MESSAGE_LEN/2) - 1] = MSG_END;

  return 0;
}

coord get_coord_from_data(uint16_t val)
{
  //gets the closest cell in the lookup table to the value provided
  coord closest = {0, 0, 0};
  uint16_t cur_diff = 65535; //max val
  for(uint8_t y = 0; y<CUBE_SIZE; y++)
  {
    for(uint8_t x = 0; x<CUBE_SIZE; x++){
      uint16_t diff = abs(lookup_table[y][x] - val);
      if(diff < cur_diff)
      {
        closest.x = x;
        closest.y = y;
        cur_diff = diff;
      }
    }
  }
  return closest;
}

coord average_coords(coord * coords, int num)
{
  coord average = {0, 0, 0};
  for(int i = 0; i<num; i++)
  {
    average.x += coords[i].x;
    average.y += coords[i].y;
    average.z += coords[i].z;
  }
  average.x = round(average.x / num);
  average.y = round(average.y / num);
  average.z = round(average.z / num);

  return average;
}

#if DEBUG
void debug_print_coord(coord coord)
{
  DEBUG_SERIAL.print("(");
  DEBUG_SERIAL.print(coord.x);
  DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(coord.y);
  DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(coord.z);
  DEBUG_SERIAL.println(")");
}
#endif