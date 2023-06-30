#include "esp32_cube.h"



void setup() {
  Serial1.setRxBufferSize(SERIAL_SIZE_RX);
  Serial1.setTxBufferSize(SERIAL_SIZE_RX);
  Serial.begin(BAUD_RATE); //usb cable, for debug
  Serial1.begin(BAUD_RATE); // connection to teensys?

}

void loop() {
  Serial.println("Testing");
  uint8_t send_val = 170;
  uint8_t send_buf[MESSAGE_LEN + 1] = {0};
  // for(uint16_t i = 0; i<MESSAGE_LEN; i++ )
  // {
  //   send_buf[i] = 0x5A;
  // }
  send_buf[0] = 0x5A;
  send_buf[MESSAGE_LEN - 1] = 0xA5;
  send_buf[MESSAGE_LEN] = 0xFF;
  Serial1.write(send_buf, MESSAGE_LEN+1);
  Serial1.flush();
  delay(1000);
}