/**
   @file RAK1806X_test_RAK18003.ino
   @author rakwireless.com
   @brief This example shows how to configure the RAK18060/1 AMP board using.
   @note This example need use RAK18003 module.
   @version 0.1
   @date 2022-11-07
   @copyright Copyright (c) 2022
*/
#include "audio.h"
#include "sound.h"

#include <mbed.h>
#include <rtos.h>
#include <cmsis_os.h>
#include <pico/time.h>
#include "drivers/Timer.h"
#include "platform/mbed_wait_api.h"

using namespace rtos;
using namespace mbed;

Audio rak_audio;

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

// Create the I2S port using a PIO state machine
I2S i2s(OUTPUT);

const int sampleRate = 22050; // sample rate in Hz
int sampleBit = 16;
int audio_length = 0;

//SemaphoreHandle_t interruptSemaphore;
Mutex i2c_mutex;

Thread _thread_handle_irq(osPriorityHigh, 4096);  //osPriorityAboveNormal1
/** Thread id for irq event thread */
osThreadId _irq_task_thread = NULL;

volatile uint8_t int_flag = 0;
void irq_task(void);
void error_irq(void);
void i2s_config();
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
  rak_audio.setVolume(12);   //The volume level can be set to 0-21
  RAK18003Init();
  pinMode(WB_IO1, INPUT_PULLUP); //    INPUT
  i2s_config();
  Serial.println("=====================================");
  // Attach interrupt for Arduino digital pin
  attachInterrupt(digitalPinToInterrupt(WB_IO1), error_irq, CHANGE); //HIGH
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);

}

void loop()
{
  _thread_handle_irq.start(callback(irq_task));
  _thread_handle_irq.set_priority(osPriorityHigh);  //osPriorityAboveNormal3
  while (1)
  {
    i2c_mutex.lock();
    AMP_Left.set_unmute();    //Unmute AMP
    AMP_Right.set_unmute();   //Unmute AMP
    i2c_mutex.unlock();

    Serial.println("start play");
    int audio_length = sizeof(sound_buff) / 2; //
    int16_t sample[2] = {0};
    for (int i = 0; i < audio_length; i++)
    {
      sample[0] = sound_buff[i * 2 + 1];
      sample[0] = (sample[0] << 8) | sound_buff[i * 2];
      sample[1] = sample[0];  //copy left channel data to the right channel.

      int32_t s32 = rak_audio.Gain(sample); // vosample2lume;
      int16_t left_channel = (s32 >> 16) & 0xffff;
      int16_t right_channel = s32 & 0xffff;
      i2s.write(left_channel, right_channel);
    }
    delay(10);
    i2c_mutex.lock();
    AMP_Left.set_mute();    //Unmute AMP
    AMP_Right.set_mute();   //Unmute AMP
    i2c_mutex.unlock();
    delay(2000);
    //    yield();
  }
}
void i2s_config()
{
  i2s.setBitsPerSample(sampleBit);   //Set SampleBits 16
  // start I2S at the sample rate with 16-bits per sample
  if (!i2s.begin(sampleRate)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // do nothing
  }
}
void RAK18003Init(void)
{
  if (!Expander1.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip1, please check !");
    delay(200);
  }

  if (!Expander2.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip2, please check !");
    delay(200);
  }
  Expander1.pinMode(0, INPUT);    //SD check
  Expander1.pinMode(1, INPUT);    //MIC check
  Expander1.pinMode(2, INPUT);    //MIC CTR1
  Expander1.pinMode(3, INPUT);    //MIC CTR2
  Expander1.pinMode(4, INPUT);    //AMP check
  Expander1.pinMode(5, INPUT);    //AMP CTR1  alarm int
  Expander1.pinMode(6, INPUT);    //AMP CTR2
  Expander1.pinMode(7, INPUT);    //AMP CTR3
  Expander1.pinMode(8, INPUT);    //DSP check
  Expander1.pinMode(9, INPUT);    //DSP CTR1  DSP int
  Expander1.pinMode(10, INPUT);   //DSP CTR2  DSP ready
  Expander1.pinMode(11, OUTPUT);  //DSP CTR3  DSP reset
  Expander1.pinMode(12, INPUT);   //DSP CTR4  not use
  Expander1.pinMode(13, INPUT);   //DSP CTR5  not use
  Expander1.pinMode(14, INPUT);   //NOT USE
  Expander1.pinMode(15, INPUT);   //NOT USE

  //  Expander1.digitalWrite(14, 0);    //set chip 1 not use pin output low
  //  Expander1.digitalWrite(15, 0);    //set chip 1 not use pin output low

  Expander2.pinMode(0, OUTPUT);  //CORE  SPI CS1 for DSPG CS
  Expander2.pinMode(1, OUTPUT);  //CORE  SPI CS2
  Expander2.pinMode(2, OUTPUT);  //CORE  SPI CS3
  Expander2.pinMode(3, OUTPUT);  //PDM switch CTR    1 to dsp   0 to core
  Expander2.pinMode(4, INPUT);  //not use
  Expander2.pinMode(5, INPUT);  //not use
  Expander2.pinMode(6, INPUT);  //not use
  Expander2.pinMode(7, INPUT);  //not use
  Expander2.pinMode(8, INPUT);  //not use
  Expander2.pinMode(9, INPUT);  //not use
  Expander2.pinMode(10, INPUT); //not use
  Expander2.pinMode(11, INPUT); //not use
  Expander2.pinMode(12, INPUT); //not use
  Expander2.pinMode(13, INPUT); //not use
  Expander2.pinMode(14, INPUT); //not use
  Expander2.pinMode(15, INPUT); //not use

  Expander2.digitalWrite(0, 1);  //set SPI CS1 High
  Expander2.digitalWrite(1, 1);  //set SPI CS2 High
  Expander2.digitalWrite(2, 1);  //set SPI CS3 High

  Expander2.digitalWrite(3, 0);   //set the PDM data direction from MIC to WisCore

  // if(Expander1.digitalRead(0) == 1)  //Check SD card
  // {
  //   Serial.println("There is no SD card on the RAK18003 board, please check !");
  // }

  //  if (Expander1.digitalRead(1) == 0) //Check if the microphone board is connected on the RAK18003
  //  {
  //    Serial.println("There is no microphone board, please check !");
  //    delay(500);
  //  }

  if (Expander1.digitalRead(4) == 0) //Check if the RAK18060 AMP board is connected on the RAK18003
  {
    Serial.println("There is no RAK18060 AMP board, please check !");
  }

  // if(Expander1.digitalRead(8) == 0)  //Check if the RAK18080 DSPG board is connected on the RAK18003
  // {
  //   Serial.println("There is no RAK18080 DSPG board, please check !");
  // }

}
void error_irq(void)
{
  if (_irq_task_thread != NULL)
  {
    osSignalSet(_irq_task_thread, 0x1);
  }
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
void irq_task(void)
{
  _irq_task_thread = osThreadGetId();
  Serial.println("irq_task created");
  for (;;)
  {
    // Wait for event
    osSignalWait(0x1, osWaitForever);
    int state = 0;
    i2c_mutex.lock();
    state = Expander1.digitalRead(5);
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
    i2c_mutex.unlock();
    yield();
  }
}
