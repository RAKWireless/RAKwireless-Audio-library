/**
   @file RAK1806X_test.ino
   @author rakwireless.com
   @brief This example shows how to configure the RAK18060/1 AMP board using.
   @version 0.1
   @date 2022-11-07
   @copyright Copyright (c) 2022
*/
#include "audio.h"
#include "sound.h"

Audio rak_audio;

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

#define I2S_DATA_BLOCK_WORDS    512

uint32_t *p_word = NULL;
const int sampleRate = 22050; // sample rate in Hz
int sampleBit = 16;
int audio_length = 0;
uint32_t writebuff[I2S_DATA_BLOCK_WORDS] = {0};
volatile uint8_t tx_flag = 1;

SemaphoreHandle_t interruptSemaphore;
SemaphoreHandle_t i2c_mutex;

volatile uint8_t int_flag = 0;
void irq_task(void *pvParameters);
void error_irq(void);
void i2s_config();
void tx_irq();
void RAK18003Init(void);

void setup() {
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
  // Initialize Serial for debug output
  time_t timeout = millis();
  Serial.begin(115200);
  while (!Serial)
  {
    if ((millis() - timeout) < 2000)
    {
      delay(100);
    }
    else
    {
      break;
    }
  }

  Serial.println("=====================================");
  Serial.println("RAK1806X test");
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
  }

  AMP_Left.set_gain(15);  //Gain setting range 0-15 db
  AMP_Left.set_pcm_channel(LeftMode);
  AMP_Left.set_mute();

  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed\r\n");
  }

  AMP_Right.set_gain(15); //Gain setting range 0-15 db
  AMP_Right.set_pcm_channel(RightMode);
  AMP_Right.set_mute();
  rak_audio.setVolume(6);   //The volume level can be set to 0-21
  pinMode(WB_IO1, INPUT);

  i2s_config();
  Serial.println("=====================================");
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  i2c_mutex = xSemaphoreCreateMutex();
  if (i2c_mutex != NULL) {
    Serial.println("i2c_mutex created");
  }
  interruptSemaphore = xSemaphoreCreateBinary();
  if (interruptSemaphore != NULL) {
    xTaskCreate(irq_task, "irq_error", 1024 * 4, NULL, 2, NULL );
    // Attach interrupt for Arduino digital pin
    attachInterrupt(digitalPinToInterrupt(WB_IO1), error_irq, CHANGE);  //RISING
  }
}

void loop()
{
  if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
  {
    AMP_Left.set_unmute();    //Unmute AMP
    AMP_Right.set_unmute();   //Unmute AMP
    xSemaphoreGive(i2c_mutex);
  }

  Serial.println("start play");
  uint32_t *p_word = NULL;
  int sound_size = sizeof(sound_buff);
  int audio_length = sound_size / 4; //The DMA transmit data width is 32 bits contains 4 bytes data
  int data_pos = 0;
  while (data_pos < audio_length)
  {
    if (tx_flag == 1)
    {
      tx_flag = 0;
      for (int i = 0; i < I2S_DATA_BLOCK_WORDS; i++)
      {
        p_word = &writebuff[i];
        int pos = data_pos << 2; //data_pos*4;
        if (pos < sound_size)
        {
          ((uint8_t *)p_word)[0]  = sound_buff[pos];    //left channel data
          ((uint8_t *)p_word)[1]  = sound_buff[pos + 1]; //left channel data

          ((uint8_t *)p_word)[2]  = sound_buff[pos + 2]; //right channel data
          ((uint8_t *)p_word)[3]  = sound_buff[pos + 3]; //right channel data
        }
        else
        {
          ((uint8_t *)p_word)[0]  = 0;  //left channel data
          ((uint8_t *)p_word)[1]  = 0;  //left channel data

          ((uint8_t *)p_word)[2]  = 0;  //right channel data
          ((uint8_t *)p_word)[3]  = 0;  //right channel data
        }
        data_pos++;
        writebuff[i] = rak_audio.Gain(writebuff[i]);
      }//for
    }// tx_flag
  }// while
  delay(10);
  memset(writebuff, 0, sizeof(writebuff));
  if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
  {
    AMP_Left.set_mute();    //Unmute AMP
    AMP_Right.set_mute();   //Unmute AMP
    xSemaphoreGive(i2c_mutex);
  }
  delay(2000);
}
void tx_irq()  ///< Pointer to the buffer with data to be sent.
{
  tx_flag = 1;
  I2S.write(&writebuff, sizeof(writebuff));
}
void i2s_config()
{
  I2S.setSize(I2S_DATA_BLOCK_WORDS);
  I2S.TxIRQCallBack(tx_irq);
  I2S.begin(Left, sampleRate, sampleBit);
  I2S.start();
}
void error_irq(void)
{
  int_flag = 1;
}
void alarm(unsigned int value)
{
  if (value & TAS2560_INTM_OVRI)
  {
    Serial.println("the speaker over current detected");
  }
  if (value & TAS2560_INTM_AUV)
  {
    Serial.println("the speaker under voltage detected");
  }
  if (value & TAS2560_INTM_CLK2)
  {
    Serial.println("the speaker clock error 2 detected");
  }
  if (value & TAS2560_INTM_OVRT)
  {
    Serial.println("the speaker die over-temperature detected");
  }
  if (value & TAS2560_INTM_BRNO)
  {
    Serial.println("the speaker brownout detected");
  }
  if (value & TAS2560_INTM_CLK1)
  {
    Serial.println("the speaker clock error 1 detected");
  }
  if (value & TAS2560_INTM_MCHLT)
  {
    Serial.println("the speaker modulator clock halt detected");
  }
  if (value & TAS2560_INT_WCHLT)
  {
    Serial.println("the speaker WCLK clock halt detected");
  }
}
void irq_task(void *pvParameters)
{
  (void) pvParameters;
  for (;;) {
    if (int_flag == 1)
    {
      int_flag = 0;
      int state = 0;
      if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
      {
        state = digitalRead(WB_IO1);
        //      Serial.printf("AMP INT:%d\r\n", state);
        if (state == 1)
        {
          Serial.println("amp left ==================");
          unsigned int alarm1 = AMP_Left.read_alarm();
          Serial.printf("alarm1:%0X\r\n", alarm1);
          if (alarm1)
          {
            alarm(alarm1);
            AMP_Left.clear_alarm();
          }
          Serial.println("amp right ==================");
          unsigned int alarm2 = AMP_Right.read_alarm();
          Serial.printf("alarm2:%0X\r\n", alarm2);
          if (alarm2)
          {
            alarm(alarm2);
            AMP_Right.clear_alarm();
          }
        }//state
        xSemaphoreGive(i2c_mutex);
      }//i2c_mutex
    }//int_flag
    vTaskDelay(10);
  }
}
