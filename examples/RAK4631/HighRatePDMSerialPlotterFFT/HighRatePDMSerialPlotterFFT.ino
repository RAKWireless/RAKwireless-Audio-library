/**
 * @file HighRatePDMSerialPlotterFFT.ino
 * @author rakwireless.com
 * @brief This example reads audio data from the PDM microphones, and prints
 * out the FFT transfer samples to the Serial console. The Serial Plotter built into the
 * Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
 * @note  This example need use the microphones that can supports higher sampling rate.
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
 
#include "audio.h"

// Audio sample buffers used for analysis and display
int approxBuffer[BUFFER_SIZE];   // ApproxFFT sample buffer
const double samplingFrequency = 16000;    
int channels = 1;
// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

int print_string[500]={0};

volatile uint8_t read_flag = 0;
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
  while (!Serial)
 
  // configure the data receive callback
  PDM.onReceive(onPDMdata);
  PDM.setPins(PDM_DATA_PIN, PDM_CLK_PIN, PDM_PWR_PIN);
  // optionally set the gain, defaults to 20
  PDM.setGain(30);
  // initialize PDM with:
  // - one channel (mono mode)
  // - a 200 kHz sample rate
  if (!PDM.begin(channels, PCM_48000)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  } 
 Serial.println("=====================FFT example =====================");  
}

static int first_flag = 0;
void loop() {
  // wait for samples to be read
  if (read_flag == 1) {
    read_flag = 0;

    // Fill the buffers with the samples
    for (int i = 0; i < BUFFER_SIZE; i++) {
       approxBuffer[i] = sampleBuffer[i];
//       Serial.println(approxBuffer[i]);
    }     

    if(first_flag>0)   //because the all of first IRQ read data is 0
    {    
      Approx_FFT(approxBuffer, BUFFER_SIZE, samplingFrequency);   
//      for (int j=0; j<BUFFER_SIZE; j++){
//        Serial.println(approxBuffer[j]);
//      }  
      memset(print_string,0,sizeof(print_string));
      memcpy(print_string,approxBuffer,sizeof(approxBuffer));    

      for (int j=0; j<500; j++)
      {
        Serial.println(print_string[j]);
      }  
      delay(1000);
    }    
    first_flag=1;      
  }
}
void onPDMdata() {
  // read into the sample buffer
  PDM.read(sampleBuffer, BUFFER_SIZE*2);
  read_flag = 1;  
}