/**
 * @file PlayBack.ino
 * @author rakwireless.com
 * @brief This example use RAK18060 Play mono audio file data with sampling rate of 22.05KHz 
 * and sampling depth of 16 bits.
 * @How to convert WAV file to HEX format .h file can use the online tool in the link.
 * https://tomeko.net/online_tools/file_to_hex.php?lang=en
 * @note This example need use the battery power for the WisBase.
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

int channels = 1;   //mono
static const int frequency = 22050;   //samplerate 22.05KHz

void setup() {
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
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
  
  Serial.println("=====================================");  
  if(!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
  }
 
  if(!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed\r\n");
  } 

  AMP_Left.set_pcm_channel(LeftMode);
  AMP_Right.set_pcm_channel(RightMode);
 
  I2S.setSampleBit(16); 
  I2S.begin(channels, frequency);
  Serial.println("start play");
  Serial.println("=====================================");
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
}

void loop() 
{
  int writeByte = I2S.write((void *)sound_buff,sizeof(sound_buff));
  if(writeByte>0)
  {
    delay(150); //Waiting for DMA to transfer remaining data
    Serial.println("paly accomplish");
    I2S.stop();
  }
}
