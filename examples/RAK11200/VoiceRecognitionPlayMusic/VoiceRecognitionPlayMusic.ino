/**
   @file VoiceRecognitionPlayMusic.ino
   @author rakwireless.com
   @brief This example shows how to use voice commands to control WisCore to play WAV audio files from SD card. You can use voice command words to control play song, switch song, pause, play, volume up, volume down.
   This example only supports playing WAV audio files, and does not support WAV audio files with 8-bit sampling and WAV audio files with a sampling frequency exceeding 44.1KHz.
   @note This example need use the RAK18003 module and RAK18060/1.
   @version 0.1
   @date 2022-8-4
   @copyright Copyright (c) 2022
**/
#include <Arduino.h>
#include "soc/rtc_wdt.h"
#include "audio.h"
#include "sound.h"
#include <SPI.h>
//#include <SD.h>
#include "SD.h"

Audio rak_audio;

#include "esp_task_wdt.h"

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

int i2s_channels = 1;

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

TAS2560 AMP_Left;
TAS2560 AMP_Right;

SemaphoreHandle_t uart_xMutex = NULL; //
QueueHandle_t integerQueue;

TaskHandle_t playMusicHandle;
TaskHandle_t recognizedHandle;
TaskHandle_t playHandle;

int cmd_case = 0;
int send_id = 0;

volatile int play_flag = 0;
volatile int playHandle_flag = 0;
volatile int i2s_init_flag = 0;

typedef enum {
  PLAY_STOP = 0,
  PLAY_MUSIC = 1,
  PLAY_PAUSE = 2,
  PLAY_RESUME = 3,
  PLAY_NULL = 4
} play_enum;

play_enum g_playState = PLAY_NULL;
//--------------------------------------------------------------
#define MUSIC_LIST  50
char playList[50][25];
volatile int musicPos = 0;
volatile int musicCount = 0;

void RAK18003Init(void);
void AMP_init(void);
void set_AMP_mute();
void set_AMP_unmute();
void increase_volume(int step);
void decrease_volume(int step);
void play_sound();
void playMusicTask(void *pvParameters);
void voiceRecognizedTask(void *pvParameters);
void playTask(void *pvParameters);

void setup()
{
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

  Serial.println("=========RAK11200 audio Test========");
  RAK18003Init();
  AMP_init();
  set_AMP_mute();
  SD_init();
  delay(100);
  recognition_init();
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  integerQueue = xQueueCreate(1, sizeof(int));
  if (integerQueue != NULL)
  {
    if (!xTaskCreate(playMusicTask, "playMusic task", 1024 * 6, NULL, 3, &playMusicHandle))
    {
      Serial.println("creat speaker task  failed");
    }

    if (!xTaskCreate(voiceRecognizedTask, "recognized task", 1024 * 12, NULL, 1, &recognizedHandle))
    {
      Serial.println("creat recognized task  failed");
    }

    if (!xTaskCreate(playTask, "play task", 1024 * 10, NULL, 2, &playHandle))
    {
      Serial.println("creat play task  failed");
    }
  }
  else
  {
    Serial.println("creat integer queue failed");
  }

}
void loop()
{
  vTaskDelay(1);//
}
void increase_volume(int step)
{
  int volume = rak_audio.getVolume();
  volume = volume + step;
  if (volume > 21) volume = 21;
  rak_audio.setVolume(volume);
  Serial.printf("current volume is:%d\r\n", volume);
}
void decrease_volume(int step)
{
  int volume = rak_audio.getVolume();
  volume = volume - step;
  if (volume < 1) volume = 1;
  rak_audio.setVolume(volume);
  Serial.printf("current volume is:%d\r\n", volume);
}
void play_sound()
{
  int16_t sample[2] = {0};
  int16_t *p_word = NULL;
  I2S.setSampleBit(16);
  I2S.begin(i2s_channels, 44100);
  uint32_t sound_size = sizeof(sound_buff) / 2;
  set_AMP_unmute();
  Serial.println("start play");
  for (int i = 0; i < sound_size; i++)
  {
    p_word = &sample[0];
    ((uint8_t *)p_word)[1] = sound_buff[i * 2 + 1];
    ((uint8_t *)p_word)[0] = sound_buff[i * 2];

    if (i2s_channels == 1)
    {
      i++;
      p_word = &sample[1];
      ((uint8_t *)p_word)[1] = sound_buff[i * 2 + 1];
      ((uint8_t *)p_word)[0] = sound_buff[i * 2];
    }
    else
    {
      sample[1] = sample[0];
    }

    uint32_t s32 = rak_audio.Gain(sample); //
    I2S.write((void *) &s32, sizeof(uint32_t));
  }
  //  delay();//vTaskDelay(100);
  I2S.clearDMA();
  I2S.end();
  set_AMP_mute();
}

void playMusicTask(void *pvParameters)
{
  (void) pvParameters;
  int last_playState = 0;
  Serial.println("playMusicTask creat");
  for (;;)
  {
    if (xQueueReceive(integerQueue, &cmd_case, ( TickType_t ) 1000 ) == pdPASS) //portMAX_DELAY
    {
      //      SD_CS_high();
      last_playState = g_playState;
      g_playState = PLAY_NULL;
      if (last_playState == PLAY_MUSIC)
      {
        set_AMP_mute();
        I2S.clearDMA();
        Serial.printf("suspend play music\r\n");
      }
      Serial.printf("cmd_case:%d\r\n", cmd_case);
      switch (cmd_case)
      {
        case 100:
          Serial.println("Hey RAK Star");
          play_sound();
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            reconfig_i2s();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 101:
          Serial.println(" Hey Helium");
          play_sound();
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 102:
          Serial.println(" Hey RAK Cloud");
          play_sound();
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 1000:    //Play Music
          Serial.printf("play:[%d]:%s\r\n", musicPos, playList[musicPos]);
          play_wav(playList[musicPos]);     //  play_wav("test.wav"); //play test.wav
          g_playState = PLAY_MUSIC;
          break;
        case 1001:    //Stop Music
          Serial.println("playMusicTask Stop Music");
          g_playState = PLAY_STOP;
          break;
        case 1002:    //Previous Song
          Serial.println("playMusicTask Previous Song");
          musicPos--;
          if (musicPos < 0) musicPos = musicCount - 1;
          Serial.printf("play:[%d]:%s\r\n", musicPos, playList[musicPos]);
          play_wav(playList[musicPos]);
          g_playState = PLAY_MUSIC;
          break;
        case 1003:     //Next song
          Serial.println("playMusicTask Next song");
          musicPos++;
          if (musicPos >= musicCount)  musicPos = 0;
          Serial.printf("play:[%d]:%s\r\n", musicPos, playList[musicPos]);
          play_wav(playList[musicPos]);
          g_playState = PLAY_MUSIC;
          break;
        case 1004:     //Volume Up
          Serial.println("playMusicTask Volume Up");
          increase_volume(2);
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 1005:      //Volume Down
          Serial.println("playMusicTask Volume Down");
          decrease_volume(2);
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 1006:      //Stop
          Serial.println("playMusicTask Stop");
          g_playState = PLAY_MUSIC; //stop_play();
          break;
        case 1007:      //Pause
          Serial.println("playMusicTask Pause");
          //          g_playState = PLAY_PAUSE;
          g_playState = PLAY_NULL;
          if (last_playState == PLAY_MUSIC)
          {
            play_pause();
          }
          break;
        case 1008:      //Next
          Serial.println("playMusicTask Next");
          musicPos++;
          if (musicPos >= musicCount)  musicPos = 0;
          Serial.printf("play:[%d]:%s\r\n", musicPos, playList[musicPos]);
          play_wav(playList[musicPos]);
          g_playState = PLAY_MUSIC;
          break;
        case 1009:      //Resume  Pause
          Serial.println("playMusicTask Resume");
          //          g_playState = PLAY_RESUME;
          if (last_playState == PLAY_MUSIC)
          {
            g_playState = PLAY_MUSIC;
            set_AMP_unmute();
          }
          break;
        case 1010:      //Previous
          Serial.println("playMusicTask Previous");
          musicPos--;
          if (musicPos < 0) musicPos = musicCount - 1;
          Serial.printf("play:[%d]:%s\r\n", musicPos, playList[musicPos]);
          play_wav(playList[musicPos]);
          g_playState = PLAY_MUSIC;
          break;
        case 1011:      //Volume Up
          Serial.println("playMusicTask Volume Up");
          increase_volume(2);
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        case 1012:      //Volume Down
          Serial.println("playMusicTask Volume Down");
          decrease_volume(2);
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
        default:
          if (last_playState == PLAY_MUSIC)
          {
            set_AMP_unmute();
            g_playState = PLAY_MUSIC;
          }
          break;
      }
      cmd_case = 0;
    }
    vTaskDelay(5);
  }
  Serial.println("playMusicTask  task uninit");
}
void playTask(void *pvParameters)
{
  (void) pvParameters;
  Serial.println("play task creat");
  for (;;)
  {

    if (g_playState == PLAY_MUSIC)
    {
      play_music();
    }
    else if (g_playState == PLAY_STOP)
    {
      g_playState = PLAY_NULL;
      stop_play();
    }
    //    else if (g_playState == PLAY_PAUSE)
    //    {
    //      g_playState = PLAY_NULL;
    //      play_pause();
    //    }
    //    else if (g_playState == PLAY_RESUME)
    //    {
    //      g_playState = PLAY_MUSIC;
    //      set_AMP_unmute();
    //    }
    else if (g_playState == PLAY_NULL)
    {
      vTaskDelay(5);
    }
    else
    {
      vTaskDelay(5);
    }

  }

}
void RAK18003Init(void)
{
  while (!Expander1.begin())
  {
    Serial.println("Did not find IO Expander Chip1");
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(200);
  }

  while (!Expander2.begin())
  {
    Serial.println("Did not find IO Expander Chip2");
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(200);
  }
  Expander1.pinMode(0, INPUT);    //SD check
  Expander1.pinMode(1, INPUT);    //MIC check
  Expander1.pinMode(2, OUTPUT);   //MIC CTR1
  Expander1.pinMode(3, OUTPUT);   //MIC CTR2
  Expander1.pinMode(4, INPUT);    //AMP check
  Expander1.pinMode(5, INPUT);    //AMP CTR1  AMP INT  check
  Expander1.pinMode(6, OUTPUT);   //AMP CTR2
  Expander1.pinMode(7, OUTPUT);   //AMP CTR3
  Expander1.pinMode(8, INPUT);    //DSP check
  Expander1.pinMode(9, INPUT);    //DSP CTR1  DSP int
  Expander1.pinMode(10, INPUT);   //DSP CTR2  DSP ready
  Expander1.pinMode(11, OUTPUT);  //DSP CTR3  DSP reset
  Expander1.pinMode(12, OUTPUT);  //DSP CTR4  not use
  Expander1.pinMode(13, OUTPUT);  //DSP CTR5  not use
  Expander1.pinMode(14, OUTPUT);  //NOT USE
  Expander1.pinMode(15, OUTPUT);  //NOT USE

  Expander2.pinMode(0, OUTPUT);  //CORE  SPI CS1   DSP CS
  Expander2.pinMode(1, OUTPUT);  //CORE  SPI CS2
  Expander2.pinMode(2, OUTPUT);  //CORE  SPI CS3
  Expander2.pinMode(3, OUTPUT);  //PDM switch CTR    1 to dsp   0 to core
  Expander2.pinMode(4, OUTPUT);  //not use
  Expander2.pinMode(5, OUTPUT);  //not use
  Expander2.pinMode(6, OUTPUT);  //not use
  Expander2.pinMode(7, OUTPUT);  //not use
  Expander2.pinMode(8, OUTPUT);  //not use
  Expander2.pinMode(9, OUTPUT);  //not use
  Expander2.pinMode(10, OUTPUT); //not use
  Expander2.pinMode(11, OUTPUT); //not use
  Expander2.pinMode(12, OUTPUT); //not use
  Expander2.pinMode(13, OUTPUT); //not use
  Expander2.pinMode(14, OUTPUT); //not use
  Expander2.pinMode(15, OUTPUT); //not use

  Expander2.digitalWrite(3, 0);   //set the PDM data direction from MIC to WisCore

  while (Expander1.digitalRead(0) == 1) //Check SD card
  {
    Serial.println("There is no SD card on the RAK18003 board, please check !");
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(200);
  }

  while (Expander1.digitalRead(4) == 0) //Check if the RAK18060 AMP board is connected on the RAK18003
  {
    Serial.println("There is no RAK18060 AMP board, please check !");
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(LED_GREEN, HIGH);
    delay(200);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(LED_GREEN, LOW);
    delay(200);
  }

  if (Expander1.digitalRead(1) == 0) //Check if the microphone board is connected on the RAK18003
  {
    Serial.println("There is no microphone board, please check !");
  }
}
