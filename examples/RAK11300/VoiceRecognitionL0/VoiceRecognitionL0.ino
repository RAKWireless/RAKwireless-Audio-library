/**
 * @file VoiceRecognitionL0.ino
 * @author rakwireless.com
 * @brief This example shows that use the WisCore and microphone to recognize the specific simple commands.
 * @version 0.1
 * @date 2022-06-6
 * @copyright Copyright (c) 2022
 */
#include "audio.h"
#include <CDSpotter.h>
#include "RAKwireless_Demo_pack_WithTxt_Lv0.h"
#define DSPOTTER_MODEL           g_lpdwRAKwireless_Demo_pack_WithTxt_Lv0
#define	COMMAND_GROUP_CHOOSE	 2
#define COMMAND_STAGE_TIMEOUT    6000                    // The minimum recording time in ms when there is no result at command stage.
#define COMMAND_STAGE_REPEAT     1                       // When it is 1, sample code will recognize repeatly at command stage till to timeout.
                                                         // Otherwise, it will switch to trigger stage immediately after command recognized.

// default number of output channels
static const char channels = 1;

// default PCM output frequency
static const int frequency = 16000;

#define SAMPLE_READ_NUM     64
// Buffer to read samples into, each sample is 16-bits
short sampleBuffer[SAMPLE_READ_NUM];

// Number of audio samples read
volatile int samplesRead;
//VR
static CDSpotter g_hDSpotter;
int g_nCurrentStage = 0;
int g_nBlink = 0;
uint32_t *g_ptr;//For license


void setup() {
  int nMempoolSize = 0, nErr = -1;
  byte *pMemPool = 0;
  int nActiveCommandGroup = COMMAND_GROUP_CHOOSE;

  //Serial.println("Before PDM init");
  //rp2040
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  while (!Serial);
 
  //For Rak4630
  //PDM.setPins(21, 4, -1);
  //PDM.setBufferSize(256);
  //For Rak4630
  
  // Configure the data receive callback
  PDM.onReceive(onPDMdata);
  
  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and 24 on the Portenta Vision Shield
  //For Rak4630 not useful
  // PDM.setGain(80);

  // Initialize PDM with:
  // - one channel (mono mode)
  // - 16 kHz sample rate
  if (!PDM.begin(right, frequency)) {
    Serial.print("Failed to start PDM!");
    while (1);
  }

  //Get License from flash
  Audio_Handle.begin();
  g_ptr = Audio_Handle.GetLicenseAddr();
  Serial.println(CDSpotter::GetSerialNumber());
  //DSpotter
  nMempoolSize = CDSpotter::GetMemoryUsage(DSPOTTER_MODEL,60);
  Serial.print("DSpotter mem usage");
  Serial.println(nMempoolSize);
  pMemPool = (byte*)malloc(sizeof(byte)*nMempoolSize);
  if(!pMemPool)
    Serial.print("allocate DSpotter mempool failed");
  //nErr = g_hDSpotter.Init(g_lpdwLicense,sizeof(g_lpdwLicense),DSPOTTER_MODEL,60,pMemPool,nMempoolSize);
  nErr = g_hDSpotter.Init(g_ptr,LICEENSE_LENGTH * sizeof(uint32_t),DSPOTTER_MODEL,60,pMemPool,nMempoolSize);
  if(nErr!=CDSpotter::Success)
  {
    Serial.print("DSpotter err: ");
    Serial.println(nErr);
    if(nErr==CDSpotter::LicenseFailed)
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
  if(nErr!=CDSpotter::Success){
	Serial.println("Set active commands group failed, using default");
	nActiveCommandGroup = 1;
  }
  //show command
  for (int nStage = 0; nStage < 2; nStage++)
  {
    char szCommand[64];
    int nID;
    int nGroupChoose;
    if(nStage==0)
	{
      Serial.println("The list of Trigger words: ");
	  nGroupChoose = 0;
	}
    else
	{
      Serial.println("The list of Command words: ");
	  nGroupChoose = nActiveCommandGroup;
	}
    for(int i = 0; i < g_hDSpotter.GetCommandCount(nGroupChoose); i++)
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
}

void loop() {
  int nRes = -1,nDataSize = 0;
  int nFil, nSG, nScore, nID;
  int nStage = 0;
  short sSample[512];
  char pCommand[64];
  
  //Serial.println("loop");
  
  //audio data lost resource not enough to do VR
//  if(g_hDSpotter.GetRecordLostCount()>0) //need remark this line too
//    Serial.println("drop data");///LEDblink();

  //Do VR
  nRes = g_hDSpotter.DoRecognition(&nStage);

  //VR return Key word get keyword info
  if(nRes==CDSpotter::Success)
  {
	if(g_nBlink)
	{
		digitalWrite(LED_BUILTIN, LOW);
		g_nBlink = 0;
	}
	else
	{
		digitalWrite(LED_BUILTIN, HIGH);
		g_nBlink = 1;
	}
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
  }
  //Check VR stage
  if(nStage != g_nCurrentStage)
  {
    g_nCurrentStage  = nStage;
    if(nStage == CDSpotter::TriggerStage)
      Serial.println("VR Switch to Trigger Stage");
    else if(nStage == CDSpotter::CommandStage)
      Serial.println("VR Switch to Command Stage");
  }
  // Clear the read count
  samplesRead = 0;
  
}

/**
   Callback function to process the data from the PDM microphone.
   NOTE: This callback is executed as part of an ISR.
   Therefore using `Serial` to print messages inside this function isn't supported.
 * */
void onPDMdata() {
  // Query the number of available bytes
  int nLeftRBBufferSize;
  //int bytesAvailable = PDM.available();
  
  //Serial.println(bytesAvailable);
  //delay(1000);
  // Read into the sample buffer
  PDM.read(sampleBuffer, SAMPLE_READ_NUM*2);

  // 16-bit, 2 bytes per sample
  //samplesRead = bytesAvailable / 2;
  //put to Ringbuffer and wait for do VR
  g_hDSpotter.PutRecordData(sampleBuffer, SAMPLE_READ_NUM);
  
}
