/**
   @file AnalogMicPlayBack.ino
   @author rakwireless.com
   @brief This example shows live playback using an analog microphone and AMP speakers.
   @note This example need use the RAK18040 analog microphone module and RAK18060/1 module.
   @version 0.1
   @date 2022-11-09
   @copyright Copyright (c) 2022
*/
#include "audio.h"

Audio rak_audio;
TAS2560 AMP_Left;
TAS2560 AMP_Right;
#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

#define I2S_DATA_BLOCK_WORDS    512

volatile uint8_t rx_flag = 0;
volatile uint8_t tx_flag = 0;

i2s_channels_t channels =  Stereo ; //Right、   Left;//
static const int frequency = 16000;  // default PCM output frequency
int sampleBit = 16;
uint32_t readbuff[I2S_DATA_BLOCK_WORDS] = {0};
uint32_t writebuff[I2S_DATA_BLOCK_WORDS] = {0};

NAU85L40B   MIC(0x1D);

void i2s_config();
void rx_irq();
void tx_irq();
void set_AMP_mute();
void set_AMP_unmute();

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);

  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 1000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }

  // initialize I2S with:
  i2s_config();
  MIC.begin();
  MIC.config(frequency, (MIC_CHANNEL1 | MIC_CHANNEL2), I2S_SAMPLE_16BIT);
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed,please check RAK18040\r\n");
    delay(500);
  }
  AMP_Left.set_pcm_channel(LeftMode);

  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed,please check RAK18040\r\n");
    delay(500);
  }
  AMP_Right.set_pcm_channel(RightMode);
  rak_audio.setVolume(12); //The volume level can be set to 0-21
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
  Serial.println("==============================================");
}
void loop()
{
  if (rx_flag == 1)
  {
    rx_flag = 0;
    memcpy(writebuff, readbuff, sizeof(readbuff));
    for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
    {
      writebuff[i] = rak_audio.Gain(writebuff[i]);
    }
    I2S.write(&writebuff, sizeof(writebuff));
  }
}
void rx_irq()
{
  rx_flag = 1;
  I2S.read(&readbuff, sizeof(readbuff));
}
void tx_irq()
{
  tx_flag = 1;
  I2S.write(&writebuff, sizeof(writebuff));
}
void i2s_config()
{
  I2S.setSize(I2S_DATA_BLOCK_WORDS);
  //  I2S.TxIRQCallBack(tx_irq);
  I2S.RxIRQCallBack(rx_irq);
  I2S.begin(channels, frequency, sampleBit);
  I2S.start();
}
void set_AMP_mute()
{
  AMP_Left.set_mute();
  AMP_Right.set_mute();
}
void set_AMP_unmute()
{
  AMP_Left.set_unmute();
  AMP_Right.set_unmute();
}
