/**
   @file AnalogMicSerialPlotterFFT.ino
   @author rakwireless.com
   @brief This example reads PCM audio datas from the analog microphones by I2S, and prints
   out the FFT transfer samples to the Serial console. The Serial Plotter built into the
   Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note 
    1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
    2) This example need use the RAK18040 analog microphone module.
   @version 0.1
   @date 2025-10-07
   @copyright Copyright (c) 2025
*/

#include <Arduino.h>
#include "audio.h"

NAU85L40B   MIC(0x1D);

int channels = 1;
// default PCM output frequency
static const int frequency = 16000;
int sampleBit = 16;
// buffer to read samples into, each sample is 16-bits
short *sampleBuffer = nullptr;

// Audio sample buffers used for analysis and display
int *approxBuffer = nullptr;
const double samplingFrequency = 16000;     // Frequency range plotted is 2.5kHz (sampling frequency/4)

int *print_string = nullptr;

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
  MIC.config(frequency, (MIC_CHANNEL1), I2S_SAMPLE_16BIT); //| MIC_CHANNEL2

  I2S.setPins(WB_IO6,WB_IO5,SDA1,SCL1);
  if (!I2S.begin(I2S_MODE_STD, frequency, (i2s_data_bit_width_t)sampleBit, I2S_SLOT_MODE_MONO)) 
  {
    Serial.println("Failed to initialize I2S!");
  }

  sampleBuffer = (short *)malloc(BUFFER_SIZE * sizeof(short));
  approxBuffer = (int *)malloc(BUFFER_SIZE * sizeof(int));
  print_string = (int *)malloc(500 * sizeof(int));

  if (!sampleBuffer || !approxBuffer || !print_string) 
  {
    Serial.println("Failed to allocate buffers!");
    while (1) yield();
  } 
  else 
  {
    Serial.println("Buffers allocated OK");
  }
  
  Serial.println("=====================FFT example =====================");
}

static uint8_t first_flag = 0;
void loop() {

  // Read data from microphone
  int availableBytes = I2S.available();
  int bytesRead = I2S.readBytes((char *)sampleBuffer, availableBytes);
  int samplesRead = bytesRead / 2;
  float Freq_with_Max_Amp;

  // wait for samples to be read
  if (samplesRead > 0) {
    // Fill the buffers with the samples
    memset(approxBuffer, 0, BUFFER_SIZE*sizeof(int));

    for (int i = 0; i < samplesRead; i += 2)
    {
      approxBuffer[i / 2] = sampleBuffer[i];
//      Serial.println(sampleBuffer[i]);
    }
    
    samplesRead = samplesRead / 2;

    if (first_flag > 20)  //Discard the first 20 samples data
    {
      Freq_with_Max_Amp = Approx_FFT(approxBuffer, samplesRead, samplingFrequency);
      //      for (int j=0; j<BUFFER_SIZE; j++){
      //      Serial.println(approxBuffer[j]);
      //      }
      memset(print_string, 0, 500 * sizeof(int));
      memcpy(print_string, approxBuffer, 500 * sizeof(int));

      for (int j = 0; j < 500; j++)
      {
        Serial.print("Data:"); 
        Serial.print(print_string[j]);
        Serial.print(", ");
        Serial.print("Peak_Freq"); 
        Serial.print(Freq_with_Max_Amp); 
        Serial.println();
      }
      delay(1000);
    }
    else
    {
      first_flag++;
    }
    samplesRead = 0;
  }
}
