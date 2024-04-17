/**
   @file AnalogMicSerialPlotterFFT.ino
   @author rakwireless.com
   @brief This example reads PCM audio datas from the analog microphones by I2S, and prints
   out the FFT transfer samples to the Serial console. The Serial Plotter built into the
   Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note 
   1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
   2) This example uses the RAK18040 analog microphone module.
   @version 0.2
   @date 2024-03-25
   @copyright Copyright (c) 2024
*/

#include <Arduino.h>
#include "audio.h"

NAU85L40B   MIC(0x1D);

int channels = 1;
// default PCM output frequency
static const int frequency = 16000;
int sampleBit = 16;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

// Audio sample buffers used for analysis and display
int approxBuffer[BUFFER_SIZE];   // ApproxFFT sample buffer
const double samplingFrequency = 16000;     // Frequency range plotted is 2.5kHz (sampling frequency/4)

int print_string[500] = {0};

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

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
  MIC.config(frequency, (MIC_CHANNEL1 ), I2S_SAMPLE_16BIT); //| MIC_CHANNEL2

  I2S.setI2Sformat(I2S_COMM_FORMAT_STAND_I2S);
  I2S.begin(channels, frequency, sampleBit);
  Serial.println("=====================FFT example =====================");
}

static uint8_t first_flag = 0;
void loop() {

  // Read data from microphone
  int sampleRead = I2S.read(sampleBuffer, sizeof(sampleBuffer));
  float Freq_with_Max_Amp;						   

  sampleRead = sampleRead >> 1; //each sample data with two byte
  // wait for samples to be read
  if (sampleRead > 0) {
    // Fill the buffers with the samples
    memset(approxBuffer, 0, sizeof(approxBuffer));

    for (int i = 0; i < BUFFER_SIZE; i++){
      approxBuffer[i] = sampleBuffer[i];
      //      Serial.println(approxBuffer[i]);
    }

    if (first_flag > 20)  //Discard the first 20 samples data
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
