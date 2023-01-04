/**
 * @file PlayBack.ino
 * @author rakwireless.com
 * @brief This example use RAK18060 Play mono audio file data with sampling rate of 22.05KHz 
 * and sampling depth of 16 bits.
 * @How to convert WAV file to HEX format .h file can use the online tool in the link.
 * https://tomeko.net/online_tools/file_to_hex.php?lang=en
 * @note Need to increase the battery when using.
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

#define I2S_DATA_BLOCK_WORDS    512

uint32_t *p_word = NULL;
const int sampleRate = 22050; // sample rate in Hz
int sampleBit = 16;
int audio_length = 0;
uint32_t writebuff[I2S_DATA_BLOCK_WORDS]= {0};
bool repeat_play = true;

volatile uint8_t tx_flag = 1; 
int data_pos = 11;
void i2s_config();
void tx_irq(); 
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
  
  audio_length = sizeof(sound_buff)/4;   //The DMA transmit data width is 32 bits contains 4 bytes data 
  Serial.println("start play");
  Serial.println("=====================================");
  delay(10);
  i2s_config();
}

void loop()
{    
  while(data_pos<audio_length)
  {
    if(tx_flag==1)
    {
      tx_flag = 0;  
      for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
      {
        p_word = &writebuff[i];      
        int pos = data_pos<<2;  //=data_pos*4
        if((pos+4)<sizeof(sound_buff))
        {          
          ((uint8_t *)p_word)[0]  = sound_buff[pos];    //left channel data
          ((uint8_t *)p_word)[1]  = sound_buff[pos+1];  //left channel data
          
          ((uint8_t *)p_word)[2]  = sound_buff[pos+2];  //right channel data
          ((uint8_t *)p_word)[3]  = sound_buff[pos+3];  //right channel data
        } 
        else
        {
          ((uint8_t *)p_word)[0]  = 0;  //left channel data
          ((uint8_t *)p_word)[1]  = 0;  //left channel data
          
          ((uint8_t *)p_word)[2]  = 0;  //right channel data
          ((uint8_t *)p_word)[3]  = 0;  //right channel data
        } 
        data_pos++; 
                     
       }//for      
      }// tx_flag  
    }// while  

    if(repeat_play == true)
      {            
        data_pos = 11;
      }
      else
      {
        I2S.stop();        
        while(1);
      }  
}

void tx_irq()  ///< Pointer to the buffer with data to be sent.
{
  tx_flag = 1;   
  I2S.write(&writebuff,sizeof(writebuff));            
}
void i2s_config()
{
  I2S.TxIRQCallBack(tx_irq);
//  I2S.begin(Left,sampleRate,sampleBit);
  I2S.begin(Right,sampleRate,sampleBit);
  I2S.start();
}