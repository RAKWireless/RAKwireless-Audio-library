/**
   @file PDMSerialPlotter_RAK18003.ino
   @author rakwireless.com
   @brief This example reads audio data from the PDM microphones, and prints out the samples to the Serial console.
   The Serial Plotter built into the Arduino IDE can be used to plot the audio data (Tools -> Serial Plotter)
   @note 
    1) To use the ploter function Arduino IDE version 1.x is needed (version 2.x plotter does not work with this code)
    2) This example need use the RAK18003 module.If using stereo, you need to use RAK18033 module and two PDM microphones.
   @version 0.1
   @date 2025-09-28
   @copyright Copyright (c) 2025
*/

#include "audio.h"
#include <math.h>

// Phase 2 full-duplex smoke test: set FULL_DUPLEX_TEST to 1 to bring
// up the RAK18060/18061 amplifier alongside the PDM mic and stream a
// 1 kHz tone out the speaker while logging mic samples. Verifies that
// the dual-port driver actually runs PDM and STD I2S concurrently —
// before Phase 2 this would have caused the second `begin()` to tear
// down the first peripheral.
#define FULL_DUPLEX_TEST     1
#define AMP_LEFT_ADDRESS     0x4c
#define AMP_RIGHT_ADDRESS    0x4f
#define TONE_HZ              1000

TPT29555   Expander1(0x23);
TPT29555   Expander2(0x25);

int channels = 1; //1 mono  2 stereo
// default PCM output frequency
static const int frequency = 48000;

#if FULL_DUPLEX_TEST
TAS2560 AMP_Left;
TAS2560 AMP_Right;
// Size the tone block to an integer multiple of the tone period so
// the block boundary never lands mid-cycle (otherwise you get an
// audible periodic click at rates where SAMPLE_RATE / TONE_HZ does
// not divide the fixed block length evenly — e.g. 1600 samples @
// 48 kHz is 33.33 cycles of 1 kHz, not an integer).
static const int TONE_CYCLES_PER_BLOCK = 40;
static const int TONE_BLOCK_SAMPLES =
    (frequency / TONE_HZ) * TONE_CYCLES_PER_BLOCK;
static int16_t toneBlock[TONE_BLOCK_SAMPLES];
#endif

// buffer to read samples into, each sample is 16-bits
short sampleBuffer[BUFFER_SIZE];

void RAK18003Init(void);

void setup() {

  pinMode(WB_IO2, OUTPUT);
  digitalWrite(WB_IO2, HIGH);
  delay(500);
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, HIGH);

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
  // initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate
  // default PCM output frequency
  if (!PDM.begin(channels, frequency))
  {
    Serial.println("Failed to start PDM!");
    while (1)
      yield();
  }

#if FULL_DUPLEX_TEST
  // Bring the speaker path up *after* PDM. Pre-Phase 2 this call
  // would have re-bound the single shared I2S peripheral to STD mode
  // and made PDM.read() return constant 0xFFFF. With the dual-port
  // fix it now binds to I2S_NUM_1 and leaves PDM running on
  // I2S_NUM_0.
  if (!AMP_Left.begin(AMP_LEFT_ADDRESS, (uint32_t)frequency)) {
    Serial.println("AMP_Left.begin failed");
  } else {
    AMP_Left.set_pcm_channel(LeftMode);
    AMP_Left.set_gain(5);
  }
  if (!AMP_Right.begin(AMP_RIGHT_ADDRESS, (uint32_t)frequency)) {
    Serial.println("AMP_Right.begin failed");
  } else {
    AMP_Right.set_pcm_channel(RightMode);
    AMP_Right.set_gain(5);
  }
  I2S.setPins(WB_IO6, WB_IO5, SDA1, SCL1);
  if (!I2S.begin(I2S_MODE_STD, frequency, I2S_DATA_BIT_WIDTH_16BIT,
                 (i2s_slot_mode_t)channels)) {
    Serial.println("I2S.begin (STD) failed");
  }
  const float w = 2.0f * 3.14159265358979f * (float)TONE_HZ / (float)frequency;
  for (int i = 0; i < TONE_BLOCK_SAMPLES; i++) {
    toneBlock[i] = (int16_t)(0.3f * 32767.0f * sinf(w * i));
  }
#endif

  Serial.println("=====================================");
#if FULL_DUPLEX_TEST
  Serial.println("PDM + speaker full-duplex test");
#else
  Serial.println("PDM test");
#endif
  Serial.println("=====================================");
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_BLUE, HIGH);
}

void loop()
{
#if FULL_DUPLEX_TEST
  // Push a tone block on the speaker every loop iteration. Non-
  // blocking — if the I2S TX DMA isn't ready it returns 0.
  I2S.write((const uint8_t *)toneBlock, sizeof(toneBlock));
#endif

  // Read data from microphone
  int samplesRead = PDM.read(sampleBuffer, BUFFER_SIZE * sizeof(short));

  samplesRead = samplesRead / 2;  //Each short data contains two Bytes
  // wait for samples to be read
  if (samplesRead > 0) 
  {
    // print samples to the serial monitor or plotter
    for (int i = 0; i < samplesRead; i++) 
    {
      if (channels == 2)
      {
        Serial.print("L:");
        Serial.print(sampleBuffer[i]);
        Serial.print(" R:");
        i++;
      }
      Serial.println(sampleBuffer[i]);
    }
    samplesRead = 0;
  }
}
void RAK18003Init(void)
{
  while (!Expander1.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip1, please check !");
    delay(200);
  }

  while (!Expander2.begin())
  {
    Serial.println("Did not find RAK18003 IO Expander Chip2, please check !");
    delay(200);
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

  while (Expander1.digitalRead(1) == 0) //Check if the microphone board is connected on the RAK18003
  {
    Serial.println("There is no microphone board, please check !");
    delay(500);
  }

  // if(Expander1.digitalRead(4) == 0)  //Check if the RAK18060 AMP board is connected on the RAK18003
  // {
  //   Serial.println("There is no RAK18060 AMP board, please check !");
  // }

  // if(Expander1.digitalRead(8) == 0)  //Check if the RAK18080 DSPG board is connected on the RAK18003
  // {
  //   Serial.println("There is no RAK18080 DSPG board, please check !");
  // }

}
