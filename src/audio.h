#ifndef AUDIO_H
#define AUDIO_H

#include "Arduino.h"
#include <stdint.h>
#include <string.h>
#include <PDM.h>
#include <I2S.h>
#include <SPI.h>
#include "tas2560.h"
#include "dspg.h"
#include "tpt29555.h"
#include "NAU85L40B.h"

#ifdef _VARIANT_RAK4630_
#include <nrf_nvic.h>
#define ID1 (0x10000060)
#define ID2 (0x10000064)

#define PDM_DATA_PIN  21
#define PDM_CLK_PIN   4
#define PDM_PWR_PIN   -1

#define BUFFER_SIZE   256
#define CYBERON_FILEPATH    "Cyberon_License"

#elif defined (_VARIANT_RAK11300_)
extern "C"
{
#include "hardware/flash.h"
#include "hardware/sync.h"
#include "pico/bootrom.h"
}
#include <LittleFS_Mbed_RP2040.h>

#define PDM_DATA_PIN  7
#define PDM_CLK_PIN   28
#define PDM_PWR_PIN   -1

#define BUFFER_SIZE   64
#define CYBERON_FILEPATH   MBED_LITTLEFS_FILE_PREFIX "/Cyberon_License.bin"
#elif defined (_VARIANT_RAK11200_)
#include <driver/i2s.h>
#include <Preferences.h>
#include <nvs.h>
#include <nvs_flash.h>
#include <esp_system.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "esp_now.h"

#define BUFFER_SIZE   256
#define DR_REG_BASE                       0x3ff5A000
#define BLK0_RDATA1_REG          (DR_REG_BASE + 0x004)
#define BLK0_RDATA2_REG          (DR_REG_BASE + 0x008)
#define CYBERON_FILEPATH    "Cyberon_License"   
#endif

// Flash
void init_flash(void);
void flash_write(char const *filepath, uint32_t *lic, uint32_t length);
void flash_read(char const *filepath, uint32_t *lic, uint32_t length);


#define LICEENSE_LENGTH   46


typedef enum { LEFTCHANNEL=0, RIGHTCHANNEL=1 } SampleIndex;

typedef enum { LOWSHELF = 0, PEAKEQ = 1, HIFGSHELF =2 } FilterType;
    typedef struct _filter{
        float a0;
        float a1;
        float a2;
        float b1;
        float b2;
    } filter_t;


class Audio{

public:
  // constructors
  Audio();
  void begin(void);
  void GetBoardId(char *array);
  void StoreLicense(uint32_t *lic);
  uint32_t* GetLicenseAddr();
  void GetLicense(uint32_t *lic);  
  uint32_t Cyberon_License[LICEENSE_LENGTH];

  void setBalance(int8_t bal); // bal -16...16
  void setVolume(uint8_t vol); // vol 22 steps, 0...21
  uint8_t getVolume();
  int32_t Gain(int16_t s[2]);
  int32_t Gain(uint32_t data);
  // uint32_t Gain(uint32_t data);
  // bool playSample(int16_t sample[2]);
  int calculateIIR_Gain(uint32_t sampleRate);
  void  IIR_calculateCoefficients(uint32_t sampleRate,int8_t G0, int8_t G1, int8_t G2);  // Infinite Impulse Response (IIR) filters
  int16_t* IIR_filterChain0(int16_t iir_in[2], bool clear= false);  // Infinite Impulse Response (IIR) filters
  int16_t* IIR_filterChain1(int16_t iir_in[2], bool clear= false);  // Infinite Impulse Response (IIR) filters
  int16_t* IIR_filterChain2(int16_t iir_in[2], bool clear= false);  // Infinite Impulse Response (IIR) filters
 
private:

  int8_t    _balance = 0;                  // -16 (mute left) ... +16 (mute right)
  uint8_t   _volume = 64;                       // volume

  float     _filterBuff[3][2][2][2];       // IIR filters memory for Audio DSP
  filter_t        _filter[3];                    // digital filters

  int8_t    _gain0 = 0;                    // cut or boost filters (EQ)
  int8_t    _gain1 = 0;
  int8_t    _gain2 = 0;
};

extern Audio Audio_Handle;
#endif