/**
   @file VoiceRecognitionL1.ino
   @author rakwireless.com
   @brief This example shows that use the WisCore and microphone to recognize the specific simple commands.
   @note Running this example requires WisCore written with a Cyberon license.
   This example need use the RAK18003 module.
   First you need select one command group, the group value can be 0-4.
        eg:If selected group 1 need to set "#define  COMMAND_GROUP_CHOOSE     1" in the code.
   The trigger words can be "Hey RAK Star"、"Hey Helium" or "Hey RAK Cloud".
   All command words will be printed on the serial port at the beginning.
   @version 0.1
   @date 2022-06-6
   @copyright Copyright (c) 2022
*/

#include "audio.h"
#include "sound.h"
//hardcode model in SDK for testing
#include <CDSpotter.h>
#include "RAKwireless_Demo_pack_WithTxt_Lv1.h"
#define DSPOTTER_MODEL           g_lpdwRAKwireless_Demo_pack_WithTxt_Lv1
#define	COMMAND_GROUP_CHOOSE	   2    //group value can be 0-4
#define COMMAND_STAGE_TIMEOUT    6000                    // The minimum recording time in ms when there is no result at command stage.
#define COMMAND_STAGE_REPEAT     1                       // When it is 1, sample code will recognize repeatly at command stage till to timeout.
// Otherwise, it will switch to trigger stage immediately after command recognized.

#define USE_STEREO //ESP32 using mono record audio quality is not enough, set stereo will be better
// default number of output channels
#ifdef USE_STEREO
static const char channels = 2;
#else
static const char channels = 1;
#endif

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address

Audio rak_audio;
TAS2560 AMP_Left;
TAS2560 AMP_Right;

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

QueueHandle_t integerQueue;
TaskHandle_t audioHandle;
TaskHandle_t recognizedHandle;

int i2s_sampleBit = 16;
int i2s_channels = 1;
int i2s_frequency = 16000;

int cmd_case = 0;
int send_id = 0;

// default PCM output frequency
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[256];

// Number of audio samples read
volatile int samplesRead;
//VR
static CDSpotter g_hDSpotter;
int g_nCurrentStage = 0;
int g_nBlink = 0;
int g_nAudioDrop = 0;
uint32_t *g_ptr;//For license

void RAK18003Init(void);
void AMP_init();
void play_sound();
void i2s_config();
void set_AMP_mute();
void set_AMP_unmute();
void voiceRecognizedTask(void *pvParameters);
void Audio_task(void *pvParameters);

void recognition_init() {
  int nMempoolSize = 0, nErr = -1;
  byte *pMemPool = 0;
  int nActiveCommandGroup = COMMAND_GROUP_CHOOSE;
  Serial.println("Before I2S init");
  //Get license from flash
  Audio_Handle.begin();
  g_ptr = Audio_Handle.GetLicenseAddr();
  Serial.println(CDSpotter::GetSerialNumber());
  //DSpotter
  nMempoolSize = CDSpotter::GetMemoryUsage(DSPOTTER_MODEL, 60);
  Serial.print("DSpotter mem usage");
  Serial.println(nMempoolSize);
  pMemPool = (byte*)malloc(sizeof(byte) * nMempoolSize);
  if (!pMemPool)
    Serial.print("allocate DSpotter mempool failed");

  //nErr = g_hDSpotter.Init(g_lpdwLicense,sizeof(g_lpdwLicense),DSPOTTER_MODEL,60,pMemPool,nMempoolSize);
  nErr = g_hDSpotter.Init(g_ptr, LICEENSE_LENGTH * sizeof(uint32_t), DSPOTTER_MODEL, 60, pMemPool, nMempoolSize);
  if (nErr != CDSpotter::Success)
  {
    Serial.print("DSpotter err: ");
    Serial.println(nErr);
    if (nErr == CDSpotter::LicenseFailed)
    {
      Serial.println("License err, please register under device ID for License and paste it in g_lpdwLicense");
      Serial.println(CDSpotter::GetSerialNumber());
    }
  }
  else
  {
    Serial.println("DSpotter init success");
  }

  //Add Set Group model API
  nErr = g_hDSpotter.SetActiveCommandGroup(nActiveCommandGroup);
  if (nErr != CDSpotter::Success) {
    Serial.println("Set active commands group failed, using default");
    nActiveCommandGroup = 1;
  }
  //show command
  for (int nStage = 0; nStage < 2; nStage++)
  {
    char szCommand[64];
    int nID;
    int nGroupChoose;
    if (nStage == 0)
    {
      Serial.println("The list of Trigger words: ");
      nGroupChoose = 0;
    }
    else
    {
      Serial.println("The list of Command words: ");
      nGroupChoose = nActiveCommandGroup;
    }
    for (int i = 0; i < g_hDSpotter.GetCommandCount(nGroupChoose); i++)
    {
      g_hDSpotter.GetCommand(nGroupChoose, i, szCommand, sizeof(szCommand), &nID);
      Serial.print(szCommand);
      Serial.print(" , ID = ");
      Serial.println(nID);
    }
    Serial.println("");
  }
  Serial.println("");
  //set 2 stage timeout
  g_hDSpotter.SetCommandStageProperty(COMMAND_STAGE_TIMEOUT, COMMAND_STAGE_REPEAT == 1);
  //Start VR
  g_hDSpotter.Start();
  Serial.println("Enter Trigger state");

  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1) yield();
  }
}
void setup() {
  
  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
  digitalWrite(LED_GREEN, HIGH);
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

  Serial.println("Before I2S init");
  RAK18003Init();
  AMP_init();
  set_AMP_mute();
  rak_audio.setVolume(21); //The volume level can be set to 0-21
  i2s_config();
  delay(100);
  
  recognition_init();
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_BLUE, LOW);

  integerQueue = xQueueCreate(1, sizeof(int));
  if (integerQueue != NULL)
  {
    //Create Audio Task otherwise audio data drop
    if (!xTaskCreate(Audio_task, "Audio", 1024 * 12, NULL, 3, &audioHandle))
    {
      Serial.println("Ups, that should never happen");
    }
    if (!xTaskCreate(voiceRecognizedTask, "recognized task", 1024 * 6, NULL, 2, &recognizedHandle))
    {
      Serial.println("creat recognized task  failed");
    }
  }
  else
  {
    Serial.println("creat integer queue failed");
  }
}

void loop() {

}
void i2s_config()
{
  I2S.setSampleBit(i2s_sampleBit);
  I2S.begin(i2s_channels, i2s_frequency);
}
void AMP_init()
{
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS))
  {
    Serial.printf("TAS2560 left init failed\r\n");
    delay(500);
  }
  AMP_Left.set_pcm_channel(LeftMode);
  if (!AMP_Right.begin(AMP_RIGTT_ADDRESS))
  {
    Serial.printf("TAS2560 rigth init failed\r\n");
    delay(500);
  }
  AMP_Right.set_pcm_channel(RightMode);
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
void play_sound()
{
  int16_t sample[2] = {0};
  int16_t *p_word = NULL;
  uint32_t sound_size = sizeof(sound_buff) / 2;
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
  I2S.clearDMA();
}

void Audio_task(void *pvParameters)
{
  (void) pvParameters;
  int nRes = -1, nDataSize = 0;
  int nFil, nSG, nScore, nID;
  int nStage = 0;
  char pCommand[64];
  //   if (!PDM.begin(channels, frequency)) {
  //    Serial.println("Failed to start PDM!");
  //    while (1) yield();
  //  }
  Serial.println("Audio_task task Start");
  for (;;)
  {
    size_t bytesRead = 0;
    samplesRead = sizeof(sampleBuffer) / 2;
    bytesRead = PDM.read((void*)sampleBuffer, sizeof(sampleBuffer));

    if (bytesRead > 0)
    {
      for (int i = 0; i < samplesRead; i++)
        sampleBuffer[i] <<= 2;
#ifdef USE_STEREO
      //need move stereo data to mono
      for (int i = 1; i < samplesRead; i++)
        sampleBuffer[i] = sampleBuffer[2 * i];
      g_hDSpotter.PutRecordData(sampleBuffer, samplesRead / 2);
#else
      g_hDSpotter.PutRecordData(sampleBuffer, samplesRead);
#endif

      //audio data lost resource not enough to do VR
      //      if (g_hDSpotter.GetRecordLostCount() > g_nAudioDrop)
      //      {
      //        g_nAudioDrop = g_hDSpotter.GetRecordLostCount();
      //        Serial.println("drop data");
      //      }
      //Do VR
      nRes = g_hDSpotter.DoRecognition(&nStage);
      //      Serial.printf("nRes:%d\r\n",nRes);
      //VR return Key word get keyword info
      if (nRes == CDSpotter::Success)
      {
        Serial.print("Detect ID: ");
        g_hDSpotter.GetRecogResult(&nID, pCommand, 20, &nScore, &nSG, &nFil);
        Serial.print(pCommand);
        Serial.print(" ");
        Serial.println(nID);
        Serial.print("SG: ");
        Serial.print(nSG);
        Serial.print(" Energy: ");
        Serial.print(nFil);
        Serial.print(" Score: ");
        Serial.println(nScore);

        send_id = nID;
        xQueueSend(integerQueue, (void *)&send_id, ( TickType_t ) portMAX_DELAY );
//        if (xQueueSend(integerQueue, (void *)&send_id, ( TickType_t ) portMAX_DELAY ) != pdPASS) //portMAX_DELAY
//        {
//          Serial.println("xQueueSend failed");
//        }
//        else
//        {
//          Serial.println("xQueueSend succeed");
//        }
      }
      //Check VR stage
      if (nStage != g_nCurrentStage)
      {
        g_nCurrentStage  = nStage;
        if (nStage == CDSpotter::TriggerStage)
        {
          Serial.println("VR Switch to Trigger Stage");
          digitalWrite(LED_GREEN, LOW);
          digitalWrite(LED_BLUE, LOW);
        }
        else if (nStage == CDSpotter::CommandStage)
        {
          Serial.println("VR Switch to Command Stage");
          digitalWrite(LED_GREEN, HIGH);
          digitalWrite(LED_BLUE, HIGH);
        }
      }

    } //bytesRead>0
    //    vTaskDelay(1);
  }//whhile
  PDM.end();
}
void voiceRecognizedTask(void *pvParameters)
{
  (void) pvParameters;
  //    i2s_config();
  //  play_sound();
  Serial.println("I2S recognized task Start");
  for (;;)
  {
    if (xQueueReceive(integerQueue, &cmd_case, ( TickType_t ) portMAX_DELAY ) == pdPASS) //portMAX_DELAY
    {
      Serial.printf("cmd_case:%d\r\n", cmd_case);
      switch (cmd_case)
      {
        case 100:
          Serial.println("Hey RAK Star");
          set_AMP_unmute();
          play_sound();
          set_AMP_mute();
          break;
        case 101:
          Serial.println(" Hey Helium");
          set_AMP_unmute();
          play_sound();
          set_AMP_mute();
          break;
        case 102:
          Serial.println(" Hey RAK Cloud");
          set_AMP_unmute();
          play_sound();
          set_AMP_mute();
          break;
        default: break;
      }
    }
    //    vTaskDelay(10);
  }
}

void RAK18003Init(void)
{
  if (!Expander1.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip1,please check!");
    delay(500);
  }

  if (!Expander2.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip2,please check!");
    delay(500);
  }
  Expander1.pinMode(0, INPUT);    //SD check
  Expander1.pinMode(1, INPUT);    //MIC check
  Expander1.pinMode(2, INPUT);    //MIC CTR1
  Expander1.pinMode(3, INPUT);    //MIC CTR2
  Expander1.pinMode(4, INPUT);    //AMP check
  Expander1.pinMode(5, INPUT);    //AMP CTR1
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

  if (Expander1.digitalRead(1) == 0) //Check if the microphone board is connected on the RAK18003
  {
    Serial.println("There is no microphone board, please check !");
    delay(500);
  }

  if (Expander1.digitalRead(4) == 0) //Check if the RAK18060 AMP board is connected on the RAK18003
  {
    Serial.println("There is no RAK18060 AMP board, please check !");
  }

  // if(Expander1.digitalRead(8) == 0)  //Check if the RAK18080 DSPG board is connected on the RAK18003
  // {
  //   Serial.println("There is no RAK18080 DSPG board, please check !");
  // }

}
