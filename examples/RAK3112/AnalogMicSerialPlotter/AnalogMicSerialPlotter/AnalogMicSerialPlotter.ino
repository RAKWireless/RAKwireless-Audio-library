/**
   @file AnalogMicSerialPlotter.ino
   @author rakwireless.com
   @brief This example reads PCM audio datas from the analog microphones by I2S, and prints
   out the samples to the Serial console. The Serial Plotter built into the
   Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note 
    1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
    2) This example need use the RAK18040 analog microphone module.
   @version 0.1
   @date 2025-10-07
   @copyright Copyright (c) 2025
*/

#include "audio.h"

NAU85L40B   MIC(0x1D);

int channels = 2;
// default PCM output frequency
static const int frequency = 16000;
int sampleBit = 16;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
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

  MIC.begin();
  MIC.config(frequency, (MIC_CHANNEL1|MIC_CHANNEL2 ), I2S_SAMPLE_16BIT); //

  I2S.setPins(WB_IO6,WB_IO5,SDA1,SCL1);
  if (!I2S.begin(I2S_MODE_STD, frequency, (i2s_data_bit_width_t)sampleBit, (i2s_slot_mode_t)channels)) 
  {
    Serial.println("Failed to initialize I2S!");
  }
}

void loop() {

  // Read data from microphone
  int availableBytes = I2S.available();
  int bytesRead = I2S.readBytes((char *)sampleBuffer, availableBytes);
  int samplesRead = bytesRead / 2;
  
  // wait for samples to be read
  if (samplesRead > 0)
  {
    // print samples to the serial monitor or plotter
    for (int i = 0; i < samplesRead; i += channels)
    {
      if (channels == 2)
      {
        short left = sampleBuffer[i];
        short right = sampleBuffer[i + 1];
        Serial.print("left:"); 
        Serial.print(left);
        
        Serial.print(",");
        
        Serial.print("right:"); 
        Serial.print(right);
        Serial.println();
      }
    }
    // Clear the read count
    samplesRead = 0;
  }
}
