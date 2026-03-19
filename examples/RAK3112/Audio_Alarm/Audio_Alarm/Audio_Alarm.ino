/*
   @file Audio_Alarm.ino
   @author rakwireless.com
   @brief When the microphone detects that the PDM value of the sound exceeds the set threshold,
          the serial port will print out an alarm message, and the LED will light up.
   @version 0.1
   @date 2025-09-20
   @copyright Copyright (c) 2025
*/
#include "audio.h"

int channels = 1;
// default PCM output frequency
static const int frequency = 16000;

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];
int audio_threshold = 200; //This value determines the threshold for triggering the warning.
int value = 0;
int alarm_count = 0;

void setup()
{
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  Serial.begin(115200);
  delay(10);

  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  // default PCM output frequency
  if (!PDM.begin(channels, frequency)) 
  {
    Serial.println("Failed to start PDM!");
    while (1) 
      yield();
  }
  Serial.println("noise alarm example");
}

int abs_int(short data)
{
  if (data > 0) 
    return data;
  else 
    return (0 - data);
}

void loop()
{
  // Read data from microphone
  int samplesRead = PDM.read(sampleBuffer, sizeof(sampleBuffer));
  samplesRead = samplesRead / 2;
  // wait for samples to be read
  if (samplesRead > 0) 
  {
    uint32_t sum = 0;
    // print samples to the serial monitor or plotter
    for (int i = 0; i < BUFFER_SIZE; i++) 
    {
      sum = sum + abs(sampleBuffer[i]);
    }
    int aver = sum / BUFFER_SIZE;
    if (aver > audio_threshold)
    {
      alarm_count++;
    }
    samplesRead = 0;

    if (alarm_count > 5)
    {
      alarm_count = 0;
      Serial.println("Alarm");
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      delay(200);
      /*You can add your warning handling tasks here.*/
    }
    else
    {
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, LOW);
    }
  }
}
