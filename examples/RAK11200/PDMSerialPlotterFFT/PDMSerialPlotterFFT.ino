/**
   @file PDMSerialPlotterFFT.ino
   @author rakwireless.com
   @brief This example reads audio data from the PDM microphones, and prints
   out the FFT transfer samples to the Serial console. The Serial Plotter built into the
   Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note  
   1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
   @version 0.2
   @date 2024-03-25
   @copyright Copyright (c) 2024
*/

#include <Arduino.h>
#include "audio.h"

int channels = 1;
// default PCM output frequency
static const int frequency = 16000;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

// Audio sample buffers used for analysis and display
int approxBuffer[BUFFER_SIZE];   // ApproxFFT sample buffer
const double samplingFrequency = 16000;     //must match frequency above!
// Frequency range plotted is 2.5kHz (sampling frequency/4)

int print_string[500] = {0};

void onPDMdata();

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);

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
  // default PCM output frequency
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  } 

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
}

static uint8_t first_flag = 0;
void loop() {

  // Read data from microphone
  int sampleRead = PDM.read(sampleBuffer, sizeof(sampleBuffer));
  float Freq_with_Max_Amp;

  sampleRead = sampleRead >> 1; //each sample data with two byte
  // wait for samples to be read
  if (sampleRead > 0) {
    // Fill the buffers with the samples
    memset(approxBuffer, 0, sizeof(approxBuffer));

    for (int i = 0; i < BUFFER_SIZE; i++) {
      approxBuffer[i] = sampleBuffer[i];
      //Serial.println(approxBuffer[i]);
    }

    if (first_flag > 10)  //Discard the first 10 samples of data
    {
      Freq_with_Max_Amp = Approx_FFT(approxBuffer, BUFFER_SIZE, samplingFrequency); // default return frequency with max aplitude
      //      for (int j=0; j<BUFFER_SIZE; j++){
      //      Serial.println(approxBuffer[j]);
      //      }
      memset(print_string, 0, sizeof(print_string));
      memcpy(print_string, approxBuffer, sizeof(approxBuffer));

      for (int j = 0; j < 500; j++)
      {
        Serial.print("Data:"); Serial.print(print_string[j]);Serial.print(", ");
        Serial.print("Peak_Freq"); Serial.print(Freq_with_Max_Amp); Serial.print(":"); Serial.print(Freq_with_Max_Amp);Serial.print(", ");
        Serial.println();
      }
      delay(3000); // wait 3 seconds so plot can be read
    }
    else
    {
      first_flag++;
    }
    sampleRead = 0;
  }
}
