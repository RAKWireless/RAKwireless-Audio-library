/**
 * @file PDMSerialPlotter.ino
 * @author rakwireless.com
 * @brief This example reads audio data from the PDM microphones, and prints
 * out the samples to the Serial console. The Serial Plotter built into the
 * Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
 
#include "audio.h"

uint32_t g_lpdwLicense[46];
uint32_t test[46];
uint32_t *ptr;
void setup() {
  test[3] = 0x12344321;
  Serial.begin(115200);
  while(!Serial);
  Audio_Handle.begin();
  Audio_Handle.StoreLicense(test);
  delay(2000);
  //while (!Serial);
  Audio_Handle.GetLicense(g_lpdwLicense);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
}

void loop() {
    for (int i = 0; i < 46; i++) {
      Serial.print(g_lpdwLicense[i],HEX);
      Serial.print(" ");
    }
    Serial.println(" ");
    ptr = Audio_Handle.GetLicenseAddr();
    Serial.println((uint32_t)ptr,HEX);
    delay(1000);
}
