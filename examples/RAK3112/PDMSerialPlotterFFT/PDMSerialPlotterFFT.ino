/**
   @file PDMSerialPlotterFFT.ino
   @author rakwireless.com
   @brief This example reads audio data from the PDM microphones, and prints
   out the FFT transfer samples to the Serial console. The Serial Plotter built into the
   Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note 
    1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
    
   @version 0.1
   @date 2025-09-20
   @copyright Copyright (c) 2025
*/

#include <Arduino.h>
#include "audio.h"

int channels = 1;
// default PCM output frequency
static const int frequency = 16000;
// buffer to read samples into, each sample is 16-bits
short *sampleBuffer = nullptr;

// Audio sample buffers used for analysis and display
int *approxBuffer = nullptr;
const double samplingFrequency = 16000;     // Frequency range plotted is 2.5kHz (sampling frequency/4)

int *print_string = nullptr;

void onPDMdata();

void setup() 
{

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
  // - a 16 kHz sample rate
  // default PCM output frequency
  if (!PDM.begin(channels, frequency)) 
  {
    Serial.println("Failed to start PDM!");
    while (1) yield();
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

  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
}

static uint8_t first_flag = 0;
void loop() 
{
  // Read data from microphone
  int samplesRead = PDM.read(sampleBuffer, BUFFER_SIZE * sizeof(short));
  float Freq_with_Max_Amp;
  
  samplesRead = samplesRead >> 1; //each sample data with two byte
  // wait for samples to be read
  if (samplesRead > 0) 
  {
    // Fill the buffers with the samples
    memset(approxBuffer, 0, BUFFER_SIZE*sizeof(int));

    for (int i = 0; i < samplesRead; i++) 
    {
      approxBuffer[i] = sampleBuffer[i];
      //Serial.println(approxBuffer[i]);
    }

    if (first_flag > 10)  //Discard the first 10 samples of data
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
