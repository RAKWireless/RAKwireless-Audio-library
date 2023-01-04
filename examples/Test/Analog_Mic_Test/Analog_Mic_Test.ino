#include "NAU85L40B.h"
#include "audio.h"

NAU85L40B   mic(0x1D);

short read_data[512] = {0};
short write_data[512] = {0};

void reader(void *pvParameters) {

  size_t bytes_read;
  while(1){
    bytes_read = I2S.read((uint8_t *)read_data, 512*2);
    //Serial.println(bytes_read);
    //Serial.println(read_data[0],HEX);
    //delay(500); // this dealy is decide by the sample rate and dma rate
  }
}  


void setup () {
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH); // Enable power supply.
  delay(300);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  Serial.begin (115200) ;
  // Initialize Serial for debug output
//  time_t timeout = millis();
//  while (!Serial)
//  {
//    if ((millis() - timeout) < 5000)
//    {
//      delay(100);
//    }
//    else
//    {
//      break;
//    }
//  }
  Serial.println("Analog Mic Test!!!");
  delay(1000);

  if (!mic.begin())
  {
//    Serial.println("Do not find mic");
//    while (1)
//      delay(500);
  }
  mic.config(24,48000);
  I2S.begin(1, 48000);
  // Create a task that will read the data
  xTaskCreatePinnedToCore(reader, "reader", 2048, NULL, 1, NULL, 1);
}

void loop () {
//  Serial.println(mic.readRegister(NAU85L40B_I2C_DEVICE_ID),HEX);
//  delay(1000);
//  Serial.println(mic.readRegister(NAU85L40B_ALC_CONTROL_4),HEX);
//  delay(1000);
}
