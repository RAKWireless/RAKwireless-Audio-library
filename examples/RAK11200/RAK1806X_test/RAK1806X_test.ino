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

TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

int channels = 1;   //mono
static const int frequency = 22050;   //samplerate 22.05KHz

SemaphoreHandle_t interruptSemaphore;
void irq_task(void *pvParameters);
volatile uint8_t int_flag = 0;
void error_irq(void);

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

  I2S.setSampleBit(16);
  I2S.begin(channels, frequency);
  Serial.println("=====================================");
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

  interruptSemaphore = xSemaphoreCreateBinary();
  if (interruptSemaphore != NULL) {
    xTaskCreate(irq_task, "irq_error", 1024 * 4, NULL, 2, NULL );
    // Attach interrupt for Arduino digital pin
    attachInterrupt(digitalPinToInterrupt(WB_IO1), error_irq, CHANGE);  //RISING
  }
}

void loop()
{
  AMP_Left.set_unmute();    //Unmute AMP
  AMP_Right.set_unmute();   //Unmute AMP
  //  int writeByte = I2S.write((void *)sound_buff, sizeof(sound_buff));
  int16_t sample[2] = {0};
  int16_t *p_word = NULL;
  uint32_t sound_size = sizeof(sound_buff) / 2;
  Serial.println("start play");
  for (int i = 0; i < sound_size; i++)
  {
    p_word = &sample[0];
    ((uint8_t *)p_word)[1] = sound_buff[i * 2 + 1];
    ((uint8_t *)p_word)[0] = sound_buff[i * 2];

    i++;
    p_word = &sample[1];
    ((uint8_t *)p_word)[1] = sound_buff[i * 2 + 1];
    ((uint8_t *)p_word)[0] = sound_buff[i * 2];

    uint32_t s32 = rak_audio.Gain(sample); //
    I2S.write((void *) &s32, sizeof(uint32_t));
  }
  delay(100);
  AMP_Left.set_mute();  //mute AMP
  AMP_Right.set_mute(); //mute AMP
  I2S.clearDMA();
  delay(1000);
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
      int state = digitalRead(WB_IO1);
      //      Serial.printf("WB_IO1:%d\r\n", state);
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
      }
    }
    vTaskDelay(10);
  }
}
