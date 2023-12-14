#include "esp32_cube.h"

void setup() {
  Serial1.setRxBufferSize(SERIAL_SIZE_RX);
  Serial1.setTxBufferSize(SERIAL_SIZE_RX);
  DEBUG_SERIAL.begin(9600); //usb cable, for debug

  Serial1.begin(BAUD_RATE); // connection to teensys?
  pinMode(PIN_EVENT_BUF, INPUT);

  // pinMode(PIN_ADC_CLOCK, OUTPUT);
  pinMode(PIN_EVENT_CLR, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
#ifndef TEST
  pinMode(PIN_ADC_TN, INPUT);
  pinMode(PIN_ADC_TE, INPUT);
  pinMode(PIN_ADC_TS, INPUT);
  pinMode(PIN_ADC_TW, INPUT);
  pinMode(PIN_ADC_BN, INPUT);
  pinMode(PIN_ADC_BE, INPUT);
  pinMode(PIN_ADC_BS, INPUT);
  pinMode(PIN_ADC_BW, INPUT);
#endif

}

void loop(){
#ifdef TEST_SPARKLES //random lights 
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
    DEBUG_SERIAL.println("INTERRUPT TRIGGERED. READING DATA");
    uint16_t data_buff[8] = {};
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

    data_buff[0] = lookup_table[ytop][xtop];
    data_buff[1] = lookup_table[xtop][CUBE_SIZE-ytop];
    data_buff[2] = lookup_table[CUBE_SIZE-ytop][CUBE_SIZE-xtop];
    data_buff[3] = lookup_table[CUBE_SIZE-xtop][ytop];

    data_buff[4] = lookup_table[ybot][xbot];
    data_buff[5] = lookup_table[xbot][CUBE_SIZE-ybot];
    data_buff[6] = lookup_table[CUBE_SIZE-ybot][CUBE_SIZE-xbot];
    data_buff[7] = lookup_table[CUBE_SIZE-xbot][ybot];
    digitalWrite(PIN_EVENT_CLR, LOW);
#else
    //reading data from the SPiM ADCs
    get_spim_data(data_buff);
#endif //TEST
  

    LEDLine line = get_line_from_data(data_buff);
    coord line_coords[CUBE_SIZE] = {0};
    line.get_line_coords(line_coords);
    for(int i = 0; i<CUBE_SIZE; i++)
    {
      debug_print_coord(line_coords[i]);
    }

    uint16_t msg[MESSAGE_LEN/2];
    create_serial_message(msg, line);
    // digitalWrite(LED_BUILTIN, HIGH);
    Serial1.write((uint8_t *)msg, MESSAGE_LEN);

    // digitalWrite(LED_BUILTIN, LOW);

    data_ready_flag = 0;
  }
  
#endif //DEF TEST_LAYERS
#endif //DEF TEST SPARKLES
}

#ifndef TEST
int get_spim_data(uint16_t * dest_buffer)
{
  //manually control clock pin:
  //read bits in from 8 ADCs on 8 pins (12 bits I think?)
  //save to buffer (8 values)

  // set event clear to high
  digitalWrite(PIN_EVENT_CLR, HIGH);
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

  DEBUG_SERIAL.println("VALUES READ:");
  DEBUG_SERIAL.print("Top North: ");
  DEBUG_SERIAL.println(dest_buffer[0], HEX);
  DEBUG_SERIAL.print("Top East: ");
  DEBUG_SERIAL.println(dest_buffer[1], HEX);
  DEBUG_SERIAL.print("Top South: ");
  DEBUG_SERIAL.println(dest_buffer[2], HEX);
  DEBUG_SERIAL.print("Top West: ");
  DEBUG_SERIAL.println(dest_buffer[3], HEX);
  DEBUG_SERIAL.print("Bottom North: ");
  DEBUG_SERIAL.println(dest_buffer[4], HEX);
  DEBUG_SERIAL.print("Bottom East: ");
  DEBUG_SERIAL.println(dest_buffer[5], HEX);
  DEBUG_SERIAL.print("Bottom South: ");
  DEBUG_SERIAL.println(dest_buffer[6], HEX);
  DEBUG_SERIAL.print("Bottom West: ");
  DEBUG_SERIAL.println(dest_buffer[7], HEX);

  // digitalWrite(PIN_EVENT_CLR, LOW);
  return 0;
}
#endif //IF NOT DEF TEST

LEDLine get_line_from_data(uint16_t *spim_data)
{
  coord closest_cells[8] = {};
  DEBUG_SERIAL.println("COORDS:");
  for(int coord=0; coord<8; coord++)
  {
    closest_cells[coord] = get_coord_from_data(spim_data[coord]);
    if(coord>0)
    {
      //"rotate" coord to adjust for sensor position
      for(int j=0; j<coord; j++)
      {
        uint8_t x = closest_cells[coord].x;
        uint8_t y = closest_cells[coord].y;
        closest_cells[coord].x = y;
        closest_cells[coord].y = CUBE_SIZE-x;
      }
    }
    if(coord>3)
    {
      closest_cells[coord].z = 15;
    }    
  }
  coord top_coords[4] = {closest_cells[0], closest_cells[1], closest_cells[2], closest_cells[3]};
  coord bot_coords[4] = {closest_cells[4], closest_cells[5], closest_cells[6], closest_cells[7]};
  coord top = average_coords(top_coords, 4);
  coord bot = average_coords(bot_coords, 4);

  //lookup values in lookup table(s), get closest square(s)
  //get x y coord for top and for bottom
  //give them to the LEDLine class, which will calculate a line of 16 LEDs
  
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
    debug_print_coord(line[i]);
    uint16_t coord_bytes = line[i].x | (line[i].y << 4) | (line[i].z << 8);
    msg[i + 1] = coord_bytes;
    DEBUG_SERIAL.print("Coord Packed to 2 byte: ");
    DEBUG_SERIAL.println(coord_bytes, HEX);
    checksum ^= coord_bytes;
  }
  
  msg[(MESSAGE_LEN/2) - 2] = checksum;
  msg[(MESSAGE_LEN/2) - 1] = MSG_END;

  DEBUG_SERIAL.print("Checksum: ");
  DEBUG_SERIAL.println(checksum, HEX);
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

// int get_resized_lookup_table(uint16_t *table_buff)
// {
//   if(LOOKUP_SIZE == CUBE_SIZE)
//   {
    
//   }

//   return 0;
// }

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