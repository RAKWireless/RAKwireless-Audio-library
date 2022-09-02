/**
 * @file PDMSerialPlotterFFT.ino
 * @author rakwireless.com
 * @brief This example reads audio data from the PDM microphones, and prints
 * out the FFT transfer samples to the Serial console. The Serial Plotter built into the
 * Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
 * @note  This example need use the microphones that can supports higher sampling rate.
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
 
#include <arduino.h>
#include "audio.h"

int channels = 1;
// default PCM output frequency
static const int frequency = 48000;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

// Audio sample buffers used for analysis and display
int approxBuffer[BUFFER_SIZE];   // ApproxFFT sample buffer
const double samplingFrequency = 200000;     // Frequency range plotted is 2.5kHz (sampling frequency/4)

void onPDMdata();

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial);  

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 200 kHz sample rate
  // default PCM output frequency
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  }
  
}
static uint8_t first_flag = 0;
void loop() {
  
  // Read data from microphone 
  int sampleRead = PDM.read(sampleBuffer, sizeof(sampleBuffer));

  sampleRead = sampleRead>>1;   //each sample data with two byte
  // wait for samples to be read
  if (sampleRead>0) {  
    // Fill the buffers with the samples
    memset(approxBuffer,0,sizeof(approxBuffer));
  
    for (int i = 0; i < BUFFER_SIZE; i++) {
       approxBuffer[i] = sampleBuffer[i];
       Serial.println(approxBuffer[i]);
    } 

    if(first_flag>10)
    {
      Approx_FFT(approxBuffer, BUFFER_SIZE, samplingFrequency); 
      for (int j=0; j<BUFFER_SIZE; j++){
      Serial.println(approxBuffer[j]);
      }  
      delay(500);
      while(1);
    }
    first_flag++;    
    sampleRead = 0;
  }
}
