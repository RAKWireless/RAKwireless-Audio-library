/*
   @file AnalogMicAudio_Alarm.ino
   @author rakwireless.com
   @brief The analog microphone detects the noise value and lights up the LED on the WisBase
    when it exceeds the set threshold.
   @note This example need use the RAK18040 analog microphone module.
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
int audio_threshold = 800;
int aver = 0;
int g_alarm = 0;
int abs_int(short data);

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
  MIC.config(frequency, (MIC_CHANNEL1 | MIC_CHANNEL2 ), I2S_SAMPLE_16BIT); 

  // MIC.writeRegister(NAU85L40B_FEPGA1, 0x70);
  // uint16_t regValue = MIC.readRegister(NAU85L40B_FEPGA1);
  // Serial.printf("FEPGA1寄存器值: 0x%04X\n", regValue);
  // Serial.printf("PGA增益设置: %ddB\n", ((regValue >> 4) & 0x07) * 6);

  I2S.setPins(WB_IO6,WB_IO5,SDA1,SCL1);
  if (!I2S.begin(I2S_MODE_STD, frequency, (i2s_data_bit_width_t)sampleBit, (i2s_slot_mode_t)channels)) 
  {
    Serial.println("Failed to initialize I2S!");
  }
  delay(500);
  Serial.println("This is an noise alarm example.");
}

void loop()
{
  // Read data from analog microphone by I2S
  int availableBytes = I2S.available();
  int bytesRead = I2S.readBytes((char *)sampleBuffer, availableBytes);
  int samplesRead = bytesRead / 2;

  for (int i = 0; i < samplesRead; i += 2)
  {
    sampleBuffer[i / 2] = sampleBuffer[i];  //左声道 left
    // Serial.println(sampleBuffer[i]);
  }
    
  samplesRead = samplesRead / 2;
  
  // wait for samples to be read
  if (samplesRead > 0)
  {
    uint32_t sum = 0;
    // print samples to the serial monitor or plotter
    for (int i = 0; i < samplesRead; i++)
    {
      sum = sum + abs(sampleBuffer[i]);
    }
    aver = sum / samplesRead;
    samplesRead = 0;
    Serial.printf("-------aver = %d--------\r\n", aver);
    if (aver > audio_threshold)
    {
      g_alarm++;
      Serial.printf("aver %d\r\n", aver);
      Serial.printf("Alarm %d\r\n", g_alarm);
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_GREEN, HIGH);
      delay(1000);
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, LOW);
    }
  }
}

int abs_int(short data)
{
  if (data > 0) return data;
  else return (0 - data);
}
