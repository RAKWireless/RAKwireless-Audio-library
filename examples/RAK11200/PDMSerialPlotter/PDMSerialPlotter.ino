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
int channels = 1;
// default PCM output frequency
static const int frequency = 16000;
 
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

void setup() {

  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 3000)
    {
      delay(100);      
    }
    else
    {
      break;
    }
  }

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  // default PCM output frequency
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  }
  
  Serial.println("=====================================");
  Serial.println("PDM test");
  Serial.println("=====================================");
}

void loop() {

   // Read data from microphone 
  int sampleRead = PDM.read(sampleBuffer, sizeof(sampleBuffer));

  sampleRead = sampleRead>>1;
  // wait for samples to be read
  if (sampleRead>0) {  
    // print samples to the serial monitor or plotter
    for (int i = 0; i < sampleRead; i++) {
      Serial.println(sampleBuffer[i]);
    }
    sampleRead = 0;
  }
}
