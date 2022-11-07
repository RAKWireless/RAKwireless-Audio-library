/**
   @file VoiceRecognitionL1_PlayBack.ino
   @author rakwireless.com
   @brief This example shows that use the WisCore and microphone to recognize the specific simple commands.
   When WisCore is awakened by the wake word, it will play a prompt sound.
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
#include <CDSpotter.h>
#include "sound.h"
#include "RAKwireless_Demo_pack_WithTxt_Lv1.h"
#define DSPOTTER_MODEL           g_lpdwRAKwireless_Demo_pack_WithTxt_Lv1
#define	COMMAND_GROUP_CHOOSE	   2     //group value can be 0-4
#define COMMAND_STAGE_TIMEOUT    6000                    // The minimum recording time in ms when there is no result at command stage.
#define COMMAND_STAGE_REPEAT     1                       // When it is 1, sample code will recognize repeatly at command stage till to timeout.
// Otherwise, it will switch to trigger stage immediately after command recognized.
#define PDM_DATA_PIN  21
#define PDM_CLK_PIN   4
#define PDM_PWR_PIN   -1
#define SAMPLE_SIZE   256

Audio rak_audio;
TAS2560 AMP_Left;
TAS2560 AMP_Right;

#define AMP_LEFT_ADDRESS    0x4c    //amplifier i2c address
#define AMP_RIGTT_ADDRESS   0x4f    //amplifier i2c address
#define I2S_DATA_BLOCK_WORDS    512
uint32_t *p_word = NULL;
uint32_t writebuff[I2S_DATA_BLOCK_WORDS] = {0};
volatile uint8_t tx_flag = 1;

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

QueueHandle_t integerQueue;
TaskHandle_t recognizedHandle;

int cmd_case = 0;
int send_id = 0;

// default number of output channels
static const char channels = 1;

// default PCM output frequency
static const int frequency = 16000;

// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[SAMPLE_SIZE];

// Number of audio samples read
volatile int samplesRead;
//VR
static CDSpotter g_hDSpotter;
int g_nCurrentStage = 0;
int g_nBlink = 0;
uint32_t *g_ptr;//For license

void RAK18003Init(void);
void tx_irq();
void AMP_init();
void play_sound();
void i2s_config();
void set_AMP_mute();
void set_AMP_unmute();
void voiceRecognizedTask(void *pvParameters);

void setup() {
  int nMempoolSize = 0, nErr = -1;
  byte *pMemPool = 0;
  int nActiveCommandGroup = COMMAND_GROUP_CHOOSE;

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(LED_GREEN, LOW);
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

  RAK18003Init();
  AMP_init();
  set_AMP_mute();
  rak_audio.setVolume(21); //The volume level can be set to 0-21
  i2s_config();
  delay(100);

  //For Rak4630
  PDM.setPins(PDM_DATA_PIN, PDM_CLK_PIN, PDM_PWR_PIN);
  //PDM.setBufferSize(256);
  //For Rak4630
  // Configure the data receive callback
  PDM.onReceive(onPDMdata);

  // Initialize PDM with:
  // - one channel (mono mode)
  // - 16 kHz sample rate
  if (!PDM.begin(channels, PCM_16000)) {
    Serial.print("Failed to start PDM!");
    while (1);
  }

  //Get license from flash
  Audio_Handle.begin();
  g_ptr = Audio_Handle.GetLicenseAddr();

  //DSpotter
  //DSpotter_Init_SetDebugCallBack(onDebugCallBack);
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
      Serial.println("License err, please check register under device ID for License");
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
  //  i2s_config();
  Serial.println("Enter Trigger state");

  taskENTER_CRITICAL();           //进入临界区

  integerQueue = xQueueCreate(1, sizeof(int));
  if (integerQueue != NULL)
  {
    if (!xTaskCreate(voiceRecognizedTask, "recognized task", 1024 * 10, NULL, 2, &recognizedHandle))
    {
      Serial.println("creat recognized task  failed");
    }
  }
  else
  {
    Serial.println("creat integer queue failed");
  }

  taskEXIT_CRITICAL();            //退出临界区
}

void loop() {

  int nRes = -1, nDataSize = 0;
  int nFil, nSG, nScore, nID;
  int nStage = 0;
  char pCommand[64];

  //Do VR
  nRes = g_hDSpotter.DoRecognition(&nStage);

  //VR return Key word get keyword info
  if (nRes == CDSpotter::Success)
  {
    Serial.print("Detect ID: ");
    g_hDSpotter.GetRecogResult(&nID, pCommand, sizeof(pCommand), &nScore, &nSG, &nFil);
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
    //    if (xQueueSend(integerQueue, (void *)&send_id, ( TickType_t ) portMAX_DELAY ) != pdPASS) //portMAX_DELAY
    //    {
    //      Serial.println("xQueueSend failed");
    //    }
    //    else
    //    {
    //      Serial.println("xQueueSend succeed");
    //    }
  }

  //Check VR stage
  if (nStage != g_nCurrentStage)
  {
    g_nCurrentStage  = nStage;
    if (nStage == CDSpotter::TriggerStage)
    {
      Serial.println("VR Switch to Trigger Stage");
      digitalWrite(LED_BLUE, LOW);
      digitalWrite(LED_GREEN, LOW);
    }
    else if (nStage == CDSpotter::CommandStage)
    {
      Serial.println("VR Switch to Command Stage");
      digitalWrite(LED_BLUE, HIGH);
      digitalWrite(LED_GREEN, HIGH);
    }
  }
  // Clear the read count
  samplesRead = 0;

  //  vTaskDelay(5);
}
void i2s_config()
{
  I2S.setSize(I2S_DATA_BLOCK_WORDS);
  I2S.TxIRQCallBack(tx_irq);
  I2S.begin(Left, 16000, 16);
  I2S.start();
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
void tx_irq()  ///< Pointer to the buffer with data to be sent.
{
  tx_flag = 1;
  I2S.write(&writebuff, sizeof(writebuff));
}

void play_sound()
{
  int audio_length = sizeof(sound_buff) / 4; //The DMA transmit data width is 32 bits contains 4 bytes data
  int sound_size = sizeof(sound_buff);
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
  memset(writebuff, 0, sizeof(writebuff));
  data_pos = 0;
}
/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int nLeftRBBufferSize;
  int bytesAvailable = SAMPLE_SIZE * 2; //PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);
  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;
  //Amplify volume
  /*for(int i=0;i<samplesRead;i++)
    sampleBuffer[i] <<= 3;*/
  //put to Ringbuffer and wait for do VR
  g_hDSpotter.PutRecordData(sampleBuffer, samplesRead);
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
void voiceRecognizedTask(void *pvParameters)
{
  (void) pvParameters;
  //  i2s_config();
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
