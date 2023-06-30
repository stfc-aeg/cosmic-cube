#include "teensy_cube.h"

void setup() {
Serial.begin(BAUD_RATE); //usb cable, for debug
Serial1.begin(BAUD_RATE); // connection to esp32?
pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
    uint8_t incoming = 0;
    uint8_t income_array[MESSAGE_LEN] = {0};
    if (Serial1.available() > 0)
        {
            digitalWrite(LED_BUILTIN, HIGH);
            // incoming = Serial1.read();
            Serial1.readBytesUntil(0xFF, income_array, MESSAGE_LEN*2);

            Serial.print(String("BOARD " + String(BOARD_NUM) + " Received: "));
            for(int i = 0; i<MESSAGE_LEN; i++){
                Serial.print(income_array[i], HEX);
                Serial.print(" ");
            }
            // Serial.println(incoming, HEX);
            Serial.println("");
            digitalWrite(LED_BUILTIN, LOW);
        }
    else
    {
        // Serial.println(String("BOARD " + String(BOARD_NUM) + " Did not Get Data"));
    }
    delay(100);
    Serial1.flush();
}