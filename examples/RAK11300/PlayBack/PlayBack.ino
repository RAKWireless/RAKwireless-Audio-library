/**
 * @file PlayBack.ino
 * @author rakwireless.com
 * @brief This example use RAK18060 Play mono audio file data with sampling rate of 22.05KHz 
 * and sampling depth of 16 bits.
 * @How to convert WAV file to HEX format .h file can use the online tool in the link.
 * https://tomeko.net/online_tools/file_to_hex.php?lang=en
 * @version 0.1
 * @date 2022-06-20
 * @copyright Copyright (c) 2022
 */
#include "audio.h"
#include "sound.h"

TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

const int sampleRate = 22050; // sample rate in Hz
int audio_length = 0;
int wavHead = 44;   //WAV file head 44 Bytes

void setup()
{   
  pinMode(WB_IO2,OUTPUT);
  digitalWrite(WB_IO2,HIGH);   
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
  
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
  }

  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed\r\n");
  }

  AMP_Left.set_pcm_channel(LeftMode);
  AMP_Right.set_pcm_channel(RightMode);
    
  i2s.setBitsPerSample(16);   //Set SampleBits 16
 // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }
  audio_length = sizeof(sound_buff)/2;   //The sampling bit depth is 16 bits contains two bytes
  Serial.println("=====================================");
  Serial.println("start play");
  Serial.println("=====================================");  
}

void loop()
{  
  int i=0;  
  for(i=(wavHead/2);i<audio_length;i++)
  {
     int16_t left_channel = sound_buff[i*2+1];
     left_channel = (left_channel<<8)|sound_buff[i*2];  
     
     int16_t right_channel = left_channel;  //copy left channel data to the right channel.
     i2s.write(left_channel,right_channel);
  }  
  while(1);   //If comment out this line can repeat play
}
